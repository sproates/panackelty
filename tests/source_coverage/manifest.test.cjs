'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const {execFileSync} = require('node:child_process');
const {validateManifest} = require('../../scripts/source_coverage_manifest.cjs');
const {writeReport} = require('../../scripts/source_coverage.cjs');

test('the reviewed manifest accounts for every tracked production source', () => {
  const manifest = JSON.parse(fs.readFileSync('tests/source_coverage/manifest.json', 'utf8'));
  const tracked = execFileSync('git', ['ls-files', 'src'], {encoding: 'utf8'}).trim().split('\n').filter(p => p.endsWith('.panack')).sort();
  assert.deepEqual(validateManifest(manifest, tracked).unmeasured.map(entry => entry.path), [
    'src/compiler/owned_intrinsics.panack',
    'src/packages/main.panack',
    'src/packages/project_graph.panack',
    'src/project_model/package_manifest.panack',
    'src/project_model/package_project.panack',
    'src/project_model/resolved_project.panack',
    'src/project_model/utf8.panack',
    'src/stdlib/async_filesystem.panack',
    'src/stdlib/async_host.panack',
    'src/stdlib/http_runtime.panack',
    'src/stdlib/tcp_service.panack',
    'src/stdlib/package_filesystem.panack',
    'src/packages/acquisition.panack',
    'src/packages/archive.panack',
    'src/packages/case_fold.panack',
    'src/packages/local_lock.panack',
    'src/packages/lock.panack',
    'src/packages/selective_update.panack',
    'src/packages/sha256.panack',
    'src/packages/tar_header.panack',
    'src/packages/tar_stream.panack',
    'src/packages/tree_digest.panack',
    'src/stdlib/bounded_filesystem.panack',
    'src/stdlib/digest.panack',
    'src/stdlib/filesystem_stage.panack',
    'src/stdlib/gzip.panack'
  ]);
});

test('HTML and text reports visibly identify unmeasured production sources', t => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'source-report-'));
  const output = path.join(root, 'report');
  t.after(() => fs.rmSync(root, {recursive: true, force: true}));
  const report = {
    commit: 'a'.repeat(40), generatedAt: '2026-10-14T00:00:00Z',
    manifest: {unmeasured: [{path: 'src/project_model/package_manifest.panack', reason: 'not in corpus'}]},
    components: {}, files: []
  };
  writeReport(output, report);
  assert.match(fs.readFileSync(path.join(output, 'html/index.html'), 'utf8'), /src\/project_model\/package_manifest\.panack/);
  assert.match(fs.readFileSync(path.join(output, 'summary.txt'), 'utf8'), /excluded from denominators/);
  assert.match(fs.readFileSync(path.join(output, 'summary.txt'), 'utf8'), /not in corpus/);
});

test('manifest rejects missing, duplicate, overlapping, and unexplained paths', () => {
  const manifest = {schema: 1, eligible: ['src/compiler/a.panack'], unmeasured: [{path: 'src/compiler/b.panack', reason: 'out of scope'}]};
  assert.doesNotThrow(() => validateManifest(manifest, ['src/compiler/a.panack', 'src/compiler/b.panack']));
  for (const invalid of [
    {...manifest, unmeasured: []},
    {...manifest, eligible: [...manifest.eligible, 'src/compiler/a.panack']},
    {...manifest, unmeasured: [{path: manifest.eligible[0], reason: 'overlap'}]},
    {...manifest, unmeasured: [{path: 'src/compiler/b.panack', reason: ' '}]}
  ]) assert.throws(() => validateManifest(invalid, ['src/compiler/a.panack', 'src/compiler/b.panack']));
});
