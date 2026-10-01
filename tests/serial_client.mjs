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
 let resolveRead,closed=false;
 const reader={read(){if(closed)return Promise.resolve({done:true});return new Promise(resolve=>resolveRead=resolve);},async cancel(){closed=true;if(resolveRead)resolveRead({done:true});},releaseLock(){}};
 const writer={async write(bytes){
  const command=new TextDecoder().decode(bytes).trim();commands.push(command);
  let reply=command==='K6/1 GET'?'K6/1 MAP 49,50,51,52,53,54,2001,2002,2003':command==='K6/1 CAPS'?(play2?'K6/1 CAPS PLAY2':'K6/1 ERR COMMAND'):command;
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
 await elements.get('disconnect').onclick();assert(games.every(b=>b.disabled));
}
await scenario(true);await scenario(false);
console.log('Serial client: Play V2 game selection, keyboard return, legacy compatibility and no keymap write: PASS');
