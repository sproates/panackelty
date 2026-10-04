#!/bin/sh
# Bounded, isolated observations; no cache policy changes or automatic optimisation.
set -eu
# POSIX time and awk must share a stable decimal separator.
LC_ALL=C
export LC_ALL
fail() { printf 'build baseline: %s\n' "$*" >&2; exit 1; }
usage() { fail 'usage: build_baseline.sh OUTPUT_DIRECTORY HOST_LABEL [REPEATS (1..5)] [all|probes]'; }
[ "$#" -ge 2 ] && [ "$#" -le 4 ] || usage
out=$1
host=$2
repeats=${3:-3}
mode=${4:-all}
case "$repeats" in 1|2|3|4|5) ;; *) usage ;; esac
case "$mode" in all|probes) ;; *) usage ;; esac
case "$host" in ''|*[!a-zA-Z0-9._-]*) fail 'use a neutral host label containing letters, digits, dot, dash or underscore' ;; esac
root=$(git rev-parse --show-toplevel)
root=$(CDPATH= cd "$root" && pwd -P)
case "$out" in /*) ;; *) fail 'output directory must be absolute' ;; esac
parent=$(CDPATH= cd "$(dirname "$out")" && pwd -P)
out=$parent/$(basename "$out")
case "$out/" in "$root/"*) fail 'output must be outside the source checkout' ;; esac
[ ! -e "$out" ] || fail 'output directory must not exist'
# Use explicit settings, not inherited alternate VM/seed/cache/flags or make jobs.
cc=${CC:-cc}
command -v "$cc" >/dev/null 2>&1 || fail 'CC must name one compiler executable (no shell arguments)'
mkdir "$out"
work=$(mktemp -d "$out/work.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
if command -v sha256sum >/dev/null 2>&1; then
    hash() { sha256sum "$@"; }
else
    hash() { shasum -a 256 "$@"; }
fi
unset CHECK_BUDGET_SECONDS INCREMENTAL_BUDGET_SECONDS FUNCTIONAL_BUDGET_SECONDS BOOTSTRAP_BUDGET_SECONDS
unset MAKEFLAGS MFLAGS GNUMAKEFLAGS MAKEFILES VALIDATION_PROFILE_PARENT PANACK_PROBE_VM PANACK_PROBE_SEED PANACK_PROBE_CACHE PANACKELTY_STDLIB_PATH PANACK_TEST_RUNNER_REPORT PANACK_CHECK_RUNNER_REPORT PANACK_TEST_COMPILER PANACK_TEST_CAPTURE_RUNNER_REPORT PANACK_NATIVE_BINARY PANACK_NATIVE_MODULE_TEST PANACK_TCP_SERVER PANACK_TCP_CLIENT BUILD_DIR SEED_COMPILER SEED_DIGEST HOST_ARCH
export CC="$cc" CFLAGS=-O2 CPPFLAGS= LDFLAGS= LDLIBS= VALIDATION_JOBS=2
export VALIDATION_PROFILE_FILE="$out/profile.tsv" VALIDATION_TIMINGS_FILE="$out/timings.tsv"
: > "$VALIDATION_PROFILE_FILE"
: > "$VALIDATION_TIMINGS_FILE"
# Clone local history only to retain Git-based policy checks, then overlay the
# complete candidate, including untracked additions and tracked deletions.
git clone --quiet --no-hardlinks "$root" "$work/tree" 2> "$out/clone.log"
git -C "$root" diff --binary HEAD > "$out/candidate.patch"
if [ -s "$out/candidate.patch" ]; then git -C "$work/tree" apply "$out/candidate.patch"; fi
git -C "$root" ls-files --others --exclude-standard > "$work/untracked"
# Paths containing newlines/tabs are outside this measurement tool's TSV contract.
git -C "$root" -c core.quotePath=false ls-files --cached --others --exclude-standard > "$work/source-paths"
if grep '[[:cntrl:]]' "$work/source-paths" >/dev/null || grep '^"' "$work/source-paths" >/dev/null; then fail 'unsupported source path'; fi
if [ -s "$work/untracked" ]; then
    (cd "$root" && tar -cf "$work/untracked.tar" -T "$work/untracked")
    (cd "$work/tree" && tar -xf "$work/untracked.tar")
fi
cd "$work/tree"
# Preserve a reproducible input archive and a relative-path content manifest.
git -c core.quotePath=false ls-files --cached --others --exclude-standard | while IFS= read -r file; do
    [ ! -f "$file" ] || printf '%s\n' "$file"
done > "$work/archive-paths"
tar -cf "$out/source.tar" -T "$work/archive-paths"
while IFS= read -r file; do hash "$file"; done < "$work/archive-paths" > "$out/source.sha256"
{
    printf 'source_commit=%s\nsource_tree=%s\n' "$(git rev-parse HEAD)" "$(git rev-parse HEAD^{tree})"
    printf 'candidate_archive_sha256=%s\ncandidate_manifest_sha256=%s\n' "$(hash "$out/source.tar" | awk '{print $1}')" "$(hash "$out/source.sha256" | awk '{print $1}')"
    printf 'host_label=%s\nos=%s\narchitecture=%s\n' "$host" "$(uname -s)" "$(uname -m)"
    printf 'kernel_release=%s\n' "$(uname -r)"
    printf 'cc=%s\ncflags=-O2\nvalidation_jobs=2\nmake_jobs=1\nrepeats=%s\nmode=%s\n' "$cc" "$repeats" "$mode"
    printf 'os_page_cache=uncontrolled; clean means project artifacts removed\n'
    printf 'concurrency=sequential scenarios; internal validation uses 2 workers\n'
    printf 'seed_sha256=%s\n' "$(hash bootstrap/compiler-v9.bc | awk '{print $1}')"
    "$cc" --version | sed -n '1p'
    make --version | sed -n '1p'
    if [ "$(uname -s)" = Darwin ]; then
        printf 'hardware=%s\n' "$(sysctl -n hw.model 2>/dev/null || printf unavailable)"
        printf 'cpu=%s\n' "$(sysctl -n machdep.cpu.brand_string 2>/dev/null || printf unavailable)"
        printf 'os_version=%s\n' "$(sw_vers -productVersion)"
    elif [ -f /etc/os-release ]; then sed -n '/^PRETTY_NAME=/p' /etc/os-release; fi
} > "$out/environment.txt"
printf 'scenario\trepeat\tseconds\tstatus\tprobe_compiles\tprobe_runs\tnative_build_commands\tprobe_compile_seconds\tprobe_run_seconds\n' > "$out/samples.tsv"
observe() {
    observed_scenario=$1; observed_repeat=$2; shift 2
    run=$observed_scenario-$observed_repeat
    export VALIDATION_PROFILE_RUN="$run"
    timer_status=0
    # Some time implementations normalize a failed child's exit status. Keep
    # the workload result independently, while retaining timer failures too.
    { command time -p sh -c '
        errors=$1; result=$2; shift 2
        status=0
        "$@" 2>"$errors" || status=$?
        printf "%s\n" "$status" > "$result" || exit 125
        exit "$status"
    ' sh "$out/$run.stderr" "$out/$run.status" "$@" > "$out/$run.log"; } \
        2> "$out/$run.time" || timer_status=$?
    printf '%s\n' "$timer_status" > "$out/$run.timer-status"
    status=125
    if [ -f "$out/$run.status" ]; then
        status=$(cat "$out/$run.status")
        case "$status" in ''|*[!0-9]*) fail "invalid workload status for $run" ;; esac
        [ "$status" -le 255 ] || fail "invalid workload status for $run"
        if [ "$status" = 0 ] && [ "$timer_status" != 0 ]; then status=$timer_status; fi
    fi
    elapsed=$(awk '$1 == "real" { print $2 }' "$out/$run.time")
    [ -n "$elapsed" ] || fail "missing wall time for $run"
    # Counts are completed instrumented invocations, not compiler-internal work.
    awk -F '\t' -v run="$run" -v scenario="$observed_scenario" -v repeat="$observed_repeat" -v elapsed="$elapsed" -v status="$status" '
      $1 == run && $2 ~ /^probe-build\// { builds++; build_seconds += $4 }
      $1 == run && $2 ~ /^probe-run\// { runs++; run_seconds += $4 }
      $1 == run && $2 ~ /^native-build\// { native++ }
      END { printf "%s\t%d\t%g\t%d\t%d\t%d\t%d\t%d\t%d\n", scenario,repeat,elapsed,status,builds,runs,native,build_seconds,run_seconds }
    ' "$out/profile.tsv" >> "$out/samples.tsv"
    printf '%s: %ss, status %s\n' "$run" "$elapsed" "$status"
    [ "$status" = 0 ] || fail "$run failed; external logs retained"
}
if [ "$mode" = all ]; then
    i=1
    while [ "$i" -le "$repeats" ]; do
        make clean > "$out/clean-$i.log" 2>&1
        observe clean-full "$i" make -j1 check
        i=$((i + 1))
    done
    i=1
    while [ "$i" -le "$repeats" ]; do
        observe warm-focused "$i" make -j1 check-compiler
        i=$((i + 1))
    done
else
    make -j1 native > "$out/native-preparation.log" 2>&1
fi
{
    printf 'vm_sha256=%s\n' "$(hash panack-vm | awk '{print $1}')"
    if [ -f build/bootstrap/stage2/compiler.bc ]; then printf 'stage2_compiler_sha256=%s\n' "$(hash build/bootstrap/stage2/compiler.bc | awk '{print $1}')"; fi
} >> "$out/environment.txt"
fixture=tests/build-baseline-fixture
mkdir "$fixture"
cat > "$fixture/main.panack" <<'PANACK'
import "direct.panack"
main(): Void { print(answer()); }
PANACK
cat > "$fixture/direct.panack" <<'PANACK'
import "transitive.panack"
pure answer(): Nat { base() + 2 }
PANACK
cat > "$fixture/transitive.panack" <<'PANACK'
pure base(): Nat { 40 }
PANACK
printf '// Unrelated baseline input.\n' > examples/build-baseline-unrelated.panack
cp "$fixture/direct.panack" "$work/direct"
cp "$fixture/transitive.panack" "$work/transitive"
cp examples/build-baseline-unrelated.panack "$work/unrelated"
export PANACK_PROBE_CACHE="$work/probe-cache" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib"
export VALIDATION_PROFILE_RUN=probe-preparation
sh tests/run_probe.sh "$fixture/main.panack" > "$out/probe-preparation.log"
printf '42\n' > "$work/expected"
cmp "$work/expected" "$out/probe-preparation.log"
cp -R "$PANACK_PROBE_CACHE" "$work/primed-cache"
for scenario in unchanged unrelated direct transitive; do
    i=1
    while [ "$i" -le "$repeats" ]; do
        cp "$work/direct" "$fixture/direct.panack"
        cp "$work/transitive" "$fixture/transitive.panack"
        cp "$work/unrelated" examples/build-baseline-unrelated.panack
        # Every edit starts with exactly the same primed baseline artifacts.
        rm -rf "$PANACK_PROBE_CACHE"
        cp -R "$work/primed-cache" "$PANACK_PROBE_CACHE"
        expected=42
        case "$scenario" in
            unrelated) printf '// unrelated edit\n' >> examples/build-baseline-unrelated.panack ;;
            direct) sed 's/+ 2/+ 3/' "$work/direct" > "$fixture/direct.panack"; expected=43 ;;
            transitive) sed 's/{ 40 }/{ 41 }/' "$work/transitive" > "$fixture/transitive.panack"; expected=43 ;;
        esac
        hash "$fixture/main.panack" "$fixture/direct.panack" "$fixture/transitive.panack" examples/build-baseline-unrelated.panack > "$out/probe-$scenario-$i.inputs"
        observe "probe-$scenario" "$i" sh tests/run_probe.sh "$fixture/main.panack"
        printf '%s\n' "$expected" > "$work/expected"
        cmp "$work/expected" "$out/probe-$scenario-$i.log" || fail "probe-$scenario-$i output mismatch"
        i=$((i + 1))
    done
done
# Sort samples before calculating min/median/max; do not add nested observations.
LC_ALL=C sort -t "$(printf '\t')" -k1,1 -k3,3n "$out/samples.tsv" | awk -F '\t' '
function emit() {
  if (!n) return
  median = n % 2 ? times[(n+1)/2] : (times[n/2]+times[n/2+1])/2
  printf "%s\t%d\t%g\t%g\t%g\n", group,n,times[1],median,times[n]
}
BEGIN { print "scenario\tsamples\tmin_seconds\tmedian_seconds\tmax_seconds" }
$1 == "scenario" { next }
$1 != group { emit(); group=$1; n=0 }
{ times[++n]=$3 }
END { emit() }
' > "$out/summary.tsv"
printf 'complete\n' > "$out/COMPLETE"
cat "$out/summary.tsv"
