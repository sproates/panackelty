// Validate every local HTML href/src in the assembled artifact, including LLVM
// source navigation. Also used after deployment to check those same URLs.
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
  for (const asset of ['vm.wasm', 'compiler.bc', 'stdlib.json', 'worker.mjs', 'provenance.json']) {
    const file = path.join(root, 'playground', asset);
    if (!fs.statSync(file).isFile() || fs.statSync(file).size === 0) {
      throw new Error(`Missing playground asset: ${asset}`);
    }
  }
  const targets = new Set(['index.html', 'coverage/index.html', 'coverage/summary.txt']);
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
    // Check a source page plus its stylesheet as well as the public entry pages.
    const sample = [...targets].filter(p => p.startsWith('coverage/html/')).slice(0, 3);
    for (const file of ['index.html', 'coverage/index.html', 'coverage/summary.txt', ...sample]) {
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
    const response = await fetch(new URL('coverage/provenance.txt', base), {signal: AbortSignal.timeout(15000)});
    if (!response.ok || await response.text() !== fs.readFileSync(path.join(root, 'coverage/provenance.txt'), 'utf8')) {
      throw new Error('Published provenance does not match the deployed artifact');
    }
  }
  return targets;
}
module.exports = checkPages;
if (require.main === module) checkPages(process.argv[2], process.argv[3]).catch(error => {
  console.error(error.message); process.exitCode = 1;
});
