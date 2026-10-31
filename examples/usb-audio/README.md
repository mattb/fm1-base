# USB audio diagnostic

This app exposes six named channels in each USB direction at 44.1 kHz. Analog
output is always zero, including during USB playback. The app never writes saved objects.

Default signals are 331, 487, 683, 947, 1289 and 1747 Hz sine waves, each peaking
at -18 dBFS. MIDI CC20 value 0 selects these tones; value 1 selects an identical
10 Hz impulse train on all channels; value 2 loops each USB playback channel
back to the corresponding capture channel. Playback channels are named Input 1
through Input 6. CC21 is an absolute channel mask, 0 through
63, default 63. These CCs work on any MIDI channel. Mode changes reset the impulse
counter; channel masks do not reset oscillator phase.

Build with `python tools/build.py examples/usb-audio` (or `--host` / `--web`).
After the web build, `node examples/usb-audio/smoke.cjs` checks all channel masks,
impulse alignment, all six playback-to-capture routes, silent analog output, and
absence of settings writes. The
WASM capture export is the last engine block at 44.1 kHz; USB descriptors,
packetization and rate negotiation require recordings from a physical FM1.

An app using this interface defines `APP_USB_CAPTURE` and fills the complete buffer
in `app_usb_capture(capture, stereo, n)`. The input stereo buffer is the physical mix;
stems can be cached during `app_render`. This example generates independent test
signals in the callback to keep analog output silent. See `include/plat.h` for the
callback contracts and `fm1.toml` here for channel names. `APP_USB_PLAYBACK`
receives each input block before rendering; this diagnostic copies it for loopback
and keeps its physical stereo render zero.
