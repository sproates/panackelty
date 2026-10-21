# Static HTTP server

Serve any directory over HTTP. Native alpha.14 includes the installed
`panack-http-server` command:

```sh
panack-http-server --root ./site --port 8000
```

From a built source checkout, run the same tool with:

```sh
./bin/panack run --project tools/http-server/panack.json --root ./site --port 8000
```

`make install PREFIX=/your/installation` installs `bin/panack-http-server` together
with its runtime and packages. The command resolves those files from its own
installation, works from any directory, and survives moving the whole installation.
It does not require an adjacent source checkout. Install/build once before use;
installation compiles the tool once and launching executes its saved bytecode. Native alpha.14 archives include this tool. See the
[installation guide](../../docs/guides/INSTALLATION.md).

```sh
/your/installation/bin/panack-http-server --root ./site --port 4173 --no-cache --noindex
```

Options:

| Option | Default | Meaning |
| --- | --- | --- |
| `--root DIRECTORY` | current directory | Directory to serve |
| `--host IPV4` | `127.0.0.1` | Numeric IPv4 bind address |
| `--port PORT` | `8000` | Port from 1 to 65535 |
| `--no-cache` | off | Add `Cache-Control: no-store` |
| `--noindex` | off | Add `X-Robots-Tag: noindex` |
| `--help` | | Show usage, as the only argument |
| `--version` | | Print `panackelty-http-server 1`, the launcher protocol version |

The tool writes exactly `Serving http://ADDRESS:PORT/` followed by a newline after
successful root validation and bind. Launchers can use that line as readiness;
stdout failure aborts startup. Startup/runtime errors go to stderr with the prefix
`http-server:`. Exit status is 0 for a normal signal-driven stop, 1 for service or
root failure, and 2 for invalid arguments. Ctrl-C/SIGINT and SIGTERM stop admission,
drain existing requests for up to one second, then cancel remaining requests.

The [package contract](../../packages/static_http/README.md) defines file confinement,
MIME types, HTTP methods, redirects and resource limits. File edits appear on the
next request. Generated sites still need their existing rebuild/restart workflow.
This is a development/preview tool with bounded responses and no TLS.
