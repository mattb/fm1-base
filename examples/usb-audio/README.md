# USB audio diagnostic

This app sends six named USB capture channels at 44.1 kHz or 48 kHz. The engine
runs at 44.1 kHz; the BSP resamples when the USB host selects 48 kHz. Analog output
is always zero and USB playback is disabled. The app never writes saved objects.

Default signals are 331, 487, 683, 947, 1289 and 1747 Hz sine waves, each peaking
at -18 dBFS. MIDI CC20 value 0 selects these tones; value 1 selects an identical
10 Hz impulse train on all channels. CC21 is an absolute channel mask, 0 through
63, default 63. These CCs work on any MIDI channel. Mode changes reset the impulse
counter; channel masks do not reset oscillator phase.

Build with `python tools/build.py examples/usb-audio` (or `--host` / `--web`).
After the web build, `node examples/usb-audio/smoke.cjs` checks all channel masks,
impulse alignment, silent analog output, and absence of settings writes. The
WASM capture export is the last engine block at 44.1 kHz; USB descriptors,
packetization and rate negotiation require the [hardware capture tests](tests/).

An app using this interface defines `APP_USB_AUDIO` and fills the complete buffer
in `app_usb_audio(capture, stereo, n)`. The input stereo buffer is the physical mix;
stems can be cached during `app_render`. This example generates independent test
signals in the callback to keep analog output silent. See `include/plat.h` for the
callback contract and `fm1.toml` here for channel names and sample-rate options.

CC22 requests a diagnostic counter snapshot; its value is echoed in the reply.
The main loop sends `F0 7D 55 41 01 token`, then 17 unsigned 32-bit counters,
then `F7`. Each counter occupies five least-significant-first 7-bit bytes.
The fields are: rate, starts, packets, frames, underruns, overruns, missed,
stalls, adjust-up, adjust-down, fill-low, fill-high, fill-current, CPU-percent,
audio-xruns, rate-sets, alternate-setting. Read before and during/after capture;
compare deltas without resetting counters. Packet/frame counters reset on rate
selection, and fill extrema reset when streaming starts. The simulated host has
zero BSP counters. Replies retry without blocking when USB MIDI is busy.

Six-channel 48 kHz resampling uses the existing `FM1B_UAC_DEEP=1` option
to keep a full render in reserve. The default shallow ring suits shorter renders;
its small reserve can run dry even when the audio interrupt meets its deadline.
The diagnostic opts in explicitly, leaving existing firmware defaults unchanged.

CC20 mode 2 outputs constant PCM16 -1 on enabled channels, exercising USB
packets rich in one bits. Analog output stays zero. Use it to check transport
counters and host stream stability, then restore tone mode 0.
