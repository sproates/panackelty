# Deliberately bounded changelog Markdown: headings, paragraphs, bullets, inline code.
function error(message) { print "release history: " message > "/dev/stderr"; failed=1; exit 1 }
function inline(text, parts,n,i,result) {
    gsub(/\&/, "\\&amp;", text); gsub(/</, "\\&lt;", text); gsub(/>/, "\\&gt;", text)
    n=split(text,parts,"`"); if(n%2==0) error("unclosed inline code")
    for(i=1;i<=n;i+=2) if(parts[i] ~ /\*|_[[:alnum:]]|\]\(/) error("unsupported inline Markdown")
    result=parts[1]; for(i=2;i<=n;i++) result=result (i%2==0 ? "<code>" parts[i] "</code>" : parts[i])
    return result
}
function flush() {
    if(buffer!="") body=body (bullet ? "<li>" : "<p>") inline(buffer) (bullet ? "</li>\n" : "</p>\n")
    buffer=""
}
function close_list() { flush(); if(list) body=body "</ul>\n"; list=0 }
function finish() {
    close_list()
    if(version=="") return
    if(!entries) error("empty section " version)
    if(version=="Unreleased") {
        pending=pending "<details class=\"release-pending\"><summary>Unreleased · current source, not a download</summary><div class=\"release-body\"><p>Development notes since the latest native release. Unreleased compiler/runtime features are not in published downloads or the pinned playground; installation-tooling updates are identified separately.</p>" body "<p><a href=\"https://github.com/sproates/panackelty/blob/main/CHANGELOG.md\">Full current-source notes</a></p></div></details>\n"
    } else {
        if(version==published) available=1
        label=available ? "Published native preview" : "Prepared notes · not yet available"
        article="<article class=\"release-entry\" id=\"v" version "\"><header><p class=\"eyebrow\">" label "</p><h2>" version "</h2><p><time datetime=\"" date "\">" date "</time></p></header>" body
        if(available) article=article "<p class=\"release-links\"><a href=\"https://github.com/sproates/panackelty/releases/tag/v" version "\">Downloads &amp; checksums</a><a href=\"https://github.com/sproates/panackelty/blob/v" version "/CHANGELOG.md\">Full release notes</a></p>"
        else article=article "<p>No published downloads are claimed for this version.</p>"
        article=article "</article>\n"
        if(available) releases=releases article; else pending=pending article
    }
}
/^## / {
    finish(); body=""; entries=0
    if($0=="## Unreleased") { version="Unreleased"; if(sections) error("Unreleased must be first") }
    else {
        if(NF!=4 || $2 !~ /^[0-9]+\.[0-9]+\.[0-9]+-alpha\.[0-9]+$/ || $3!="—" || $4 !~ /^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]$/) error("invalid version/date heading")
        version=$2; date=$4
        if(date < "2000-01-01" || substr(date,6,2)<"01" || substr(date,6,2)>"12" || substr(date,9,2)<"01" || substr(date,9,2)>"31") error("invalid date")
        year=substr(date,1,4)+0; month=substr(date,6,2)+0; day=substr(date,9,2)+0
        days=(month==2 ? 28+(year%4==0 && (year%100!=0 || year%400==0)) : (month==4 || month==6 || month==9 || month==11 ? 30 : 31))
        if(day>days) error("invalid calendar date")
        split(version, numbers, /[.-]/); ordinal=numbers[5]+0
        if(last && ordinal>=last) error("versions must descend within preview series")
        if(series && series!=numbers[1] "." numbers[2] "." numbers[3]) error("unsupported mixed preview series")
        series=numbers[1] "." numbers[2] "." numbers[3]; last=ordinal
    }
    if(seen[version]++) error("duplicate version " version)
    sections++; next
}
version=="" { next }
/^### / { close_list(); title=substr($0,5); if(title=="") error("empty category"); body=body "<h3>" inline(title) "</h3>\n"; next }
/^```/ { error("unsupported code fence") }
/^#/ { error("unsupported heading") }
/^$/ { flush(); next }
/^- / { flush(); if(!list) body=body "<ul>\n"; list=1; bullet=1; buffer=substr($0,3); entries++; next }
/^  / { if(buffer=="") error("orphan continuation"); buffer=buffer " " substr($0,3); next }
{ if(list) close_list(); if(buffer!="") buffer=buffer " "; bullet=0; buffer=buffer $0; entries++ }
END {
    if(failed) exit 1
    finish()
    if(!seen["Unreleased"] || !seen[published]) error("missing Unreleased or published version")
    print "<p class=\"release-availability\">Latest published native preview: <a href=\"#v" published "\">" published "</a>. Browser playground: <a href=\"https://github.com/sproates/panackelty-browser/releases/tag/" browser "\">" browser "</a>, a separately pinned WebAssembly package with fewer host capabilities. Native release notes do not promise browser support.</p>"
    print pending
    print releases
}
