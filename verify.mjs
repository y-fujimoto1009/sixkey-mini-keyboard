import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import {actions,defaults,encode,decode,parseReply} from './docs/model.js';
assert.deepEqual(decode(encode(defaults)),defaults);
for(const a of actions){const m=defaults.map(()=>a.id);assert.deepEqual(decode(encode(m)),m);assert.deepEqual(parseReply('K6/1 MAP '+m.join(',')),m);}
for(const text of ['{}','null','[]','{"format":"sixkey-keymap","version":2}',encode(defaults).replace('49','9999')])assert.throws(()=>decode(text));
assert.throws(()=>parseReply('K6/1 MAP 49,50'));
assert.throws(()=>parseReply('K6/1 MAP ,50,51,52,53,54,2001,2002,2003'));
const source=fs.readFileSync('firmware/SixKey_Config/SixKey_Config.ino','utf8');
const expression=source.match(/bool validAction\(uint16_t a\)\{return (.*?);\}/)[1];
const valid=new Function('a','return '+expression);
for(let i=0;i<=3000;i++)assert.equal(valid(i),actions.some(a=>a.id===i),'action '+i);
assert.equal(source,fs.readFileSync('docs/SixKey_Config.ino','utf8'));
const uf2=fs.readFileSync('docs/sixkey-config-v1.uf2');assert.equal(uf2.length%512,0);
for(let off=0;off<uf2.length;off+=512){assert.equal(uf2.readUInt32LE(off),0x0a324655);assert.equal(uf2.readUInt32LE(off+4),0x9e5d5157);assert.equal(uf2.readUInt32LE(off+508),0x0ab16f30);assert.equal(uf2.readUInt32LE(off+20),off/512);assert.equal(uf2.readUInt32LE(off+24),uf2.length/512);assert.equal(uf2.readUInt32LE(off+28),0xe48bff56);assert.equal(uf2.readUInt32LE(off+16),256);}
const html=fs.readFileSync('docs/index.html','utf8');
for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){if(!m[1].startsWith('data:'))assert(fs.existsSync('docs/'+m[1]),m[1]);}
const report={softwareChecksPass:true,actionCount:actions.length,jsonRoundTripAllActions:true,invalidFilesRejected:true,firmwareActionSetMatches:true,sourceDownloadMatchesCompiledSource:true,uf2Blocks:uf2.length/512,uf2Sha256:crypto.createHash('sha256').update(uf2).digest('hex'),hardwareVerified:false,remaining:['Physical USB configuration write','Power-cycle persistence','New firmware LCD and HID behaviour']};
fs.writeFileSync('verification.json',JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report));
