// Called only after finding an exact-source trusted browser certificate.
module.exports = async function duplicate(selected, siteSha, fetchLive = fetch) {
  if (selected.site.head_sha !== siteSha) throw new Error('Main advanced: retry publication for current main');
  const response = await fetchLive('https://panackelty.com/publication.json', {
    cache:'no-store', headers:{'Cache-Control':'no-cache'}, signal:AbortSignal.timeout(10000),
  });
  if (response.status === 404) return false;
  if (!response.ok) throw new Error(`Live publication lookup failed: HTTP ${response.status}`);
  const live = await response.json();
  return live.schema === 1 && live.site_sha === siteSha && live.check_run === selected.site.id;
};
