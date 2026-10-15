# Emit the closed set of source files loaded for one ordinary probe compile.
# An unsupported or unresolved import makes the caller use the conservative
# repository-wide fingerprint instead.

function normalize(path, parts, count, item, absolute, result, segment) {
    absolute = substr(path, 1, 1) == "/"
    if (!absolute) path = cwd "/" path
    absolute = substr(path, 1, 1) == "/"
    count = split(path, parts, "/")
    depth = 0
    for (item = 1; item <= count; item++) {
        segment = parts[item]
        if (segment == "" || segment == ".") continue
        if (segment == "..") {
            if (depth > 0 && stack[depth] != "..") depth--
            else if (!absolute) stack[++depth] = segment
        } else {
            stack[++depth] = segment
        }
    }
    result = absolute ? "/" : ""
    for (item = 1; item <= depth; item++) {
        if (result != "" && result != "/") result = result "/"
        result = result stack[item]
    }
    return result == "" ? "." : result
}

function enqueue(path) {
    if (path == "" || path ~ /[\t\r\n]/) {
        uncertain = 1
        return
    }
    path = normalize(path)
    if (!(path in queued)) {
        queued[path] = 1
        queue[++tail] = path
    }
}

function resolve_import(imported, current, relative, suffix, parent, path) {
    if (substr(imported, 1, 7) == "stdlib/") {
        relative = substr(imported, 8)
        if (relative == "" || relative ~ /[^A-Za-z0-9_.\/-]/) return ""
        if (relative ~ /^\// || relative ~ /(^|\/)\.\.?($|\/)/) return ""
        suffix = relative ~ /\.panack$/ ? "" : ".panack"
        return normalize(stdlib_root "/" relative suffix)
    }
    if (substr(imported, 1, 8) == "project/") {
        if (project_root == "") return ""
        relative = substr(imported, 9)
        if (relative == "" || relative ~ /[^A-Za-z0-9_.\/-]/) return ""
        if (relative ~ /^\// || relative ~ /(^|\/)\.\.?($|\/)/) return ""
        suffix = relative ~ /\.panack$/ ? "" : ".panack"
        return normalize(project_root "/" relative suffix)
    }
    if (imported !~ /\.panack$/ || substr(imported, 1, 1) == "/") return ""
    parent = current
    sub(/\/[^\/]*$/, "", parent)
    return normalize(parent "/" imported)
}

function scan_import(line, current, rest, imported, item, character, closed, target) {
    sub(/^[ \t]*/, "", line)
    if (line ~ /^pub[ \t]+import[ \t]+/) sub(/^pub[ \t]+/, "", line)
    if (line !~ /^import[ \t]+/) return
    sub(/^import[ \t]+/, "", line)
    sub(/^[ \t]*/, "", line)
    rest = line
    imported = ""
    if (substr(rest, 1, 1) == "\"") {
        closed = 0
        for (item = 2; item <= length(rest); item++) {
            character = substr(rest, item, 1)
            if (character == "\\") {
                uncertain = 1
                return
            }
            if (character == "\"") {
                closed = 1
                break
            }
            imported = imported character
        }
        if (!closed) {
            uncertain = 1
            return
        }
    } else {
        imported = rest
        sub(/::.*/, "", imported)
        sub(/[;,{].*$/, "", imported)
        sub(/[ \t].*$/, "", imported)
    }
    target = resolve_import(imported, current)
    if (target == "") {
        uncertain = 1
    }
    else enqueue(target)
}

BEGIN {
    cwd = normalize(cwd)
    stdlib_root = normalize(stdlib_root)
    if (project_root != "") project_root = normalize(project_root)
    source = normalize(source)
    enqueue(source)
    enqueue(stdlib_root "/core.panack")

    for (head = 1; head <= tail && !uncertain; head++) {
        current = queue[head]
        status = (getline line < current)
        if (status < 0) {
            uncertain = 1
            close(current)
            break
        }
        while (status > 0) {
            scan_import(line, current)
            if (uncertain) break
            status = (getline line < current)
            if (status < 0) uncertain = 1
        }
        close(current)
    }

    if (uncertain) exit 2
    for (path in queued) print path
    exit 0
}
