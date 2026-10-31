# USB audio diagnostic

This app sends six named USB capture channels at 44.1 kHz. Analog output is
always zero and USB playback is disabled. The app never writes saved objects.

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
