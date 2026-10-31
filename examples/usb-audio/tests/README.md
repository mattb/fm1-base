# Hardware capture checks

These tools exercise the [muted USB diagnostic](../). Use its firmware for the
tone and impulse tests: it keeps analog output zero at every MASTER position.
`diagnostic-midi.py` sends its mode/mask CCs; `diagnostic-counters.py` requests a
counter snapshot. Both check the USB product before sending MIDI. They neither
save settings nor change MASTER. The other Python tools only read the device or
record audio. `coreaudio-rate` explicitly changes the selected device's rate.

The capture checks require macOS: CoreAudio's physical format and a USB endpoint
GET_CUR read distinguish a real rate change from host resampling. The WAV analyzer
runs anywhere Python and NumPy are available. Swift tools require Apple's command
line developer tools. From this directory:

```sh
brew install libusb portaudio
python3 -m venv .venv
.venv/bin/pip install numpy sounddevice mido python-rtmidi pyusb
swiftc coreaudio-inspect.swift -o coreaudio-inspect
swiftc coreaudio-rate.swift -o coreaudio-rate
mkdir -p out
```

Find the exact audio name with `./coreaudio-inspect`, and MIDI names with
`.venv/bin/python -c 'import mido; print(mido.get_input_names()); print(mido.get_output_names())'`.
macOS may retain an older device name after flashing. Use the exact diagnostic
MIDI port in `--port`; the scripts also verify the current USB product. Replace
`AUDIO_NAME` and `MIDI_PORT` below with those names.

```sh
./coreaudio-rate 'AUDIO_NAME' 44100
.venv/bin/python diagnostic-midi.py --port 'MIDI_PORT' --mode 0 --mask 63
.venv/bin/python diagnostic-counters.py --port 'MIDI_PORT' --output out/before.json
.venv/bin/python capture.py --device 'AUDIO_NAME' --rate 44100 --seconds 10 \
  --expected-names 'Tone 331,Tone 487,Tone 683,Tone 947,Tone 1289,Tone 1747' \
  --usb-product 'FM1 USB Audio Test' --counter-port 'MIDI_PORT' --output out/tones-44100
.venv/bin/python diagnostic-counters.py --port 'MIDI_PORT' --output out/after.json
.venv/bin/python analyze.py out/tones-44100.wav --expected-peak-dbfs -18
```

Repeat at 48000 Hz, explicitly setting that device rate first. A successful client
stream request alone does not establish the physical rate. `capture.py` fails if
physical format differs, and `--usb-product` additionally requires GET_CUR to
match during capture. The USB probe never claims interfaces or detaches drivers;
if the OS denies access, record that limitation rather than infer a device rate.

For isolation, repeat tone capture with masks 1, 2, 4, 8, 16 and 32, passing the
same `--mask` to `analyze.py`. For alignment, set mode 1 and mask 63, capture, then
run `analyze.py CAPTURE.wav --mode alignment`. Identical impulses must agree within
two PCM16 counts across all channels. This measures channel alignment, not USB or
analog latency. Longer captures (60 seconds or more) exercise continuity; retain
both the PCM analysis and capture callback/timing reports.

`--counter-port` also records start/end snapshots while the capture stream is open.
Compare before/after counters for underruns, overruns, missed packets, stalls and
audio xruns. CPU-percent is a snapshot of recent render load. Packet/frame counters
reset on rate selection and fill extrema reset on stream start, so compare like
stream states. A clean sample count alone cannot establish continuity. Save the
firmware hash, descriptor/format reports, WAV files, counters and analysis together.
Reopening capture tests stream recovery; cable unplug/replug is a separate test.

The checks do not measure analog silence: that comes from the diagnostic's source
contract. Restore the intended application after testing; these scripts do not
flash firmware or restore application state.
