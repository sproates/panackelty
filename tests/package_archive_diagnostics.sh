#!/bin/sh
# Public package commands preserve the detected archive cause and clean staging.
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/archive-diagnostics.XXXXXX")
cleanup() { status=$?; if [ "$status" != 0 ]; then cat "$work/out" "$work/err" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
mkdir -p "$work/app/src"
printf 'main(): Void {}\n' > "$work/app/src/main.panack"
node - "$work" "$(cat VERSION)" <<'JS'
const fs=require('fs'),zlib=require('zlib');const [w,version]=process.argv.slice(2);
const m={schema:1,package:'test.app',version:'1.0.0',language:'modules-v1',toolchain:version,source:'src',entry:'main.panack',exports:{},dependencies:{lib:{archive:'../source.tar.gz'}}};fs.writeFileSync(w+'/app/panack.json',JSON.stringify(m));
function checksum(h){h.fill(32,148,156);h.write([...h].reduce((a,b)=>a+b,0).toString(8).padStart(6,'0')+'\0 ',148);return h;}
function header(name,size=0,type='0'){let h=Buffer.alloc(512);h.write(name);h.write('0000644\0',100);h.write(size.toString(8).padStart(11,'0')+'\0',124);h.write(type,156);h.write('ustar\0',257);h.write('00',263);return checksum(h);}
const end=Buffer.alloc(1024);function file(name,body=Buffer.alloc(0),type='0'){body=Buffer.from(body);return Buffer.concat([header(name,body.length,type),body,Buffer.alloc((512-body.length%512)%512)]);}
const pax=(key,value)=>{let tail=` ${key}=${value}\n`,n=tail.length+1;while(String(n).length+tail.length!==n)n=String(n).length+tail.length;return `${n}${tail}`;};
const badsum=header('data');badsum[1]^=1;
const badnumber=header('data');badnumber[124]=120;checksum(badnumber);
const utf8=header('data');utf8[0]=255;checksum(utf8);
const numberlimit=header('data',268435457);
const cases={
'archive-gzip':Buffer.from('not gzip'),
'archive-unsupported':zlib.gzipSync(Buffer.concat([file('link','','2'),end])),
'archive-header-checksum':zlib.gzipSync(Buffer.concat([badsum,end])),
'archive-number':zlib.gzipSync(Buffer.concat([badnumber,end])),
'archive-number-limit':zlib.gzipSync(Buffer.concat([numberlimit,end])),
'archive-utf8':zlib.gzipSync(Buffer.concat([utf8,end])),
'archive-parent':zlib.gzipSync(Buffer.concat([file('a/../b'),end])),
'archive-duplicate':zlib.gzipSync(Buffer.concat([file('data'),file('data'),end])),
'archive-case':zlib.gzipSync(Buffer.concat([file('Data'),file('data'),end])),
'archive-no-manifest':zlib.gzipSync(Buffer.concat([file('src/api.panack'),end])),
'archive-pax-field':zlib.gzipSync(Buffer.concat([file('metadata',pax('unknown','value'),'x'),file('data'),end])),
'archive-pax-duplicate':zlib.gzipSync(Buffer.concat([file('metadata',pax('path','one')+pax('path','two'),'x'),file('data'),end])),
'archive-pax-size':zlib.gzipSync(Buffer.concat([file('metadata',pax('size','x'),'x'),file('data'),end])),
'archive-incomplete':zlib.gzipSync(Buffer.concat([file('data'),Buffer.alloc(512)])),
'archive-trailing':zlib.gzipSync(Buffer.concat([end,Buffer.from('bad')]))
};for(const [name,data] of Object.entries(cases))fs.writeFileSync(w+'/'+name+'.tar.gz',data);
JS
while read -r scenario code detail; do
    cp "$work/$scenario.tar.gz" "$work/source.tar.gz"
    status=0
    "$root/bin/panack" deps lock --project "$work/app/panack.json" > "$work/out" 2> "$work/err" || status=$?
    test "$status" = 1
    test ! -s "$work/out"
    grep -F "panack: error[$code]:" "$work/err" >/dev/null
    grep -F "$detail" "$work/err" >/dev/null
    grep -F "Source archive: $work/source.tar.gz" "$work/err" >/dev/null
    test ! -e "$work/app/panack.lock.json"
    test -z "$(ls -A "$work/app/.panack_modules")"
    node tests/package_diagnostic_capture.cjs "$scenario" "$work/err" "$work"
done <<'CASES'
archive-gzip PKG_ARCHIVE_INVALID not valid gzip
archive-unsupported PKG_ARCHIVE_UNSUPPORTED unsupported entry type
archive-header-checksum PKG_ARCHIVE_INVALID header's checksum
archive-number PKG_ARCHIVE_INVALID invalid digits or spacing
archive-number-limit PKG_ARCHIVE_NUMBER_LIMIT 268435456
archive-utf8 PKG_ARCHIVE_PATH_INVALID not valid UTF-8
archive-parent PKG_ARCHIVE_PATH_INVALID parent-directory component
archive-duplicate PKG_ARCHIVE_PATH_CONFLICT same path more than once
archive-case PKG_ARCHIVE_PATH_CONFLICT differ only in letter case
archive-no-manifest PKG_MANIFEST_MISSING no panack.json at its top level
archive-pax-field PKG_ARCHIVE_UNSUPPORTED PAX field: unknown
archive-pax-duplicate PKG_ARCHIVE_INVALID PAX field: path
archive-pax-size PKG_ARCHIVE_INVALID decimal digits only
archive-incomplete PKG_ARCHIVE_INCOMPLETE before its contents and end marker were complete
archive-trailing PKG_ARCHIVE_INVALID unexpected data after its end marker
CASES
printf '%s\n' 'PASS archive diagnostics: precise causes, source paths and no partial publication'
