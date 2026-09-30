import {test,expect} from '../../build/playground-tools/node_modules/@playwright/test/index.mjs';
import {exampleGuides} from '../../src/playground/examples.mjs';

async function run(page,source){
  await page.locator('#source').fill(source);
  await page.getByRole('button',{name:'Run program',exact:true}).click();
  await expect(page.getByRole('button',{name:'Run program',exact:true})).toBeEnabled();
}
test('real worker compiles stdlib and exact values, reports source errors, renders literal text',async({page})=>{
  await page.goto('/playground/');
  await page.getByRole('button',{name:'Run program',exact:true}).click();
  await expect(page.locator('#status')).toHaveText('Finished');
  await expect(page.locator('#output')).toHaveText('Hello, browser!\n');
  await run(page,'main(): Void { x: Nat = "bad"; print(x) }');
  await expect(page.locator('#output')).toContainText('cannot assign Str to Nat');
  await run(page,'main(): Void { print("<b>λ🙂</b>"); print((1/8).dec()) }');
  await expect(page.locator('#output')).toHaveText('<b>λ🙂</b>\n0.125\n');
  await expect(page.locator('#output b')).toHaveCount(0);
});
test('stop interrupts running code and fresh execution succeeds',async({page})=>{
  await page.goto('/playground/');await page.locator('#source').fill('main(): Void { while true {} }');
  await page.getByRole('button',{name:'Run program',exact:true}).click();
  await expect(page.locator('#status')).toHaveText('Running…');
  await page.getByRole('button',{name:'Stop',exact:true}).click();
  await expect(page.locator('#output')).toContainText('Stopped.');
  await run(page,'main(): Void { print("fresh") }');await expect(page.locator('#output')).toHaveText('fresh\n');
});
test('output, input, host and timeout failures leave the UI usable',async({page})=>{
  await page.goto('/playground/');
  await run(page,'main(): Void { while true { print("flood") } }');await expect(page.locator('#output')).toContainText('Output exceeds');
  await run(page,'x'.repeat(32769));await expect(page.locator('#output')).toContainText('Source exceeds');
  await run(page,'import "stdlib/host"\nmain(): Void { print(host_decode_utf8(utf8_encode("hi"))) }');
  await expect(page.locator('#output')).toContainText('host capability unavailable');
  await page.locator('#source').fill('main(): Void { while true {} }');
  await page.getByRole('button',{name:'Run program',exact:true}).click();
  await expect(page.locator('#output')).toContainText('Time limit reached',{timeout:20000});
  await run(page,'main(): Void { print("after timeout") }');await expect(page.locator('#output')).toHaveText('after timeout\n');
});
test('asset failures are explicit and narrow layout stays within viewport',async({page})=>{
  await page.route('**/compiler.bc',route=>route.abort());
  await page.goto('/playground/');await page.getByRole('button',{name:'Run program',exact:true}).click();
  await expect(page.getByRole('button',{name:'Run program',exact:true})).toBeEnabled();
  await expect(page.locator('#status')).toHaveText('Stopped or failed');
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth)).toBe(true);
});

test('homepage navigation and every selectable example work at website paths',async({page})=>{
  await page.goto('/');
  await page.getByRole('link',{name:'Try it online',exact:true}).click();
  await expect(page).toHaveURL(/\/playground\/$/);
  await expect(page.locator('#example option')).toHaveCount(9);
  for(const [name,guide] of Object.entries(exampleGuides)){
    await page.locator('#example').selectOption(name);
    await page.getByRole('button',{name:'Load example',exact:true}).click();
    await expect(page.locator('#source')).toBeFocused();
    await expect(page.locator('#example-title')).toHaveText(guide.title);
    await expect(page.locator('#example-description')).toHaveText(guide.description);
    await expect(page.locator('#example-edit')).toHaveText(guide.edit);
    await expect(page.locator('#expected-output')).toHaveText(guide.expected);
    await page.getByRole('button',{name:'Run program',exact:true}).click();
    await expect(page.locator('#status')).toHaveText('Finished');
    await expect(page.locator('#output')).toHaveText(guide.expected);
  }
  const edited = await page.locator('#source').inputValue();
  await page.locator('#example').selectOption('hello');
  await expect(page.locator('#source')).toHaveValue(edited);
  await expect(page.locator('#example-title')).toHaveText(exampleGuides.order.title);
  await page.getByText('What can I run here?',{exact:true}).click();
  await expect(page.getByText('This is not a persistent REPL.',{exact:false})).toBeVisible();
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth)).toBe(true);
  await page.getByRole('link',{name:'Back to Panackelty',exact:true}).click();
  await expect(page).toHaveURL(/\/$/);
  await expect(page.getByRole('link',{name:'Try it online',exact:true})).toBeVisible();
});

test('core types and chained methods need no imports',async({page})=>{
  await page.goto('/playground/');
  await run(page,'pure less(a: Nat,b: Nat): Bool { a < b } main(): Void { value: Result[Nat,Str] = Ok(42); print(value); print([3,1,2].sort_by(@less).first()); print("hello.panack".ends_with(".panack")); print("42".parse_nat()) }');
  await expect(page.locator('#output')).toHaveText('Ok(42)\nSome(1)\ntrue\n42\n');
  await run(page,'main(): Void { print(1.ends_with("x")) }');
  await expect(page.locator('#output')).toContainText('expected Str');
});
