'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
test('release has no network dependency on coverage reports',()=>{
  const workflow=fs.readFileSync('.github/workflows/release.yml','utf8');
  assert.doesNotMatch(workflow,/source_coverage|panackelty-coverage|source-provenance|source-summary/);
  assert.equal(fs.existsSync('scripts/release_source_coverage.cjs'),false);
});
test('release still requires exact identity, full tests and native package validation',()=>{
  const workflow=fs.readFileSync('.github/workflows/release.yml','utf8');
  assert.match(workflow,/sh scripts\/release_request.sh/);
  assert.match(workflow,/run: make check/);
  assert.match(workflow,/package:\n    needs: validate/);
  assert.match(workflow,/needs: \[validate, package\]/);
  assert.match(workflow,/target: linux-x86_64/);
  assert.match(workflow,/target: macos-arm64/);
  assert.match(workflow,/run: make package/);
});
