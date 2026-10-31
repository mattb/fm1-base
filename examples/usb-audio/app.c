/* SPDX-License-Identifier: MPL-2.0 */
#include "fastmath.h"
static uint32_t test_mode, test_mask = 63u, test_frame;
static uint32_t test_phase[6];
static volatile uint32_t test_query; /* CC value + 1; zero means no request */
static const uint32_t test_hz[6] = {331, 487, 683, 947, 1289, 1747};
void app_init(void) { plat_usb_audio_full(1); }
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    uint32_t pkt, i;
    while (plat_midi_in(&pkt, 0)) {
        if ((pkt >> 8 & 0xf0u) != 0xb0u) continue;
        uint32_t cc = pkt >> 16 & 127u, v = pkt >> 24 & 127u;
        if (cc == 20u && v <= 2u) { test_mode = v; test_frame = 0; }
        if (cc == 21u && v <= 63u) test_mask = v;
        if (cc == 22u) test_query = v + 1u;
    }
    for (i = 0; i < 2u * n; i++) lr[i] = 0; /* Analog is always silent. */
}
void app_usb_audio(int32_t *capture, const int32_t *stereo, uint32_t n)
{
    uint32_t i, c;
    for (i = 0; i < n; i++) {
        for (c = 0; c < 6; c++) {
            float v = test_mode ? (test_frame % 4410u == 0u ? 1.0f : 0.0f)
                               : fm_sinf(FM_TWO_PI * (float)test_phase[c] / 44100.0f);
            capture[i * 6u + c] = !(test_mask & (1u << c)) ? 0 :
                test_mode == 2u ? -256 : (int32_t)(1056063.0f * v);
            test_phase[c] = (test_phase[c] + test_hz[c]) % 44100u;
        }
        test_frame++;
    }
}
void app_panic(void) { test_mask = 0; }
void app_ui_init(void) { lcd_fill(0, 0, 240, 240, 0); cv_begin(240, 80, 0); cv_text(8, 16, &FONT_M, "USB AUDIO TEST", 0xffff); cv_text(8, 48, &FONT_XS, "ANALOG MUTED", 0xffff); cv_blit(0, 0); }
/* Diagnostic-only access to the BSP counters in this unity build. No public API. */
static void test_counters(void)
{
    uint32_t values[17] = {0}, i, j, request, flags;
    uint8_t message[92] = {0xf0, 0x7d, 'U', 'A', 1};
    if (!test_query) return;
    flags = plat_lock();
    request = test_query;
#ifndef PLAT_HOST
    values[0] = uac.r48 ? 48000u : 44100u;
    values[1] = uac.starts; values[2] = uac.pkts; values[3] = uac.frames;
    values[4] = uac.underruns; values[5] = uac.overruns;
    values[6] = uac.missed; values[7] = uac.stalls;
    values[8] = uac.adj_up; values[9] = uac.adj_down;
    values[10] = uac.fill_lo; values[11] = uac.fill_hi;
    values[12] = uac.fill_min; values[15] = uac.rate_sets; values[16] = uac.alt;
#endif
    values[13] = plat_cpu_pct(); values[14] = plat_xruns();
    plat_unlock(flags);
    message[5] = (uint8_t)(request - 1u);
    for (i = 0; i < 17u; i++)
        for (j = 0; j < 5u; j++) message[6u + 5u*i + j] = (uint8_t)(values[i] >> (7u*j) & 127u);
    message[91] = 0xf7;
    if (plat_sysex_try_send(message, sizeof message) == 0) {
        flags = plat_lock();
        if (test_query == request) test_query = 0;
        plat_unlock(flags);
    }
}
void app_frame(void) { test_counters(); }
void app_input_only(void) { test_counters(); }
void app_say(const char *a, const char *b) { }

#ifdef PLAT_HOST
int app_expect(const char *what, int *got)
{
    if (!strcmp(what, "mode")) *got = (int)test_mode;
    else if (!strcmp(what, "mask")) *got = (int)test_mask;
    else return 0;
    return 1;
}
#endif
