// Exercise the actual browser-source script without needing an OBS installation.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

async function main() {
    const source = fs.readFileSync(path.join(__dirname, '../src/sources/streamoutput.cpp'), 'utf8');
    const script = source.match(/<script>([\s\S]*?)<\/script>/)[1];
    const image = {style: {}, removeAttribute(name) { delete this[name]; }};
    const timers = [], watchdogs = [];
    let now = 0;
    let fetchState = async () => ({ok: true, json: async () => ({revision: '1'})});
    const context = vm.createContext({
        document: {querySelector: () => image},
        Date: {now: () => now},
        AbortSignal: {timeout: milliseconds => ({milliseconds})},
        fetch: async (url, options) => {
            assert.equal(url, '/stream/state');
            assert.ok(options.signal.milliseconds <= 1500);
            return fetchState();
        },
        setTimeout: callback => timers.push(callback),
        setInterval: callback => watchdogs.push(callback)
    });
    vm.runInContext(script, context);
    await new Promise(resolve => setImmediate(resolve));
    assert.equal(image.src, '/stream/image?v=1');
    assert.equal(image.style.visibility, 'visible');

    // A connection failure clears the last frame rather than leaving it frozen.
    fetchState = async () => { throw new Error('Server exited'); };
    await timers.shift()();
    assert.equal(image.src, undefined);
    assert.equal(image.style.visibility, 'hidden');

    // Restarting the server, even with the same revision, loads a fresh image.
    fetchState = async () => ({ok: true, json: async () => ({revision: '1'})});
    await timers.shift()();
    assert.equal(image.src, '/stream/image?v=1');
    assert.equal(image.style.visibility, 'visible');

    // A hung request must also blank the source promptly via the watchdog.
    let release;
    fetchState = () => new Promise(resolve => { release = resolve; });
    const request = timers.shift()();
    now = 1800;
    watchdogs[0]();
    assert.equal(image.src, undefined);
    assert.equal(image.style.visibility, 'hidden');
    release({ok: true, json: async () => ({revision: '2'})});
    await request;
    assert.equal(image.src, '/stream/image?v=2');
    assert.equal(image.style.visibility, 'visible');
    console.log('Network viewer clears on exit or timeout and recovers on restart: OK');
}

main().catch(error => { console.error(error); process.exitCode = 1; });
