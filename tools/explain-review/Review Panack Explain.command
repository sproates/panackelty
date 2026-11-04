#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
exec ./bin/panack explain --interactive tools/explain-review/samples/booking.panack --function book_pair
