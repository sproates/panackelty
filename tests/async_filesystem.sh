#!/bin/sh
set -eu
work=$(mktemp -d tests/async-filesystem.XXXXXX)
server_pid=
cleanup() {
    if [ -n "$server_pid" ]; then kill "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
http_client=${PANACK_HTTP_CLIENT:?}
mkdir -p "$work/root/empty" "$work/app/src"
printf 'ok' > "$work/root/file"
ln -s file "$work/root/inside"
ln -s ../outside "$work/root/escape"
printf 'private' > "$work/outside"
version=$(sed -n '1p' VERSION)
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.async_filesystem","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
cat > "$work/template.panack" <<'SOURCE'
import stdlib/async_filesystem as files
import stdlib/path as path
import http/server as http

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

pure file_path(value: Str): Path {
  path.from_text(value).value_or(path.current())
}

async respond(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  match await files.read_beneath(file_path("ROOT_PATH"), file_path("file"), 2) {
    Error(problem) => Error(problem.code),
    Ok(read) => { response.send_bytes(read.contents); Ok(()) }
  }
}

async main(): Unit {
  root: Path = file_path("ROOT_PATH")
  identity = await files.root_identity(root)
  expect(match identity { Ok(value) => true, Error(problem) => false })
  match await files.read_beneath(root, file_path("inside"), 2) {
    Error(problem) => expect(false),
    Ok(read) => {
      expect(read.contents == "ok".encode_utf8())
      expect(match identity { Ok(value) => value == read.identity, Error(problem) => false })
    }
  }
  expect(match await files.root_identity(file_path("ROOT_PATH/empty")) {
    Ok(value) => true, Error(problem) => false
  })
  expect(match await files.read_beneath(root, file_path("file"), 1) {
    Ok(value) => false, Error(problem) => problem.code == "output_limit"
  })
  expect(match await files.read_beneath(root, file_path("missing"), 2) {
    Ok(value) => false, Error(problem) => problem.code == "not_found"
  })
  expect(match await files.read_beneath(root, file_path("escape"), 64) {
    Ok(value) => false, Error(problem) => problem.code == "invalid_argument"
  })
  expect(match await files.read_beneath(root, file_path("empty"), 64) {
    Ok(value) => false, Error(problem) => problem.code == "not_regular_file"
  })
  server = http.create_bound_server("127.0.0.1", PORT)
  server.get("/", @respond)
  expect(match await server.listen() { Ok(done) => true, Error(problem) => false })
  ()
}
SOURCE
port=$("$http_client" port)
sed -e "s|ROOT_PATH|$PWD/$work/root|g" -e "s/PORT/$port/g" "$work/template.panack" > "$work/app/src/main.panack"
./bin/panack check --project "$work/app/panack.json" > /dev/null
./bin/panack compile --project "$work/app/panack.json" -o "$work/app.bc" > /dev/null
for mode in source bytecode; do
    if [ "$mode" = source ]; then
        ./bin/panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr" &
    else
        ./bin/panack run "$work/app.bc" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$http_client" "$port" concurrent || { cat "$work/stderr"; exit 1; }
    kill -TERM "$server_pid"
    wait "$server_pid" || { cat "$work/stderr"; exit 1; }
    server_pid=
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
done
cp "$work/app/src/main.panack" "$work/valid.panack"
sed 's/identity = await files.root_identity(root)/identity = files.root_identity(root)/'     "$work/valid.panack" > "$work/app/src/main.panack"
if ./bin/panack check --project "$work/app/panack.json" > "$work/invalid.stdout" 2> "$work/invalid.stderr"; then
    echo 'missing await unexpectedly accepted' >&2
    exit 1
fi
grep -q 'async call requires await' "$work/invalid.stderr"
printf '%s\n' 'Asynchronous confined filesystem reads and HTTP handler integration: ok'
