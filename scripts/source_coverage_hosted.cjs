'use strict';
// Hosted transport is trusted only after the workflow selects a successful core
// producer and downloads that exact immutable artifact with digest checking.
// Never infer provenance from files or a receipt inside a downloaded directory.
const fs = require('node:fs'), path = require('node:path');
const {regular,sha} = require('./source_coverage.cjs');
const {createPlanCache} = require('./source_coverage_cache.cjs');
const repository = {owner:'sproates',repo:'panackelty'};
const fullName = 'sproates/panackelty', workflow = '.github/workflows/source-coverage.yml';
const MiB = 1024*1024;
function artifactName(context) {
  return 'source-preparation-'+sha(Buffer.from(JSON.stringify({context,importer:sha(regular(__filename))})));
}
async function select(github,name,currentRun) {
  if (!/^source-preparation-[a-f0-9]{64}$/.test(name)) throw Error('Invalid preparation artifact identity');
  const {data} = await github.rest.actions.listArtifactsForRepo({...repository,name,per_page:20});
  for (const artifact of data.artifacts) {
    const origin = artifact.workflow_run;
    if (artifact.name !== name || artifact.expired || !origin || origin.id === Number(currentRun) ||
        origin.head_branch !== 'next' || origin.repository_id !== origin.head_repository_id ||
        !Number.isSafeInteger(artifact.id) || artifact.id <= 0 ||
        !Number.isSafeInteger(artifact.size_in_bytes) || artifact.size_in_bytes <= 0 || artifact.size_in_bytes > 2*1024*MiB ||
        !/^sha256:[a-f0-9]{64}$/.test(artifact.digest)) continue;
    const {data:run} = await github.rest.actions.getWorkflowRun({...repository,run_id:origin.id});
    if (run.id !== origin.id || run.event !== 'workflow_dispatch' || run.head_branch !== 'next' ||
        run.status !== 'completed' || run.conclusion !== 'success' || run.path !== workflow ||
        run.repository?.full_name !== fullName || run.head_repository?.full_name !== fullName ||
        run.head_sha !== origin.head_sha || !/^[a-f0-9]{40}$/.test(run.head_sha)) continue;
    return {artifact:artifact.id,run:run.id,commit:run.head_sha,digest:artifact.digest};
  }
  return null;
}
function importedFile(file,limit) {
  const s=fs.lstatSync(file);
  if (!s.isFile() || s.isSymbolicLink() || s.nlink !== 1 || s.uid !== process.getuid()) throw Error('Downloaded preparation member is not owned regular data');
  return regular(file,limit);
}
// The caller must establish transport provenance before invoking this function.
// It is deliberately separate from ordinary local cache lookup: a downloaded
// receipt cannot authenticate itself, and the previous runner's key is absent.
function importVerifiedArtifact(source,cache,workspace,log=console.log) {
  const root=fs.lstatSync(source);
  if (!root.isDirectory() || root.isSymbolicLink() || root.uid !== process.getuid()) throw Error('Invalid preparation download directory');
  const slots=fs.readdirSync(source);
  if (slots.length > 64 || slots.some(s=>! /^[a-f0-9]{64}$/.test(s))) throw Error('Invalid downloaded preparation slots');
  let total=0, imported=0;
  for (const key of slots) {
    const slot=path.join(source,key),stat=fs.lstatSync(slot);
    if (!stat.isDirectory() || stat.isSymbolicLink() || stat.uid !== process.getuid()) throw Error('Invalid downloaded preparation slot');
    const receipt=JSON.parse(importedFile(path.join(slot,'receipt.json'),MiB));
    if (typeof receipt.body !== 'string') throw Error('Invalid downloaded preparation receipt');
    const record=JSON.parse(receipt.body);
    if (JSON.stringify(record.context) !== JSON.stringify(cache.context) || record.key !== key ||
        typeof record.entry !== 'string' || !/^(src|tests)\/[A-Za-z0-9_./-]+\.panack$/.test(record.entry) || record.entry.split('/').includes('..') ||
        sha(Buffer.from(JSON.stringify({context:cache.context,entry:record.entry}))) !== key) throw Error('Downloaded preparation inputs do not match');
    if (!Array.isArray(record.files) || record.files.length < 4 || record.files.length > 259) throw Error('Invalid downloaded preparation member count');
    const names=['plan.bc','plan.inv','plan.plan',...Array.from({length:record.files.length-3},(_,i)=>`plan.source-${i}.plan`)];
    if (record.files.some((f,i)=>f.name !== names[i]) || JSON.stringify(fs.readdirSync(slot).sort()) !== JSON.stringify([...names,'receipt.json'].sort())) throw Error('Unexpected downloaded preparation member');
    // Read and hash the actual bytes copied. No paths from the archive are used
    // until their complete names and context have been checked.
    const prefix=path.join(workspace,'import-'+imported);
    cache.get(record.entry,prefix,target=>{
      for (const member of record.files) {
        const limit=member.name==='plan.plan'?128*MiB:member.name==='plan.inv'?64*MiB:16*MiB;
        const bytes=importedFile(path.join(slot,member.name),limit);
        total+=bytes.length;
        if (total > 2*1024*MiB) throw Error('Downloaded preparation exceeds 2 GiB');
        if (bytes.length !== member.size || sha(bytes) !== member.sha256) throw Error('Downloaded preparation content changed');
        fs.writeFileSync(target+member.name.slice(4),bytes,{flag:'wx',mode:0o600});
      }
    });
    imported++;
    for (const name of names) fs.unlinkSync(prefix+name.slice(4));
  }
  cache.assertCurrent();
  log(`Imported ${imported} preparations from verified hosted artifact; all executions remain fresh`);
  return imported;
}
function localCache(checkout) {
  return createPlanCache({checkout,directory:path.join(checkout,'build/source-plan-cache'),
    compiler:regular(path.join(checkout,'bootstrap/compiler-v9.bc')),vm:regular(path.join(checkout,'panack-vm'))});
}
module.exports={select,artifactName,importVerifiedArtifact};
if (require.main === module) {
  const checkout=path.resolve(__dirname,'..'),action=process.argv[2];
  if (action==='identity') {
    const name=artifactName(localCache(checkout).context);
    fs.appendFileSync(process.env.GITHUB_OUTPUT,`name=${name}\n`);
  } else if (action==='import') {
    const source=process.argv[3],directory=path.join(checkout,'build/source-plan-cache');
    const work=fs.mkdtempSync(path.join(checkout,'build/source-plan-import-'));
    try { importVerifiedArtifact(source,localCache(checkout),work); }
    catch (error) {
      // No partially imported set is reused after an invalid download.
      fs.rmSync(directory,{recursive:true,force:true});
      console.log(`Hosted preparation unavailable; generating fresh plans: ${error.message}`);
    } finally { fs.rmSync(work,{recursive:true,force:true}); }
  } else throw Error('Usage: source_coverage_hosted.cjs identity | import VERIFIED_DOWNLOAD');
}
