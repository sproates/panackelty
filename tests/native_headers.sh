#!/bin/sh
# Every native header must compile independently and tolerate repeated inclusion.
set -eu
for header in src/vm/*.h; do
  name=${header##*/}
  # CC may include compiler arguments, as it can in Make recipes.
  printf '#include "%s"\n#include "%s"\nint main(void) { return 0; }\n' "$name" "$name" |
    ${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic -fsyntax-only -x c -I src/vm -
done
# The HTTP transport's unavailable-host branch must compile without curl headers.
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic -D__wasi__ \
  -fsyntax-only -I src/vm src/vm/http_transport.c
printf 'native header contracts: ok\n'
