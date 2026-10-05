# Original website recovery

Production is now published by `sproates/panackelty-website`. This retained
manual-only `Website recovery` workflow republishes exactly one previously
published archive without restoring website source or the automatic publisher.
It runs only from current main only after this repository has the restored
`panackelty.com` Pages assignment. It never changes domain settings or DNS.

The archive is GitHub artifact `11317802847`, from run `37241687324`, website
source `5f92782aad468f242fc8edbc56434d618d495904`, validated by Check run
`37241526858`. Its tar SHA-256 is
`11968f21cb0339feb2b24165858b8b9fe0d5ef21b6f2784b2c21c08a4293533c`.
The workflow checks identity, expiry, exact archive digest, safe entries and
embedded provenance, then uploads the original tar unchanged for Pages to deploy.
It shares the former publisher's `pages-production` concurrency group.

Dispatch only for this approved recovery. After deployment, verify the root URL
in a browser as well as exact HTTP bytes, nested pages and playground assets.
Successful deployment or command-line availability alone does not prove that
visitors have recovered. This is a retained rollback facility, not an automatic
publisher. Retire it through a reviewed follow-up when the rollback archive is
no longer required. If the archive expires, fail closed and review a
new recovery source; do not silently substitute current website source.

Focused offline validation:

```sh
node --test tests/website_recovery.test.cjs
node scripts/verify_website_recovery.cjs /path/to/original/artifact.tar
```
