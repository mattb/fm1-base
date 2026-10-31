// Run after tools/build.py examples/usb-audio --web: node examples/usb-audio/smoke.cjs
const assert = require('node:assert/strict');
const fs = require('node:fs');
const moduleBytes = fs.readFileSync(`${__dirname}/build/emu/app.wasm`);
const e = new WebAssembly.Instance(new WebAssembly.Module(moduleBytes), {
  wasi_snapshot_preview1: { clock_time_get: () => 0 }
}).exports;
e._initialize();
e.web_boot();
assert.equal(e.web_usb_channels(), 6);
assert.equal(e.web_usb_frames(), 256);
function render() {
  e.web_render(1024);
  for (const ptr of [e.web_out_l(), e.web_out_r()])
    assert.ok(new Float32Array(e.memory.buffer, ptr, 1024).every(v => v === 0), 'analog output must remain silent');
  return new Int32Array(e.memory.buffer, e.web_usb_audio(), 6 * 256);
}
function cc(number, value) { e.web_midi_in(0, (0x0b | 0xb0 << 8 | number << 16 | value << 24) >>> 0); }
let capture = render();
for (let c = 0; c < 6; c++) {
  const samples = Array.from(capture).filter((_, i) => i % 6 === c);
  assert.ok(Math.max(...samples.map(Math.abs)) > 1000000);
}
for (let selected = 0; selected < 6; selected++) {
  cc(21, 1 << selected);
  capture = render();
  for (let c = 0; c < 6; c++) {
    const nonzero = Array.from(capture).some((v, i) => i % 6 === c && v !== 0);
    assert.equal(nonzero, c === selected, `channel mask ${selected}/${c}`);
  }
}
cc(21, 63); cc(20, 1);
let impulses = 0;
for (let block = 0; block < 50; block++) {
  capture = render();
  for (let i = 0; i < capture.length; i += 6) {
    if (capture[i]) impulses++;
    for (let c = 1; c < 6; c++) assert.equal(capture[i+c], capture[i], 'impulses must align');
  }
}
assert.ok(impulses > 0);
assert.equal(e.web_store_writes(), 0);
console.log('PASS: six capture channels, all channel masks, aligned impulses, silent analog, no settings writes');
