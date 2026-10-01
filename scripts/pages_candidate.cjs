// Automatic publication consumes only Check's exact-source browser certificate.
// Core/docs-only checks have no certificate and must never bootstrap website work.
module.exports = async function candidate(github, repo, {sha, fingerprint, automatic, rebuild}, log) {
  if (rebuild) return null;
  const checked = await require('./pages_checked_website.cjs')(github, repo, sha);
  if (checked || automatic) return checked;
  return require('./pages_artifact.cjs')(github, repo, fingerprint, log);
};
