// Download a reviewed browser release. No browser sources or build tools are
// executed by the website publisher; the archive digest is the trust boundary.
const fs = require('node:fs');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {execFileSync} = require('node:child_process');

const LIMIT = 16 * 1024 * 1024;
function validatePin(pin) {
  if (pin.repository !== 'sproates/panackelty-browser' ||
      !/^v\d+\.\d+\.\d+$/.test(pin.tag) ||
      !/^[0-9a-f]{64}$/.test(pin.sha256) ||
      !/^[0-9a-f]{64}$/.test(pin.assetVersion)) {
    throw new Error('Invalid browser release pin');
  }
  return `https://github.com/${pin.repository}/releases/download/${pin.tag}/playground.tar.gz`;
}

async function download(pin, fetcher = fetch) {
  const response = await fetcher(validatePin(pin), {signal: AbortSignal.timeout(60000)});
  if (!response.ok) throw new Error(`Browser release download failed (${response.status})`);
  const chunks = [];
  let size = 0;
  for await (const chunk of response.body) {
    size += chunk.length;
    if (size > LIMIT) throw new Error('Browser archive exceeds 16 MiB');
    chunks.push(chunk);
  }
  const data = Buffer.concat(chunks);
  if (createHash('sha256').update(data).digest('hex') !== pin.sha256) {
    throw new Error('Browser archive SHA-256 mismatch');
  }
  return data;
}

function unpack(data, pin, destination) {
  validatePin(pin);
  if (fs.existsSync(destination)) throw new Error('Browser destination already exists');
  const parent = path.dirname(path.resolve(destination));
  fs.mkdirSync(parent, {recursive: true});
  const temporary = fs.mkdtempSync(path.join(parent, '.playground-'));
  try {
    const archive = path.join(temporary, 'archive.tar.gz');
    fs.writeFileSync(archive, data);
    if (createHash('sha256').update(data).digest('hex') !== pin.sha256) {
      throw new Error('Browser archive SHA-256 mismatch');
    }
    const list = args => execFileSync('tar', args, {encoding: 'utf8', maxBuffer: LIMIT}).trim().split('\n');
    for (const name of list(['-tzf', archive])) {
      if (name.startsWith('/') || name.includes('\\') || name.split('/').includes('..')) {
        throw new Error('Unsafe browser archive path');
      }
    }
    // Reject links, devices and special entries before extraction.
    if (list(['-tvzf', archive]).some(line => !/^[d-]/.test(line))) {
      throw new Error('Browser archive contains a link or special file');
    }
    const output = path.join(temporary, 'output');
    fs.mkdirSync(output);
    execFileSync('tar', ['--no-same-owner', '--no-same-permissions', '-xzf', archive, '-C', output]);
    const version = fs.readFileSync(path.join(output, 'asset-version.txt'), 'utf8').trim();
    if (version !== pin.assetVersion) throw new Error('Browser asset version mismatch');
    for (const file of ['index.html', `assets/${version}/vm.wasm`, `assets/${version}/compiler.bc`,
      `assets/${version}/stdlib.json`, `assets/${version}/provenance.json`]) {
      if (!fs.statSync(path.join(output, file)).isFile() || !fs.statSync(path.join(output, file)).size) {
        throw new Error(`Missing browser asset: ${file}`);
      }
    }
    fs.writeFileSync(path.join(output, 'release.json'), JSON.stringify(pin, null, 2) + '\n');
    fs.renameSync(output, destination);
  } finally {
    fs.rmSync(temporary, {recursive: true, force: true});
  }
}

module.exports = {validatePin, download, unpack};
if (require.main === module) {
  (async () => {
    const pin = JSON.parse(fs.readFileSync(process.argv[2], 'utf8'));
    unpack(await download(pin), pin, process.argv[3]);
    console.log(`Verified browser release ${pin.tag}: ${pin.sha256}`);
  })().catch(error => {console.error(error.message); process.exitCode = 1;});
}
