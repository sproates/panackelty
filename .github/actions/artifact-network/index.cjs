const dns = require('node:dns');
const fs = require('node:fs');
const path = require('node:path');
const https = require('node:https');
const {execFile} = require('node:child_process');

// Runtime URLs/tokens are injected into JavaScript actions by the runner.
// Only hostnames are retained: never print URLs, tokens or all environment values.
function hostname(value) {
  try { return new URL(value).hostname; } catch { return null; }
}
function errorInfo(error) {
  return {code: error.code || error.name, syscall: error.syscall || null};
}
function bounded(operation, milliseconds = 3500) {
  let timer;
  return Promise.race([
    operation,
    new Promise((_, reject) => { timer = setTimeout(() => reject(Object.assign(new Error(), {code: 'DIAGNOSTIC_TIMEOUT'})), milliseconds); })
  ]).finally(() => clearTimeout(timer));
}
function head(host) {
  return new Promise((resolve, reject) => {
    const request = https.request({hostname: host, method: 'HEAD', path: '/', timeout: 5000}, response => {
      const headers = {};
      for (const key of ['date', 'server', 'x-ms-request-id', 'x-github-request-id']) {
        if (response.headers[key]) headers[key] = response.headers[key];
      }
      resolve({status: response.statusCode, headers, address: response.socket.remoteAddress,
        tls: response.socket.getProtocol(), authorised: response.socket.authorized});
      response.resume();
    });
    request.on('timeout', () => request.destroy(Object.assign(new Error(), {code: 'HTTPS_TIMEOUT'})));
    request.on('error', reject);
    request.end();
  });
}
function command(program, args) {
  return new Promise(resolve => execFile(program, args, {timeout: 5000, maxBuffer: 32768}, (error, stdout, stderr) =>
    resolve({program, args, error: error ? errorInfo(error) : null, stdout, stderr})));
}
async function collect({env = process.env, log, probe = async (label, operation) => {
  const started = performance.now();
  try { const result = await bounded(operation()); log({probe: label, milliseconds: performance.now() - started, result}); }
  catch (error) { log({probe: label, milliseconds: performance.now() - started, error: errorInfo(error)}); }
}}) {
  const host = hostname(env.ACTIONS_RESULTS_URL);
  log({event: 'upload-failed', artifact: env.INPUT_ARTIFACT, attempt: env.INPUT_ATTEMPT,
    artifactServiceHostname: host, runtimeServiceHostname: hostname(env.ACTIONS_RUNTIME_URL),
    runnerOS: env.RUNNER_OS, image: env.ImageOS, imageVersion: env.ImageVersion,
    node: process.version, run: env.GITHUB_RUN_ID, runAttempt: env.GITHUB_RUN_ATTEMPT, job: env.GITHUB_JOB,
    resolvers: dns.getServers(), proxyHostnames: ['HTTPS_PROXY', 'https_proxy', 'HTTP_PROXY', 'http_proxy']
      .map(key => ({key, hostname: hostname(env[key])})).filter(item => item.hostname)});
  try { log({file: '/etc/resolv.conf', text: fs.readFileSync('/etc/resolv.conf', 'utf8').slice(0, 32768)}); }
  catch (error) { log({file: '/etc/resolv.conf', error: errorInfo(error)}); }
  const config = process.platform === 'darwin'
    ? [['scutil', ['--dns']], ['route', ['-n', 'get', 'default']]]
    : [['resolvectl', ['status']], ['ip', ['route']]];
  await Promise.all(config.map(([program, args]) => probe(program, () => command(program, args))));
  if (!host) log({event: 'missing-results-hostname', note: 'Runner did not provide a parseable ACTIONS_RESULTS_URL; do not infer the failed hostname.'});
  const hosts = [...new Set([host, 'api.github.com'].filter(Boolean))];
  for (let round = 1; round <= 3; round++) {
    await Promise.all(hosts.flatMap(name => {
      const resolver = new dns.promises.Resolver({timeout: 1000, tries: 1});
      return [
        probe(`${round} ${name} system lookup`, () => dns.promises.lookup(name, {all: true, verbatim: true})),
        probe(`${round} ${name} A records`, () => resolver.resolve4(name, {ttl: true})),
        probe(`${round} ${name} AAAA records`, () => resolver.resolve6(name, {ttl: true})),
        probe(`${round} ${name} CNAME`, () => resolver.resolveCname(name))
      ];
    }));
    if (round < 3) await new Promise(resolve => setTimeout(resolve, 1000));
  }
  if (host) await probe(`${host} unauthenticated HTTPS HEAD /`, () => head(host));
  log({event: 'diagnostics-complete', note: 'Probes run after the upload failure. Successful resolution now does not prove resolution worked then. HTTP error status can still demonstrate connectivity; this is not an authenticated upload test.'});
}
async function main() {
  const directory = path.join(process.env.GITHUB_WORKSPACE || process.cwd(), 'artifact-upload-diagnostics');
  fs.mkdirSync(directory, {recursive: true});
  const file = path.join(directory, `${Date.now()}-${process.pid}.jsonl`);
  const log = value => {
    const line = JSON.stringify({time: new Date().toISOString(), ...value});
    console.log(line);
    try { fs.appendFileSync(file, line + '\n'); } catch { /* Job log remains authoritative. */ }
  };
  // Bound even a stuck system resolver: abandoning a Promise alone leaves libuv alive.
  const deadline = setTimeout(() => { log({event: 'diagnostic-deadline', seconds: 30}); process.exit(0); }, 30000);
  try { await collect({log}); }
  catch (error) { log({event: 'diagnostic-error', error: errorInfo(error)}); }
  finally { clearTimeout(deadline); }
  process.exit(0);
}
module.exports = {hostname, errorInfo, bounded, collect};
if (require.main === module) main();
