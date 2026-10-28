/* SPDX-License-Identifier: MPL-2.0 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../host/sysex.c"
int main(int argc, char **argv) {
    uint8_t frame[SXQ * 3u + 1u] = {0xf0};
    frame[SXQ * 3u - 1u] = 0xf7;
    assert(plat_usb_connected());
    assert(plat_sysex_try_send(NULL, 2) == -1);
    assert(plat_sysex_try_send(frame, 1) == -1);
    assert(plat_sysex_try_send(frame, sizeof frame) == -1);
    assert(plat_sysex_try_send(frame, UINT32_MAX) == -1);
    if (argc > 1 && !strcmp(argv[1], "fail")) {
        assert(plat_sysex_try_send(frame,SXQ * 3u) == -1);
        assert(plat_sysex_try_send(frame,SXQ * 3u) == -1);
    } else {
        assert(plat_sysex_try_send(frame,SXQ * 3u) == 0);
#ifndef PLAT_WEB
        const char *path = getenv("FM1B_SYSEX_LOG");
        if (path && *path) {
            FILE *f = fopen(path, "rb"); uint8_t got[sizeof frame];
            assert(f && fread(got,1,sizeof got,f) == SXQ * 3u);
            assert(!memcmp(got,frame,SXQ * 3u)); fclose(f);
        }
#endif
    }
    puts("PASS host SysEx limits/capture");
}
