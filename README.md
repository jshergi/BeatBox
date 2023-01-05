# BeatBox
An application using multi-threading and real time programming in C which plays a drum beat, cross compliled to a BeagleBone Green

# Hardware Configuration

Solder Joystick and follow this configuration to BBG

P9.32 (VDD_ADC) to +V to the joystick

P9.34 (GNDA_ADC) to -V to the joystick

P9.37 (AIN2) to X output on the joystick

P9.38 (AIN3) to Y output on the joysticks

Place joystick with breakout board’s text (“Analog Mini Thumbsick”) to the left, and its
“+ Y X -” pins at the top and bottom.

Solder 8x8 LED Matrix and follow: 

P9.01 (Ground) to “-” pin on the LED Matrix

P9.03 (3.3V) to “+” pin on the LED Matrix

P9.17 (I2C1-SCL) to “C” pin on the LED Matrix

P9.18 (I2C1-SDA) to “D” pin on the LED Matrix

Orient display with pins at the bottom, and display pointing out the top of the
breadboard. (This is 180 degree rotation: text on the “backpack” board upside down).

Configuration for GPIO buttons

All buttons:
Insert button across the trench on the breadboard.

P9.03 (3.3V) Connect left pin to 3.3V

P9.01 (Ground) Connect right pin through a 1K ohm resistor to ground

Connect the GPIO pin to read the button onto the right pin of button
(Circuit: 3.3V ---- Button ---- [GPIO probe point] ---- Resistor ---- GND

P8.15 (GPIO) Mode (gray) button’s GPIO

P8.16 (GPIO) Base drum (red) button’s GPIO

P8.17 (GPIO) Snare drum (yellow) button’s GPIO

P8.18 (GPIO) Hihat (green) button’s GPIO

# Implementation Details

Generate audio in real-time from a C program using the ALSA API1, and play that
audio through the USB Audio Adapter’s headphone output

2 threads:
1. A low-level audio mixing thread which provides raw PCM data to the ALSA
functions. 
2. Higher-level beat-generation thread which generates a rock beat and tells
lower-level audio playback module (thread) to play rock beat sounds as needed.

1. No drum beat (i.e., beat turned off)
2. Standard rock drum beat
3. Some other drum beat 



Control the beat's tempo (in beats-per-minute) in range [40, 300] BPM (inclusive); default
120 BPM. (using Joystick)

Conrol the volume [0, 100] (using joystick)

display mode (rock, custom, no beat) (using LED matrix)

GPIO buttons (play hi-hat, snare, base)

At times, multiple sounds will need to be played simultaneously. The program then adds
together PCM wave values to generate the sound

