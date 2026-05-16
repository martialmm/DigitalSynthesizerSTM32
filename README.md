# DIGITAL POLYPHONIC SYNTHESIZER

This project aims to develop a basic 8-voices polyphonic digital synthesizer. It features 2 oscillators, a filter section, effects, and standard synthesizer controls.

The device is designed as a desktop module (no built-in keyboard) and is controlled via an external USB MIDI keyboard.

## Technical Stack
- Core Architecture: Bare-metal C development, ARM Cortex-M4 (STM32).
- Audio & DSP:  wave generation, digital filters, audio effects algorithms.
- Peripherals & Hardware: DMA, Timers, ADC, DAC.
- Protocols: I2S (Audio), I2C, SPI, MIDI.
- Drivers: Custom drivers for External Audio DAC & LCD/RGB Displays.

## Development Status
- Current Phase: Breadboard prototyping and functional validation.
- Hardware: Initial development with STM32F407 Discovery Board and now updated with a STM32F401.
- Future Goal: Custom PCB design for a standalone unit.

## Features

### Oscillators (x2)
- Waveforms: Sine, Triangle, Sawtooth, Square (with ON/OFF toggle). For now, only one waveform can be selected at a time.
- Octave selection: controlled via potentiometer.
- Detuning: only for one oscillator.

### Mixer
- Dedicated volume gain for each oscillator.
- Noise generator.
- Master volume output.

### Filter and Enveloppe
#### Low-pass filter
- Cuttoff frequency.
- Resonance.
- Enveloppe amount.

#### Enveloppe ADSR
- Attack.
- Decay.
- Sustain.
- Release.

### FX
#### Reverb
- ON/OFF toggle.
- High-Pass.
- Decay time.
- Dry/Wet mix.

#### Delay
- ON/OFF toggle.
- Rate control.
- Mono/Stereo switch.
- Dry/Wet mix.

### Display
- TBD

### Connection
#### Input
MIDI In via DIN 5 pins (Option with Midi USB, TBD).

#### Output
- Stereo headphone output via mini jack 3.5mm.
- Line output via 3.5mm or 6.3mm (TBD).
