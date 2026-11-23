#!/bin/sh
set -eu
for selected in pattern ok missing chunked close truncated duplicate oversize bad-url ipv6 timeout connect-failure tls-ok tls-wrong-host tls-untrusted tls-late dns-failure dns-late; do
  echo "Diagnostic HTTP client fixture: $selected"
  PANACK_HTTP_DIAGNOSTIC_CLIENT=1 sh tests/http_client_package.sh "$selected"
done
