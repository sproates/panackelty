// Test peer: Git CGI behind a private TLS endpoint, with no external service.
const fs = require('node:fs');
const https = require('node:https');
const { spawn, execFileSync } = require('node:child_process');
const path = require('node:path');
const root = process.argv[2];
const backend = path.join(execFileSync('/usr/bin/git', ['--exec-path'], {encoding: 'utf8'}).trim(), 'git-http-backend');
const server = https.createServer({key: fs.readFileSync(root + '/test.key'), cert: fs.readFileSync(root + '/test.crt')}, (req, res) => {
  fs.appendFileSync(root + '/requests', req.method + ' ' + req.url + '\n');
  const url = new URL(req.url, 'https://localhost');
  // Switch failures without changing the locked URL, as a real outage would.
  if (fs.existsSync(root + '/interrupt-peer') &&
      (fs.readFileSync(root + '/interrupt-peer', 'utf8').trim() !== 'git' || url.pathname.startsWith('/sha.git/'))) {
    req.socket.destroy(); return;
  }
  if (url.pathname === '/package.tar.gz') {
    const data = fs.readFileSync(root + '/package.tar.gz');
    res.writeHead(200, {'Content-Length': data.length});
    return res.end(data);
  }
  if (url.pathname.startsWith('/redirect/')) { res.writeHead(302, {Location: '/repo.git/info/refs?service=git-upload-pack'}); return res.end(); }
  if (url.pathname.startsWith('/auth/')) { res.writeHead(401, {'WWW-Authenticate': 'Basic realm="fixture"'}); return res.end(); }
  if (url.pathname.startsWith('/interrupt/')) { req.socket.destroy(); return; }
  const env = {PATH: '/usr/bin:/bin', HOME: root, GIT_CONFIG_NOSYSTEM: '1', GIT_CONFIG_GLOBAL: '/dev/null',
    GIT_PROJECT_ROOT: root, GIT_HTTP_EXPORT_ALL: '1', REQUEST_METHOD: req.method,
    PATH_INFO: url.pathname, QUERY_STRING: url.search.slice(1), CONTENT_TYPE: req.headers['content-type'] || '',
    CONTENT_LENGTH: req.headers['content-length'] || '', HTTP_GIT_PROTOCOL: req.headers['git-protocol'] || '',
    REMOTE_ADDR: '127.0.0.1', SERVER_PROTOCOL: 'HTTP/1.1'};
  const child = spawn(backend, [], {env});
  let header = Buffer.alloc(0), sent = false;
  child.stdout.on('data', chunk => {
    if (sent) return res.write(chunk);
    header = Buffer.concat([header, chunk]);
    const index = header.indexOf('\r\n\r\n');
    if (index < 0) return;
    for (const line of header.subarray(0, index).toString().split('\r\n')) {
      const colon = line.indexOf(':');
      if (colon < 0) continue;
      const name = line.slice(0, colon), value = line.slice(colon + 1).trim();
      if (name.toLowerCase() === 'status') res.statusCode = Number(value.split(' ')[0]);
      else res.setHeader(name, value);
    }
    sent = true;
    res.write(header.subarray(index + 4));
    header = Buffer.alloc(0);
  });
  child.stderr.on('data', data => fs.appendFileSync(root + '/backend.log', data));
  child.on('error', error => { res.statusCode = 500; res.end('backend failed'); });
  child.on('close', () => { if (!sent) res.statusCode = 500; res.end(); });
  req.pipe(child.stdin);
  child.stdin.on('error', () => {});
});
server.on('connection', () => fs.appendFileSync(root + '/connections', 'connection\n'));
server.listen(0, '127.0.0.1', () => fs.writeFileSync(root + '/port', String(server.address().port)));
process.on('SIGTERM', () => server.close());
