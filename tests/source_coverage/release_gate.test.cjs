'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const {createHash} = require('node:crypto');
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
  generatedAt: provenance.archived_at,
  manifest: {schema: 1, eligible: ['src/compiler/a.panack'], unmeasured: [{path: 'src/stdlib/b.panack', reason: 'out of scope'}]},
  files: [{path: 'src/compiler/a.panack', metrics: {lines: {percent: 0}}}]
};
summary.manifestHash = createHash('sha256').update(JSON.stringify(summary.manifest)).digest('hex');
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

test('zero reported coverage does not block a current, valid report from release', () => {
  const lowCoverage = {...summary, metrics: {lines: {covered: 0, total: 100, percent: 0}}};
  assert.deepEqual(validate({...options, summary: lowCoverage}), {coverageSha: sha, checkRun: 42});
  const incomplete = {...summary, metrics: {lines: {covered: 0, total: 100, unavailable: 100, percent: null}}};
  assert.deepEqual(validate({...options, summary: incomplete}), {coverageSha: sha, checkRun: 42});
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

test('requires the report to identify measured and unmeasured source scope', () => {
  assert.throws(() => validate({...options, summary: {...summary, manifest: {eligible: summary.manifest.eligible, unmeasured: []}, files: []}}), /source scope/);
});

test('requires the measured commit to precede the release commit', () => {
  assert.throws(() => validate({...options, isAncestor: () => false}), /not an ancestor/);
});

test('requires every coverage input to remain unchanged after measurement', () => {
  assert.throws(() => validate({...options, changedPaths: ['src/compiler/main.panack']}), /inputs changed/);
});
