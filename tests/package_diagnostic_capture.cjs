// Verify captured remote explanations and their documentation. Recording never
// skips assertions or updates expected output.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const [name, file, root, port] = process.argv.slice(2);
let text = fs.readFileSync(file, 'utf8').split(root).join('<workspace>');
if (port) text = text.split('localhost:' + port).join('localhost:<port>');
text = text.replace(/ at commit [0-9a-f]{40,64}/g, ' at commit <locked-commit>');
text = text.replace(/Requested commit: [0-9a-f]{40,64}/g, 'Requested commit: <locked-commit>');
if (process.env.PANACK_DIAGNOSTICS_RECORD) {
  fs.mkdirSync(process.env.PANACK_DIAGNOSTICS_RECORD, {recursive: true});
  fs.writeFileSync(path.join(process.env.PANACK_DIAGNOSTICS_RECORD, name + '.txt'), text);
}

assert.equal(text, fs.readFileSync('tests/fixtures/package_diagnostics/' + name + '.stderr', 'utf8'));
assert(fs.readFileSync('docs/design/PACKAGE_DIAGNOSTIC_EXAMPLES.md', 'utf8').includes('```text\n' + text + '```'));
