# Microdexed with Teensy 4.0 and 4.1

## Microdexed with Teensy 4.0 and PCM5102A and 16x2Character i2c LCD and a 6N137 Midi In
This is variation of the standard branch of the MicroDexed 6-operator-FM-YanahDX7-Synth by [**Codeberg-dcoredump**](https://codeberg.org/dcoredump/MicroDexed). It replaced the Teensy Audio board with an inexpensive PCM5102A module, and used a 6N137 for the Midi input. Changes made to the config.h file are included here, and the firmware hex file that can be uploaded to this Teensy 4.0 synth using the Teensyloader are also included here.

It is currently constructed on a breadboard and the connections between the Teensy 4.0 and the [**SDCard module Front**](images/SDCard.png) and [**Back**](images/SDCardBack.png), the two encoders, the PCM5102A DAC and the LCD are given in [**Connections.txt**](Connections.txt). The changed DAC connections are not described in config.h and had to be traced from a [**Teensy 4.1 to teensy 3.6 adapter board**](images/T41-Adapter-Board.jpg) from [**another repository**](https://codeberg.org/dcoredump/TeensyMIDIAudio/src/branch/master/T4.1-Adapter-Board). The USB Host connection (D- and D+) for a USB Midi Keyboard are taken from the two bottom pads underneath the Teensy 4.0.

<p align="left">
<img src="images/mdt40.jpg" height="180" /> 
<img src="images/Connections.jpg" height="180" /> 
<img src="images/T41-Adapter-Board.jpg" height="180" /> 
</p>

## Microdexed with Teensy 4.1 with 8MB PSRAM and Audioboard Rev D  and 16x2Character i2c LCD and a 6N137 Midi In
This is variation of the development branch of the MicroDexed 6-operator-FM-YanahDX7-Synth by [**Codeberg-dcoredump**](https://codeberg.org/dcoredump/MicroDexed/src/branch/dev).  The full set of libraries and the changes made to the config.h file are included here, and the firmware hex file that can be uploaded to this Teensy 4.1 synth using the Teensyloader are also included here. It is currently constructed on a breadboard.

<p align="left">
<img src="images/MicrodexedDev1.jpg" height="280" /> 
</p>

Note the warning when [**using i2c Backpacks with 5v LCD displays and 3v3 MCU's or Raspberry Pi's**](https://github.com/TobiasVanDyk/Microdexed-Synth-Variations/tree/main/i2cbackpack)
