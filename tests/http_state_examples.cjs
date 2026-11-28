'use strict';

// Exercise the shipped examples unchanged, plus explicitly fault-injected copies.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');
const http = require('node:http');
const net = require('node:net');
const {spawn, spawnSync} = require('node:child_process');
const {once} = require('node:events');
const root = path.resolve(__dirname, '..');
const command = path.resolve(process.argv[2] || path.join(root, 'bin/panack'));
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-state-examples-'));
const env = Object.fromEntries(Object.entries(process.env).filter(([key]) => !key.startsWith('PANACK')));
const delay = ms => new Promise(resolve => setTimeout(resolve, ms));

function run(args, expected = 0) {
  const result = spawnSync(command, args, {cwd: work, env, encoding: 'utf8', timeout: 60000});
  assert.equal(result.status, expected, `${args.join(' ')}: ${result.error || ''}\n${result.stdout}\n${result.stderr}`);
  return result;
}

async function freePort() {
  const listener = net.createServer();
  listener.listen(0, '127.0.0.1');
  await once(listener, 'listening');
  const port = listener.address().port;
  await new Promise(resolve => listener.close(resolve));
  return port;
}

function request(port, route = '/status') {
  return new Promise((resolve, reject) => {
    const req = http.get({host: '127.0.0.1', port, path: route, agent: false}, response => {
      let body = '';
      response.setEncoding('utf8');
      response.on('data', chunk => { body += chunk; });
      response.on('end', () => resolve({status: response.statusCode, body}));
      response.on('error', reject);
    });
    req.on('error', reject);
    req.setTimeout(5000, () => req.destroy(new Error('example response timed out')));
  });
}

function seconds(text) {
  const match = /^(\d+)(?:\/(\d+)|\.(\d+))?$/.exec(text);
  assert.ok(match, `invalid exact uptime: ${text}`);
  if (match[3]) return [BigInt(match[1] + match[3]), 10n ** BigInt(match[3].length)];
  return [BigInt(match[1]), BigInt(match[2] || '1')];
}

async function exercise(args, fault) {
  const port = await freePort();
  const child = spawn(command, [...args, String(port)], {cwd: work, env, stdio: ['ignore', 'pipe', 'pipe']});
  let output = '', errors = '';
  child.stdout.on('data', data => { output += data; });
  child.stderr.on('data', data => { errors += data; });
  const exited = once(child, 'exit');
  try {
    let ready = false;
    for (let attempt = 0; attempt < 500; attempt++) {
      assert.equal(child.exitCode, null, errors);
      try { assert.equal((await request(port, '/not-counted')).status, 404); ready = true; break; }
      catch (error) { if (error.code !== 'ECONNREFUSED') throw error; await delay(20); }
    }
    assert.ok(ready, 'example did not start');
    function check(result) {
      const match = /^Requests counted: (\d+); (.*)$/.exec(result.body);
      assert.ok(match, result.body);
      assert.equal(result.status, fault ? 503 : 200);
      if (fault) assert.equal(match[2], 'uptime unavailable: the clock could not be read.');
      else assert.match(match[2], /^uptime: .+ seconds$/);
      return Number(match[1]);
    }
    const first = await request(port);
    assert.equal(check(first), 1);
    await delay(20);
    const second = await request(port);
    assert.equal(check(second), 2);
    if (!fault) {
      const a = seconds(first.body.match(/uptime: (.+) seconds$/)[1]);
      const b = seconds(second.body.match(/uptime: (.+) seconds$/)[1]);
      assert.ok(b[0] * a[1] > a[0] * b[1], 'monotonic uptime must increase');
    }
    // Stay below the server's bounded admission limit; test four overlapping
    // requests per batch rather than confusing overload rejection with lost updates.
    const replies = [];
    for (let batch = 0; batch < 4; batch++) {
      replies.push(...await Promise.all(Array.from({length: 4}, () => request(port))));
    }
    const counts = replies.map(check).sort((a,b) => a-b);
    assert.deepEqual(counts, Array.from({length: 16}, (_, i) => i + 3));
    assert.equal((await request(port, '/not-counted')).status, 404);
    assert.equal(check(await request(port)), 19);
  } finally {
    if (child.exitCode === null) child.kill('SIGTERM');
    const result = await Promise.race([exited, delay(5000).then(() => null)]);
    if (!result) { child.kill('SIGKILL'); await exited; }
    assert.ok(result, 'server did not stop');
    assert.equal(result[0], 0, errors);
    assert.equal(output, '');
    assert.equal(errors, '');
  }
}

(async () => {
  try {
    const counter = path.join(work, 'counter.panack');
    fs.copyFileSync(path.join(root, 'examples/state/counter.panack'), counter);
    assert.equal(run(['run', counter]).stdout, '1\n2\n2\n');
    run(['compile', counter, '-o', path.join(work, 'counter.bc')]);
    assert.equal(run(['run', path.join(work, 'counter.bc')]).stdout, '1\n2\n2\n');
    console.log('Standalone State counter: source and bytecode passed');
    fs.cpSync(path.join(root, 'packages/http'), path.join(work, 'http'), {recursive: true, filter: source => path.basename(source) !== '.panack'});
    for (const kind of ['closure', 'contextual']) {
      const original = path.join(root, 'examples/network/http_state_' + kind);
      for (const fault of ['none', 'request', 'startup']) {
        const app = path.join(work, kind + '-' + fault);
        fs.mkdirSync(path.join(app, 'src'), {recursive: true});
        const manifest = JSON.parse(fs.readFileSync(path.join(original, 'panack.json')));
        manifest.dependencies.http.path = '../http';
        fs.writeFileSync(path.join(app, 'panack.json'), JSON.stringify(manifest));
        let source = fs.readFileSync(path.join(original, 'src/main.panack'), 'utf8');
        if (fault !== 'none') {
          const before = fault === 'request' ? 'reading: Result[Instant,time.ClockError] = await time.now()' : 'match await time.now()';
          const after = fault === 'request' ? 'reading: Result[Instant,time.ClockError] = await unavailable()' : 'match await unavailable()';
          assert.equal(source.split(before).length, 2, 'fault injection must target exactly one clock read');
          source = source.replace(before, after) + '\nasync unavailable(): Result[Instant,time.ClockError] { Error(time.ClockUnavailable()) }\n';
        }
        fs.writeFileSync(path.join(app, 'src/main.panack'), source);
        const project = path.join(app, 'panack.json'), artifact = path.join(app, 'server.bc');
        run(['deps', 'lock', '--project', project]);
        run(['compile', '--project', project, '-o', artifact]);
        for (const args of [['run', '--project', project], ['run', artifact]]) {
          if (fault === 'startup') {
            const result = run([...args, '0'], 1);
            assert.equal(result.stdout, '');
            assert.match(result.stderr, /Cannot start: the monotonic clock is unavailable\./);
          } else await exercise(args, fault === 'request');
        }
        console.log(`State HTTP ${kind}/${fault}: source and bytecode passed`);
      }
    }
  } finally { fs.rmSync(work, {recursive: true, force: true}); }
})().catch(error => { console.error(error); process.exitCode = 1; });
