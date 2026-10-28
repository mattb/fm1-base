/* SPDX-License-Identifier: MPL-2.0
 * Host stand-in: connected, immediately drained; optional native raw capture.
 * Included after app.h so an app's SXQ matches its device build. */
#ifndef SXQ
#define SXQ 128u
#endif
int plat_usb_connected(void) { return 1; }
int plat_sysex_try_send(const uint8_t *frame, uint32_t n)
{
    if (!frame || n < 2u || n > SXQ * 3u || frame[0] != 0xf0 || frame[n - 1u] != 0xf7)
        return -1;
#ifndef PLAT_WEB
    static FILE *log;
    static int opened, failed;
    if (!opened) {
        const char *path = getenv("FM1B_SYSEX_LOG");
        opened = 1;
        if (path && *path) {
            log = fopen(path, "wb");
            if (!log) failed = 1;
        }
    }
    if (failed) return -1;
    if (log && (fwrite(frame, 1, n, log) != n || fflush(log) != 0)) {
        failed = 1;
        return -1;
    }
#endif
    return 0;
}
