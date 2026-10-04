# Homepage owns site chrome; only page-relative links and current state vary.
FNR==NR {
    if(index($0,"<header class=\"site-header\"")) { reading=1; headers++ }
    if(reading) header=header $0 "\n"
    if(reading && index($0,"</header>")) reading=0
    if(match($0, /<footer>.*<\/footer>/)) { footer=substr($0,RSTART,RLENGTH); footers++ }
    next
}
{ output=output $0 "\n" }
function replace_once(text, needle, replacement, position) {
    position=index(text,needle)
    if(!position || index(substr(text,position+length(needle)),needle)) {
        print "site chrome: expected one " needle > "/dev/stderr"; failed=1; return text
    }
    return substr(text,1,position-1) replacement substr(text,position+length(needle))
}
END {
    if(headers!=1 || footers!=1 || reading) { print "site chrome: invalid homepage chrome" > "/dev/stderr"; exit 1 }
    root=(page=="playground" ? "../" : "./")
    gsub(/href="#/, "href=\"" root "#", header)
    gsub(/href="\.\/"/, "href=\"" root "\"", header)
    gsub(/href="releases.html"/, "href=\"" root "releases.html\"", header)
    gsub(/href="playground\/"/, "href=\"" root "playground/\"", header)
    current=(page=="home" ? root : root (page=="history" ? "releases.html" : "playground/"))
    header=replace_once(header,"href=\"" current "\"", "href=\"" current "\" aria-current=\"page\"")
    sub(/href="\.\/"/, "href=\"" root "\"", footer)
    if(page=="history") {
        output=replace_once(output,"<!-- SITE_HEADER -->",header)
        output=replace_once(output,"<!-- SITE_FOOTER -->",footer)
    } else {
        start=index(output,"<header class=\"site-header\"")
        end=index(substr(output,start),"</header>")
        if(!start || !end) { print "site chrome: missing site header" > "/dev/stderr"; exit 1 }
        output=substr(output,1,start-1) header substr(output,start+end+8)
        if(page=="playground") {
            output=replace_once(output,"</body>",footer "\n</body>")
            output=replace_once(output,"</head>","<link rel=\"stylesheet\" href=\"../chrome.css\">\n</head>")
        }
    }
    if(failed) exit 1
    printf "%s",output
}
