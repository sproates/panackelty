'use strict';

const fs = require('node:fs');
const {execFileSync} = require('node:child_process');

function validate({releaseSha, provenance, summary, isAncestor, changedPaths}) {
  if (!/^[a-f0-9]{40}$/.test(releaseSha)) throw new Error('Invalid release commit');
  if (provenance?.schema !== 2 || provenance.branch !== 'next' ||
      provenance.kind !== 'panack-source' || provenance.repository !== 'sproates/panackelty' ||
      !/^[a-f0-9]{40}$/.test(provenance.coverage_sha) ||
      !Number.isSafeInteger(provenance.check_run) || provenance.check_run < 1 ||
      !Number.isSafeInteger(provenance.artifact_id) || provenance.artifact_id < 1 ||
      !Number.isSafeInteger(provenance.run_attempt) || provenance.run_attempt < 1 ||
      !Number.isSafeInteger(provenance.run_number) || provenance.run_number < 1 ||
      !/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d(?:\.\d+)?Z$/.test(provenance.archived_at) ||
      !Number.isFinite(Date.parse(provenance.archived_at))) {
    throw new Error('Published source coverage provenance is invalid or is not the next source channel');
  }
  if (summary?.schema !== 1 || summary.repository !== provenance.repository ||
      summary.branch !== 'next' || summary.clean !== true ||
      summary.commit !== provenance.coverage_sha || !Number.isFinite(Date.parse(summary.generatedAt)) ||
      Date.parse(summary.generatedAt) > Date.parse(provenance.archived_at) + 300000) {
    throw new Error('Published source summary does not match its provenance');
  }
  if (!isAncestor(provenance.coverage_sha, releaseSha)) {
    throw new Error('Measured source coverage commit is not an ancestor of the release commit');
  }
  if (changedPaths.length) {
    throw new Error(`Coverage inputs changed after measurement: ${changedPaths.join(', ')}`);
  }
  return {coverageSha: provenance.coverage_sha, checkRun: provenance.check_run};
}

function main() {
  const [releaseSha, provenancePath, summaryPath] = process.argv.slice(2);
  if (!releaseSha || !provenancePath || !summaryPath || process.argv.length !== 5) {
    throw new Error('usage: node scripts/release_source_coverage.cjs RELEASE_SHA PROVENANCE_JSON SUMMARY_JSON');
  }
  const provenance = JSON.parse(fs.readFileSync(provenancePath, 'utf8'));
  const summary = JSON.parse(fs.readFileSync(summaryPath, 'utf8'));
  const git = args => execFileSync('git', args, {encoding: 'utf8'}).trim();
  const isAncestor = (candidate, release) => {
    try {
      execFileSync('git', ['merge-base', '--is-ancestor', candidate, release], {stdio: 'ignore'});
      return true;
    } catch {
      return false;
    }
  };
  const changedPaths = git(['diff', '--name-only', '--no-renames', provenance.coverage_sha, releaseSha,
    '--', 'src', 'bootstrap', 'Makefile', 'tests', ':(glob)scripts/source_coverage*.cjs',
    '.github/workflows/check.yml']).split('\n').filter(Boolean);
  const result = validate({releaseSha, provenance, summary, isAncestor, changedPaths});
  console.log(`Source coverage ${result.coverageSha} (Check run ${result.checkRun}) covers the release inputs.`);
}

if (require.main === module) {
  try {
    main();
  } catch (error) {
    console.error(`Release source coverage gate failed: ${error.message}`);
    process.exitCode = 1;
  }
}

module.exports = {validate};
