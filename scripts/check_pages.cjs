// Validate website navigation and deployed bytes. The external coverage host
// is deliberately outside this deployment gate.
const fs = require('node:fs');
const path = require('node:path');

async function checkPages(root, base) {
  const files = [];
  function walk(dir) {
    for (const entry of fs.readdirSync(dir, {withFileTypes: true})) {
      const file = path.join(dir, entry.name);
      if (entry.isSymbolicLink()) throw new Error(`Symlink: ${file}`);
      if (entry.isDirectory()) walk(file);
      else files.push(file);
    }
  }
  walk(root);
  const version = fs.readFileSync(path.join(root, 'playground/asset-version.txt'), 'utf8').trim();
  if (!/^[0-9a-f]{64}$/.test(version)) throw new Error('Invalid playground asset version');
  for (const asset of ['vm.wasm', 'compiler.bc', 'stdlib.json', 'worker.mjs', 'provenance.json']) {
    const file = path.join(root, 'playground/assets', version, asset);
    if (!fs.statSync(file).isFile() || fs.statSync(file).size === 0) {
      throw new Error(`Missing playground asset: ${asset}`);
    }
  }
  const landing = new Set(['coverage/index.html', 'coverage/html/index.html']);
  for (const file of files.map(file => path.relative(root, file))) {
    if (file.startsWith('coverage/') && !landing.has(file)) {
      throw new Error(`Unexpected bundled coverage report: ${file}`);
    }
  }
  const targets = new Set(['index.html', 'releases.html', ...landing]);
  for (const file of targets) {
    if (!fs.statSync(path.join(root, file)).isFile()) throw new Error(`Missing entry point: ${file}`);
  }
  for (const file of files.filter(f => f.endsWith('.html'))) {
    const html = fs.readFileSync(file, 'utf8');
    for (const match of html.matchAll(/(?:href|src)="([^"]+)"/g)) {
      const href = match[1].split(/[?#]/)[0];
      if (!href || /^(https:|mailto:|data:)/.test(href)) continue;
      if (/^[a-z]+:|^\/\//i.test(href)) throw new Error(`Unsafe URL: ${href}`);
      const target = path.resolve(path.dirname(file), decodeURIComponent(href));
      const relative = path.relative(path.resolve(root), target);
      if (relative.startsWith('..') || path.isAbsolute(relative)) throw new Error(`Escaping link: ${href}`);
      const resolved = fs.statSync(target).isDirectory() ? path.join(target, 'index.html') : target;
      if (!fs.statSync(resolved).isFile()) throw new Error(`Missing link: ${href}`);
      targets.add(path.relative(root, resolved));
    }
  }
  if (base) {
    for (const file of targets) {
      const response = await fetch(new URL(file, base), {signal: AbortSignal.timeout(15000)});
      if (!response.ok) throw new Error(`Published URL failed (${response.status}): ${file}`);
      if (await response.text() !== fs.readFileSync(path.join(root, file), 'utf8')) {
        throw new Error(`Published content does not match: ${file}`);
      }
    }
    // Compare every playground asset, including worker imports and binary inputs.
    // A successful HTML response alone does not establish a usable playground.
    for (const absolute of files.filter(file => path.relative(root, file).startsWith('playground/'))) {
      const file = path.relative(root, absolute);
      const response = await fetch(new URL(file, base), {signal: AbortSignal.timeout(15000)});
      if (!response.ok) throw new Error(`Published playground failed (${response.status}): ${file}`);
      if (file.endsWith('.wasm') && !response.headers.get('content-type')?.startsWith('application/wasm')) {
        throw new Error('Published Wasm MIME type is incorrect');
      }
      if (!Buffer.from(await response.arrayBuffer()).equals(fs.readFileSync(absolute))) {
        throw new Error(`Published playground content does not match: ${file}`);
      }
    }
    const response = await fetch(new URL('publication.json', base), {signal: AbortSignal.timeout(15000)});
    if (!response.ok || await response.text() !== fs.readFileSync(path.join(root, 'publication.json'), 'utf8')) {
      throw new Error('Published provenance does not match the deployed artifact');
    }
  }
  return targets;
}
module.exports = checkPages;
if (require.main === module) checkPages(process.argv[2], process.argv[3]).catch(error => {
  console.error(error.message); process.exitCode = 1;
});
