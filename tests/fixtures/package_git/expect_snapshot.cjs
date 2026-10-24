const fs = require('node:fs'), crypto = require('node:crypto'), assert = require('node:assert/strict');
const [root, url, commit] = process.argv.slice(2);
const length = n => { const b = Buffer.alloc(8); b.writeBigUInt64BE(BigInt(n)); return b; };
const source = crypto.createHash('sha256').update('PANACK-GIT-SOURCE\0v1\0');
for (const field of [url, 'sha1', commit]) {
  const b = Buffer.from(field);
  source.update(length(b.length)).update(b);
}
const tree = crypto.createHash('sha256').update('PANACK-PACKAGE-TREE\0v1\0');
for (const [name, type] of [['.gitattributes', 'F'], ['data', 'F'], ['nested', 'D'], ['nested/link', 'L'], ['nested/value', 'F']]) {
  const n = Buffer.from(name), b = type === 'D' ? Buffer.alloc(0) : type === 'L' ? Buffer.from('../data') : fs.readFileSync(root + '/source/' + name);
  tree.update(type).update(length(n.length)).update(n).update(length(b.length)).update(b);
}
const lines = fs.readFileSync(root + '/out', 'utf8').trim().split('\n');
assert.deepEqual(lines, [source.digest('hex'), tree.digest('hex'), 'true', 'true']);
console.log('Independent source key, tree digest, raw file bytes and link target match.');
