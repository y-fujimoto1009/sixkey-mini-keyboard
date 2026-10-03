import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
import {defaults,encode,decode,parseReply} from './docs/model.js';

// Exercise the actual action/held/tap function bodies, adapting only C++
// declarations to JS. USB transport is modeled, not a physical HID test.
const path=process.argv[2] || 'firmware/SixKey_Config/SixKey_Config.ino';
const source=fs.readFileSync(path,'utf8');
const buffered=source.includes('class ChordKeyboard');
const transport={held:new Set(),reports:[],dirty:false,ready:true,
  snapshot(){return [...this.held].sort((a,b)=>a-b);},
  emit(){this.dirty=true;if(!buffered)this.flushReport();},
  press(k){this.held.add(k);this.emit();},
  release(k){this.held.delete(k);this.emit();},
  flushReport(){if(this.dirty&&this.ready){this.reports.push(this.snapshot());this.dirty=false;}return !this.dirty;},
  waitReport(){return this.flushReport();}
};
function translate(name){
  const body=source.match(new RegExp(`void ${name}\\([^\\n]*?\\)\\{([^\\n]*)\\}`))?.[1];
  assert(body,`missing ${name}`);
  return body.replace(/bool desired\[256\]=\{\}/g,'let desired=Array(256).fill(false)')
    .replace(/const char keys\[\]/g,'const keys').replace(/\(uint8_t\)keys\[([^\]]+)\]/g,'keys.charCodeAt($1)')
    .replace(/\bint i=/g,'let i=')
    .replace(/memcpy\(sent,desired,sizeof\(sent\)\)/g,'desired.forEach((v,i)=>sent[i]=v)');
}
const ctx=vm.createContext({Keyboard:transport,chordKeyboard:transport,KEY_LEFT_CTRL:128,KEY_LEFT_ALT:130,KEY_LEFT_GUI:131,
  sent:Array(256).fill(false),buttons:Array.from({length:7},()=>({down:false})),
  config:{map:[...defaults]},delay(){},mediaTap(){}});
vm.runInContext(`function addAction(desired,a){${translate('addAction')}}
function syncHeld(){${translate('syncHeld')}}
function tap(a){${translate('tap')}}`,ctx);
const run=s=>vm.runInContext(s,ctx);
function reset(){transport.held.clear();transport.reports=[];transport.dirty=false;transport.ready=true;ctx.sent.fill(false);ctx.buttons.forEach(b=>b.down=false);ctx.config.map=[...defaults];}
const letters='cvxzsa';
const shortcuts=[...[...letters].map((c,i)=>[1001+i,c,128]),...[...letters].map((c,i)=>[1101+i,c,131]),[1201,'a',130],[1202,'v',130]];
for(const [action,letter,modifier] of shortcuts){
  const k=letter.charCodeAt(0);
  const map=defaults.map(()=>action);
  assert.deepEqual(decode(encode(map)),map);
  assert.deepEqual(parseReply('K6/1 MAP '+map.join(',')),map);
  reset();ctx.config.map[0]=action;ctx.buttons[0].down=true;run('syncHeld()');
  assert.deepEqual(transport.reports,[[k,modifier]],`shortcut ${action} must not emit a bare letter`);
  run('syncHeld()');assert.equal(transport.reports.length,1,'no repeat reports for unchanged held keys');
  ctx.buttons[0].down=false;run('syncHeld()');assert.deepEqual(transport.reports.at(-1),[]);
  reset();run(`tap(${action})`);assert.deepEqual(transport.reports,[[k,modifier],[]]);
}
reset();ctx.config.map[0]=1001;ctx.config.map[1]=1006;ctx.buttons[0].down=ctx.buttons[1].down=true;run('syncHeld()');
assert.deepEqual(transport.reports,[[97,99,128]]);
ctx.buttons[0].down=false;run('syncHeld()');assert.deepEqual(transport.reports.at(-1),[97,128]);
ctx.buttons[1].down=false;run('syncHeld()');assert.deepEqual(transport.reports.at(-1),[]);
reset();ctx.config.map[0]=128;ctx.buttons[0].down=true;run('syncHeld();tap(1001)');
assert.deepEqual(transport.reports,[[128],[99,128],[128]],'encoder tap preserves held Ctrl');
reset();ctx.config.map[8]=1006;ctx.buttons[6].down=true;run('syncHeld()');assert.deepEqual(transport.reports,[[97,128]]);
reset();ctx.config.map[0]=99;ctx.buttons[0].down=true;run('syncHeld()');assert.deepEqual(transport.reports,[[99]],'plain C remains plain');
reset();transport.ready=false;ctx.config.map[0]=1001;ctx.buttons[0].down=true;run('syncHeld()');assert.equal(transport.reports.length,0);
transport.ready=true;run('syncHeld()');assert.deepEqual(transport.reports,[[99,128]],'busy endpoint retries full chord');
ctx.buttons[0].down=false;transport.ready=false;run('syncHeld()');transport.ready=true;run('syncHeld()');assert.deepEqual(transport.reports.at(-1),[]);
assert.match(source,/tud_hid_keyboard_report\(USB.findHIDReportID\(_id\),pendingReport.modifiers,pendingReport.keys\)\)pendingReportDirty=false/);
console.log('PASS: fourteen Ctrl/Command/Alt shortcuts, JSON/serial round trips, held/released keys, shared Ctrl, encoder, plain C, modeled USB retry. Physical device untested.');
