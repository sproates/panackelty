// Test-only TLS peer: raw framing and request evidence cannot be supplied by
// the production HTTP server. No external network or host credentials are used.
const tls = require('node:tls');
const fs = require('node:fs');
const path = require('node:path');
const root = process.argv[2];
const server = tls.createServer({key:fs.readFileSync(path.join(root,'test.key')),cert:fs.readFileSync(path.join(root,'test.crt'))}, socket => {
  socket.on('error',()=>{});
  let request='';
  socket.on('data', chunk => {
    request += chunk.toString('latin1');
    if (!request.includes('\r\n\r\n')) return;
    socket.removeAllListeners('data');
    const target = request.split(' ')[1];
    fs.appendFileSync(path.join(root,'requests'),target+'\n');
    const reply = (status,headers,body=Buffer.alloc(0)) => socket.end(Buffer.concat([Buffer.from(`HTTP/1.1 ${status}\r\nConnection: close\r\n${headers}\r\n`),body]));
    if(target==='/redirect.tar.gz') return reply('302 Found','Location: /lib.tar.gz\r\nContent-Length: 0\r\n');
    if(target==='/loop.tar.gz') return reply('302 Found','Location: /loop.tar.gz\r\nContent-Length: 0\r\n');
    if(target==='/downgrade.tar.gz') return reply('302 Found','Location: http://localhost/forbidden\r\nContent-Length: 0\r\n');
    if(target==='/secret.tar.gz') return reply('302 Found','Location: https://secret:password@localhost/forbidden\r\nContent-Length: 0\r\n');
    if(target==='/query.tar.gz') return reply('302 Found','Location: /forbidden?secret=token\r\nContent-Length: 0\r\n');
    const redirectFailures = {
      '/no-location.tar.gz': '',
      '/empty-location.tar.gz': 'Location: \r\n',
      '/two-locations.tar.gz': 'Location: /lib.tar.gz\r\nLocation: /other.tar.gz\r\n',
      '/scheme.tar.gz': 'Location: ftp://localhost/forbidden\r\n',
      '/fragment.tar.gz': 'Location: /forbidden#secret=token\r\n',
      '/space.tar.gz': 'Location: /forbidden path\r\n',
      '/backslash.tar.gz': 'Location: /forbidden\\path\r\n'
    };
    if (target in redirectFailures) return reply('302 Found', redirectFailures[target]+'Content-Length: 0\r\n');
    const status = /^\/status-(\d+)\.tar\.gz$/.exec(target);
    if (status) return reply(status[1]+' Test', 'Content-Length: 0\r\n');
    if(target==='/bad-header.tar.gz') return reply('200 OK','Not a header\r\nContent-Length: 0\r\n');
    if(target==='/transfer.tar.gz') return reply('200 OK','Transfer-Encoding: gzip\r\n');
    if(target==='/missing.tar.gz') return reply('404 Not Found','Content-Length: 0\r\n');
    if(target==='/oversize.tar.gz') return reply('200 OK','Content-Length: 67108865\r\n');
    if(target==='/invalid-gzip.tar.gz') return reply('200 OK','Content-Length: 8\r\n',Buffer.from('not gzip'));
    const name = target==='/other.tar.gz'?'other.tar.gz':'lib.tar.gz';
    const data=fs.readFileSync(path.join(root,name));
    if(target==='/truncated.tar.gz') return reply('200 OK',`Content-Length: ${data.length+10}\r\n`,data);
    if(target==='/encoded.tar.gz') return reply('200 OK',`Content-Encoding: gzip\r\nContent-Length: ${data.length}\r\n`,data);
    if(target==='/chunked.tar.gz') {
      socket.write('HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n');
      for(let i=0;i<data.length;i+=7){const c=data.subarray(i,i+7);socket.write(c.length.toString(16)+'\r\n');socket.write(c);socket.write('\r\n');}
      return socket.end('0\r\nX-Test: complete\r\n\r\n');
    }
    reply('200 OK',`Content-Length: ${data.length}\r\n`,data);
  });
});
server.on('connection',()=>fs.appendFileSync(path.join(root,'connections'),'connection\n'));
server.listen(0,'127.0.0.1',()=>fs.writeFileSync(path.join(root,'port'),String(server.address().port)));
