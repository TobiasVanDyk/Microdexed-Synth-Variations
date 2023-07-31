/*
   MicroDexed

   MicroDexed is a port of the Dexed sound engine
   (https://github.com/asb2m10/dexed) for the Teensy-3.5/3.6/4.x with audio shield.
   Dexed ist heavily based on https://github.com/google/music-synthesizer-for-android

   (c)2018-2023 H. Wirtz <wirtz@parasitstudio.de>
   (c)2021-2022 H. Wirtz <wirtz@parasitstudio.de>, M. Koslowski <positionhigh@gmx.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this programf; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/

#pragma once

#ifndef ARDUINO_TEENSY41
#error This version 1.2.7 the code does only work for Teensy-4.1 with enabled FX. It was to difficult to support all other flavors of the Teensy universe in one code base.
#endif

#include <Arduino.h>
#include "midinotes.h"

// If you want to test the system with Linux and without any keyboard and/or audio equipment, you can do the following:
// 1. In Arduino-IDE enable "Tools->USB-Type->Serial + MIDI + Audio"
// 2. Build the firmware with "MIDI_DEVICE_USB" enabled in config.h.
// 3. Afterconnecting to a Linux system there should be a MIDI an audio device available that is called "MicroMDexed", so you can start the following:
// $ aplaymidi -p 20:0 <MIDI-File> # e.g. test.mid
// $ vkeybd --addr 20:0
// $ arecord -f cd -Dhw:1,0 /tmp/<AUDIO_FILE_NAME>.wav
//
// Tools for testing MIDI:  https://github.com/gbevin/SendMIDI
//                          https://github.com/gbevin/ReceiveMIDI
//
// e.g.:
// * sendmidi dev "MicroDexed MIDI" on 80 127 && sleep 1.0 && sendmidi dev "MicroDexed MIDI" off 80 0
// * sendmidi dev "MicroDexed MIDI" syf addon/SD/90/RitCh1.syx
// * amidi -p hw:2,0,0 -s addon/SD/90/RitCh1.syx
//
// Receiving and storing MIDI SYSEX with Linux:
// amidi -p hw:2,0,0 -d -r /tmp/bkup1.syx
//
// For SYSEX Bank upload via USB:
// sed -i.orig 's/SYSEX_MAX_LEN = 290/SYSEX_MAX_LEN = 4104/' /usr/local/arduino-teensy/hardware/teensy/avr/libraries/USBHost_t36/USBHost_t36.h
// sed -i.orig 's/^#define USB_MIDI_SYSEX_MAX 290/#define USB_MIDI_SYSEX_MAX 4104/' /usr/local/arduino-teensy/hardware/teensy/avr/cores/teensy3/usb_midi.h
// sed -i.orig 's/^#define USB_MIDI_SYSEX_MAX 290/#define USB_MIDI_SYSEX_MAX 4104/' /usr/local/arduino-teensy/hardware/teensy/avr/cores/teensy4/usb_midi.h
//#define USB_MIDI_SYSEX_MAX 4104
//
// Information about memory layout, etc.: https://www.pjrc.com/store/teensy41.html

#define VERSION "1.2.7"

//*************************************************************************************************
//* DEVICE SETTINGS
//*************************************************************************************************

//*************************************************************************************************
//* MIDI HARDWARE SETTINGS
//*************************************************************************************************
#define MIDI_DEVICE_DIN Serial1
#define MIDI_DEVICE_USB 1
#define MIDI_DEVICE_USB_HOST 1

//*************************************************************************************************
//* AUDIO HARDWARE SETTINGS
//*************************************************************************************************
// If nothing is defined Teensy internal DAC is used as audio output device!
// Left and right channel audio signal is presented on pins A21 and A22.
#define AUDIO_DEVICE_USB
#define TEENSY_AUDIO_BOARD
//#define PT8211_AUDIO
//#define TGA_AUDIO_BOARD
//#define TEENSY_DAC
//#define TEENSY_DAC_SYMMETRIC
//#define I2S_AUDIO_ONLY

//*************************************************************************************************
//* MIDI SOFTWARE SETTINGS
//*************************************************************************************************
#define DEFAULT_MIDI_CHANNEL MIDI_CHANNEL_OMNI
#define SYSEXBANK_DEFAULT 0
#define SYSEXSOUND_DEFAULT 0

//*************************************************************************************************
//* DEBUG OUTPUT SETTINGS
//*************************************************************************************************
#define DEBUG 1
//#define DEBUG_SHOW_JSON 1
#define SERIAL_SPEED 230400
#define SHOW_XRUN 1
#define SHOW_CPU_LOAD_MSEC 5000

//*************************************************************************************************
//* DEXED, EPIANO AND EFFECTS SETTINGS
//*************************************************************************************************
// Number of Dexed instances
#define NUM_DEXED 2  // 1 or 2 - nothing else!

// NUMBER OF PARALLEL SAMPLEDRUMS
#define NUM_DRUMS 8

// NUMBER OF SAMPLES IN DRUMSET
#define NUM_DRUMSET_CONFIG 64

// EPIANO
#define NUM_EPIANO_VOICES 16
#define DEFAULT_EP_MIDI_CHANNEL 3

// CHORUS parameters
#define MOD_DELAY_SAMPLE_BUFFER int32_t(TIME_MS2SAMPLES(15.0))  // 15.0 ms delay buffer.
#define MOD_WAVEFORM WAVEFORM_TRIANGLE                          // WAVEFORM_SINE WAVEFORM_TRIANGLE WAVEFORM_SAWTOOTH WAVEFORM_SAWTOOTH_REVERSE
#define MOD_FILTER_OUTPUT MOD_BUTTERWORTH_FILTER_OUTPUT         // MOD_LINKWITZ_RILEY_FILTER_OUTPUT MOD_BUTTERWORTH_FILTER_OUTPUT MOD_NO_FILTER_OUTPUT
#define MOD_FILTER_CUTOFF_HZ 2000

// SGTL5000
#ifdef TEENSY_AUDIO_BOARD
#define SGTL5000_AUDIO_ENHANCE 1
//#define SGTL5000_AUDIO_THRU 1
#define SGTL5000_HEADPHONE_VOLUME 0.8
#endif

// DELAYTIME
#define USE_DELAY_8M 1
#if defined(USE_DELAY_8M)
#define DELAY_MAX_TIME 9990
#else
#define DELAY_MAX_TIME 500
#endif

//*************************************************************************************************
//* AUDIO SOFTWARE SETTINGS
//*************************************************************************************************
#define SAMPLE_RATE 44100

#ifdef USE_DELAY_8M
#define AUDIO_MEM 36 + 14  // Delay in EXTMEM
#else
#define AUDIO_MEM 36 + 14 + SAMPLE_RATE* NUM_DEXED* DELAY_MAX_TIME / 128000  // Delay in AUDIO_MEM
#endif

#ifdef TEENSY_AUDIO_BOARD
/*
  13: 3.16 Volts p-p
  14: 2.98 Volts p-p
  15: 2.83 Volts p-p
  16: 2.67 Volts p-p
  17: 2.53 Volts p-p
  18: 2.39 Volts p-p
  19: 2.26 Volts p-p
  20: 2.14 Volts p-p
  21: 2.02 Volts p-p
  22: 1.91 Volts p-p
  23: 1.80 Volts p-p
  24: 1.71 Volts p-p
  25: 1.62 Volts p-p
  26: 1.53 Volts p-p
  27: 1.44 Volts p-p
  28: 1.37 Volts p-p
  29: 1.29 Volts p-p  (default)
  30: 1.22 Volts p-p
  31: 1.16 Volts p-p
*/
#define SGTL5000_LINEOUT_LEVEL 29
#endif

//*************************************************************************************************
//* UI
//*************************************************************************************************
#define STANDARD_LCD_I2C
//#define OLED_SPI

// LCD Display
//I2C_DISPLAY only
#ifdef STANDARD_LCD_I2C
#define LCD_I2C_ADDRESS 0x27
//#define LCD_I2C_ADDRESS 0x3f
//Display size, must be set for U8X8 as well
#define LCD_cols 16
#define LCD_rows 2

#define I2C_DISPLAY
// [I2C] SCL: Pin 19, SDA: Pin 18 (https://www.pjrc.com/teensy/td_libs_Wire.html)
//#define LCD_GFX 1
#endif

#ifdef OLED_SPI
#define LCD_cols 16
#define LCD_rows 4
//enable U8X8 support
#define U8X8_DISPLAY
//enable SPI CS switching
#define DISPLAY_LCD_SPI
#define U8X8_DISPLAY_CLASS U8X8_SSD1322_NHD_256X64_4W_HW_SPI
//#define U8X8_DISPLAY_CLASS U8X8_SSD1306_128X64_NONAME_HW_I2C
#define U8X8_CS_PIN 9
#define U8X8_DC_PIN 15
#define U8X8_RESET_PIN 14
#endif

#define VOICE_SELECTION_MS 60000
#define BACK_FROM_VOLUME_MS 2000
#define MESSAGE_WAIT_TIME 1000
#define MIDI_DECAY_TIMER 50

//*************************************************************************************************
//* HARDWARE SETTINGS
//*************************************************************************************************

// Teensy Audio Shield
#define SDCARD_AUDIO_CS_PIN 10
#define SDCARD_AUDIO_MOSI_PIN 7
#define SDCARD_AUDIO_SCK_PIN 14
#define SDCARD_TEENSY_CS_PIN BUILTIN_SDCARD
#define SDCARD_TEENSY_MOSI_PIN 11
#define SDCARD_TEENSY_SCK_PIN 13

// Encoder with button
//#define ENCODER_USE_INTERRUPTS
#define NUM_ENCODER 2
#define ENC_L_PIN_A 3
#define ENC_L_PIN_B 2
#define BUT_L_PIN 4
#define ENC_R_PIN_A 24
#define ENC_R_PIN_B 5
#define BUT_R_PIN 9
#define BUT_DEBOUNCE_MS 20
#define LONG_BUTTON_PRESS 500

// Internal timer
#define AUTOSTORE_MS 5000

// EEPROM address
#define EEPROM_START_ADDRESS 0xFF

#define MAX_BANKS 100
#define MAX_VOICES 32      // voices per bank
#define BANK_NAME_LEN 11   // 10 (plus '\0')
#define VOICE_NAME_LEN 12  // 11 (plus '\0')
#define FILENAME_LEN BANK_NAME_LEN + VOICE_NAME_LEN
#define CONFIG_FILENAME_LEN 50
#define DRUM_NAME_LEN 9
#define PERFORMANCE_NAME_LEN 11

#define FAV_CONFIG_PATH "FAVCFG"
#define FAV_CONFIG_NAME "FAVCFG"

#define PERFORMANCE_CONFIG_PATH "PERFORMANCE"
#define PERFORMANCE_CONFIG_NAME "performance"
#define DRUMS_CONFIG_NAME "drums"
#define DRUMS_MAPPING_NAME "drmmap"
#define FX_CONFIG_NAME "fx"
#define VOICE_CONFIG_NAME "voice"
#define SYS_CONFIG_NAME "sys"
#define EPIANO_CONFIG_NAME "epiano"

#define MAX_PERF_MOD 30

//*************************************************************************************************
//* DO NO CHANGE ANYTHING BEYOND IF YOU DON'T KNOW WHAT YOU ARE DOING !!!
//*************************************************************************************************
#define MAX_DEXED 2  // No! - even don't think about increasing this number! IT _WILL_ PRODUCE MASSIVE PROBLEMS!
#define CONTROL_RATE_MS 50
#define LED_BLINK_MS 1000
#define SAVE_SYS_MS 5000
#define VOL_MAX_FLOAT 0.95
#define VOLUME_TRANSFORM_EXP 2.5

#define EEPROM_MARKER 0x4243

#ifndef NUM_DRUMS
#define NUM_DRUMS 0
#endif

// MAX_NOTES SETTINGS
#define MAX_NOTES 32
#define MIDI_DECAY_LEVEL_TIME 500

// MIDI
#ifdef MIDI_DEVICE_USB
#define USBCON 1
#endif

// Some optimizations
#define USE_TEENSY_DSP 1

// HELPER MACROS
#define TIME_MS2SAMPLES(x) floor(uint32_t(x) * AUDIO_SAMPLE_RATE / 1000) * sizeof(int16_t)
#define SAMPLES2TIME_MS(x) float(uint32_t(x) * 1000 / AUDIO_SAMPLE_RATE * sizeof(int16_t)
#define XSTR(x) STR(x)
#define STR(x) #x

// Modulated delay options
#define MOD_NO_FILTER_OUTPUT 0
#define MOD_BUTTERWORTH_FILTER_OUTPUT 1
#define MOD_LINKWITZ_RILEY_FILTER_OUTPUT 2

#if defined(TEENSY_DAC_SYMMETRIC)
#define MONO_MIN 1
#define MONO_MAX 1
#define MONO_DEFAULT 1
#else
#define MONO_MIN 0
#define MONO_MAX 3
#define MONO_DEFAULT 0
#endif

#define MIDI_CONTROLLER_MODE_MAX 2

#define VOLUME_MIN 0
#define VOLUME_MAX 100
#define VOLUME_DEFAULT 80
//#define VOLUME_ENC_STEPS 5

#define PANORAMA_MIN 0
#define PANORAMA_MAX 40
#define PANORAMA_DEFAULT 20

#define MIDI_CHANNEL_MIN MIDI_CHANNEL_OMNI
#define MIDI_CHANNEL_MAX 16
#define MIDI_CHANNEL_DEFAULT MIDI_CHANNEL_OMNI

#define INSTANCE_LOWEST_NOTE_MIN 21
#define INSTANCE_LOWEST_NOTE_MAX 108
#define INSTANCE_LOWEST_NOTE_DEFAULT 21

#define INSTANCE_HIGHEST_NOTE_MIN 21
#define INSTANCE_HIGHEST_NOTE_MAX 108
#define INSTANCE_HIGHEST_NOTE_DEFAULT 108

#define CHORUS_FREQUENCY_MIN 0
#define CHORUS_FREQUENCY_MAX 100
#define CHORUS_FREQUENCY_DEFAULT 0

#define CHORUS_WAVEFORM_MIN 0
#define CHORUS_WAVEFORM_MAX 1
#define CHORUS_WAVEFORM_DEFAULT 0

#define CHORUS_DEPTH_MIN 0
#define CHORUS_DEPTH_MAX 100
#define CHORUS_DEPTH_DEFAULT 0

#define CHORUS_LEVEL_MIN 0
#define CHORUS_LEVEL_MAX 100
#define CHORUS_LEVEL_DEFAULT 0

#define DELAY_TIME_MIN 0
#define DELAY_TIME_MAX DELAY_MAX_TIME / 10
#define DELAY_TIME_DEFAULT 0

#define DELAY_FEEDBACK_MIN 0
#define DELAY_FEEDBACK_MAX 100
#define DELAY_FEEDBACK_DEFAULT 0

#define DELAY_LEVEL_MIN 0
#define DELAY_LEVEL_MAX 100
#define DELAY_LEVEL_DEFAULT 0

#define REVERB_ROOMSIZE_MIN 0
#define REVERB_ROOMSIZE_MAX 100
#define REVERB_ROOMSIZE_DEFAULT 0

#define REVERB_DAMPING_MIN 0
#define REVERB_DAMPING_MAX 100
#define REVERB_DAMPING_DEFAULT 0

#define REVERB_LOWPASS_MIN 0
#define REVERB_LOWPASS_MAX 100
#define REVERB_LOWPASS_DEFAULT 0

#define REVERB_LODAMP_MIN 0
#define REVERB_LODAMP_MAX 100
#define REVERB_LODAMP_DEFAULT 0

#define REVERB_HIDAMP_MIN 0
#define REVERB_HIDAMP_MAX 100
#define REVERB_HIDAMP_DEFAULT 0

#define REVERB_DIFFUSION_MIN 0
#define REVERB_DIFFUSION_MAX 100
#define REVERB_DIFFUSION_DEFAULT 0

#define REVERB_LEVEL_MIN 0
#define REVERB_LEVEL_MAX 100
#define REVERB_LEVEL_DEFAULT 0

#define REVERB_SEND_MIN 0
#define REVERB_SEND_MAX 100
#define REVERB_SEND_DEFAULT 0

#define FILTER_CUTOFF_MIN 0
#define FILTER_CUTOFF_MAX 100
#define FILTER_CUTOFF_DEFAULT 0

#define FILTER_RESONANCE_MIN 0
#define FILTER_RESONANCE_MAX 100
#define FILTER_RESONANCE_DEFAULT 0

#define TRANSPOSE_MIN 0
#define TRANSPOSE_MAX 48
#define TRANSPOSE_DEFAULT 12

#define TUNE_MIN 1
#define TUNE_MAX 199
#define TUNE_DEFAULT 100

#define SOUND_INTENSITY_MIN 0
#define SOUND_INTENSITY_MAX 100
#define SOUND_INTENSITY_DEFAULT 100

#define POLYPHONY_MIN 0
#define POLYPHONY_MAX MAX_NOTES
#define POLYPHONY_DEFAULT MAX_NOTES / NUM_DEXED

#define MONOPOLY_MIN 0
#define MONOPOLY_MAX 1
#define MONOPOLY_DEFAULT 0

#define NOTE_REFRESH_MIN 0
#define NOTE_REFRESH_MAX 1
#define NOTE_REFRESH_DEFAULT 0

#define PB_RANGE_MIN 0
#define PB_RANGE_MAX 12
#define PB_RANGE_DEFAULT 1

#define PB_STEP_MIN 0
#define PB_STEP_MAX 12
#define PB_STEP_DEFAULT 0

#define MW_RANGE_MIN 0
#define MW_RANGE_MAX 99
#define MW_RANGE_DEFAULT 50

#define MW_ASSIGN_MIN 0
#define MW_ASSIGN_MAX 7
#define MW_ASSIGN_DEFAULT 0  // Bitmapped: 0: Pitch, 1: Amp, 2: Bias

#define MW_MODE_MIN 0
#define MW_MODE_MAX MIDI_CONTROLLER_MODE_MAX
#define MW_MODE_DEFAULT 0

#define FC_RANGE_MIN 0
#define FC_RANGE_MAX 99
#define FC_RANGE_DEFAULT 50

#define FC_ASSIGN_MIN 0
#define FC_ASSIGN_MAX 7
#define FC_ASSIGN_DEFAULT 0  // Bitmapped: 0: Pitch, 1: Amp, 2: Bias

#define FC_MODE_MIN 0
#define FC_MODE_MAX MIDI_CONTROLLER_MODE_MAX
#define FC_MODE_DEFAULT 0

#define BC_RANGE_MIN 0
#define BC_RANGE_MAX 99
#define BC_RANGE_DEFAULT 50

#define BC_ASSIGN_MIN 0
#define BC_ASSIGN_MAX 7
#define BC_ASSIGN_DEFAULT 0  // Bitmapped: 0: Pitch, 1: Amp, 2: Bias

#define BC_MODE_MIN 0
#define BC_MODE_MAX MIDI_CONTROLLER_MODE_MAX
#define BC_MODE_DEFAULT 0

#define AT_RANGE_MIN 0
#define AT_RANGE_MAX 99
#define AT_RANGE_DEFAULT 50

#define AT_ASSIGN_MIN 0
#define AT_ASSIGN_MAX 7
#define AT_ASSIGN_DEFAULT 0  // Bitmapped: 0: Pitch, 1: Amp, 2: Bias

#define AT_MODE_MIN 0
#define AT_MODE_MAX MIDI_CONTROLLER_MODE_MAX
#define AT_MODE_DEFAULT 0

#define OP_ENABLED_MIN 0
#define OP_ENABLED_MAX 0x3f
#define OP_ENABLED_DEFAULT OP_ENABLED_MAX

#define PORTAMENTO_MODE_MIN 0
#define PORTAMENTO_MODE_MAX 1
#define PORTAMENTO_MODE_DEFAULT 0  // 0: Retain, 1: Follow

#define PORTAMENTO_GLISSANDO_MIN 0
#define PORTAMENTO_GLISSANDO_MAX 1
#define PORTAMENTO_GLISSANDO_DEFAULT 0

#define PORTAMENTO_TIME_MIN 0
#define PORTAMENTO_TIME_MAX 99
#define PORTAMENTO_TIME_DEFAULT 0

#define INSTANCES_MIN 1
#define INSTANCES_MAX NUM_DEXED
#define INSTANCES_DEFAULT 1

#define INSTANCE_NOTE_START_MIN MIDI_AIS0
#define INSTANCE_NOTE_START_MAX MIDI_B7
#define INSTANCE_NOTE_START_DEFAULT INSTANCE_NOTE_START_MIN

#define INSTANCE_NOTE_END_MIN MIDI_AIS0
#define INSTANCE_NOTE_END_MAX MIDI_B7
#define INSTANCE_NOTE_END_DEFAULT INSTANCE_NOTE_END_MAX

#define SOFT_MIDI_THRU_MIN 0
#define SOFT_MIDI_THRU_MAX 1
#define SOFT_MIDI_THRU_DEFAULT 1

#define VELOCITY_LEVEL_MIN 100
#define VELOCITY_LEVEL_MAX 127
#define VELOCITY_LEVEL_DEFAULT 100

#define ENGINE_MIN 0
#define ENGINE_MAX 2
#define ENGINE_DEFAULT 1 // 0=Modern, 1=MkI, 2=OPL

#define PERFORMANCE_NUM_MIN 0
#define PERFORMANCE_NUM_MAX 99
#define PERFORMANCE_NUM_DEFAULT 0

#define DRUMS_MAIN_VOL_MIN 0
#define DRUMS_MAIN_VOL_MAX 100
#define DRUMS_MAIN_VOL_DEFAULT 80

#define DRUMS_MIDI_CHANNEL_MIN MIDI_CHANNEL_OMNI
#define DRUMS_MIDI_CHANNEL_MAX 16
#define DRUMS_MIDI_CHANNEL_DEFAULT 10

#define DRUMS_MIDI_NOTE_MIN 21
#define DRUMS_MIDI_NOTE_MAX 108

#define DRUMS_PANORAMA_MIN -10
#define DRUMS_PANORAMA_MAX 10

#define DRUMS_VOL_MIN 0
#define DRUMS_VOL_MAX 99

#define DRUMS_REVERB_SEND_MIN 0
#define DRUMS_REVERB_SEND_MAX 99

#define DRUMS_PITCH_MIN -120
#define DRUMS_PITCH_MAX 120

#define EQ_1_MIN 15
#define EQ_1_MAX 250
#define EQ_1_DEFAULT 50

#define EQ_2_MIN -99
#define EQ_2_MAX 99
#define EQ_2_DEFAULT 0

#define EQ_3_MIN -99
#define EQ_3_MAX 99
#define EQ_3_DEFAULT 0

#define EQ_4_MIN -99
#define EQ_4_MAX 99
#define EQ_4_DEFAULT 0

#define EQ_5_MIN -99
#define EQ_5_MAX 99
#define EQ_5_DEFAULT 0

#define EQ_6_MIN -99
#define EQ_6_MAX 99
#define EQ_6_DEFAULT 0

#define EQ_7_MIN 8
#define EQ_7_MAX 18
#define EQ_7_DEFAULT 15

#define EP_CHORUS_FREQUENCY_MIN 0
#define EP_CHORUS_FREQUENCY_MAX 100
#define EP_CHORUS_FREQUENCY_DEFAULT 0

#define EP_CHORUS_WAVEFORM_MIN 0
#define EP_CHORUS_WAVEFORM_MAX 1
#define EP_CHORUS_WAVEFORM_DEFAULT 0

#define EP_CHORUS_DEPTH_MIN 0
#define EP_CHORUS_DEPTH_MAX 100
#define EP_CHORUS_DEPTH_DEFAULT 0

#define EP_CHORUS_LEVEL_MIN 0
#define EP_CHORUS_LEVEL_MAX 100
#define EP_CHORUS_LEVEL_DEFAULT 0

#define EP_REVERB_SEND_MIN 0
#define EP_REVERB_SEND_MAX 100
#define EP_REVERB_SEND_DEFAULT 0

#define EP_DECAY_MIN 0
#define EP_DECAY_MAX 100
#define EP_DECAY_DEFAULT 50

#define EP_RELEASE_MIN 0
#define EP_RELEASE_MAX 100
#define EP_RELEASE_DEFAULT 50

#define EP_HARDNESS_MIN 0
#define EP_HARDNESS_MAX 100
#define EP_HARDNESS_DEFAULT 50

#define EP_TREBLE_MIN 0
#define EP_TREBLE_MAX 100
#define EP_TREBLE_DEFAULT 50

#define EP_PAN_TREMOLO_MIN 0
#define EP_PAN_TREMOLO_MAX 100
#define EP_PAN_TREMOLO_DEFAULT 20

#define EP_PAN_LFO_MIN 0
#define EP_PAN_LFO_MAX 100
#define EP_PAN_LFO_DEFAULT 65

#define EP_VELOCITY_SENSE_MIN 0
#define EP_VELOCITY_SENSE_MAX 100
#define EP_VELOCITY_SENSE_DEFAULT 25

#define EP_PANORAMA_MIN 0
#define EP_PANORAMA_MAX 40
#define EP_PANORAMA_DEFAULT 20

#define EP_STEREO_MIN 0
#define EP_STEREO_MAX 100
#define EP_STEREO_DEFAULT 50

#define EP_POLYPHONY_MIN 0
#define EP_POLYPHONY_MAX NUM_EPIANO_VOICES
#define EP_POLYPHONY_DEFAULT NUM_EPIANO_VOICES

#define EP_TUNE_MIN 0
#define EP_TUNE_MAX 100
#define EP_TUNE_DEFAULT 0

#define EP_DETUNE_MIN 0
#define EP_DETUNE_MAX 100
#define EP_DETUNE_DEFAULT 15

#define EP_OVERDRIVE_MIN 0
#define EP_OVERDRIVE_MAX 100
#define EP_OVERDRIVE_DEFAULT 0

#define EP_REVERB_SEND_MIN 0
#define EP_REVERB_SEND_MAX 100
#define EP_REVERB_SEND_DEFAULT 0

#define EP_LOWEST_NOTE_MIN 21
#define EP_LOWEST_NOTE_MAX 108
#define EP_LOWEST_NOTE_DEFAULT EP_LOWEST_NOTE_MIN

#define EP_HIGHEST_NOTE_MIN 21
#define EP_HIGHEST_NOTE_MAX 108
#define EP_HIGHEST_NOTE_DEFAULT EP_HIGHEST_NOTE_MAX

#define EP_TRANSPOSE_MIN 0
#define EP_TRANSPOSE_MAX 48
#define EP_TRANSPOSE_DEFAULT 24

#define EP_SOUND_INTENSITY_MIN 0
#define EP_SOUND_INTENSITY_MAX 100
#define EP_SOUND_INTENSITY_DEFAULT 100

#define EP_MONOPOLY_MIN 0
#define EP_MONOPOLY_MAX 1
#define EP_MONOPOLY_DEFAULT 0

#define EP_MIDI_CHANNEL_MIN MIDI_CHANNEL_OMNI
#define EP_MIDI_CHANNEL_MAX 16
#define EP_MIDI_CHANNEL_DEFAULT DEFAULT_EP_MIDI_CHANNEL

#define FAVORITES_NUM_MIN 0
#define FAVORITES_NUM_MAX 100
#define FAVORITES_NUM_DEFAULT 0

#define STARTUP_NUM_MIN 0
#define STARTUP_NUM_MAX 255
#define STARTUP_NUM_DEFAULT 0

#define VOLUME_MULTIPLIER 1.4

// Buffer-size define for load/save configuration as JSON
#define JSON_BUFFER_SIZE 8192

// Internal configuration structure
typedef struct dexed_s {
  uint8_t bank;
  uint8_t voice;
  uint8_t lowest_note;
  uint8_t highest_note;
  uint8_t transpose;
  uint8_t tune;
  uint8_t sound_intensity;
  uint8_t pan;
  uint8_t polyphony;
  uint8_t velocity_level;
  uint8_t monopoly;
  uint8_t note_refresh;
  uint8_t pb_range;
  uint8_t pb_step;
  uint8_t mw_range;
  uint8_t mw_assign;
  uint8_t mw_mode;
  uint8_t fc_range;
  uint8_t fc_assign;
  uint8_t fc_mode;
  uint8_t bc_range;
  uint8_t bc_assign;
  uint8_t bc_mode;
  uint8_t at_range;
  uint8_t at_assign;
  uint8_t at_mode;
  uint8_t portamento_mode;
  uint8_t portamento_glissando;
  uint8_t portamento_time;
  uint8_t op_enabled;
  uint8_t midi_channel;
  uint8_t engine;
} dexed_t;

typedef struct fx_s {
  uint8_t filter_cutoff[MAX_DEXED];
  uint8_t filter_resonance[MAX_DEXED];
  uint8_t chorus_frequency[MAX_DEXED];
  uint8_t chorus_waveform[MAX_DEXED];
  uint8_t chorus_depth[MAX_DEXED];
  uint8_t chorus_level[MAX_DEXED];
  uint16_t delay_time[MAX_DEXED];
  uint8_t delay_feedback[MAX_DEXED];
  uint8_t delay_level[MAX_DEXED];
  uint8_t reverb_send[MAX_DEXED];
  uint8_t reverb_roomsize;
  uint8_t reverb_damping;
  uint8_t reverb_lowpass;
  uint8_t reverb_lodamp;
  uint8_t reverb_hidamp;
  uint8_t reverb_diffusion;
  uint8_t reverb_level;
  uint8_t eq_1;
  int8_t eq_2;
  int8_t eq_3;
  int8_t eq_4;
  int8_t eq_5;
  int8_t eq_6;
  uint8_t eq_7;
  uint8_t ep_chorus_frequency;
  uint8_t ep_chorus_waveform;
  uint8_t ep_chorus_depth;
  uint8_t ep_chorus_level;
  uint8_t ep_reverb_send;
} fx_t;

typedef struct epiano_s {
  uint8_t decay;
  uint8_t release;
  uint8_t hardness;
  uint8_t treble;
  uint8_t pan_tremolo;
  uint8_t pan_lfo;
  uint8_t velocity_sense;
  uint8_t stereo;
  uint8_t polyphony;
  uint8_t tune;
  uint8_t detune;
  uint8_t overdrive;
  uint8_t lowest_note;
  uint8_t highest_note;
  uint8_t transpose;
  uint8_t sound_intensity;
  uint8_t pan;
  uint8_t midi_channel;
} epiano_t;

typedef struct sys_s {
  uint8_t vol;
  uint8_t mono;
  uint8_t soft_midi_thru;
  uint8_t performance_number;
  uint8_t favorites;
  uint8_t load_at_startup;
} sys_t;

typedef struct performance_s {
  char name[PERFORMANCE_NAME_LEN];
} performance_t;

typedef struct drums_s {
  uint8_t main_vol;
  uint8_t midi_channel;
  uint8_t midinote[NUM_DRUMSET_CONFIG];
  int8_t pitch[NUM_DRUMSET_CONFIG];
  int8_t pan[NUM_DRUMSET_CONFIG];
  uint8_t vol_max[NUM_DRUMSET_CONFIG];
  uint8_t vol_min[NUM_DRUMSET_CONFIG];
  uint8_t reverb_send[NUM_DRUMSET_CONFIG];
} drum_t;

typedef struct configuration_s {
  performance_t performance;
  sys_t sys;
  dexed_t dexed[MAX_DEXED];
  epiano_t epiano;
  drum_t drums;
  fx_t fx;
} config_t;

enum master_mixer_ports {
  MASTER_MIX_CH_DEXED2,
  MASTER_MIX_CH_DEXED1,
  MASTER_MIX_CH_REVERB,
  MASTER_MIX_CH_DRUMS,
  MASTER_MIX_CH_EPIANO
};

enum reverb_mixer_ports {
  REVERB_MIX_CH_DEXED2,
  REVERB_MIX_CH_DEXED1,
  REVERB_MIX_CH_DRUMS,
  REVERB_MIX_CH_EPIANO
};

enum midi_learn_modes {
  MIDI_LEARN_MODE_OFF,
  MIDI_LEARN_MODE_ON
};

#ifndef _MAPFLOAT
#define _MAPFLOAT
inline float mapfloat(float val, float in_min, float in_max, float out_min, float out_max) {
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif

/*const float fconstrain(const float x, const float a, const float b) {
    if(x < a) {
        return a;
    }
    else if(b < x) {
        return b;
    }
    else
        return x;
}*/

/* From: https://forum.pjrc.com/threads/68048-Predefined-Macros
  ARDUINO_ARCH_AVR - Avr Architecture / (all Teensy < 3.0)
  TEENSYDUINO=Version - Is a Teensy + gives TD version
  KINETISL - Teensy LC
  KINETISK - All Teensy 3.x
  __MK20DX128__ - Teensy 3.0
  __MK20DX256__ - Teensy 3.1, 3.2
  __MK64FX512__ - Teensy 3.5
  __MK66FX1M0__ - Teensy 3.6
  __IMXRT1062__ - all Teensy 4.x + Teensy 4 MM
  ARDUINO_TEENSY2 - Teensy 2
  ARDUINO_TEENSY30 - Teensy 3.0
  [...]
  ARDUINO_TEENSY40 - Teensy 4.0
  ARDUINO_TEENSY41 - Teensy 4.1
  ARDUINO_TEENSYMM - Teensy 4 MM
*/
