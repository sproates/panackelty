import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawnSync} from 'node:child_process';
import {createHash} from 'node:crypto';
import {versionAssets} from './assets.mjs';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '../..');
const sdk = process.env.WASI_SDK_PATH;
if (!sdk || !path.isAbsolute(sdk)) throw new Error('Set WASI_SDK_PATH to an absolute WASI SDK 34.0 directory.');
const clang = path.join(sdk, 'bin/clang');
const version = spawnSync(clang, ['--version'], {encoding:'utf8'});
if (version.status !== 0 || !version.stdout.includes('23.1.0-wasi-sdk')) {
  throw new Error('Expected WASI SDK 34.0 / Clang 23.1.0-wasi-sdk.');
}
const out = path.join(root, 'build/playground');
fs.rmSync(out, {recursive:true, force:true});
fs.mkdirSync(out, {recursive:true});
const sources = fs.readdirSync(path.join(root, 'src/vm')).filter(n => n.endsWith('.c') && n !== 'host_capabilities.c').sort().map(n => path.join(root, 'src/vm', n));
// The compiler/linker subprocess has only SDK binaries in PATH: no interpreter.
const result = spawnSync(clang, [...sources, path.join(here, 'host_capabilities.c'),
  '-I'+path.join(root, 'src/vm'), '-O2', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
  '-Wl,-z,stack-size=2097152', '-Wl,--max-memory=268435456', '-Wl,--strip-all',
  '-o', path.join(out, 'vm.wasm')], {stdio:'inherit', env:{...process.env, PATH:path.join(sdk, 'bin')}});
if (result.status !== 0) throw new Error('WASI compilation failed.');
for (const name of ['runtime.mjs','worker.mjs','controller.mjs','app.mjs','examples.mjs','index.html','style.css']) {
  fs.copyFileSync(path.join(here, name), path.join(out, name));
}
const vendor = path.join(out, 'vendor');
fs.mkdirSync(vendor, {recursive:true});
const dependency = path.join(root, 'build/playground-tools/node_modules/@bjorn3/browser_wasi_shim');
for (const name of fs.readdirSync(path.join(dependency, 'dist')).filter(n => n.endsWith('.js'))) {
  fs.copyFileSync(path.join(dependency, 'dist', name), path.join(vendor, name));
}
fs.copyFileSync(path.join(dependency, 'LICENSE-MIT'), path.join(vendor, 'LICENSE-MIT'));
fs.copyFileSync(path.join(root, 'LICENSE'), path.join(out, 'LICENSE'));
fs.copyFileSync(path.join(root, 'bootstrap/compiler-v9.bc'), path.join(out, 'compiler.bc'));
const stdlib = Object.fromEntries(fs.readdirSync(path.join(root, 'src/stdlib')).filter(n => n.endsWith('.panack')).sort().map(n => [n, fs.readFileSync(path.join(root, 'src/stdlib', n), 'utf8')]));
fs.writeFileSync(path.join(out, 'stdlib.json'), JSON.stringify(stdlib));
fs.writeFileSync(path.join(out, 'package.json'), '{"type":"module"}\n');
const inputs = [...sources, path.join(here,'host_capabilities.c'), path.join(root,'bootstrap/compiler-v9.bc')];
const hash = file => createHash('sha256').update(fs.readFileSync(file)).digest('hex');
fs.writeFileSync(path.join(out, 'provenance.json'), JSON.stringify({toolchain:version.stdout.trim(),
  inputs:Object.fromEntries(inputs.map(file => [path.relative(root,file),hash(file)])),
  artifacts:Object.fromEntries(['vm.wasm','compiler.bc','stdlib.json'].map(name => [name,hash(path.join(out,name))]))}, null, 2)+'\n');
fs.copyFileSync(path.join(root, 'site/styles.css'), path.join(out, 'site.css'));
fs.copyFileSync(path.join(root, 'site/favicon.svg'), path.join(out, 'favicon.svg'));
versionAssets(out);
console.log('Built '+out+' with SDK-only compiler PATH.');
// Browser tests exercise the real website-relative paths, not a root-only demo.
const website = path.join(root, 'build/website');
fs.rmSync(website, {recursive:true, force:true});
fs.cpSync(path.join(root, 'site'), website, {recursive:true});
fs.cpSync(out, path.join(website, 'playground'), {recursive:true});
