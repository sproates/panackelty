#!/bin/sh
. tests/unit/harness/common.sh
unset VALIDATION_TIMINGS_FILE VALIDATION_PROFILE_FILE VALIDATION_PROFILE_PARENT VALIDATION_PROFILE_RUN
case_name=timing-warning
capture 0 5 sh tests/run_timed.sh sample -1 sh -c 'exit 0'
contains "$work/stdout" 'timing: sample '
contains "$work/stdout" '(budget -1s)'
contains "$work/stderr" 'warning: sample exceeded its -1s validation budget'
pass
case_name=timing-failure-status
capture 7 5 sh tests/run_timed.sh sample 15 sh -c 'exit 7'
contains "$work/stdout" 'timing: sample '
pass
case_name=timing-record-append
for status in 0 7; do
    capture "$status" 5 env VALIDATION_TIMINGS_FILE="$work/timings.tsv" sh tests/run_timed.sh sample 15 sh -c "exit $status"
done
awk -F '\t' 'NF!=4 || $1!="sample" || $2!~/^[0-9]+$/ || $3!="15" || $4!=(NR==1?0:7) { bad=1 }
    END { exit bad || NR!=2 }' "$work/timings.tsv" || fail 'invalid appended timing rows'
pass
case_name=timing-report-write-failure
capture 7 5 env VALIDATION_TIMINGS_FILE="$work/missing/report" sh tests/run_timed.sh sample 15 sh -c 'exit 7'
contains "$work/stderr" 'warning: could not record sample timing'
pass
case_name=supervisor-failure-contracts
# Ensure the helper cannot turn an unexpected status, signal or timeout into a pass.
for mode in status signal timeout; do
    case "$mode" in
        status) command='exit 7'; seconds=5 ;;
        signal) command='kill -TERM $$'; seconds=5 ;;
        timeout) command='sleep 5'; seconds=1 ;;
    esac
    if "$root/panack-vm" run "$HARNESS_COMMAND" 0 "$seconds" "$work/out" "$work/err" "$root" /bin/sh -c "$command" > "$work/supervisor.stdout" 2> "$work/supervisor.stderr"; then
        fail "supervisor accepted $mode"
    fi
done
contains "$work/err" 'harness host error: '
capture 0 5 sh -c 'printf "a\000b"; printf "e\000f" >&2'
printf 'a\000b' > "$work/expected.out"
printf 'e\000f' > "$work/expected.err"
equal_files "$work/stdout" "$work/expected.out"
equal_files "$work/stderr" "$work/expected.err"
pass
case_name=profile-preserves-streams-arguments-and-status
for enabled in no yes; do
    for status in 0 7; do
        if [ "$enabled" = yes ]; then
            VALIDATION_PROFILE_FILE="$work/profile with spaces.tsv"
            export VALIDATION_PROFILE_FILE
        else
            unset VALIDATION_PROFILE_FILE
        fi
        capture "$status" 5 sh tests/profile_command.sh sample sh -c \
            'printf "%s\000b" "$1"; printf "e\000f" >&2; exit "$2"' sh 'a with spaces' "$status"
        printf 'a with spaces\000b' > "$work/expected.out"
        printf 'e\000f' > "$work/expected.err"
        equal_files "$work/stdout" "$work/expected.out"
        equal_files "$work/stderr" "$work/expected.err"
    done
done
awk -F '\t' 'NF!=5 || $1!="unspecified" || $2!="sample" || $3!="-" || $4!~/^[0-9]+$/ || $5!=(NR==1?0:7) { bad=1 }
    END { exit bad || NR!=2 }' "$VALIDATION_PROFILE_FILE" || fail 'invalid profile append'
pass
case_name=profile-nesting-and-run-context
VALIDATION_PROFILE_RUN=warm
export VALIDATION_PROFILE_RUN
capture 7 5 sh tests/profile_command.sh parent sh tests/profile_command.sh child sh -c 'exit 7'
awk -F '\t' 'NR==3 && ($1!="warm" || $2!="child" || $3!="parent" || $5!=7) { bad=1 }
    NR==4 && ($1!="warm" || $2!="parent" || $3!="-" || $5!=7) { bad=1 }
    END { exit bad || NR!=4 }' "$VALIDATION_PROFILE_FILE" || fail 'lost nesting/context'
pass
case_name=profile-write-failure-preserves-command-status
for status in 0 7; do
    capture "$status" 5 env VALIDATION_PROFILE_FILE="$work/missing/profile" sh tests/profile_command.sh sample sh -c "exit $status"
    contains "$work/stderr" 'warning: could not record profile for sample'
done
pass
case_name=profile-public-cli-output
capture 0 10 sh tests/profile_command.sh cli ./panack run tests/fixtures/compiler_contracts/driver/basic.panack
printf '42\n' > "$work/expected.out"
equal_files "$work/stdout" "$work/expected.out"
test ! -s "$work/stderr" || fail 'profile polluted CLI stderr'
pass

case_name=sanitizer-partition-equivalence
sh tests/ci_sanitize.sh
pass

case_name=runtime-parallel-dispatch-and-failure-propagation
mkdir "$work/runtime"
cat > "$work/runtime/make" <<'MAKE'
#!/bin/sh
set -eu
group=${2#unit-runtime-}
if [ "$group" = native ]; then expected_workers=1; else expected_workers=${RUNTIME_PROBE_JOBS:-1}; fi
[ "${VALIDATION_JOBS:-}" = "$expected_workers" ] || exit 9
printf '%s\n' "$group" > "$RUNTIME_CONTROL/$group.started"
if [ "${RUNTIME_PARALLEL:-}" = yes ]; then
    if [ "$group" = native ]; then
        printf 'ready\n' > "$RUNTIME_CONTROL/barrier"
    else
        IFS= read -r ready < "$RUNTIME_CONTROL/barrier"
        [ "$ready" = ready ] || exit 10
    fi
fi
printf '%s stdout\n' "$group"
printf '%s stderr\n' "$group" >&2
if [ "${RUNTIME_FAIL:-}" = "$group" ]; then exit 7; fi
MAKE
chmod +x "$work/runtime/make"
mkfifo "$work/runtime/barrier"
for workers in 1 2 3; do
    for fault in none native probes; do
        rm -f "$work/runtime/native.started" "$work/runtime/probes.started"
        expected=0
        if [ "$fault" != none ]; then expected=7; fi
        parallel=no
        if [ "$workers" -gt 1 ]; then parallel=yes; fi
        probe_workers=1
        if [ "$workers" -gt 1 ]; then probe_workers=$((workers - 1)); fi
        capture "$expected" 5 env VALIDATION_JOBS="$workers" RUNTIME_CONTROL="$work/runtime" \
            RUNTIME_FAIL="$fault" RUNTIME_PARALLEL="$parallel" RUNTIME_PROBE_JOBS="$probe_workers" \
            sh tests/run_suites.sh "$work/runtime/make" unit-runtime-native unit-runtime-probes
        contains "$work/stdout" 'native stdout'
        contains "$work/stderr" 'native stderr'
        if [ "$workers" -gt 1 ] || [ "$fault" != native ]; then
            contains "$work/stdout" 'probes stdout'
            contains "$work/stderr" 'probes stderr'
            test -f "$work/runtime/probes.started" || fail 'lost runtime probes'
        fi
    done
done
for workers in 01 08; do
    parallel=no; probe_workers=1
    if [ "$workers" = 08 ]; then parallel=yes; probe_workers=7; fi
    capture 0 5 env VALIDATION_JOBS="$workers" RUNTIME_CONTROL="$work/runtime" \
        RUNTIME_FAIL=none RUNTIME_PARALLEL="$parallel" RUNTIME_PROBE_JOBS="$probe_workers" \
        sh tests/run_suites.sh "$work/runtime/make" unit-runtime-native unit-runtime-probes
done
for invalid in 0 33 invalid; do
    rm -f "$work/runtime/native.started" "$work/runtime/probes.started"
    capture 2 5 env VALIDATION_JOBS="$invalid" RUNTIME_CONTROL="$work/runtime" \
        sh tests/run_suites.sh "$work/runtime/make" unit-runtime-native unit-runtime-probes
    test ! -f "$work/runtime/native.started" && test ! -f "$work/runtime/probes.started" || fail 'invalid worker count performed work'
done
pass
