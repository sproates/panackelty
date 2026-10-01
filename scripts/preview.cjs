// Portable static review artifact and loopback server. No deployment credentials.
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const os = require('node:os');
const {once} = require('node:events');
const {execFileSync} = require('node:child_process');
const {download, unpack} = require('./fetch_playground.cjs');

async function build(root, destination, metadata, archive) {
  if (!/^[0-9a-f]{40}$/.test(metadata.commit) ||
      typeof metadata.dirty !== 'boolean' ||
      !/^[\w.-]+\/[\w.-]+$/.test(metadata.repository)) throw new Error('Invalid preview provenance');
  destination = path.resolve(destination);
  for (const key of ['headCommit', 'baseCommit']) {
    if (metadata[key] !== undefined && !/^[0-9a-f]{40}$/.test(metadata[key])) throw new Error('Invalid preview provenance');
  }
  if (fs.existsSync(destination)) throw new Error('Preview destination already exists');
  fs.mkdirSync(path.dirname(destination), {recursive: true});
  const temporary = fs.mkdtempSync(path.join(path.dirname(destination), '.preview-'));
  try {
    const pin = JSON.parse(fs.readFileSync(path.join(root, 'site/playground.json'), 'utf8'));
    const playground = path.join(temporary, 'playground');
    unpack(archive || await download(pin), pin, playground);
    const output = path.join(temporary, 'site');
    execFileSync('sh', [path.join(__dirname, 'assemble_site.sh'), path.join(root, 'site'), playground, output]);
    const provenance = {...metadata, browser: pin};
    fs.writeFileSync(path.join(output, 'preview.json'), JSON.stringify(provenance, null, 2) + '\n');
    const notice = `<aside style="padding:1rem;background:#fff3cd;color:#222;overflow-wrap:anywhere;min-width:0;box-sizing:border-box">Review preview: ${metadata.headCommit || metadata.commit}${metadata.headCommit ? ' (merged with PR base)' : ''}${metadata.dirty ? ' (local changes)' : ''}. <a href="/preview.json">Build identity</a></aside>`;
    for (const file of ['index.html', 'playground/index.html']) {
      const target = path.join(output, file);
      const html = fs.readFileSync(target, 'utf8');
      if (!/<body\b[^>]*>/i.test(html)) throw new Error('Preview page has no body');
      fs.writeFileSync(target, html.replace(/<body\b[^>]*>/i, match => match + notice));
    }
    // Never label main's coverage as coverage of an unvalidated PR revision.
    fs.mkdirSync(path.join(output, 'coverage'));
    fs.writeFileSync(path.join(output, 'coverage/index.html'), '<!doctype html><title>Preview coverage</title><h1>Coverage is not generated for this preview</h1><p><a href="https://panackelty.com/coverage/">Published main coverage (different revision)</a></p>');
    fs.writeFileSync(path.join(output, 'robots.txt'), 'User-agent: *\nDisallow: /\n');
    fs.renameSync(output, destination);
  } finally { fs.rmSync(temporary, {recursive: true, force: true}); }
}

function serve(directory, port = 4173) {
  const root = fs.realpathSync(directory);
  const types = {'.html':'text/html; charset=utf-8', '.css':'text/css', '.js':'text/javascript', '.mjs':'text/javascript', '.json':'application/json', '.wasm':'application/wasm', '.svg':'image/svg+xml'};
  const server = http.createServer((request, response) => {
    response.setHeader('Cache-Control', 'no-store');
    response.setHeader('X-Robots-Tag', 'noindex');
    if (!['GET', 'HEAD'].includes(request.method)) { response.writeHead(405).end(); return; }
    try {
      const pathname = decodeURIComponent(request.url.split('?')[0]);
      let file = path.resolve(root, '.' + pathname);
      if (fs.statSync(file).isDirectory()) file = path.join(file, 'index.html');
      file = fs.realpathSync(file);
      const relative = path.relative(root, file);
      if (relative.startsWith('..') || path.isAbsolute(relative) || !fs.statSync(file).isFile()) throw new Error('Invalid path');
      response.setHeader('Content-Type', types[path.extname(file)] || 'application/octet-stream');
      response.end(request.method === 'HEAD' ? undefined : fs.readFileSync(file));
    } catch { response.writeHead(404).end('Not found'); }
  });
  server.listen(port, '127.0.0.1');
  return server;
}

async function start(root, metadata, port = 4173, archive) {
  // Own only this freshly allocated directory: never erase a user's saved build.
  const temporary = fs.mkdtempSync(path.join(os.tmpdir(), 'panackelty-preview-'));
  const directory = path.join(temporary, 'site');
  let server;
  let stopped = false;
  const cleanup = () => fs.rmSync(temporary, {recursive:true, force:true});
  const stop = () => {
    if (stopped) return;
    stopped = true;
    if (server) {
      server.close();
      server.closeAllConnections();
    }
    cleanup();
    process.removeListener('exit', cleanup);
    process.removeListener('SIGINT', interrupt);
    process.removeListener('SIGTERM', interrupt);
  };
  const interrupt = () => { stop(); process.exit(0); };
  process.once('exit', cleanup);
  process.once('SIGINT', interrupt);
  process.once('SIGTERM', interrupt);
  try {
    await build(root, directory, metadata, archive);
    server = serve(directory, port);
    server.once('close', stop);
    await once(server, 'listening');
    return {server, directory, stop};
  } catch (error) {
    stop();
    throw error;
  }
}

module.exports = {build, serve, start};
if (require.main === module) (async () => {
  const args = process.argv.slice(2);
  const [command = 'start', output = 'build/preview', port = '4173'] = args;
  const validPort = value => /^\d+$/.test(value) && +value > 0 && +value < 65536;
  const metadata = () => {
    const git = args => execFileSync('git', args, {encoding: 'utf8'}).trim();
    return {commit: git(['rev-parse','HEAD']),
      dirty: git(['status','--porcelain','--untracked-files=normal']) !== '',
      repository: process.env.GITHUB_REPOSITORY || 'sproates/panackelty',
      ...(process.env.PREVIEW_HEAD_SHA ? {headCommit:process.env.PREVIEW_HEAD_SHA,baseCommit:process.env.PREVIEW_BASE_SHA} : {})};
  };
  if (command === 'start' && args.length <= 2 && validPort(args[1] || '4173')) {
    const selectedPort = +(args[1] || '4173');
    console.log('Building a fresh local preview...');
    await start(process.cwd(), metadata(), selectedPort);
    console.log(`Preview: http://127.0.0.1:${selectedPort}/\nStop with Ctrl-C. After editing, stop and rerun to rebuild, then refresh your browser.`);
  } else if (command === 'build' && args.length <= 2) {
    await build(process.cwd(), output, metadata());
    console.log(`Preview built at ${output}`);
  } else if (command === 'serve' && args.length <= 3 && validPort(port)) {
    serve(output, +port).on('listening', () => console.log(`Preview: http://127.0.0.1:${port}/`))
      .on('error', error => { console.error(error.message); process.exitCode = 1; });
  } else throw new Error('Usage: node scripts/preview.cjs [start [port] | build [output] | serve [output] [port]]');
})().catch(error => {
  console.error(error.code === 'EADDRINUSE' ? 'Preview port is already in use. Stop the other server or choose another port: node scripts/preview.cjs start 4180' : error.message);
  process.exitCode = 1;
});
