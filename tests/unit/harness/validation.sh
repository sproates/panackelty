#!/bin/sh
. tests/unit/harness/common.sh
unset VALIDATION_TIMINGS_FILE
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
