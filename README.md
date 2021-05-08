# sa2-midi
Casio SA-2 midi &amp; curcuit bending board

This is a connection schematic and stm32f103 blue pill board firmware for a casio SA-2 (works with SA-21, SA-38 and other OKI M6387-11 casio keyboards as well) MIDI and circuit bending mod.

License: BSD 3-Clause License according to STM files
This code uses CMSIS Library; it is provided along with the STM32CubeIDE project file for the ease of compilation.

### Folders and files description
- code/stm32-sa2 - CubeIDE project for the blue pill firmware
- hex - firmware hex files
- schematic/blue pill.pdf - connection schematic for the blue pill

### Supported MIDI features and MIDI bindings

See code/stm32-sa2/Src/midi_sa2_handler.c for midi bindings.

- All keys from F3 to C6
- 4-note polyphony enable/disable via CC16 (<64 - disabled, >= 64 - enabled (default))
- Drum triggers from C3 to E3
- CC123 Midi panic
- Pitchbend
- Pitchbend ranges: Press E2-F#2 for pitchbend ranges: full tone, octave, full range respectively
- Modulation (CC1 - amount, CC3 - rate)
- LFO (0.1-12.8Hz), audiorate (12.8-1638.4Hz) and high-frequency (102.4-13107.2Hz) modulation rate ranges selected by CC17 (<42 - LFO, >84 - HF, From 42 to 84 - audiorate)
- Modulation sync via B2 key
- Octave selection: C2-D#2, D2 is the standard octave (21.7MHz clock)
- Select (C0) and Stop (C#0) buttons
- Tempo up/down (D#0/D0), Volume up/down (F0/E0)
- G0-B0: demos
- C1-A1: instrument selection digits
- G2 and G#2: glitch triggers (makes the clock noisy for a short period)
- A#2: Casio CPU (OKI M6387-11) reset

### Key matrix map
See code/stm32-sa2/Src/sa2.c

### TODO

- [ ] Add video and article links when they will be released
- [X] Add HEX files
- [ ] Add SA-5, SA-35 and similar keyboards support
- [ ] Make a custom midi-mod PCB
- [ ] Make a standalone custom PCB with a socket for OKI M6387-11
