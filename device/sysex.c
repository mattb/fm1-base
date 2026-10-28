/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Charles Vestal
 * Nonblocking USB SysEx, included after usb.c and backup_streaming in unity.c.
 * The main loop is the producer. USB interrupts consume/reset the ring. */
int plat_usb_connected(void) { return usb.config && !usb.suspended; }
int plat_sysex_try_send(const uint8_t *frame, uint32_t n)
{
    uint32_t flags;
    int result = -1;
    if (!frame || n < 2u || n > SXQ * 3u || frame[0] != 0xf0 || frame[n - 1u] != 0xf7)
        return -1;
    flags = plat_lock();
    /* Reserve room for the entire frame before calling the existing packetizer.
     * Holding the lock also prevents a USB reset from clearing a partial frame.
     * With enough room, ota_wire_send cannot reach its wait/timeout path. */
    if (plat_usb_connected() && !backup_streaming() && !sx_busy &&
        SXQ - (so_w - so_r) >= (n + 2u) / 3u)
        result = ota_wire_send(frame, n);
    plat_unlock(flags);
    return result;
}
