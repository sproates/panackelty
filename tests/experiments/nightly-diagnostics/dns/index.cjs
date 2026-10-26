// Runtime service URLs are injected into JavaScript actions, not shell steps.
const dns = require('node:dns').promises;
const raw = process.env.ACTIONS_RESULTS_URL;
if (!raw) throw Error('Runner did not provide ACTIONS_RESULTS_URL');
const hostname = new URL(raw).hostname;
console.log('Artifact results hostname:', hostname);
(async () => {
  for (let attempt = 1; attempt <= 3; attempt++) {
    for (const method of ['lookup', 'resolve4', 'resolve6']) {
      const start = performance.now();
      try { console.log(JSON.stringify({attempt, method, addresses: await dns[method](hostname), milliseconds: performance.now() - start})); }
      catch (error) { console.log(JSON.stringify({attempt, method, code: error.code, milliseconds: performance.now() - start})); }
    }
  }
})();
