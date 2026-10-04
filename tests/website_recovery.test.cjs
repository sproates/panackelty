const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const os=require('node:os');
const path=require('node:path');
const {validateArtifact,validateEntries,validateProvenance,verify}=require('../scripts/verify_website_recovery.cjs');
const artifact={id:11317802847,name:'github-pages',expired:false,workflow_run:{id:37241687324,head_sha:'5f92782aad468f242fc8edbc56434d618d495904'}};
test('only the original nonexpired artifact identity is accepted',()=>{
  validateArtifact(artifact);
  for (const changed of [{id:1},{name:'other'},{expired:true},{workflow_run:{id:1,head_sha:artifact.workflow_run.head_sha}},{workflow_run:{id:37241687324,head_sha:'a'.repeat(40)}}]) {
    assert.throws(()=>validateArtifact({...artifact,...changed}),/identity/);
  }
});
test('only regular archive paths and required entry points are accepted',()=>{
  const names='./\n./index.html\n./publication.json\n./playground/index.html\n./capabilities/index.html\n./releases.html\n';
  validateEntries(names,'drwxr-xr-x root/root 0 date ./\n-rw-r--r-- root/root 10 date ./index.html');
  for (const name of ['/escape','./../escape','./bad\\name','./index.html']) assert.throws(()=>validateEntries(names+name,'-regular'),/path/);
  assert.throws(()=>validateEntries(names,'lrwxr-xr-x link'),/links/);
  assert.throws(()=>validateEntries('./index.html','-regular'),/entry point/);
});
test('provenance and actual archive bytes are pinned',t=>{
  const provenance={schema:1,site_sha:artifact.workflow_run.head_sha,check_run:37241526858};
  validateProvenance(provenance);
  for (const changed of [{schema:2},{site_sha:'a'.repeat(40)},{check_run:1}]) assert.throws(()=>validateProvenance({...provenance,...changed}),/provenance/);
  const dir=fs.mkdtempSync(path.join(os.tmpdir(),'recovery-test-'));
  t.after(()=>fs.rmSync(dir,{recursive:true,force:true}));
  const file=path.join(dir,'artifact.tar');fs.writeFileSync(file,'different archive');
  assert.throws(()=>verify(file),/SHA-256/);
});
