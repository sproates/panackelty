# Validate member paths and both textual and numeric ownership before extraction.
# GNU tar prints owner/group in one column; BSD tar prints separate columns.
archive_contract() (
    archive=$1
    tar -tzf "$archive" > "$work/members" || exit 1
    awk 'BEGIN { count=0 } { count++ }
        $0 !~ /^panackelty(\/|$)/ || $0 ~ /(^|\/)\.\.(\/|$)/ || $0 ~ /^\// { bad=1 }
        END { exit bad || !count }' "$work/members" || exit 1
    tar -tvzf "$archive" > "$work/owners" || exit 1
    awk 'substr($1,1,1)!="-" && substr($1,1,1)!="d" { bad=1 }
        $2!="root/root" && !($3=="root" && $4=="root") { bad=1 }
        END { exit bad || !NR }' "$work/owners" || exit 1
    tar --numeric-owner -tvzf "$archive" > "$work/ids" || exit 1
    awk '$2!="0/0" && !($3=="0" && $4=="0") { bad=1 }
        END { exit bad || !NR }' "$work/ids" || exit 1
)
