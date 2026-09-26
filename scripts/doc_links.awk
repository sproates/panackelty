# Extract inline/image destinations and reference definitions, outside fences.
# This checks local files, not remote availability or heading fragments.
function emit(raw, target) {
    target=raw
    sub(/^[ \t]+/, "", target)
    if (target ~ /^</) { sub(/^</,"",target); sub(/>.*/,"",target) }
    else { sub(/[ \t]+["\047].*/,"",target); sub(/[ \t]+$/, "", target) }
    if (target!="") print target
}
/^[ \t]*(```|~~~)/ { fence=!fence; next }
fence { next }
{
    text=$0
    if (text ~ /^[ \t]*\[[^]]+\]:[ \t]*/) {
        sub(/^[ \t]*\[[^]]+\]:[ \t]*/, "", text)
        emit(text)
        next
    }
    # Remove inline code; backticks used to style link labels are harmless.
    gsub(/`[^`]*`/, "", text)
    while (match(text, /\]\(/)) {
        text=substr(text,RSTART+2)
        depth=1; stop=0
        for (i=1;i<=length(text);i++) {
            c=substr(text,i,1)
            if(c=="\\") { i++; continue }
            if(c=="(") depth++
            if(c==")") depth--
            if(depth==0) { stop=i; break }
        }
        if(!stop) {
            if(strict) { print "docs: unclosed inline link in " FILENAME > "/dev/stderr"; exit 1 }
            break
        }
        emit(substr(text,1,stop-1))
        text=substr(text,stop+1)
    }
}
