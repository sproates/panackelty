// Bounded incident recovery: accept only the previously published archive.
const fs = require('node:fs');
const {createHash} = require('node:crypto');
const {execFileSync} = require('node:child_process');
const SHA256 = '11968f21cb0339feb2b24165858b8b9fe0d5ef21b6f2784b2c21c08a4293533c';
const SOURCE = '5f92782aad468f242fc8edbc56434d618d495904';
const RUN = 37241687324;
const ARTIFACT = 11317802847;
function validateArtifact(artifact) {
  if (artifact.id !== ARTIFACT || artifact.name !== 'github-pages' || artifact.expired ||
      artifact.workflow_run?.id !== RUN || artifact.workflow_run?.head_sha !== SOURCE) {
    throw new Error('Recovery artifact identity is not the approved original deployment');
  }
}
function validateEntries(names, listing) {
  const seen = new Set();
  for (const name of names.trim().split('\n')) {
    if (!name.startsWith('./') || name.includes('\\') || name.split('/').includes('..') || seen.has(name)) {
      throw new Error('Unsafe or duplicate recovery archive path');
    }
    seen.add(name);
  }
  if (listing.trim().split('\n').some(line => !/^[d-]/.test(line))) throw new Error('Recovery archive contains links or special files');
  for (const required of ['./index.html','./publication.json','./playground/index.html','./capabilities/index.html','./releases.html']) {
    if (!seen.has(required)) throw new Error('Recovery archive is missing an entry point');
  }
}
function validateProvenance(value) {
  if (value.schema !== 1 || value.site_sha !== SOURCE || value.check_run !== 37241526858) {
    throw new Error('Recovery provenance differs from the approved original website');
  }
}
function verify(archive) {
  if (createHash('sha256').update(fs.readFileSync(archive)).digest('hex') !== SHA256) throw new Error('Recovery archive SHA-256 mismatch');
  const tar = args => execFileSync('tar', args, {encoding:'utf8',maxBuffer:16*1024*1024});
  validateEntries(tar(['-tf',archive]),tar(['-tvf',archive]));
  validateProvenance(JSON.parse(tar(['-xOf',archive,'./publication.json'])));
}
module.exports = {validateArtifact,validateEntries,validateProvenance,verify};
if (require.main === module) {
  try {verify(process.argv[2]);console.log('Verified original recovery artifact and provenance');}
  catch (error) {console.error(error.message);process.exitCode=1;}
}
