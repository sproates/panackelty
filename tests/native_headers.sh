#!/bin/sh
# Every native header must compile independently and tolerate repeated inclusion.
set -eu
for header in src/vm/*.h; do
  name=${header##*/}
  # CC may include compiler arguments, as it can in Make recipes.
  printf '#include "%s"\n#include "%s"\nint main(void) { return 0; }\n' "$name" "$name" |
    ${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic -fsyntax-only -x c -I src/vm -
done
printf 'native header contracts: ok\n'
