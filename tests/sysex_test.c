/* SPDX-License-Identifier: MPL-2.0 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define SXQ 4u
static struct { int config, suspended; } usb;
static uint32_t so_w, so_r, sx_busy, queue[SXQ];
static int backup, locked, calls, disconnect_on_lock;
static int backup_streaming(void) { return backup; }
static uint32_t plat_lock(void) {
    assert(!locked); locked = 1;
    if (disconnect_on_lock) usb.config = 0;
    return 0x1234;
}
static void plat_unlock(uint32_t flags) { assert(locked && flags == 0x1234); locked = 0; }
/* Stand-in for the unchanged BSP packetizer. Assert the production caller has
 * made waiting impossible before entering it, and simulate advancing the ring. */
static int ota_wire_send(const uint8_t *p, uint32_t n) {
    assert(locked && usb.config && !usb.suspended && !backup && !sx_busy);
    assert(SXQ - (so_w - so_r) >= (n + 2u) / 3u);
    calls++;
    for (uint32_t i = 0; i < (n + 2u) / 3u; i++) queue[so_w++ % SXQ] = p[0];
    return 0;
}
#include "../device/sysex.c"
static void rejected(const uint8_t *p, uint32_t n) {
    uint32_t before[SXQ], w = so_w, r = so_r; int sent = calls;
    memcpy(before, queue, sizeof queue);
    assert(plat_sysex_try_send(p,n) == -1);
    assert(!locked && calls == sent && so_w == w && so_r == r);
    assert(!memcmp(before,queue,sizeof queue));
}
int main(void) {
    uint8_t f[13] = {0xf0};
    f[11] = 0xf7;
    rejected(f,12); usb.config = 1;
    usb.suspended = 1; rejected(f,12); usb.suspended = 0;
    backup = 1; rejected(f,12); backup = 0;
    sx_busy = 1; rejected(f,12); sx_busy = 0;
    rejected(0,12); rejected(f,0); rejected(f,1); rejected(f,UINT32_MAX);
    rejected(f,13); f[0] = 1; rejected(f,12); f[0] = 0xf0;
    f[11] = 0; rejected(f,12); f[11] = 0xf7;
    disconnect_on_lock = 1; rejected(f,12); disconnect_on_lock = 0; usb.config = 1;
    assert(plat_sysex_try_send(f,12) == 0 && so_w == 4 && calls == 1 && !locked);
    rejected(f,12); so_r++; rejected(f,12); so_r = so_w;
    /* Ring counters wrapping is valid unsigned arithmetic. */
    so_w = so_r = UINT32_MAX - 1u;
    assert(plat_sysex_try_send(f,12) == 0 && so_w == 2);
    so_r = so_w;
    for (uint32_t n = 2; n <= 12; n++) {
        memset(f, 0, sizeof f); f[0] = 0xf0; f[n-1] = 0xf7;
        so_r = so_w;
        uint32_t before = so_w;
        assert(plat_sysex_try_send(f,n) == 0 && so_w - before == (n+2u)/3u);
        so_r = so_w - (SXQ - (n+2u)/3u + 1u);
        rejected(f,n); /* one packet short, including all remainder lengths */
    }
    assert(plat_usb_connected()); usb.suspended = 1; assert(!plat_usb_connected());
    puts("PASS SysEx: whole-frame capacity, rejection unchanged, disconnect at lock, counter wrap, balanced locks");
}
