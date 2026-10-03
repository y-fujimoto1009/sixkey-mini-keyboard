import fs from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
import * as model from '../docs/model.js';

// Execute the real site client against an in-memory CDC device, without USB access.
async function scenario(play2){
 const slots=[],games=[],elements=new Map(),commands=[];
 class Element{
  constructor(){this.children=[];this.dataset={};this.classList={toggle(){}};this.disabled=false;}
  append(...children){this.children.push(...children);if(this.id==='keys')slots.push(...children);}
  setAttribute(name,value){this[name]=value;}
  querySelector(){return this.children[1];}
 }
 for(const id of ['status','action','keys','edit-title','slot-kind','selection','connect','disconnect','read','write','download','import','file','connection','game-connection']){const e=new Element();e.id=id;elements.set(id,e);}
 for(const id of [6,7,8]){const e=new Element();e.dataset.slot=id;slots.push(e);}
 for(const id of [0,1,2,3]){const e=new Element();e.dataset.game=id;games.push(e);}
 let resolveRead,closed=false,deviceMap=[...model.defaults];
 const reader={read(){if(closed)return Promise.resolve({done:true});return new Promise(resolve=>resolveRead=resolve);},async cancel(){closed=true;if(resolveRead)resolveRead({done:true});},releaseLock(){}};
 const writer={async write(bytes){
  const command=new TextDecoder().decode(bytes).trim();commands.push(command);
  if(command.startsWith('K6/1 SET '))deviceMap=model.validateMap(command.slice(9).split(',').map(Number));
  let reply=command==='K6/1 GET'||command.startsWith('K6/1 SET ')?'K6/1 MAP '+deviceMap.join(','):command==='K6/1 CAPS'?(play2?'K6/1 CAPS PLAY2':'K6/1 ERR COMMAND'):command;
  resolveRead({done:false,value:new TextEncoder().encode(reply+'\n')});
 },releaseLock(){}};
 const port={async open(){},async close(){},readable:{getReader:()=>reader},writable:{getWriter:()=>writer}};
 const context=vm.createContext({...model,console,setTimeout,clearTimeout,TextEncoder,TextDecoder,Blob,URL,confirm:()=>true,navigator:{serial:{requestPort:async()=>port}},window:{addEventListener(){}},document:{getElementById:id=>elements.get(id),createElement:()=>new Element(),querySelectorAll:selector=>selector==='[data-slot]'?slots:games}});
 vm.runInContext(fs.readFileSync('docs/app.js','utf8').replace(/^import[^;]+;/,''),context);
 assert(games.every(b=>b.disabled),'Cannot select a game before connection');
 await elements.get('connect').onclick();
 assert.deepEqual(commands,['K6/1 GET','K6/1 CAPS']);
 assert.equal(elements.get('write').disabled,false,'Old firmware remains configurable');
 assert(games.every(b=>b.disabled===!play2));
 if(play2){
  await games[1].onclick();assert.equal(commands.at(-1),'K6/1 GAME 1');
  assert.match(elements.get('game-connection').textContent,/うさぎ/);
  await games[0].onclick();assert.equal(commands.at(-1),'K6/1 GAME 0');
  assert.match(elements.get('game-connection').textContent,/キーボード/);
 }
 assert(!commands.some(s=>s.startsWith('K6/1 SET')),'Game selection must not overwrite keymap');
 const expected=[1101,1102,1106,1201,1202,1001,1105,1201,1202];
 for(let i=0;i<9;i++){
  slots.find(b=>Number(b.dataset.slot)===i).onclick();
  elements.get('action').value=expected[i];elements.get('action').onchange();
 }
 await elements.get('write').onclick();
 assert.deepEqual(deviceMap,expected);
 assert.deepEqual(commands.slice(-2),['K6/1 SET '+expected.join(','),'K6/1 GET']);
 assert.match(elements.get('status').textContent,/読み戻しが一致/);
 await elements.get('read').onclick();
 for(let i=0;i<9;i++){slots.find(b=>Number(b.dataset.slot)===i).onclick();assert.equal(Number(elements.get('action').value),expected[i]);}
 await elements.get('disconnect').onclick();assert(games.every(b=>b.disabled));
}
await scenario(true);await scenario(false);
console.log('Serial client: game selection, legacy compatibility, Command/Alt/Ctrl UI save and readback: PASS (simulated device)');
