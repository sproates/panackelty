// Only an automatic coverage refresh may be redundant. Call this after finding
// trusted, fingerprint-identical website bytes and selecting validated coverage.
module.exports = async function duplicate(selected, siteSha, fetchLive = fetch) {
  if (selected.site.head_sha !== siteSha) throw new Error('Main advanced: retry publication for current main');
  const response = await fetchLive('https://panackelty.com/coverage/provenance.txt', {
    cache:'no-store', headers:{'Cache-Control':'no-cache'}, signal:AbortSignal.timeout(10000),
  });
  if (response.status === 404) return false;
  if (!response.ok) throw new Error(`Live publication lookup failed: HTTP ${response.status}`);
  const expected = `coverage_commit=${selected.run.head_sha}\narchived_at=${selected.report.created_at}\ncheck_run=${selected.run.id}\nsite_commit=${siteSha}\n`;
  return await response.text() === expected;
};
