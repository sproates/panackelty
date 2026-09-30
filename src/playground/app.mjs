import {Playground} from './controller.mjs';
const source=document.querySelector('#source'),run=document.querySelector('#run'),stop=document.querySelector('#stop');
const output=document.querySelector('#output'),status=document.querySelector('#status');
const playground=new Playground(data=>{
  const busy=data.type==='phase';run.disabled=busy;stop.disabled=!busy;
  status.textContent=busy?data.text:data.status===0?'Finished':'Stopped or failed';
  if(!busy)output.textContent=(data.stdout||'')+(data.stderr||'')||'(No output)';
});
run.addEventListener('click',()=>{output.textContent='';playground.run(source.value);});
stop.addEventListener('click',()=>playground.stop());
