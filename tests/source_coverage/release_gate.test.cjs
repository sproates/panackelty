'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const {validate} = require('../../scripts/release_source_coverage.cjs');

const sha = 'a'.repeat(40);
const releaseSha = 'b'.repeat(40);
const provenance = {
  schema: 2,
  branch: 'next',
  kind: 'panack-source',
  repository: 'sproates/panackelty',
  coverage_sha: sha,
  check_run: 42,
  run_number: 42,
  run_attempt: 1,
  artifact_id: 123,
  archived_at: '2026-10-13T08:00:00Z'
};
const summary = {
  schema: 1,
  repository: provenance.repository,
  branch: 'next',
  commit: sha,
  clean: true,
  generatedAt: provenance.archived_at
};
const options = {
  releaseSha,
  provenance,
  summary,
  isAncestor: () => true,
  changedPaths: []
};

test('accepts a published next report covering an unchanged release candidate', () => {
  assert.deepEqual(validate(options), {coverageSha: sha, checkRun: 42});
});

test('rejects missing or mismatched source-channel provenance', () => {
  for (const patch of [{schema: 1}, {branch: 'main'}, {kind: 'native'},
    {repository: 'someone/fork'}, {coverage_sha: 'invalid'}, {check_run: 0}]) {
    assert.throws(() => validate({...options, provenance: {...provenance, ...patch}}));
  }
});

test('rejects a summary that does not match the published provenance', () => {
  assert.throws(() => validate({...options, summary: {...summary, commit: releaseSha}}), /does not match/);
});

test('requires the measured commit to precede the release commit', () => {
  assert.throws(() => validate({...options, isAncestor: () => false}), /not an ancestor/);
});

test('requires every coverage input to remain unchanged after measurement', () => {
  assert.throws(() => validate({...options, changedPaths: ['src/compiler/main.panack']}), /inputs changed/);
});
