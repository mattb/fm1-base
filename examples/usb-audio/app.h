/* SPDX-License-Identifier: MPL-2.0 */
#pragma once
#include "plat.h"
#define APP_NAME "USBTEST"
/* Six-channel 48 kHz resampling exceeds the shallow ring render budget. */
#define FM1B_UAC_DEEP 1
#define APP_USB_AUDIO
#define APP_HOST_EXPECT
enum { OBJ_PANEL, OBJ_NOBJ };
#define APP_STORE_MAP {0xDC000u}
void app_init(void);
void app_render(int32_t *, uint32_t, uint32_t);
void app_usb_audio(int32_t *, const int32_t *, uint32_t);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *, const char *);
int app_expect(const char *, int *);
