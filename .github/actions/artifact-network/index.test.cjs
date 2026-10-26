const {test} = require('node:test');
const assert = require('node:assert/strict');
const {hostname, errorInfo, bounded, collect} = require('./index.cjs');

test('runtime and proxy credentials are never retained with hostnames', () => {
  assert.equal(hostname('https://user:secret@results.example.test/a?token=secret'), 'results.example.test');
  assert.equal(hostname('not a URL'), null);
  assert.equal(hostname(undefined), null);
  assert.deepEqual(errorInfo(Object.assign(new Error('signed URL secret'), {code: 'ENOTFOUND', syscall: 'getaddrinfo'})),
    {code: 'ENOTFOUND', syscall: 'getaddrinfo'});
});
test('a stuck probe has a deadline and failures keep their error codes', async () => {
  await assert.rejects(bounded(new Promise(() => {}), 10), {code: 'DIAGNOSTIC_TIMEOUT'});
  await assert.rejects(bounded(Promise.reject(Object.assign(new Error(), {code: 'EAI_AGAIN'}))), {code: 'EAI_AGAIN'});
  assert.equal(await bounded(Promise.resolve('ok')), 'ok');
});
test('capture the injected service host, controls and all three DNS observations without exposing credentials', async () => {
  const records = [], probes = [];
  await collect({env: {ACTIONS_RESULTS_URL: 'https://user:secret@results.example.test/private?token=secret',
    HTTPS_PROXY: 'https://user:secret@proxy.example.test', ACTIONS_RUNTIME_TOKEN: 'secret', INPUT_ATTEMPT: '2'},
    log: row => records.push(row), probe: async name => probes.push(name)});
  assert.equal(records[0].artifactServiceHostname, 'results.example.test');
  assert.equal(records[0].attempt, '2');
  assert.equal(JSON.stringify(records).includes('secret'), false);
  assert.equal(probes.filter(name => /system lookup|A records|AAAA records|CNAME/.test(name)).length, 24);
  assert.ok(probes.includes('3 results.example.test system lookup'));
  assert.ok(probes.includes('results.example.test unauthenticated HTTPS HEAD /'));
});
test('missing runtime URL is reported, never replaced with an assumed artifact host', async () => {
  const records = [], probes = [];
  await collect({env: {}, log: row => records.push(row), probe: async name => probes.push(name)});
  assert.equal(records[0].artifactServiceHostname, null);
  assert.ok(records.some(row => row.event === 'missing-results-hostname'));
  assert.equal(probes.some(name => name.includes('HTTPS HEAD')), false);
});
