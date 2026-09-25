# Normalize fixed-point decimal spelling using strings only: do not round large
# integers through awk's floating-point number representation.
{
    if ($0 !~ /^-?[0-9]+([.][0-9]+)?$/) {
        print "invalid decimal output" > "/dev/stderr"
        exit 1
    }
    value = $0
    sign = ""
    if (substr(value, 1, 1) == "-") {
        sign = "-"
        value = substr(value, 2)
    }
    point = index(value, ".")
    whole = point ? substr(value, 1, point - 1) : value
    fraction = point ? substr(value, point + 1) : ""
    sub(/^0+/, "", whole)
    sub(/0+$/, "", fraction)
    if (whole == "") whole = "0"
    if (whole == "0" && fraction == "") sign = ""
    printf "%s%s%s%s\n", sign, whole, (fraction == "" ? "" : "."), fraction
}
