/* SPDX-License-Identifier: MPL-2.0 */
#include "fastmath.h"
static uint32_t test_mode, test_mask = 63u, test_frame;
static uint32_t test_phase[6];
static const uint32_t test_hz[6] = {331, 487, 683, 947, 1289, 1747};
void app_init(void) { plat_usb_audio_full(1); }
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    uint32_t pkt, i;
    while (plat_midi_in(&pkt, 0)) {
        if ((pkt >> 8 & 0xf0u) != 0xb0u) continue;
        uint32_t cc = pkt >> 16 & 127u, v = pkt >> 24 & 127u;
        if (cc == 20u && v <= 1u) { test_mode = v; test_frame = 0; }
        if (cc == 21u && v <= 63u) test_mask = v;
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
            capture[i * 6u + c] = (test_mask & (1u << c)) ? (int32_t)(1056063.0f * v) : 0;
            test_phase[c] = (test_phase[c] + test_hz[c]) % 44100u;
        }
        test_frame++;
    }
}
void app_panic(void) { test_mask = 0; }
void app_ui_init(void) { lcd_fill(0, 0, 240, 240, 0); cv_begin(240, 80, 0); cv_text(8, 16, &FONT_M, "USB AUDIO TEST", 0xffff); cv_text(8, 48, &FONT_XS, "ANALOG MUTED", 0xffff); cv_blit(0, 0); }
void app_frame(void) { }
void app_input_only(void) { }
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
