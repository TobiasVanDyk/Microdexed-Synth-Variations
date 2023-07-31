/*
   MicroDexed

   MicroDexed is a port of the Dexed sound engine
   Dexed ist heavily based on https://github.com/google/music-synthesizer-for-android

   (c)2018-2023 H. Wirtz <wirtz@parasitstudio.de>
   (c)2021-2022 H. Wirtz <wirtz@parasitstudio.de>, M. Koslowski <positionhigh@gmx.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <limits.h>
#include "config.h"
#include <Audio.h>
#include <Wire.h>
#include <MIDI.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <TeensyVariablePlayback.h>
#include "UI.hpp"
#include "midi_devices.hpp"
#include "synth_dexed.h"
#include "dexed_sd.h"
#include <effect_modulated_delay.h>
#include <effect_stereo_mono.h>
#include <effect_mono_stereo.h>
#include <effect_platervbstereo.h>
//#include <effect_compressor.h>
#include <template_mixer.hpp>
#if NUM_DRUMS > 0
#include "midinotes.h"
#include "drumset.h"
#endif
#ifdef SGTL5000_AUDIO_ENHANCE
#include "control_sgtl5000plus.h"
#endif
#include "synth_mda_epiano.h"
#include <effect_stereo_panorama.h>
#if defined(USE_DELAY_8M)
#include <effect_delay_ext8.h>
#endif

// Audio engines
AudioSynthDexed* MicroDexed[NUM_DEXED];
AudioSynthEPiano ep(NUM_EPIANO_VOICES);

//AudioEffectCompressor2* compressor[NUM_DEXED];
AudioSynthWaveform* chorus_modulator[NUM_DEXED];
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
AudioFilterBiquad* modchorus_filter[NUM_DEXED];
#endif
AudioEffectModulatedDelay* modchorus[NUM_DEXED];
AudioMixer<2>* chorus_mixer[NUM_DEXED];
AudioMixer<2>* delay_fb_mixer[NUM_DEXED];
#if defined(USE_DELAY_8M)
AudioEffectDelayExternal8* delay_fx[NUM_DEXED];
#else
AudioEffectDelay* delay_fx[NUM_DEXED];
#endif
AudioMixer<2>* delay_mixer[NUM_DEXED];
AudioEffectMonoStereo* mono2stereo[NUM_DEXED];

AudioEffectStereoPanorama ep_stereo_panorama;
AudioSynthWaveform ep_chorus_modulator;
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
AudioFilterBiquad ep_modchorus_filter;
#endif
AudioEffectModulatedDelayStereo ep_modchorus;
AudioMixer<2> ep_chorus_mixer_r;
AudioMixer<2> ep_chorus_mixer_l;

AudioMixer<2> microdexed_peak_mixer;
AudioAnalyzePeak microdexed_peak;
AudioMixer<4> reverb_mixer_r;
AudioMixer<4> reverb_mixer_l;
AudioEffectPlateReverb reverb;

AudioMixer<5> master_mixer_r;
AudioMixer<5> master_mixer_l;
//AudioEffectCompressor2 compressor_r;
//AudioEffectCompressor2 compressor_l;
AudioAmplifier volume_r;
AudioAmplifier volume_l;
AudioEffectStereoMono stereo2mono;
AudioAnalyzePeak master_peak_r;
AudioAnalyzePeak master_peak_l;
#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
AudioMixer<2> audio_thru_mixer_r;
AudioMixer<2> audio_thru_mixer_l;
#endif

// Drumset
#if NUM_DRUMS > 0
AudioPlayArrayResmp* Drum[NUM_DRUMS];
AudioMixer<NUM_DRUMS> drum_mixer_r;
AudioMixer<NUM_DRUMS> drum_mixer_l;
#if NUM_DRUMS < 5
AudioMixer<4> drum_reverb_send_mixer_r;
AudioMixer<4> drum_reverb_send_mixer_l;
#else
AudioMixer<8> drum_reverb_send_mixer_r;
AudioMixer<8> drum_reverb_send_mixer_l;
#endif
#endif

// Outputs
#if defined(TEENSY_AUDIO_BOARD)
AudioOutputI2S i2s1;
#if defined(SGTL5000_AUDIO_ENHANCE)
AudioControlSGTL5000Plus sgtl5000;
#else
AudioControlSGTL5000 sgtl5000;
#endif
#elif defined(I2S_AUDIO_ONLY)
AudioOutputI2S i2s1;
#elif defined(TGA_AUDIO_BOARD)
AudioOutputI2S i2s1;
AudioControlWM8731master wm8731_1;
#elif defined(PT8211_AUDIO)
AudioOutputPT8211 pt8211_1;
#elif defined(TEENSY_DAC_SYMMETRIC)
AudioOutputAnalogStereo dacOut;
AudioMixer<4> invMixer;
#elif defined(TEENSY_DAC)
AudioOutputAnalogStereo dacOut;
#endif
#ifdef AUDIO_DEVICE_USB
AudioOutputUSB usb1;
#endif

#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
AudioInputI2S i2s1in;
#endif

//
// Static patching of audio objects
//
AudioConnection patchCord[] = {
  // Audio chain tail
  { reverb_mixer_r, 0, reverb, 0 },
  { reverb_mixer_l, 0, reverb, 1 },
  { reverb, 0, master_mixer_r, MASTER_MIX_CH_REVERB },
  { reverb, 1, master_mixer_l, MASTER_MIX_CH_REVERB },
//  { master_mixer_r, compressor_r },
//  { master_mixer_l, compressor_l },
//  { compressor_r, volume_r },
//  { compressor_l, volume_l },
  { master_mixer_r, volume_r },
  { master_mixer_l, volume_l },
  { volume_r, 0, stereo2mono, 0 },
  { volume_l, 0, stereo2mono, 1 },
  { stereo2mono, 0, master_peak_r, 0 },
  { stereo2mono, 0, master_peak_l, 0 },

// Outputs
#if defined(TEENSY_AUDIO_BOARD)
#ifndef SGTL5000_AUDIO_THRU
  { stereo2mono, 0, i2s1, 0 },
  { stereo2mono, 1, i2s1, 1 },
#endif
#elif defined(I2S_AUDIO_ONLY)
  { stereo2mono, 0, i2s1, 0 },
  { stereo2mono, 1, i2s1, 1 },
#elif defined(TGA_AUDIO_BOARD)
  { stereo2mono, 0, i2s1, 0 },
  { stereo2mono, 1, i2s1, 1 },
#elif defined(PT8211_AUDIO)
  { stereo2mono, 0, pt8211_1, 0 },
  { stereo2mono, 1, pt8211_1, 1 },
#elif defined(TEENSY_DAC_SYMMETRIC)
  { stereo2mono, 0, dacOut, 0 },
  { stereo2mono, 1, invMixer, 0 },
  { invMixer, 0, dacOut, 1 },
#elif defined(TEENSY_DAC)
  { stereo2mono, 0, dacOut, 0 },
  { stereo2mono, 1, dacOut, 1 },
#endif
#ifdef AUDIO_DEVICE_USB
  { stereo2mono, 0, usb1, 0 },
  { stereo2mono, 1, usb1, 1 },
#endif

#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
  { stereo2mono, 0, audio_thru_mixer_r, 0 },
  { stereo2mono, 1, audio_thru_mixer_l, 0 },
  { i2s1in, 0, audio_thru_mixer_r, 1 },
  { i2s1in, 1, audio_thru_mixer_l, 1 },
  { audio_thru_mixer_r, 0, i2s1, 0 },
  { audio_thru_mixer_l, 0, i2s1, 1 },
#endif

#if NUM_DRUMS > 0
  { drum_reverb_send_mixer_r, 0, reverb_mixer_r, REVERB_MIX_CH_DRUMS },
  { drum_reverb_send_mixer_l, 0, reverb_mixer_l, REVERB_MIX_CH_DRUMS },
  { drum_mixer_r, 0, master_mixer_r, MASTER_MIX_CH_DRUMS },
  { drum_mixer_l, 0, master_mixer_l, MASTER_MIX_CH_DRUMS },
#endif

  { ep, 0, ep_stereo_panorama, 0 },
  { ep, 1, ep_stereo_panorama, 1 },
  { ep_stereo_panorama, 0, ep_chorus_mixer_r, 0 },
  { ep_stereo_panorama, 1, ep_chorus_mixer_l, 0 },
  { ep_stereo_panorama, 0, ep_modchorus, 0 },
  { ep_stereo_panorama, 1, ep_modchorus, 1 },
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
  { ep_chorus_modulator, 0, ep_modchorus_filter, 0 },
  { ep_modchorus_filter, 0, ep_modchorus, 2 },
#else
  { ep_chorus_modulator, 0, ep_modchorus, 2 },
#endif
  { ep_modchorus, 0, ep_chorus_mixer_r, 1 },
  { ep_modchorus, 1, ep_chorus_mixer_l, 1 },
  { ep_chorus_mixer_r, 0, reverb_mixer_r, REVERB_MIX_CH_EPIANO },
  { ep_chorus_mixer_l, 0, reverb_mixer_l, REVERB_MIX_CH_EPIANO },
  { ep_chorus_mixer_r, 0, master_mixer_r, MASTER_MIX_CH_EPIANO },
  { ep_chorus_mixer_l, 0, master_mixer_l, MASTER_MIX_CH_EPIANO },
};

//
// Dynamic patching of MicroDexed objects
//
uint8_t nDynamic = 0;
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
AudioConnection* dynamicConnections[NUM_DEXED * 18 + NUM_DRUMS * 4];
#elif MOD_FILTER_OUTPUT == MOD_NO_FILTER_OUTPUT
AudioConnection* dynamicConnections[NUM_DEXED * 17 + NUM_DRUMS * 4];
#endif
FLASHMEM void create_audio_dexed_chain(uint8_t instance_id) {
  MicroDexed[instance_id] = new AudioSynthDexed(MAX_NOTES / NUM_DEXED, SAMPLE_RATE);
  //compressor[instance_id] = new AudioEffectCompressor2();
  mono2stereo[instance_id] = new AudioEffectMonoStereo();
  chorus_modulator[instance_id] = new AudioSynthWaveform();
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
  modchorus_filter[instance_id] = new AudioFilterBiquad();
#endif
  modchorus[instance_id] = new AudioEffectModulatedDelay();
  chorus_mixer[instance_id] = new AudioMixer<2>();
  delay_fb_mixer[instance_id] = new AudioMixer<2>();
#if defined(USE_DELAY_8M)
  delay_fx[instance_id] = new AudioEffectDelayExternal8(AUDIO_MEMORY8_EXTMEM, DELAY_MAX_TIME);
#else
  delay_fx[instance_id] = new AudioEffectDelay();
#endif
  delay_mixer[instance_id] = new AudioMixer<2>();

  //dynamicConnections[nDynamic++] = new AudioConnection(*MicroDexed[instance_id], 0, *compressor[instance_id], 0);
  //dynamicConnections[nDynamic++] = new AudioConnection(*compressor[instance_id], 0, microdexed_peak_mixer, instance_id);
  //dynamicConnections[nDynamic++] = new AudioConnection(*compressor[instance_id], 0, *chorus_mixer[instance_id], 0);
  //dynamicConnections[nDynamic++] = new AudioConnection(*compressor[instance_id], 0, *modchorus[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*MicroDexed[instance_id], 0, microdexed_peak_mixer, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*MicroDexed[instance_id], 0, *chorus_mixer[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*MicroDexed[instance_id], 0, *modchorus[instance_id], 0);

#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
  dynamicConnections[nDynamic++] = new AudioConnection(*chorus_modulator[instance_id], 0, *modchorus_filter[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*modchorus_filter[instance_id], 0, *modchorus[instance_id], 1);
#else
  dynamicConnections[nDynamic++] = new AudioConnection(*chorus_modulator[instance_id], 0, *modchorus[instance_id], 1);
#endif
  dynamicConnections[nDynamic++] = new AudioConnection(*modchorus[instance_id], 0, *chorus_mixer[instance_id], 1);
  dynamicConnections[nDynamic++] = new AudioConnection(*chorus_mixer[instance_id], 0, *delay_fb_mixer[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*chorus_mixer[instance_id], 0, *delay_mixer[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*delay_fb_mixer[instance_id], 0, *delay_fx[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*delay_fx[instance_id], 0, *delay_fb_mixer[instance_id], 1);
  dynamicConnections[nDynamic++] = new AudioConnection(*delay_fx[instance_id], 0, *delay_mixer[instance_id], 1);
  dynamicConnections[nDynamic++] = new AudioConnection(*delay_mixer[instance_id], 0, *mono2stereo[instance_id], 0);
  dynamicConnections[nDynamic++] = new AudioConnection(*mono2stereo[instance_id], 0, reverb_mixer_r, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*mono2stereo[instance_id], 1, reverb_mixer_l, instance_id);

  dynamicConnections[nDynamic++] = new AudioConnection(*mono2stereo[instance_id], 0, master_mixer_r, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*mono2stereo[instance_id], 1, master_mixer_l, instance_id);

#ifdef DEBUG
  Serial.print(F("Dexed-Instance: "));
  Serial.println(instance_id);
#endif
}

//
// Dynamic patching of Drum objects
//
#if NUM_DRUMS > 0
FLASHMEM void create_audio_drum_chain(uint8_t instance_id) {
  //Drum[instance_id] = new AudioPlayMemory();
  Drum[instance_id] = new AudioPlayArrayResmp();
  Drum[instance_id]->enableInterpolation(false);
  Drum[instance_id]->setPlaybackRate(1.0);

  dynamicConnections[nDynamic++] = new AudioConnection(*Drum[instance_id], 0, drum_mixer_r, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*Drum[instance_id], 0, drum_mixer_l, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*Drum[instance_id], 0, drum_reverb_send_mixer_r, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*Drum[instance_id], 0, drum_reverb_send_mixer_l, instance_id);
}
#endif

// other global vars
uint8_t sd_card = 0;
Sd2Card card;
SdVolume volume;
const float midi_ticks_factor[10] = { 0.0, 0.25, 0.375, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0 };
uint8_t midi_bpm_counter = 0;
uint8_t midi_bpm = 0;
int16_t _midi_bpm = -1;
elapsedMillis midi_bpm_timer;
elapsedMillis long_button_pressed;
elapsedMillis control_rate;
elapsedMillis led_blink;
elapsedMillis save_sys;
bool led_status = false;
bool save_sys_flag = false;
uint8_t active_voices[NUM_DEXED];
uint8_t midi_voices[NUM_DEXED];
#ifdef SHOW_CPU_LOAD_MSEC
elapsedMillis cpu_mem_millis;
#endif
uint8_t midi_learn_mode = MIDI_LEARN_MODE_OFF;
uint32_t cpumax = 0;
uint32_t peak_dexed = 0;
float peak_dexed_value = 0.0;
uint32_t peak_r = 0;
uint32_t peak_l = 0;
config_t configuration;
const uint8_t cs_pins[] = { SDCARD_TEENSY_CS_PIN, SDCARD_AUDIO_CS_PIN };
const uint8_t mosi_pins[] = { SDCARD_TEENSY_MOSI_PIN, SDCARD_AUDIO_MOSI_PIN };
const uint8_t sck_pins[] = { SDCARD_TEENSY_SCK_PIN, SDCARD_AUDIO_SCK_PIN };
char version_string[LCD_cols + 1];
char sd_string[LCD_cols + 1];
char g_voice_name[NUM_DEXED][VOICE_NAME_LEN];
char g_bank_name[NUM_DEXED][BANK_NAME_LEN];
char receive_bank_filename[FILENAME_LEN];
uint8_t selected_instance_id = 0;
uint8_t active_sample = 0;
int8_t midi_decay[NUM_DEXED] = { -1, -1 };
elapsedMillis midi_decay_timer;
int perform_attack_mod[NUM_DEXED] = { 0, 0 };
int perform_release_mod[NUM_DEXED] = { 0, 0 };
// Allocate the delay lines for chorus
int16_t* delayline[NUM_DEXED];
int16_t* ep_delayline_r;
int16_t* ep_delayline_l;
uint8_t midinote_old[NUM_DRUMSET_CONFIG];

#if NUM_DRUMS > 0
//extern drum_config_t drum_config[NUM_DRUMSET_CONFIG];
uint8_t drum_counter;
uint8_t drum_type[NUM_DRUMS];
#endif

extern LCDMenuLib2 LCDML;

extern void getNoteName(char* noteName, uint8_t noteNumber);

/***********************************************************************
   SETUP
 ***********************************************************************/
void setup() {
#ifdef DEBUG
  Serial.begin(SERIAL_SPEED);
#else
  delay(50);  // seems to be needed when no serial debugging is enabled
#endif

  setup_ui();

#if defined(DEBUG)
  Serial.println(F("-------------------------------------------------------------------------------"));
  Serial.println(F("Latest crash report:"));
  Serial.println(CrashReport);
  Serial.println(F("-------------------------------------------------------------------------------"));
#endif
#ifdef DEBUG
  setup_debug_message();
#endif

  pinMode(LED_BUILTIN, OUTPUT);

  generate_version_string(version_string, sizeof(version_string));

#ifdef DEBUG
  Serial.println(F("MicroDexed based on https://github.com/asb2m10/dexed"));
  Serial.println(F("(c)2018-2023 H. Wirtz <wirtz@parasitstudio.de>"));
  Serial.println(F("(c)2018-2022 H. Wirtz <wirtz@parasitstudio.de>, M. Koslowski <positionhigh@gmx.de>"));
  Serial.println(F("https://codeberg.org/dcoredump/MicroDexed"));
  Serial.print(F("Version: "));
  Serial.println(version_string);
  Serial.print(F("CPU-Speed: "));
  Serial.print(F_CPU / 1000000.0, 1);
  Serial.println(F(" MHz"));
  Serial.println(F("<setup start>"));
  Serial.flush();
#endif

  // Setup MIDI devices
  setup_midi_devices();

  // Start audio system
  AudioMemory(AUDIO_MEM);
#if defined(TEENSY_AUDIO_BOARD)
  sgtl5000.enable();
  sgtl5000.lineOutLevel(SGTL5000_LINEOUT_LEVEL);
  sgtl5000.dacVolumeRamp();
  sgtl5000.dacVolume(1.0);
  //sgtl5000.dacVolumeRampLinear();
  //sgtl5000.dacVolumeRampDisable();
  sgtl5000.unmuteHeadphone();
  sgtl5000.unmuteLineout();
  sgtl5000.volume(SGTL5000_HEADPHONE_VOLUME, SGTL5000_HEADPHONE_VOLUME);  // Headphone volume
#ifdef SGTL5000_AUDIO_THRU
  //sgtl5000.audioPreProcessorEnable();
  sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000.lineInLevel(5);
  //sgtl5000.adcHighPassFilterEnable();
#endif
#ifdef SGTL5000_AUDIO_ENHANCE
  sgtl5000.audioPostProcessorEnable();
  sgtl5000.init_parametric_eq(7);
  //sgtl5000.enhanceBassEnable();
  //sgtl5000.enhanceBass(1.0, 1.5, 0, 5); // enhanceBass(1.0, 1.0, 1, 2); // Configures the bass enhancement by setting the levels of the original stereo signal and the bass-enhanced mono level which will be mixed together. The high-pass filter may be enabled (0) or bypassed (1).
  //sgtl5000.surroundSoundEnable();
  //sgtl5000.surroundSound(7, 3); // Configures virtual surround width from 0 (mono) to 7 (widest). select may be set to 1 (disable), 2 (mono input) or 3 (stereo input).
#else
  sgtl5000.audioProcessorDisable();
  sgtl5000.autoVolumeDisable();
  sgtl5000.surroundSoundDisable();
  sgtl5000.enhanceBassDisable();
#endif
#ifdef DEBUG
  Serial.println(F("Teensy-Audio-Board enabled."));
#endif
#elif defined(TGA_AUDIO_BOARD)
  wm8731_1.enable();
  wm8731_1.volume(1.0);
#ifdef DEBUG
  Serial.println(F("TGA board enabled."));
#endif
#elif defined(I2S_AUDIO_ONLY)
#ifdef DEBUG
  Serial.println(F("I2S enabled."));
#endif
#elif defined(PT8211_AUDIO)
#ifdef DEBUG
  Serial.println(F("PT8211 enabled."));
#endif
#elif defined(TEENSY_DAC_SYMMETRIC)
  invMixer.gain(0, -1.f);
#ifdef DEBUG
  Serial.println(F("Internal DAC using symmetric outputs enabled."));
#endif
#else
#ifdef DEBUG
  Serial.println(F("Internal DAC enabled."));
#endif
#endif

  // create dynamic Dexed instances
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
#ifdef DEBUG
    Serial.print(F("Creating MicroDexed instance "));
    Serial.println(instance_id, DEC);
#endif
    create_audio_dexed_chain(instance_id);
  }
#ifdef DEBUG
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    Serial.print(F("Instance "));
    Serial.print(instance_id);
    Serial.print(F(": "));
    Serial.print(MicroDexed[instance_id]->getMaxNotes());
    Serial.println(F(" voices"));
  }
#endif

  // Setup (PROGMEM) sample drums
#if NUM_DRUMS > 0
  // create dynamic Drum instances
  for (uint8_t instance_id = 0; instance_id < NUM_DRUMS; instance_id++) {
#ifdef DEBUG
    Serial.print(F("Creating Drum instance "));
    Serial.println(instance_id, DEC);
#endif
    create_audio_drum_chain(instance_id);

    drum_mixer_r.gain(instance_id, 1.0);
    drum_mixer_l.gain(instance_id, 1.0);
    drum_reverb_send_mixer_r.gain(instance_id, 0.0);
    drum_reverb_send_mixer_l.gain(instance_id, 0.0);
  }
  // Init drumset config
  configuration.drums.main_vol = DRUMS_MAIN_VOL_DEFAULT;
  configuration.drums.midi_channel = DRUMS_MIDI_CHANNEL_DEFAULT;

  master_mixer_r.gain(MASTER_MIX_CH_DRUMS, configuration.drums.main_vol);
  master_mixer_l.gain(MASTER_MIX_CH_DRUMS, configuration.drums.main_vol);
#endif

  // Setup EPiano
  // EP_CHORUS
  ep_delayline_r = (int16_t*)malloc(MOD_DELAY_SAMPLE_BUFFER * sizeof(int16_t));
  if (ep_delayline_r == NULL) {
#ifdef DEBUG
    Serial.println(F("AudioEffectModulatedDelay R - memory allocation failed EP"));
#endif
    while (1)
      ;
  }
  ep_delayline_l = (int16_t*)malloc(MOD_DELAY_SAMPLE_BUFFER * sizeof(int16_t));
  if (ep_delayline_l == NULL) {
#ifdef DEBUG
    Serial.println(F("AudioEffectModulatedDelay L - memory allocation failed EP"));
#endif
    while (1)
      ;
  }

  if (!ep_modchorus.begin(ep_delayline_r, ep_delayline_l, MOD_DELAY_SAMPLE_BUFFER)) {
#ifdef DEBUG
    Serial.println(F("AudioEffectModulatedDelayStereo - begin failed EP"));
#endif
    while (1)
      ;
  }

#if MOD_FILTER_OUTPUT == MOD_BUTTERWORTH_FILTER_OUTPUT
  // Butterworth filter, 12 db/octave
  ep_modchorus_filter.setLowpass(0, MOD_FILTER_CUTOFF_HZ, 0.707);
#elif MOD_FILTER_OUTPUT == MOD_LINKWITZ_RILEY_FILTER_OUTPUT
  // Linkwitz-Riley filter, 48 dB/octave
  ep_modchorus_filter.setLowpass(0, MOD_FILTER_CUTOFF_HZ, 0.54);
  ep_modchorus_filter.setLowpass(1, MOD_FILTER_CUTOFF_HZ, 1.3);
  ep_modchorus_filter.setLowpass(2, MOD_FILTER_CUTOFF_HZ, 0.54);
  ep_modchorus_filter.setLowpass(3, MOD_FILTER_CUTOFF_HZ, 1.3);
#endif
  ep_chorus_mixer_r.gain(0, 1.0);
  ep_chorus_mixer_l.gain(0, 1.0);
  ep_chorus_mixer_r.gain(1, mapfloat(EP_CHORUS_LEVEL_DEFAULT, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX, 0.0, 0.5));
  ep_chorus_mixer_l.gain(1, mapfloat(EP_CHORUS_LEVEL_DEFAULT, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX, 0.0, 0.5));
  ep_stereo_panorama.panorama(mapfloat(EP_PANORAMA_DEFAULT, EP_PANORAMA_MIN, EP_PANORAMA_MAX, -1.0, 1.0));

  // Setup effects
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    delayline[instance_id] = (int16_t*)malloc(MOD_DELAY_SAMPLE_BUFFER * sizeof(int16_t));
    if (delayline[instance_id] != NULL) {
      memset(delayline[instance_id], 0, MOD_DELAY_SAMPLE_BUFFER * sizeof(int16_t));
      if (!modchorus[instance_id]->begin(delayline[instance_id], MOD_DELAY_SAMPLE_BUFFER)) {
#ifdef DEBUG
        Serial.print(F("AudioEffectModulatedDelay - begin failed ["));
        Serial.print(instance_id);
        Serial.println(F("]"));
#endif
        while (1)
          ;
      }
    } else {
#ifdef DEBUG
      Serial.print(F("AudioEffectModulatedDelay - memory allocation failed ["));
      Serial.print(instance_id);
      Serial.println(F("]"));
#endif
      while (1)
        ;
    }
  }
#ifdef DEBUG
  Serial.print(F("MOD_DELAY_SAMPLE_BUFFER="));
  Serial.print(MOD_DELAY_SAMPLE_BUFFER, DEC);
  Serial.println(F(" samples"));
#endif

  // Start SD card
  sd_card = check_sd_cards();

  if (sd_card < 1) {
#ifdef DEBUG
    Serial.println(F("SD card not accessable."));
#endif
  } else {
    Serial.println(F("SD card found."));
    check_and_create_directories();

    for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
      // load default SYSEX data
      //load_sd_voice(configuration.dexed[instance_id].bank, configuration.dexed[instance_id].voice, instance_id);
      memset(g_voice_name[instance_id], 0, VOICE_NAME_LEN);
      memset(g_bank_name[instance_id], 0, BANK_NAME_LEN);
      memset(receive_bank_filename, 0, FILENAME_LEN);
    }
  }

  // Load initial Performance or the last used one
  initial_values(false);

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  // Load voices
#ifdef DEBUG
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    Serial.print(F("Dexed instance "));
    Serial.print(instance_id);
    Serial.println(F(":"));
    Serial.print(F("Bank/Voice ["));
    Serial.print(configuration.dexed[instance_id].bank, DEC);
    Serial.print(F("/"));
    Serial.print(configuration.dexed[instance_id].voice, DEC);
    Serial.println(F("]"));
    Serial.print(F("Polyphony: "));
    Serial.println(configuration.dexed[instance_id].polyphony, DEC);
  }
  Serial.print(F("AUDIO_BLOCK_SAMPLES="));
  Serial.print(AUDIO_BLOCK_SAMPLES);
  Serial.print(F(" (Time per block="));
  Serial.print(1000000 / (SAMPLE_RATE / AUDIO_BLOCK_SAMPLES));
  Serial.println(F("ms)"));
#endif

#if defined(DEBUG) && defined(SHOW_CPU_LOAD_MSEC)
  show_cpu_and_mem_usage();
#endif

  // Init master_mixer
#if NUM_DEXED > 1
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    master_mixer_r.gain(instance_id, VOL_MAX_FLOAT);
    master_mixer_l.gain(instance_id, VOL_MAX_FLOAT);
  }
#else
  master_mixer_r.gain(MASTER_MIX_CH_DEXED1, VOL_MAX_FLOAT);
  master_mixer_l.gain(MASTER_MIX_CH_DEXED1, VOL_MAX_FLOAT);
  master_mixer_r.gain(MASTER_MIX_CH_DEXED2, 0.0);
  master_mixer_l.gain(MASTER_MIX_CH_DEXED2, 0.0);
#endif
  master_mixer_r.gain(MASTER_MIX_CH_REVERB, VOL_MAX_FLOAT);
  master_mixer_l.gain(MASTER_MIX_CH_REVERB, VOL_MAX_FLOAT);
  master_mixer_r.gain(MASTER_MIX_CH_EPIANO, VOL_MAX_FLOAT);
  master_mixer_l.gain(MASTER_MIX_CH_EPIANO, VOL_MAX_FLOAT);
#if NUM_DRUMS > 0
  master_mixer_r.gain(MASTER_MIX_CH_DRUMS, VOL_MAX_FLOAT);
  master_mixer_l.gain(MASTER_MIX_CH_DRUMS, VOL_MAX_FLOAT);
#else
  master_mixer_r.gain(MASTER_MIX_CH_DRUMS, 0.0);
  master_mixer_l.gain(MASTER_MIX_CH_DRUMS, 0.0);
#endif

#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
  audio_thru_mixer_r.gain(0, VOL_MAX_FLOAT);  // MD signal sum
  audio_thru_mixer_l.gain(0, VOL_MAX_FLOAT);  // MD signal sum
#ifdef TEENSY_AUDIO_BOARD
  audio_thru_mixer_r.gain(1, VOL_MAX_FLOAT);  // I2S input
  audio_thru_mixer_l.gain(1, VOL_MAX_FLOAT);  // I2S input
#else
  audio_thru_mixer_r.gain(1, 0.0);
  audio_thru_mixer_l.gain(1, 0.0);
#endif
  audio_thru_mixer_r.gain(2, 0.0);
  audio_thru_mixer_l.gain(2, 0.0);
  audio_thru_mixer_r.gain(3, 0.0);
  audio_thru_mixer_l.gain(3, 0.0);
#endif

#ifdef DEBUG
  Serial.println(F("<setup end>"));
#endif

  //ep_modchorus.set_bypass(true);

  strlcpy(configuration.performance.name, "INIT Perf", sizeof(configuration.performance.name));
  LCDML.OTHER_jumpToFunc(UI_func_voice_select);
}

void loop() {
  // MIDI input handling
  check_midi_devices();

  // check encoder
  ENCODER[ENC_L].update();
  ENCODER[ENC_R].update();

  LCDML.loop();

  // CONTROL-RATE-EVENT-HANDLING
  if (control_rate > CONTROL_RATE_MS) {
    control_rate = 0;

    // check for value changes, unused voices and CPU overload
    for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
      active_voices[instance_id] = MicroDexed[instance_id]->getNumNotesPlaying();
      if (active_voices[instance_id] == 0)
        midi_voices[instance_id] = 0;
    }

    if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select)) {
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
        if (midi_decay_timer > MIDI_DECAY_TIMER && midi_decay[instance_id] > 0) {
          midi_decay[instance_id]--;
          display.createChar(6 + instance_id, (uint8_t*)special_chars[15 - (7 - midi_decay[instance_id])]);
          display.setCursor(14 + instance_id, 1);
          display.write(6 + instance_id);
        } else if (midi_voices[instance_id] == 0 && midi_decay[instance_id] == 0 && !MicroDexed[instance_id]->getSustain()) {
          midi_decay[instance_id]--;
          display.setCursor(14 + instance_id, 1);
          display.write(20);  // blank
        }
#else
        static bool midi_playing[NUM_DEXED];
        if (midi_voices[instance_id] > 0 && midi_playing[instance_id] == false) {
          midi_playing[instance_id] = true;
          display.setCursor(14 + instance_id, 1);
          display.write(6 + instance_id);
        } else if (midi_voices[instance_id] == 0 && !MicroDexed[instance_id]->getSustain()) {
          midi_playing[instance_id] = false;
          display.setCursor(14 + instance_id, 1);
          display.write(20);  // blank
        }
#endif
      }
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
      if (midi_decay_timer > MIDI_DECAY_LEVEL_TIME) {
        midi_decay_timer = 0;
      }
#endif
    }
  } else
    yield();

  // SAVE-SYS-EVENT-HANDLING
  if (save_sys > SAVE_SYS_MS && save_sys_flag == true) {
#ifdef DEBUG
    Serial.println(F("Check if we can save configuration.sys"));
#endif
    bool instance_is_playing = false;
    for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
      if (active_voices[instance_id] > 0) {
        instance_is_playing = true;
        break;
      }
    }
    if (instance_is_playing == false) {
      for (uint8_t instance_id = 0; instance_id < NUM_DRUMS; instance_id++) {
        if (Drum[instance_id]->isPlaying()) {
          instance_is_playing = true;
          break;
        }
      }
    }
    if (instance_is_playing == false)
      save_sd_sys_json();
    else {
#ifdef DEBUG
      Serial.println(F("System is playing, next try..."));
#endif
      save_sys = 0;
    }
  }

#if defined(DEBUG) && defined(SHOW_CPU_LOAD_MSEC)
  if (cpu_mem_millis >= SHOW_CPU_LOAD_MSEC) {
    if (master_peak_r.available())
      if (master_peak_r.read() == 1.0)
        peak_r++;
    if (master_peak_l.available())
      if (master_peak_l.read() == 1.0)
        peak_l++;
    if (microdexed_peak.available()) {
      peak_dexed_value = microdexed_peak.read();
      if (peak_dexed_value > 0.99)
        peak_dexed++;
    }
    cpu_mem_millis -= SHOW_CPU_LOAD_MSEC;
    show_cpu_and_mem_usage();
  }
#endif

  // LED blink
  if (led_blink > LED_BLINK_MS) {
    digitalWrite(LED_BUILTIN, led_status);
    led_status = !led_status;
    led_blink = 0;
  }
}

/******************************************************************************
  MIDI HELPER
******************************************************************************/
bool checkMidiChannel(byte inChannel, uint8_t instance_id) {
  // check for MIDI channel
  if (configuration.dexed[instance_id].midi_channel == MIDI_CHANNEL_OMNI) {
    return (true);
  } else if (inChannel != configuration.dexed[instance_id].midi_channel) {
#ifdef DEBUG
    Serial.print(F("INSTANCE "));
    Serial.print(instance_id, DEC);
    Serial.print(F(": Ignoring MIDI data on channel "));
    Serial.print(inChannel);
    Serial.print(F("(listening on "));
    Serial.print(configuration.dexed[instance_id].midi_channel);
    Serial.println(F(")"));
#endif
    return (false);
  }
  return (true);
}

void init_MIDI_send_CC(void) {
#ifdef DEBUG
  Serial.println(F("init_MIDI_send_CC():"));
#endif
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 7, configuration.dexed[selected_instance_id].sound_intensity);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 10, configuration.dexed[selected_instance_id].pan);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 91, configuration.fx.reverb_send[selected_instance_id]);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 93, configuration.fx.chorus_level[selected_instance_id]);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 94, configuration.dexed[selected_instance_id].tune);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 103, configuration.fx.filter_resonance[selected_instance_id]);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 104, configuration.fx.filter_cutoff[selected_instance_id]);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 105, configuration.fx.delay_time[selected_instance_id]);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 106, configuration.fx.delay_feedback[selected_instance_id]);
  MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 107, configuration.fx.delay_level[selected_instance_id]);
}

/******************************************************************************
  MIDI MESSAGE HANDLER
******************************************************************************/
void handleNoteOn(byte inChannel, byte inNumber, byte inVelocity) {
  //
  // MIDI learn mode
  //
  if (midi_learn_mode > MIDI_LEARN_MODE_OFF) {
    int8_t tmp_channel = handle_midi_learn(inNumber);
    if (tmp_channel >= 0)
      inChannel = tmp_channel;
  }

  //
  // Play Notes
  //

  // MicroDexed
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (checkMidiChannel(inChannel, instance_id)) {
      if (inNumber >= configuration.dexed[instance_id].lowest_note && inNumber <= configuration.dexed[instance_id].highest_note) {
        if (configuration.dexed[instance_id].polyphony > 0) {
          MicroDexed[instance_id]->keydown(inNumber, uint8_t(float(configuration.dexed[instance_id].velocity_level / 127.0) * inVelocity + 0.5));
          midi_voices[instance_id]++;
        }
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
        if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select)) {
          midi_decay_timer = 0;
          midi_decay[instance_id] = min(inVelocity / 5, 7);
        }
#endif
#ifdef DEBUG
        char note_name[4];
        getNoteName(note_name, inNumber);
        Serial.print(F("KeyDown "));
        Serial.print(inNumber);
        Serial.print(F("/"));
        Serial.print(note_name);
        Serial.print(F(" instance "));
        Serial.print(instance_id, DEC);
        Serial.print(F(" MIDI-channel "));
        Serial.println(inChannel, DEC);
        Serial.flush();
#endif
      }
    }
  }

  // E-Piano
  if (configuration.epiano.midi_channel == MIDI_CHANNEL_OMNI || configuration.epiano.midi_channel == inChannel) {
    if (inNumber >= configuration.epiano.lowest_note && inNumber <= configuration.epiano.highest_note) {
      ep.noteOn(inNumber + configuration.epiano.transpose - 24, inVelocity);
#ifdef DEBUG
      char note_name[4];
      getNoteName(note_name, inNumber);
      Serial.print(F("KeyDown "));
      Serial.print(inNumber);
      Serial.print(F("/"));
      Serial.print(note_name);
      Serial.print(F(" EPIANO "));
      Serial.print(F(" MIDI-channel "));
      Serial.println(inChannel, DEC);
      Serial.flush();
#endif
    }
  }

  // Drums
#if NUM_DRUMS > 0
  if (inChannel == configuration.drums.midi_channel || configuration.drums.midi_channel == MIDI_CHANNEL_OMNI) {
    if (drum_counter >= NUM_DRUMS)
      drum_counter = 0;

    if (midi_learn_mode > MIDI_LEARN_MODE_OFF) {
      inNumber = configuration.drums.midinote[active_sample];
    }

#ifdef DEBUG
    char note_name[4];
    getNoteName(note_name, inNumber);
    if (midi_learn_mode > MIDI_LEARN_MODE_OFF) {
      if (midi_learn_mode & 0x80)
        Serial.printf_P(PSTR("MIDI LEARN MODE DRUM (%s) for note (%s)\n"), byte_to_binary(midi_learn_mode), note_name);
      else
        Serial.printf_P(PSTR("MIDI LEARN MODE NOTE (%s) for note (%s)\n"), byte_to_binary(midi_learn_mode), note_name);
    }
    Serial.print(F("Triggring Drum["));
    Serial.print(drum_counter, DEC);
    Serial.print(F("] with note "));
    Serial.print(note_name);
    Serial.print(F("/"));
    Serial.println(inNumber, DEC);
    Serial.flush();
#endif

    uint8_t use_drum = 0;

    for (uint8_t d = 0; d < NUM_DRUMSET_CONFIG; d++) {
      if (inNumber == configuration.drums.midinote[d]) {
        use_drum = d;
        break;
      }
    }

    if (drum_config[use_drum].midinote > 0) {
      uint8_t slot = drum_get_slot(drum_config[use_drum].drum_class);
      float pan = mapfloat(configuration.drums.pan[use_drum], DRUMS_PANORAMA_MIN, DRUMS_PANORAMA_MAX, 0.0, 1.0);
      float reverb_send = configuration.drums.reverb_send[use_drum] / 100.0f;
      float vol_min = configuration.drums.vol_min[use_drum] / 100.0f;
      float vol_max = configuration.drums.vol_max[use_drum] / 100.0f;

      drum_mixer_r.gain(slot, pan * volume_transform(mapfloat(inVelocity, 0, 127, vol_min, vol_max)));
      drum_mixer_l.gain(slot, (1.0 - pan) * volume_transform(mapfloat(inVelocity, 0, 127, vol_min, vol_max)));

      drum_reverb_send_mixer_r.gain(slot, pan * volume_transform(reverb_send));
      drum_reverb_send_mixer_l.gain(slot, (1.0 - pan) * volume_transform(reverb_send));

      if (drum_config[use_drum].drum_data != NULL && drum_config[use_drum].len > 0) {
        if (configuration.drums.pitch[use_drum] != 0) {
          Drum[slot]->enableInterpolation(true);
          Drum[slot]->setPlaybackRate(pow(2, float(configuration.drums.pitch[use_drum]) / 120.0));
        } else {
          Drum[slot]->enableInterpolation(false);
          Drum[slot]->setPlaybackRate(1.0);
        }
        Drum[slot]->playRaw((int16_t*)drum_config[use_drum].drum_data, drum_config[use_drum].len, 1);
#ifdef DEBUG
        Serial.printf_P(PSTR("Playing sample [%s] on slot [%d] main volume [%d]: drum_data=%p, len=%d\n"), drum_config[use_drum].name, slot, configuration.drums.main_vol, drum_config[use_drum].drum_data, drum_config[use_drum].len);

        Serial.print(F("Drum Slot ["));
        Serial.print(slot);
        Serial.print(F("]: Velocity="));
        Serial.print(mapfloat(inVelocity, 0, 127, vol_min, vol_max), 2);
        Serial.print(F(" Pan="));
        Serial.print(pan, 2);
        Serial.print(F(" ReverbSend="));
        Serial.print(reverb_send, 2);
        Serial.print(F(" Pitch="));
        Serial.print(configuration.drums.pitch[use_drum] / 10.0f, 1);
        Serial.print(F(" Playback speed="));
        Serial.println(pow(2, float(configuration.drums.pitch[use_drum]) / 120.0));
#endif
      }
    }
  }
#endif
}

void handleNoteOff(byte inChannel, byte inNumber, byte inVelocity) {
  //
  // MIDI learn mode
  //
  if (midi_learn_mode > MIDI_LEARN_MODE_OFF) {
    int8_t tmp_channel = handle_midi_learn(inNumber);
    if (tmp_channel >= 0)
      inChannel = tmp_channel;
  }

  // Dexed
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (checkMidiChannel(inChannel, instance_id)) {
      if (inNumber >= configuration.dexed[instance_id].lowest_note && inNumber <= configuration.dexed[instance_id].highest_note) {
        if (configuration.dexed[instance_id].polyphony > 0)
          MicroDexed[instance_id]->keyup(inNumber);

        midi_voices[instance_id]--;
#ifdef DEBUG
        char note_name[4];
        getNoteName(note_name, inNumber);
        Serial.print(F("KeyUp "));
        Serial.print(note_name);
        Serial.print(F(" instance "));
        Serial.print(instance_id, DEC);
        Serial.print(F(" MIDI-channel "));
        Serial.print(inChannel, DEC);
        Serial.println();
#endif
      }
    }
  }

  // EPiano
  if (configuration.epiano.midi_channel == MIDI_CHANNEL_OMNI || configuration.epiano.midi_channel == inChannel) {
    if (inNumber >= configuration.epiano.lowest_note && inNumber <= configuration.epiano.highest_note) {
      ep.noteOff(inNumber + configuration.epiano.transpose - 24);
#ifdef DEBUG
      char note_name[4];
      getNoteName(note_name, inNumber);
      Serial.print(F("KeyUp "));
      Serial.print(note_name);
      Serial.print(F(" EPIANO "));
      Serial.print(F(" MIDI-channel "));
      Serial.print(inChannel, DEC);
      Serial.println();
#endif
    }
  }
}

void handleControlChange(byte inChannel, byte inCtrl, byte inValue) {
  inCtrl = constrain(inCtrl, 0, 127);
  inValue = constrain(inValue, 0, 127);

  // EPiano
  if (configuration.epiano.midi_channel == MIDI_CHANNEL_OMNI || configuration.epiano.midi_channel == inChannel)
    ep.processMidiController(inCtrl, inValue);

  // Dexed
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (checkMidiChannel(inChannel, instance_id)) {
#ifdef DEBUG
      Serial.print(F("INSTANCE "));
      Serial.print(instance_id, DEC);
      Serial.print(F(": CC#"));
      Serial.print(inCtrl, DEC);
      Serial.print(F(":"));
      Serial.println(inValue, DEC);
#endif

      switch (inCtrl) {
        case 0:  // BankSelect MSB
#ifdef DEBUG
          Serial.println(F("BANK-SELECT MSB CC"));
#endif
          configuration.dexed[instance_id].bank = constrain((inValue << 7) & configuration.dexed[instance_id].bank, 0, MAX_BANKS - 1);
          /* load_sd_voice(configuration.dexed[instance_id].bank, configuration.dexed[instance_id].voice, instance_id);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
            {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
            } */
          break;
        case 1:
#ifdef DEBUG
          Serial.println(F("MODWHEEL CC"));
#endif
          MicroDexed[instance_id]->setModWheel(inValue);
          MicroDexed[instance_id]->ControllersRefresh();
          break;
        case 2:
#ifdef DEBUG
          Serial.println(F("BREATH CC"));
#endif
          MicroDexed[instance_id]->setBreathController(inValue);
          MicroDexed[instance_id]->ControllersRefresh();
          break;
        case 4:
#ifdef DEBUG
          Serial.println(F("FOOT CC"));
#endif
          MicroDexed[instance_id]->setFootController(inValue);
          MicroDexed[instance_id]->ControllersRefresh();
          break;
        case 5:  // Portamento time
          configuration.dexed[instance_id].portamento_time = inValue;
          MicroDexed[instance_id]->setPortamentoTime(configuration.dexed[instance_id].portamento_time);
          break;
        case 7:  // Instance Volume
#ifdef DEBUG
          Serial.println(F("VOLUME CC"));
#endif
          configuration.dexed[instance_id].sound_intensity = map(inValue, 0, 127, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX);
          MicroDexed[instance_id]->setGain(midi_volume_transform(map(configuration.dexed[instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 0, 127)));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_sound_intensity)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 10:  // Pan
#ifdef DEBUG
          Serial.println(F("PANORAMA CC"));
#endif
          configuration.dexed[instance_id].pan = map(inValue, 0, 0x7f, PANORAMA_MIN, PANORAMA_MAX);
          mono2stereo[instance_id]->panorama(mapfloat(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_panorama)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 32:  // BankSelect LSB
#ifdef DEBUG
          Serial.println(F("BANK-SELECT LSB CC"));
#endif
          configuration.dexed[instance_id].bank = constrain(inValue, 0, MAX_BANKS - 1);
          /*load_sd_voice(configuration.dexed[instance_id].bank, configuration.dexed[instance_id].voice, instance_id);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
            {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
            }*/
          break;
        case 64:
          MicroDexed[instance_id]->setSustain(inValue > 63);
          /*
            if (!MicroDexed[instance_id]->getSustain())
            {
            for (uint8_t note = 0; note < MicroDexed[instance_id]->getMaxNotes(); note++)
            {
              if (MicroDexed[instance_id]->voices[note].sustained && !MicroDexed[instance_id]->voices[note].keydown)
              {
                MicroDexed[instance_id]->voices[note].dx7_note->keyup();
                MicroDexed[instance_id]->voices[note].sustained = false;
              }
            }
            }
          */
          break;
        case 65:
          MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode);
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_mode)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 94:  // CC 94: (de)tune
          configuration.dexed[selected_instance_id].tune = map(inValue, 0, 0x7f, TUNE_MIN, TUNE_MAX);
          MicroDexed[selected_instance_id]->setMasterTune((int((configuration.dexed[selected_instance_id].tune - 100) / 100.0 * 0x4000) << 11) * (1.0 / 12));
          MicroDexed[selected_instance_id]->doRefreshVoice();
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_tune)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 91:  // CC 91: reverb send
          configuration.fx.reverb_send[selected_instance_id] = map(inValue, 0, 0x7f, REVERB_SEND_MIN, REVERB_SEND_MAX);
          reverb_mixer_r.gain(selected_instance_id, volume_transform(mapfloat(configuration.fx.reverb_send[selected_instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
          reverb_mixer_l.gain(selected_instance_id, volume_transform(mapfloat(configuration.fx.reverb_send[selected_instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_reverb_send)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 93:  // CC 93: chorus level
          configuration.fx.chorus_level[selected_instance_id] = map(inValue, 0, 0x7f, CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX);
          chorus_mixer[selected_instance_id]->gain(1, volume_transform(mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5)));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_chorus_level)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 103:  // CC 103: filter resonance
          configuration.fx.filter_resonance[instance_id] = map(inValue, 0, 0x7f, FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX);
          MicroDexed[instance_id]->setFilterResonance(mapfloat(configuration.fx.filter_resonance[instance_id], FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 1.0, 0.0));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_filter_resonance)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 104:  // CC 104: filter cutoff
          configuration.fx.filter_cutoff[instance_id] = map(inValue, 0, 0x7f, FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX);
          MicroDexed[instance_id]->setFilterCutoff(mapfloat(configuration.fx.filter_cutoff[instance_id], FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 1.0, 0.0));
          ;
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_filter_cutoff)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 105:  // CC 105: delay time
          configuration.fx.delay_time[instance_id] = map(inValue, 0, 0x7f, DELAY_TIME_MIN, DELAY_TIME_MAX);
          delay_fx[instance_id]->delay(0, constrain(configuration.fx.delay_time[instance_id] * 10, DELAY_TIME_MIN * 10, DELAY_TIME_MAX * 10));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_delay_time)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 106:  // CC 106: delay feedback
          configuration.fx.delay_feedback[instance_id] = map(inValue, 0, 0x7f, DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX);
          delay_fb_mixer[instance_id]->gain(1, midi_volume_transform(map(configuration.fx.delay_feedback[instance_id], DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 0, 127)));  // amount of feedback
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_delay_feedback)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 107:  // CC 107: delay volume
          configuration.fx.delay_level[instance_id] = map(inValue, 0, 0x7f, DELAY_LEVEL_MIN, DELAY_LEVEL_MAX);
          delay_mixer[instance_id]->gain(1, midi_volume_transform(map(configuration.fx.delay_level[instance_id], DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 0, 127)));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_delay_level)) {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 120:
          MicroDexed[instance_id]->panic();
          break;
        case 121:
          MicroDexed[instance_id]->resetControllers();
          break;
        case 123:
          MicroDexed[instance_id]->notesOff();
          break;
        case 126:
          if (inValue > 0)
            MicroDexed[instance_id]->setMonoMode(true);
          else
            MicroDexed[instance_id]->setMonoMode(false);
          break;
        case 127:
          if (inValue > 0)
            MicroDexed[instance_id]->setMonoMode(true);
          else
            MicroDexed[instance_id]->setMonoMode(false);
          break;
      }
    }
  }
}

void handleAfterTouch(byte inChannel, byte inPressure) {
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (checkMidiChannel(inChannel, instance_id)) {
      MicroDexed[instance_id]->setAftertouch(inPressure);
      MicroDexed[instance_id]->ControllersRefresh();
    }
  }
}

void handlePitchBend(byte inChannel, int inPitch) {
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (checkMidiChannel(inChannel, instance_id)) {
      MicroDexed[instance_id]->setPitchbend(inPitch);
    }
  }
}

void handleProgramChange(byte inChannel, byte inProgram) {
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (checkMidiChannel(inChannel, instance_id)) {
      configuration.dexed[instance_id].voice = constrain(inProgram, 0, MAX_VOICES - 1);
      load_sd_voice(configuration.dexed[instance_id].bank, configuration.dexed[instance_id].voice, instance_id);

      if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select)) {
        LCDML.OTHER_updateFunc();
        LCDML.loop_menu();
      }
    }
  }
}

void handleSystemExclusive(byte* sysex, unsigned int len) {
  int16_t sysex_return;

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (!checkMidiChannel((sysex[2] & 0x0f) + 1, instance_id)) {
#ifdef DEBUG
      Serial.print(F("INSTANCE "));
      Serial.print(instance_id, DEC);
      Serial.println(F(": SYSEX-MIDI-Channel mismatch"));
#endif
      return;
    }

#ifdef DEBUG
    Serial.print(F("SysEx data length: ["));
    Serial.print(len);
    Serial.println(F("]"));

    Serial.println(F("SysEx data:"));
    for (uint16_t i = 0; i < len; i++) {
      Serial.print(F("[0x"));
      uint8_t s = sysex[i];
      if (s < 16)
        Serial.print(F("0"));
      Serial.print(s, HEX);
      Serial.print(F("|"));
      if (s < 100)
        Serial.print(F("0"));
      if (s < 10)
        Serial.print(F("0"));
      Serial.print(s, DEC);
      Serial.print(F("]"));
      if ((i + 1) % 16 == 0)
        Serial.println();
    }
    Serial.println();
#endif

    sysex_return = MicroDexed[instance_id]->checkSystemExclusive(sysex, len);
#ifdef DEBUG
    Serial.print(F("SYSEX handler return value:"));
    Serial.print(sysex_return, DEC);
    Serial.println();
#endif

    switch (sysex_return) {
      case -1:
#ifdef DEBUG
        Serial.println(F("E: SysEx end status byte not detected."));
#endif
        break;
      case -2:
#ifdef DEBUG
        Serial.println(F("E: SysEx vendor not Yamaha."));
#endif
        break;
      case -3:
#ifdef DEBUG
        Serial.println(F("E: Unknown SysEx parameter change."));
#endif
        break;
      case -4:
#ifdef DEBUG
        Serial.println(F("E: Unknown SysEx voice or function."));
#endif
        break;
      case -5:
#ifdef DEBUG
        Serial.println(F("E: Not a SysEx voice bulk upload."));
#endif
        break;
      case -6:
#ifdef DEBUG
        Serial.println(F("E: Wrong length for SysEx voice bulk upload (not 155)."));
#endif
        break;
      case -7:
#ifdef DEBUG
        Serial.println(F("E: Checksum error for one voice."));
#endif
        break;
      case -8:
#ifdef DEBUG
        Serial.println(F("E: Not a SysEx bank bulk upload."));
#endif
        display.setCursor(0, 1);
        display.print(F("Error (TYPE)      "));
        delay(MESSAGE_WAIT_TIME);
        LCDML.FUNC_goBackToMenu();
        break;
      case -9:
#ifdef DEBUG
        Serial.println(F("E: Wrong length for SysEx bank bulk upload (not 4096)."));
#endif
        display.setCursor(0, 1);
        display.print(F("Error (SIZE)     "));
        delay(MESSAGE_WAIT_TIME);
        LCDML.FUNC_goBackToMenu();
        break;
      case -10:
#ifdef DEBUG
        Serial.println(F("E: Checksum error for bank."));
#endif
        break;
      case -11:
#ifdef DEBUG
        Serial.println(F("E: Unknown SysEx message."));
#endif
        break;
      case 64:
      case 65:
      case 66:
      case 67:
      case 68:
      case 69:
      case 70:
      case 71:
      case 72:
      case 73:
      case 74:
      case 75:
      case 76:
      case 77:
#ifdef DEBUG
        Serial.println(F("SysEx Function parameter change : "));
        Serial.print(F("Parameter #"));
        Serial.print(sysex[4], DEC);
        Serial.print(F(" Value : "));
        Serial.println(sysex[5], DEC);
#endif
        switch (sysex[4]) {
          case 65:
            configuration.dexed[instance_id].pb_range = constrain(sysex[5], PB_RANGE_MIN, PB_RANGE_MAX);
            MicroDexed[instance_id]->setPitchbendRange(configuration.dexed[instance_id].pb_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_pb_range)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 66:
            configuration.dexed[instance_id].pb_step = constrain(sysex[5], PB_STEP_MIN, PB_STEP_MAX);
            MicroDexed[instance_id]->setPitchbendRange(configuration.dexed[instance_id].pb_step);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_pb_step)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 67:
            configuration.dexed[instance_id].portamento_mode = constrain(sysex[5], PORTAMENTO_MODE_MIN, PORTAMENTO_MODE_MAX);
            MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_mode)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 68:
            configuration.dexed[instance_id].portamento_glissando = constrain(sysex[5], PORTAMENTO_GLISSANDO_MIN, PORTAMENTO_GLISSANDO_MAX);
            MicroDexed[instance_id]->setPortamentoGlissando(configuration.dexed[instance_id].portamento_glissando);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_glissando)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 69:
            configuration.dexed[instance_id].portamento_time = constrain(sysex[5], PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX);
            MicroDexed[instance_id]->setPortamentoTime(configuration.dexed[instance_id].portamento_time);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_time)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 70:
            configuration.dexed[instance_id].mw_range = constrain(sysex[5], MW_RANGE_MIN, MW_RANGE_MAX);
            MicroDexed[instance_id]->setModWheelRange(configuration.dexed[instance_id].mw_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_mw_range)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 71:
            configuration.dexed[instance_id].mw_assign = constrain(sysex[5], MW_ASSIGN_MIN, MW_ASSIGN_MAX);
            MicroDexed[instance_id]->setModWheelTarget(configuration.dexed[instance_id].mw_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_mw_assign)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 72:
            configuration.dexed[instance_id].fc_range = constrain(sysex[5], FC_RANGE_MIN, FC_RANGE_MAX);
            MicroDexed[instance_id]->setFootControllerRange(configuration.dexed[instance_id].fc_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_fc_range)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 73:
            configuration.dexed[instance_id].fc_assign = constrain(sysex[5], FC_ASSIGN_MIN, FC_ASSIGN_MAX);
            MicroDexed[instance_id]->setFootControllerTarget(configuration.dexed[instance_id].fc_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_fc_assign)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 74:
            configuration.dexed[instance_id].bc_range = constrain(sysex[5], BC_RANGE_MIN, BC_RANGE_MAX);
            MicroDexed[instance_id]->setBreathControllerRange(configuration.dexed[instance_id].bc_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_bc_range)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 75:
            configuration.dexed[instance_id].bc_assign = constrain(sysex[5], BC_ASSIGN_MIN, BC_ASSIGN_MAX);
            MicroDexed[instance_id]->setBreathControllerTarget(configuration.dexed[instance_id].bc_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_bc_assign)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 76:
            configuration.dexed[instance_id].at_range = constrain(sysex[5], AT_RANGE_MIN, AT_RANGE_MAX);
            MicroDexed[instance_id]->setAftertouchRange(configuration.dexed[instance_id].at_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_at_range)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 77:
            configuration.dexed[instance_id].at_assign = constrain(sysex[5], AT_ASSIGN_MIN, AT_ASSIGN_MAX);
            MicroDexed[instance_id]->setAftertouchTarget(configuration.dexed[instance_id].at_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_at_assign)) {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
        }
        break;
      case 100:
        // fix voice name
        for (uint8_t i = 0; i < 10; i++) {
          if (sysex[151 + i] > 126)  // filter characters
            sysex[151 + i] = 32;
        }

        // load sysex-data into voice memory
        MicroDexed[instance_id]->loadVoiceParameters(&sysex[6]);

#ifdef DEBUG
        Serial.println(F("One Voice bulk upload"));
        show_patch(instance_id);
#endif
        strlcpy(g_voice_name[instance_id], (char*)&sysex[151], sizeof(g_voice_name[instance_id]));

        if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select)) {
          LCDML.OTHER_updateFunc();
          LCDML.loop_menu();
        }
        break;
      case 200:
        if (strlen(receive_bank_filename) > 0 && LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_sysex_receive_bank)) {
#ifdef DEBUG
          Serial.println(F("Bank bulk upload."));
#endif
          if (save_sd_bank(receive_bank_filename, sysex)) {
#ifdef DEBUG
            Serial.print(F("Bank saved as ["));
            Serial.print(receive_bank_filename);
            Serial.println(F("]"));
#endif
            display.setCursor(0, 1);
            display.print(F("Done.           "));
            delay(MESSAGE_WAIT_TIME);
            LCDML.FUNC_goBackToMenu();
          } else {
#ifdef DEBUG
            Serial.println(F("Error during saving bank as ["));
            Serial.print(receive_bank_filename);
            Serial.println(F("]"));
#endif
            display.setCursor(0, 1);
            display.print(F("Error.          "));
            delay(MESSAGE_WAIT_TIME);
            LCDML.FUNC_goBackToMenu();
          }
          memset(receive_bank_filename, 0, FILENAME_LEN);
        }
#ifdef DEBUG
        else
          Serial.println(F("E : Not in MIDI receive bank mode."));
#endif
        break;
      case 300:
#ifdef DEBUG
        Serial.println(F("SysEx Voice parameter:"));
        Serial.print(F("Parameter #"));
        Serial.print(sysex[4] + ((sysex[3] & 0x03) * 128), DEC);
        Serial.print(F(" Value: "));
        Serial.println(sysex[5], DEC);
#endif
        break;
      default:
#ifdef DEBUG
        Serial.println(F("SysEx Voice parameter change : "));
        Serial.print(F("Parameter #"));
        Serial.print(sysex_return);
        Serial.print(F(" Value : "));
        Serial.println(sysex[5], DEC);
#endif
        break;
    }
  }
}

void handleTimeCodeQuarterFrame(byte data) {
  ;
}

void handleAfterTouchPoly(byte inChannel, byte inNumber, byte inVelocity) {
  ;
}

void handleSongSelect(byte inSong) {
  ;
}

void handleTuneRequest(void) {
  ;
}

void handleClock(void) {
  ;
}

void handleStart(void) {
  ;
}

void handleContinue(void) {
  ;
}

void handleStop(void) {
  MicroDexed[0]->panic();
#if NUM_DEXED > 1
  MicroDexed[1]->panic();
#endif
}

void handleActiveSensing(void) {
  ;
}

void handleSystemReset(void) {
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
#ifdef DEBUG
    Serial.println(F("MIDI SYSEX RESET"));
#endif
    MicroDexed[instance_id]->notesOff();
    MicroDexed[instance_id]->panic();
    MicroDexed[instance_id]->resetControllers();
  }
}

/******************************************************************************
  VOLUME HELPER
******************************************************************************/
void dac_mute(void) {
  sgtl5000.lineOutLevel(0.0);
  sgtl5000.dacVolume(0.0);
  sgtl5000.volume(0.0, 0.0);  // Headphone volume
}

void dac_unmute(void) {
  sgtl5000.lineOutLevel(SGTL5000_LINEOUT_LEVEL);
  sgtl5000.dacVolume(1.0);
  sgtl5000.volume(SGTL5000_HEADPHONE_VOLUME, SGTL5000_HEADPHONE_VOLUME);  // Headphone volume
}

void set_drums_volume(float vol) {
  master_mixer_r.gain(MASTER_MIX_CH_DRUMS, vol);
  master_mixer_l.gain(MASTER_MIX_CH_DRUMS, vol);
}

void set_volume(uint8_t v, uint8_t m) {
  float tmp_v;

  configuration.sys.vol = v;

  if (configuration.sys.vol > 100)
    configuration.sys.vol = 100;
  tmp_v = float(v);

  configuration.sys.mono = m;

#ifdef DEBUG
  Serial.print(F("Setting volume: VOL="));
  Serial.println(v, DEC);
  Serial.print(F(" V="));
  Serial.println(volume_transform(tmp_v / 100.0));
#endif

  volume_r.gain(volume_transform(tmp_v / 100.0) * VOLUME_MULTIPLIER);
  volume_l.gain(volume_transform(tmp_v / 100.0) * VOLUME_MULTIPLIER);

  switch (m) {
    case 0:  // stereo
      stereo2mono.stereo(true);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
    case 1:  // mono both
      stereo2mono.stereo(false);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(PANORAMA_DEFAULT, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
    case 2:  // mono right
      volume_l.gain(0.0);
      stereo2mono.stereo(false);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(PANORAMA_MAX, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
    case 3:  // mono left
      volume_r.gain(0.0);
      stereo2mono.stereo(false);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(PANORAMA_MIN, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
  }
}

/******************************************************************************
  EEPROM HELPER
******************************************************************************/

void initial_values(bool init) {
  uint16_t _m_;

  if (init == true)
    init_configuration();
  else {
    _m_ = (EEPROM[EEPROM_START_ADDRESS] << 8) | EEPROM[EEPROM_START_ADDRESS + 1];
    if (_m_ != EEPROM_MARKER) {
#ifdef DEBUG
      Serial.println(F("Found wrong EEPROM marker, initializing EEPROM..."));
#endif
      init_configuration();
      //load_sd_performance_json(PERFORMANCE_NUM_MIN);
    } else {
      load_sd_sys_json();
      if (configuration.sys.load_at_startup == 0xff) {
#ifdef DEBUG
        Serial.print(F("Loading initial system data from performance "));
        Serial.println(configuration.sys.performance_number, DEC);
#endif
        load_sd_performance_json(configuration.sys.performance_number);
      } else if (configuration.sys.load_at_startup < 100) {
#ifdef DEBUG
        Serial.print(F("Loading initial system data from performance "));
        Serial.println(configuration.sys.load_at_startup, DEC);
#endif
        load_sd_performance_json(configuration.sys.load_at_startup);
      } else {
#ifdef DEBUG
        Serial.print(F("Loading initial system data from default performance "));
        Serial.println(STARTUP_NUM_DEFAULT, DEC);
#endif
        load_sd_performance_json(STARTUP_NUM_DEFAULT);
      }
    }
#ifdef DEBUG
    Serial.println(F("OK, loaded!"));
#endif

    check_configuration();
  }
  configuration.sys.vol = EEPROM[EEPROM_START_ADDRESS + 2];
  set_volume(configuration.sys.vol, configuration.sys.mono);

#ifdef DEBUG
  show_configuration();
#endif
}

void check_configuration(void) {
  check_configuration_sys();
  check_configuration_fx();
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
    check_configuration_dexed(instance_id);
  check_configuration_epiano();
  check_configuration_drums();
}

void check_configuration_sys(void) {
  configuration.sys.vol = constrain(configuration.sys.vol, VOLUME_MIN, VOLUME_MAX);
  configuration.sys.mono = constrain(configuration.sys.mono, MONO_MIN, MONO_MAX);
  configuration.sys.soft_midi_thru = constrain(configuration.sys.soft_midi_thru, SOFT_MIDI_THRU_MIN, SOFT_MIDI_THRU_MAX);
  configuration.sys.favorites = constrain(configuration.sys.favorites, FAVORITES_NUM_MIN, FAVORITES_NUM_MAX);
  configuration.sys.performance_number = constrain(configuration.sys.performance_number, PERFORMANCE_NUM_MIN, PERFORMANCE_NUM_MAX);
  configuration.sys.load_at_startup = constrain(configuration.sys.load_at_startup, STARTUP_NUM_MIN, STARTUP_NUM_MAX);
}

void check_configuration_fx(void) {
#ifdef USE_PLATEREVERB
  configuration.fx.reverb_lowpass = constrain(configuration.fx.reverb_lowpass, REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX);
  configuration.fx.reverb_lodamp = constrain(configuration.fx.reverb_lodamp, REVERB_LODAMP_MIN, REVERB_LODAMP_MAX);
  configuration.fx.reverb_hidamp = constrain(configuration.fx.reverb_hidamp, REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX);
  configuration.fx.reverb_diffusion = constrain(configuration.fx.reverb_diffusion, REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX);
#else
  configuration.fx.reverb_damping = constrain(configuration.fx.reverb_damping, REVERB_DAMPING_MIN, REVERB_DAMPING_MAX);
#endif
  configuration.fx.reverb_roomsize = constrain(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX);
  configuration.fx.reverb_level = constrain(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX);

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    configuration.fx.filter_cutoff[instance_id] = constrain(configuration.fx.filter_cutoff[instance_id], FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX);
    configuration.fx.filter_resonance[instance_id] = constrain(configuration.fx.filter_resonance[instance_id], FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX);
    configuration.fx.chorus_frequency[instance_id] = constrain(configuration.fx.chorus_frequency[instance_id], CHORUS_FREQUENCY_MIN, CHORUS_FREQUENCY_MAX);
    configuration.fx.chorus_waveform[instance_id] = constrain(configuration.fx.chorus_waveform[instance_id], CHORUS_WAVEFORM_MIN, CHORUS_WAVEFORM_MAX);
    configuration.fx.chorus_depth[instance_id] = constrain(configuration.fx.chorus_depth[instance_id], CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX);
    configuration.fx.chorus_level[instance_id] = constrain(configuration.fx.chorus_level[instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX);
    configuration.fx.delay_time[instance_id] = constrain(configuration.fx.delay_time[instance_id], DELAY_TIME_MIN, DELAY_TIME_MAX);
    configuration.fx.delay_feedback[instance_id] = constrain(configuration.fx.delay_feedback[instance_id], DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX);
    configuration.fx.delay_level[instance_id] = constrain(configuration.fx.delay_level[instance_id], DELAY_LEVEL_MIN, DELAY_LEVEL_MAX);
    configuration.fx.reverb_send[instance_id] = constrain(configuration.fx.reverb_send[instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX);
  }

  configuration.fx.eq_1 = constrain(configuration.fx.eq_1, EQ_1_MIN, EQ_1_MAX);
  configuration.fx.eq_2 = constrain(configuration.fx.eq_2, EQ_2_MIN, EQ_2_MAX);
  configuration.fx.eq_3 = constrain(configuration.fx.eq_3, EQ_3_MIN, EQ_3_MAX);
  configuration.fx.eq_4 = constrain(configuration.fx.eq_4, EQ_4_MIN, EQ_4_MAX);
  configuration.fx.eq_5 = constrain(configuration.fx.eq_5, EQ_5_MIN, EQ_5_MAX);
  configuration.fx.eq_6 = constrain(configuration.fx.eq_6, EQ_6_MIN, EQ_6_MAX);
  configuration.fx.eq_7 = constrain(configuration.fx.eq_7, EQ_7_MIN, EQ_7_MAX);

  configuration.fx.ep_chorus_frequency = constrain(configuration.fx.ep_chorus_frequency, EP_CHORUS_FREQUENCY_MIN, EP_CHORUS_FREQUENCY_MAX);
  configuration.fx.ep_chorus_waveform = constrain(configuration.fx.ep_chorus_waveform, EP_CHORUS_WAVEFORM_MIN, EP_CHORUS_WAVEFORM_MAX);
  configuration.fx.ep_chorus_depth = constrain(configuration.fx.ep_chorus_depth, EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX);
  configuration.fx.ep_chorus_level = constrain(configuration.fx.ep_chorus_level, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX);
  configuration.fx.ep_reverb_send = constrain(configuration.fx.ep_reverb_send, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX);
}

void check_configuration_dexed(uint8_t instance_id) {
  configuration.dexed[instance_id].bank = constrain(configuration.dexed[instance_id].bank, 0, MAX_BANKS - 1);
  configuration.dexed[instance_id].voice = constrain(configuration.dexed[instance_id].voice, 0, MAX_VOICES - 1);
  configuration.dexed[instance_id].midi_channel = constrain(configuration.dexed[instance_id].midi_channel, MIDI_CHANNEL_MIN, MIDI_CHANNEL_MAX);
  configuration.dexed[instance_id].engine = constrain(configuration.dexed[instance_id].engine, ENGINE_MIN, ENGINE_MAX);
  configuration.dexed[instance_id].lowest_note = constrain(configuration.dexed[instance_id].lowest_note, INSTANCE_LOWEST_NOTE_MIN, INSTANCE_LOWEST_NOTE_MAX);
  configuration.dexed[instance_id].highest_note = constrain(configuration.dexed[instance_id].highest_note, INSTANCE_HIGHEST_NOTE_MIN, INSTANCE_HIGHEST_NOTE_MAX);
  configuration.dexed[instance_id].sound_intensity = constrain(configuration.dexed[instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX);
  configuration.dexed[instance_id].pan = constrain(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX);
  configuration.dexed[instance_id].transpose = constrain(configuration.dexed[instance_id].transpose, TRANSPOSE_MIN, TRANSPOSE_MAX);
  configuration.dexed[instance_id].tune = constrain(configuration.dexed[instance_id].tune, TUNE_MIN, TUNE_MAX);
  configuration.dexed[instance_id].polyphony = constrain(configuration.dexed[instance_id].polyphony, POLYPHONY_MIN, POLYPHONY_MAX);
  configuration.dexed[instance_id].velocity_level = constrain(configuration.dexed[instance_id].velocity_level, VELOCITY_LEVEL_MIN, VELOCITY_LEVEL_MAX);
  configuration.dexed[instance_id].monopoly = constrain(configuration.dexed[instance_id].monopoly, MONOPOLY_MIN, MONOPOLY_MAX);
  configuration.dexed[instance_id].note_refresh = constrain(configuration.dexed[instance_id].note_refresh, NOTE_REFRESH_MIN, NOTE_REFRESH_MAX);
  configuration.dexed[instance_id].pb_range = constrain(configuration.dexed[instance_id].pb_range, PB_RANGE_MIN, PB_RANGE_MAX);
  configuration.dexed[instance_id].pb_step = constrain(configuration.dexed[instance_id].pb_step, PB_STEP_MIN, PB_STEP_MAX);
  configuration.dexed[instance_id].mw_range = constrain(configuration.dexed[instance_id].mw_range, MW_RANGE_MIN, MW_RANGE_MAX);
  configuration.dexed[instance_id].mw_assign = constrain(configuration.dexed[instance_id].mw_assign, MW_ASSIGN_MIN, MW_ASSIGN_MAX);
  configuration.dexed[instance_id].mw_mode = constrain(configuration.dexed[instance_id].mw_mode, MW_MODE_MIN, MW_MODE_MAX);
  configuration.dexed[instance_id].fc_range = constrain(configuration.dexed[instance_id].fc_range, FC_RANGE_MIN, FC_RANGE_MAX);
  configuration.dexed[instance_id].fc_assign = constrain(configuration.dexed[instance_id].fc_assign, FC_ASSIGN_MIN, FC_ASSIGN_MAX);
  configuration.dexed[instance_id].fc_mode = constrain(configuration.dexed[instance_id].fc_mode, FC_MODE_MIN, FC_MODE_MAX);
  configuration.dexed[instance_id].bc_range = constrain(configuration.dexed[instance_id].bc_range, BC_RANGE_MIN, BC_RANGE_MAX);
  configuration.dexed[instance_id].bc_assign = constrain(configuration.dexed[instance_id].bc_assign, BC_ASSIGN_MIN, BC_ASSIGN_MAX);
  configuration.dexed[instance_id].bc_mode = constrain(configuration.dexed[instance_id].bc_mode, BC_MODE_MIN, BC_MODE_MAX);
  configuration.dexed[instance_id].at_range = constrain(configuration.dexed[instance_id].at_range, AT_RANGE_MIN, AT_RANGE_MAX);
  configuration.dexed[instance_id].at_assign = constrain(configuration.dexed[instance_id].at_assign, AT_ASSIGN_MIN, AT_ASSIGN_MAX);
  configuration.dexed[instance_id].at_mode = constrain(configuration.dexed[instance_id].at_mode, AT_MODE_MIN, AT_MODE_MAX);
  configuration.dexed[instance_id].portamento_mode = constrain(configuration.dexed[instance_id].portamento_mode, PORTAMENTO_MODE_MIN, PORTAMENTO_MODE_MAX);
  configuration.dexed[instance_id].portamento_glissando = constrain(configuration.dexed[instance_id].portamento_glissando, PORTAMENTO_GLISSANDO_MIN, PORTAMENTO_GLISSANDO_MAX);
  configuration.dexed[instance_id].portamento_time = constrain(configuration.dexed[instance_id].portamento_time, PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX);
  configuration.dexed[instance_id].op_enabled = constrain(configuration.dexed[instance_id].op_enabled, OP_ENABLED_MIN, OP_ENABLED_MAX);
}

void check_configuration_epiano(void) {
  configuration.epiano.decay = constrain(configuration.epiano.decay, EP_DECAY_MIN, EP_DECAY_MAX);
  configuration.epiano.release = constrain(configuration.epiano.release, EP_RELEASE_MIN, EP_RELEASE_MAX);
  configuration.epiano.hardness = constrain(configuration.epiano.hardness, EP_HARDNESS_MIN, EP_HARDNESS_MAX);
  configuration.epiano.treble = constrain(configuration.epiano.treble, EP_TREBLE_MIN, EP_TREBLE_MAX);
  configuration.epiano.pan_tremolo = constrain(configuration.epiano.pan_tremolo, EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX);
  configuration.epiano.pan_lfo = constrain(configuration.epiano.pan_lfo, EP_PAN_LFO_MIN, EP_PAN_LFO_MAX);
  configuration.epiano.velocity_sense = constrain(configuration.epiano.velocity_sense, EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX);
  configuration.epiano.stereo = constrain(configuration.epiano.stereo, EP_STEREO_MIN, EP_STEREO_MAX);
  configuration.epiano.polyphony = constrain(configuration.epiano.polyphony, EP_POLYPHONY_MIN, EP_POLYPHONY_MAX);
  configuration.epiano.tune = constrain(configuration.epiano.tune, EP_TUNE_MIN, EP_TUNE_MAX);
  configuration.epiano.detune = constrain(configuration.epiano.detune, EP_DETUNE_MIN, EP_DETUNE_MAX);
  configuration.epiano.overdrive = constrain(configuration.epiano.overdrive, EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX);
  configuration.epiano.lowest_note = constrain(configuration.epiano.lowest_note, EP_LOWEST_NOTE_MIN, EP_LOWEST_NOTE_MAX);
  configuration.epiano.highest_note = constrain(configuration.epiano.highest_note, EP_HIGHEST_NOTE_MIN, EP_HIGHEST_NOTE_MAX);
  configuration.epiano.transpose = constrain(configuration.epiano.transpose, EP_TRANSPOSE_MIN, EP_TRANSPOSE_MAX);
  configuration.epiano.sound_intensity = constrain(configuration.epiano.sound_intensity, EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX);
  configuration.epiano.pan = constrain(configuration.epiano.pan, EP_PANORAMA_MIN, EP_PANORAMA_MAX);
  configuration.epiano.velocity_sense = constrain(configuration.epiano.velocity_sense, EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX);
  configuration.epiano.midi_channel = constrain(configuration.epiano.midi_channel, EP_MIDI_CHANNEL_MIN, EP_MIDI_CHANNEL_MAX);
}

void check_configuration_drums(void) {
  configuration.drums.main_vol = constrain(configuration.drums.main_vol, DRUMS_MAIN_VOL_MIN, DRUMS_MAIN_VOL_MAX);
  configuration.drums.midi_channel = constrain(configuration.drums.midi_channel, DRUMS_MIDI_CHANNEL_MIN, DRUMS_MIDI_CHANNEL_MAX);

  for (uint8_t i = 0; i < NUM_DRUMSET_CONFIG - 1; i++) {
    configuration.drums.midinote[i] = constrain(configuration.drums.midinote[i], DRUMS_MIDI_NOTE_MIN, DRUMS_MIDI_NOTE_MAX);
    configuration.drums.pitch[i] = constrain(configuration.drums.pitch[i], DRUMS_PITCH_MIN, DRUMS_PITCH_MAX);
    configuration.drums.pan[i] = constrain(configuration.drums.pan[i], DRUMS_PANORAMA_MIN, DRUMS_PANORAMA_MAX);
    configuration.drums.vol_max[i] = constrain(configuration.drums.vol_max[i], DRUMS_VOL_MIN, DRUMS_VOL_MAX);
    configuration.drums.vol_min[i] = constrain(configuration.drums.vol_min[i], DRUMS_VOL_MIN, DRUMS_VOL_MAX);
    configuration.drums.reverb_send[i] = constrain(configuration.drums.reverb_send[i], DRUMS_REVERB_SEND_MIN, DRUMS_REVERB_SEND_MAX);
  }
}

void init_configuration(void) {
#ifdef DEBUG
  Serial.println(F("INITIALIZING CONFIGURATION"));
#endif

  configuration.sys.vol = VOLUME_DEFAULT;
  configuration.sys.mono = MONO_DEFAULT;
  configuration.sys.soft_midi_thru = SOFT_MIDI_THRU_DEFAULT;
  configuration.sys.performance_number = PERFORMANCE_NUM_DEFAULT;
  configuration.sys.load_at_startup = STARTUP_NUM_DEFAULT;

#ifdef USE_PLATEREVERB
  configuration.fx.reverb_lowpass = REVERB_LOWPASS_DEFAULT;
  configuration.fx.reverb_lodamp = REVERB_LODAMP_DEFAULT;
  configuration.fx.reverb_hidamp = REVERB_HIDAMP_DEFAULT;
  configuration.fx.reverb_diffusion = REVERB_DIFFUSION_DEFAULT;
#else
  configuration.fx.reverb_damping = REVERB_DAMPING_DEFAULT;
#endif

  configuration.fx.reverb_roomsize = REVERB_ROOMSIZE_DEFAULT;
  configuration.fx.reverb_level = REVERB_LEVEL_DEFAULT;

  configuration.fx.ep_chorus_frequency = EP_CHORUS_FREQUENCY_DEFAULT;
  configuration.fx.ep_chorus_waveform = EP_CHORUS_WAVEFORM_DEFAULT;
  configuration.fx.ep_chorus_depth = EP_CHORUS_DEPTH_DEFAULT;
  configuration.fx.ep_chorus_level = EP_CHORUS_LEVEL_DEFAULT;
  configuration.fx.ep_reverb_send = EP_REVERB_SEND_DEFAULT;

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    configuration.dexed[instance_id].bank = SYSEXBANK_DEFAULT;
    configuration.dexed[instance_id].voice = SYSEXSOUND_DEFAULT;
    configuration.dexed[instance_id].midi_channel = DEFAULT_MIDI_CHANNEL;
    configuration.dexed[instance_id].engine = ENGINE_DEFAULT;
    configuration.dexed[instance_id].lowest_note = INSTANCE_LOWEST_NOTE_MIN;
    configuration.dexed[instance_id].highest_note = INSTANCE_HIGHEST_NOTE_MAX;
    configuration.dexed[instance_id].sound_intensity = SOUND_INTENSITY_DEFAULT;
    configuration.dexed[instance_id].pan = PANORAMA_DEFAULT;
    configuration.dexed[instance_id].transpose = TRANSPOSE_DEFAULT;
    configuration.dexed[instance_id].tune = TUNE_DEFAULT;
    configuration.dexed[instance_id].polyphony = POLYPHONY_DEFAULT;
    configuration.dexed[instance_id].velocity_level = VELOCITY_LEVEL_DEFAULT;
    configuration.dexed[instance_id].monopoly = MONOPOLY_DEFAULT;
    configuration.dexed[instance_id].note_refresh = NOTE_REFRESH_DEFAULT;
    configuration.dexed[instance_id].pb_range = PB_RANGE_DEFAULT;
    configuration.dexed[instance_id].pb_step = PB_STEP_DEFAULT;
    configuration.dexed[instance_id].mw_range = MW_RANGE_DEFAULT;
    configuration.dexed[instance_id].mw_assign = MW_ASSIGN_DEFAULT;
    configuration.dexed[instance_id].mw_mode = MW_MODE_DEFAULT;
    configuration.dexed[instance_id].fc_range = FC_RANGE_DEFAULT;
    configuration.dexed[instance_id].fc_assign = FC_ASSIGN_DEFAULT;
    configuration.dexed[instance_id].fc_mode = FC_MODE_DEFAULT;
    configuration.dexed[instance_id].bc_range = BC_RANGE_DEFAULT;
    configuration.dexed[instance_id].bc_assign = BC_ASSIGN_DEFAULT;
    configuration.dexed[instance_id].bc_mode = BC_MODE_DEFAULT;
    configuration.dexed[instance_id].at_range = AT_RANGE_DEFAULT;
    configuration.dexed[instance_id].at_assign = AT_ASSIGN_DEFAULT;
    configuration.dexed[instance_id].at_mode = AT_MODE_DEFAULT;
    configuration.dexed[instance_id].portamento_mode = PORTAMENTO_MODE_DEFAULT;
    configuration.dexed[instance_id].portamento_glissando = PORTAMENTO_GLISSANDO_DEFAULT;
    configuration.dexed[instance_id].portamento_time = PORTAMENTO_TIME_DEFAULT;
    configuration.dexed[instance_id].op_enabled = OP_ENABLED_DEFAULT;

    configuration.fx.filter_cutoff[instance_id] = FILTER_CUTOFF_DEFAULT;
    configuration.fx.filter_resonance[instance_id] = FILTER_RESONANCE_DEFAULT;
    configuration.fx.chorus_frequency[instance_id] = CHORUS_FREQUENCY_DEFAULT;
    configuration.fx.chorus_waveform[instance_id] = CHORUS_WAVEFORM_DEFAULT;
    configuration.fx.chorus_depth[instance_id] = CHORUS_DEPTH_DEFAULT;
    configuration.fx.chorus_level[instance_id] = CHORUS_LEVEL_DEFAULT;
    configuration.fx.delay_time[instance_id] = DELAY_TIME_DEFAULT / 10;
    configuration.fx.delay_feedback[instance_id] = DELAY_FEEDBACK_DEFAULT;
    configuration.fx.delay_level[instance_id] = DELAY_LEVEL_DEFAULT;
    configuration.fx.reverb_send[instance_id] = REVERB_SEND_DEFAULT;

    MicroDexed[instance_id]->ControllersRefresh();
  }

  configuration.epiano.decay = EP_DECAY_DEFAULT;
  configuration.epiano.release = EP_RELEASE_DEFAULT;
  configuration.epiano.hardness = EP_HARDNESS_DEFAULT;
  configuration.epiano.treble = EP_TREBLE_DEFAULT;
  configuration.epiano.pan_tremolo = EP_PAN_TREMOLO_DEFAULT;
  configuration.epiano.pan_lfo = EP_PAN_LFO_DEFAULT;
  configuration.epiano.velocity_sense = EP_VELOCITY_SENSE_DEFAULT;
  configuration.epiano.stereo = EP_STEREO_DEFAULT;
  configuration.epiano.polyphony = EP_POLYPHONY_DEFAULT;
  configuration.epiano.tune = EP_TUNE_DEFAULT;
  configuration.epiano.detune = EP_DETUNE_DEFAULT;
  configuration.epiano.overdrive = EP_OVERDRIVE_DEFAULT;
  configuration.epiano.lowest_note = EP_LOWEST_NOTE_DEFAULT;
  configuration.epiano.highest_note = EP_HIGHEST_NOTE_DEFAULT;
  configuration.epiano.transpose = EP_TRANSPOSE_DEFAULT;
  configuration.epiano.sound_intensity = EP_SOUND_INTENSITY_DEFAULT;
  configuration.epiano.pan = EP_PANORAMA_DEFAULT;
  configuration.epiano.velocity_sense = EP_VELOCITY_SENSE_DEFAULT;
  configuration.epiano.midi_channel = EP_MIDI_CHANNEL_DEFAULT;

#if NUM_DRUMS > 0
  configuration.drums.main_vol = DRUMS_MAIN_VOL_DEFAULT;
  configuration.drums.midi_channel = DRUMS_MIDI_CHANNEL_DEFAULT;
#endif

  strlcpy(configuration.performance.name, "INIT Perf", sizeof(configuration.performance.name));

  eeprom_update();
}

void eeprom_update(void) {
  EEPROM.update(EEPROM_START_ADDRESS, (EEPROM_MARKER & 0xff00) >> 8);
  EEPROM.update(EEPROM_START_ADDRESS + 1, EEPROM_MARKER & 0xff);
  EEPROM.update(EEPROM_START_ADDRESS + 2, configuration.sys.vol);
}

void set_fx_params(void) {
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    // CHORUS
    switch (configuration.fx.chorus_waveform[instance_id]) {
      case 0:
        chorus_modulator[instance_id]->begin(WAVEFORM_TRIANGLE);
        break;
      case 1:
        chorus_modulator[instance_id]->begin(WAVEFORM_SINE);
        break;
      default:
        chorus_modulator[instance_id]->begin(WAVEFORM_TRIANGLE);
    }
    chorus_modulator[instance_id]->phase(0);
    chorus_modulator[instance_id]->frequency(configuration.fx.chorus_frequency[instance_id] / 10.0);
    chorus_modulator[instance_id]->amplitude(mapfloat(configuration.fx.chorus_depth[instance_id], CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX, 0.0, 1.0));
    chorus_modulator[instance_id]->offset(0.0);
#if MOD_FILTER_OUTPUT == MOD_BUTTERWORTH_FILTER_OUTPUT
    // Butterworth filter, 12 db/octave
    modchorus_filter[instance_id]->setLowpass(0, MOD_FILTER_CUTOFF_HZ, 0.707);
#elif MOD_FILTER_OUTPUT == MOD_LINKWITZ_RILEY_FILTER_OUTPUT
    // Linkwitz-Riley filter, 48 dB/octave
    modchorus_filter[instance_id]->setLowpass(0, MOD_FILTER_CUTOFF_HZ, 0.54);
    modchorus_filter[instance_id]->setLowpass(1, MOD_FILTER_CUTOFF_HZ, 1.3);
    modchorus_filter[instance_id]->setLowpass(2, MOD_FILTER_CUTOFF_HZ, 0.54);
    modchorus_filter[instance_id]->setLowpass(3, MOD_FILTER_CUTOFF_HZ, 1.3);
#endif
    chorus_mixer[instance_id]->gain(0, 1.0);
    chorus_mixer[instance_id]->gain(1, mapfloat(configuration.fx.chorus_level[instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5));

    // DELAY
    delay_mixer[instance_id]->gain(0, 1.0);
    delay_mixer[instance_id]->gain(1, midi_volume_transform(map(configuration.fx.delay_level[instance_id], DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 0, 127)));
    delay_fb_mixer[instance_id]->gain(0, 1.0);
    delay_fb_mixer[instance_id]->gain(1, midi_volume_transform(map(configuration.fx.delay_feedback[instance_id], DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 0, 127)));
    if (configuration.fx.delay_level[selected_instance_id] <= DELAY_LEVEL_MIN)
      delay_fx[instance_id]->disable(0);
    else
      delay_fx[instance_id]->delay(0, constrain(configuration.fx.delay_time[instance_id], DELAY_TIME_MIN, DELAY_TIME_MAX) * 10);

    // REVERB SEND
    reverb_mixer_r.gain(instance_id, volume_transform(mapfloat(configuration.fx.reverb_send[instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
    reverb_mixer_l.gain(instance_id, volume_transform(mapfloat(configuration.fx.reverb_send[instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));

    // DEXED FILTER
    MicroDexed[instance_id]->setFilterResonance(mapfloat(configuration.fx.filter_resonance[instance_id], FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 1.0, 0.0));
    MicroDexed[instance_id]->setFilterCutoff(mapfloat(configuration.fx.filter_cutoff[instance_id], FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 1.0, 0.0));
    MicroDexed[instance_id]->doRefreshVoice();
  }

  // REVERB
  reverb.size(mapfloat(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 0.0, 1.0));
  reverb.lowpass(mapfloat(configuration.fx.reverb_lowpass, REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX, 0.0, 1.0));
  reverb.lodamp(mapfloat(configuration.fx.reverb_lodamp, REVERB_LODAMP_MIN, REVERB_LODAMP_MAX, 0.0, 1.0));
  reverb.hidamp(mapfloat(configuration.fx.reverb_hidamp, REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX, 0.0, 1.0));
  reverb.diffusion(mapfloat(configuration.fx.reverb_diffusion, REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX, 0.0, 1.0));

#if NUM_DRUMS > 0
  reverb_mixer_r.gain(REVERB_MIX_CH_DRUMS, 1.0);  // Drums Reverb-Send
  reverb_mixer_l.gain(REVERB_MIX_CH_DRUMS, 1.0);  // Drums Reverb-Send
#endif

  reverb_mixer_r.gain(REVERB_MIX_CH_EPIANO, mapfloat(configuration.fx.ep_reverb_send, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX, 0.0, 1.0));  // EPiano Reverb-Send
  reverb_mixer_l.gain(REVERB_MIX_CH_EPIANO, mapfloat(configuration.fx.ep_reverb_send, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX, 0.0, 1.0));  // EPiano Reverb-Send

  // EP_CHORUS
  switch (configuration.fx.ep_chorus_waveform) {
    case 0:
      ep_chorus_modulator.begin(WAVEFORM_TRIANGLE);
      break;
    case 1:
      ep_chorus_modulator.begin(WAVEFORM_SINE);
      break;
    default:
      ep_chorus_modulator.begin(WAVEFORM_TRIANGLE);
  }
  ep_chorus_modulator.phase(0);
  ep_chorus_modulator.frequency(configuration.fx.ep_chorus_frequency / 10.0);
  ep_chorus_modulator.amplitude(mapfloat(configuration.fx.ep_chorus_depth, EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX, 0.0, 1.0));
  ep_chorus_modulator.offset(0.0);

#if MOD_FILTER_OUTPUT == MOD_BUTTERWORTH_FILTER_OUTPUT
  // Butterworth filter, 12 db/octave
  ep_modchorus_filter.setLowpass(0, MOD_FILTER_CUTOFF_HZ, 0.707);
#elif MOD_FILTER_OUTPUT == MOD_LINKWITZ_RILEY_FILTER_OUTPUT
  // Linkwitz-Riley filter, 48 dB/octave
  ep_modchorus_filter.setLowpass(0, MOD_FILTER_CUTOFF_HZ, 0.54);
  ep_modchorus_filter.setLowpass(1, MOD_FILTER_CUTOFF_HZ, 1.3);
  ep_modchorus_filter.setLowpass(2, MOD_FILTER_CUTOFF_HZ, 0.54);
  ep_modchorus_filter.setLowpass(3, MOD_FILTER_CUTOFF_HZ, 1.3);
#endif
  ep_chorus_mixer_r.gain(0, 1.0);
  ep_chorus_mixer_l.gain(0, 1.0);
  ep_chorus_mixer_r.gain(1, mapfloat(configuration.fx.ep_chorus_level, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX, 0.0, 0.5));
  ep_chorus_mixer_l.gain(1, mapfloat(configuration.fx.ep_chorus_level, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX, 0.0, 0.5));

  master_mixer_r.gain(MASTER_MIX_CH_REVERB, volume_transform(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, VOL_MAX_FLOAT)));
  master_mixer_l.gain(MASTER_MIX_CH_REVERB, volume_transform(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, VOL_MAX_FLOAT)));

#ifdef SGTL5000_AUDIO_ENHANCE
  sgtl5000.setEQFc(1, float(configuration.fx.eq_1));
  sgtl5000.setEQGain(2, mapfloat(configuration.fx.eq_2, EQ_2_MIN, EQ_2_MAX, -9.9, 9.9));
  sgtl5000.setEQGain(3, mapfloat(configuration.fx.eq_3, EQ_3_MIN, EQ_3_MAX, -9.9, 9.9));
  sgtl5000.setEQGain(4, mapfloat(configuration.fx.eq_4, EQ_4_MIN, EQ_4_MAX, -9.9, 9.9));
  sgtl5000.setEQGain(5, mapfloat(configuration.fx.eq_5, EQ_5_MIN, EQ_5_MAX, -9.9, 9.9));
  sgtl5000.setEQGain(6, mapfloat(configuration.fx.eq_6, EQ_6_MIN, EQ_6_MAX, -9.9, 9.9));
  sgtl5000.setEQFc(7, float(configuration.fx.eq_7));
  for (uint8_t band = 1; band <= 7; band++) {
    sgtl5000.commitFilter(band);
#ifdef DEBUG
    sgtl5000.show_params(band);
#endif
  }
#endif

  init_MIDI_send_CC();
}

void set_voiceconfig_params(uint8_t instance_id) {
  // INIT PEAK MIXER
  microdexed_peak_mixer.gain(instance_id, 1.0);

  // Controller
  MicroDexed[instance_id]->setMaxNotes(configuration.dexed[instance_id].polyphony);
  MicroDexed[instance_id]->setPBController(configuration.dexed[instance_id].pb_range, configuration.dexed[instance_id].pb_step);
  MicroDexed[instance_id]->setMWController(configuration.dexed[instance_id].mw_range, configuration.dexed[instance_id].mw_assign, configuration.dexed[instance_id].mw_mode);
  MicroDexed[instance_id]->setFCController(configuration.dexed[instance_id].fc_range, configuration.dexed[instance_id].fc_assign, configuration.dexed[instance_id].fc_mode);
  MicroDexed[instance_id]->setBCController(configuration.dexed[instance_id].bc_range, configuration.dexed[instance_id].bc_assign, configuration.dexed[instance_id].bc_mode);
  MicroDexed[instance_id]->setATController(configuration.dexed[instance_id].at_range, configuration.dexed[instance_id].at_assign, configuration.dexed[instance_id].at_mode);
  MicroDexed[instance_id]->ControllersRefresh();
  MicroDexed[instance_id]->setOPAll(configuration.dexed[instance_id].op_enabled);
  MicroDexed[instance_id]->doRefreshVoice();
  MicroDexed[instance_id]->setMonoMode(configuration.dexed[instance_id].monopoly);
  MicroDexed[instance_id]->setNoteRefreshMode(configuration.dexed[instance_id].note_refresh);
  MicroDexed[instance_id]->setEngineType(configuration.dexed[instance_id].engine);

  // Dexed output level
  MicroDexed[instance_id]->setGain(midi_volume_transform(map(configuration.dexed[instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 0, 127)));

  // PANORAMA
  mono2stereo[instance_id]->panorama(mapfloat(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
}

void set_epiano_params(void) {
#ifdef DEBUG
  Serial.print(F("Setting EPiano parameters... "));
#endif
  ep.setDecay(mapfloat(configuration.epiano.decay, EP_DECAY_MIN, EP_DECAY_MAX, 0.0, 1.0));
  ep.setRelease(mapfloat(configuration.epiano.release, EP_RELEASE_MIN, EP_RELEASE_MAX, 0.0, 1.0));
  ep.setHardness(mapfloat(configuration.epiano.hardness, EP_HARDNESS_MIN, EP_HARDNESS_MAX, 0.0, 1.0));
  ep.setTreble(mapfloat(configuration.epiano.treble, EP_TREBLE_MIN, EP_TREBLE_MAX, 0.0, 1.0));
  ep.setPanTremolo(mapfloat(configuration.epiano.pan_tremolo, EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX, 0.0, 1.0));
  ep.setPanLFO(mapfloat(configuration.epiano.pan_lfo, EP_PAN_LFO_MIN, EP_PAN_LFO_MAX, 0.0, 1.0));
  ep.setVelocitySense(mapfloat(configuration.epiano.velocity_sense, EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX, 0.0, 1.0));
  ep.setStereo(mapfloat(configuration.epiano.stereo, EP_STEREO_MIN, EP_STEREO_MAX, 0.0, 1.0));
  ep.setPolyphony(configuration.epiano.polyphony);
  ep.setTune(mapfloat(configuration.epiano.tune, EP_TUNE_MIN, EP_TUNE_MAX, 0.0, 1.0));
  ep.setDetune(mapfloat(configuration.epiano.detune, EP_DETUNE_MIN, EP_DETUNE_MAX, 0.0, 1.0));
  ep.setOverdrive(mapfloat(configuration.epiano.overdrive, EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX, 0.0, 1.0));
  ep.setVolume(mapfloat(configuration.epiano.sound_intensity, EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX, 0.0, 1.0));
#ifdef DEBUG
  Serial.println(F("done."));
#endif
}

void set_sys_params(void) {
  // set initial volume
  set_volume(configuration.sys.vol, configuration.sys.mono);
}

/******************************************************************************
  HELPERS
******************************************************************************/

// https://www.reddit.com/r/Teensy/comments/7r19uk/reset_and_reboot_teensy_lc_via_code/
#define SCB_AIRCR (*(volatile uint32_t*)0xE000ED0C)  // Application Interrupt and Reset Control location
void _softRestart(void) {
  Serial.end();            //clears the serial monitor if used
  SCB_AIRCR = 0x05FA0004;  //write value for restart
}

/*float pseudo_log_curve(float value)
  {
  //return (mapfloat(_pseudo_log * arm_sin_f32(value), 0.0, _pseudo_log * arm_sin_f32(1.0), 0.0, 1.0));
  //return (1 - sqrt(1 - value * value));
  //return (pow(2, value) - 1);
  return (pow(value, 2.2));
  }*/

#if NUM_DRUMS > 0
uint8_t drum_get_slot(uint8_t dt) {
  // Cleanup not playing drums
  for (uint8_t i = 0; i < NUM_DRUMS; i++) {
    if ((dt == DRUM_HIHAT || dt == DRUM_HANDCLAP) && drum_type[i] == dt) {
      Drum[i]->stop();
      drum_type[i] = DRUM_NONE;
      Drum[i]->enableInterpolation(false);
      Drum[i]->setPlaybackRate(1.0);
#ifdef DEBUG
      Serial.print(F("Stopping Drum "));
      Serial.print(i);
      Serial.print(F(" type "));
      Serial.println(dt);
#endif
      drum_counter = i + 1;
      return (i);
    } else if (!Drum[i]->isPlaying()) {
      drum_type[i] = DRUM_NONE;
      Drum[i]->enableInterpolation(false);
      Drum[i]->setPlaybackRate(1.0);
      drum_counter = i + 1;
      return (i);
    }
  }
#ifdef DEBUG
  Serial.print(F("Using next drum slot "));
  Serial.println(drum_counter % NUM_DRUMS);
#endif
  drum_type[drum_counter % NUM_DRUMS] = dt;
  drum_counter++;
  return (drum_counter - 1 % NUM_DRUMS);
}
#endif

#if NUM_DRUMSET_CONFIG > 0
uint8_t get_drums_id_by_note(uint8_t note) {
  uint8_t ret = NUM_DRUMSET_CONFIG - 1;

  for (uint8_t i = 0; i < NUM_DRUMSET_CONFIG - 1; i++) {
    if (configuration.drums.midinote[i] == note) {
      ret = i;
      break;
    }
  }
  return (ret);
}
#endif

int8_t handle_midi_learn(int8_t note) {
  int8_t ret_channel = -1;

#ifdef DEBUG
  Serial.printf_P(PSTR("MIDI learning for note %d with midi learn mode %d\n"), note, midi_learn_mode);
#endif

  if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_drum_pan)) {
    ret_channel = configuration.drums.midi_channel;
    active_sample = get_drums_id_by_note(note);
    LCDML.OTHER_jumpToFunc(UI_func_drum_pan);
  } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_drum_reverb_send)) {
    ret_channel = configuration.drums.midi_channel;
    active_sample = get_drums_id_by_note(note);
    LCDML.OTHER_jumpToFunc(UI_func_drum_reverb_send);
  } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_drum_vol_min_max)) {
    ret_channel = configuration.drums.midi_channel;
    active_sample = get_drums_id_by_note(note);
    LCDML.OTHER_jumpToFunc(UI_func_drum_vol_min_max);
  } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_drum_midi_note)) {
    ret_channel = configuration.drums.midi_channel;
    if (midi_learn_mode & 0x80) {
      configuration.drums.midinote[active_sample] = midinote_old[active_sample];
      active_sample = get_drums_id_by_note(note);
    } else configuration.drums.midinote[active_sample] = note;
    midi_learn_mode |= note;
    LCDML.OTHER_jumpToFunc(UI_func_drum_midi_note);
  } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_drum_pitch)) {
    ret_channel = configuration.drums.midi_channel;
    active_sample = get_drums_id_by_note(note);
    LCDML.OTHER_jumpToFunc(UI_func_drum_pitch);
  } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_epiano_lowest_note)) {
    if (note > configuration.epiano.highest_note)
      configuration.epiano.lowest_note = configuration.epiano.highest_note;
    else
      configuration.epiano.lowest_note = note;
    ret_channel = configuration.epiano.midi_channel;
#ifdef DEBUG
    Serial.print(F("MIDI learned lowest note: "));
    Serial.print(note);
    Serial.print(F(" for EPiano, ghosting MIDI channel "));
    Serial.println(ret_channel);
#endif
  } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_epiano_highest_note)) {
    if (note < configuration.epiano.lowest_note)
      configuration.epiano.highest_note = configuration.epiano.lowest_note;
    else
      configuration.epiano.highest_note = note;
    ret_channel = configuration.epiano.midi_channel;
#ifdef DEBUG
    Serial.print(F("MIDI learned highest note: "));
    Serial.print(note);
    Serial.print(F(" for EPiano, ghosting MIDI channel "));
    Serial.println(ret_channel);
#endif
  }

  // Check for Dexed
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_lowest_note)) {
      if (note > configuration.dexed[selected_instance_id].highest_note)
        configuration.dexed[selected_instance_id].lowest_note = configuration.dexed[selected_instance_id].highest_note;
      else
        configuration.dexed[selected_instance_id].lowest_note = note;
      ret_channel = configuration.dexed[selected_instance_id].midi_channel;
#ifdef DEBUG
      Serial.print(F("MIDI learned lowest note: "));
      Serial.print(note);
      Serial.print(F(" for instance: "));
      Serial.print(selected_instance_id);
      Serial.print(F(", ghosting MIDI channel "));
      Serial.println(ret_channel);
#endif
    } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_highest_note)) {
      if (note < configuration.dexed[selected_instance_id].lowest_note)
        configuration.dexed[selected_instance_id].highest_note = configuration.dexed[selected_instance_id].lowest_note;
      else
        configuration.dexed[selected_instance_id].highest_note = note;
      ret_channel = configuration.dexed[selected_instance_id].midi_channel;
#ifdef DEBUG
      Serial.print(F("MIDI learned highest note: "));
      Serial.print(note);
      Serial.print(F(" for instance: "));
      Serial.print(selected_instance_id);
      Serial.print(F(", ghosting MIDI channel "));
      Serial.println(ret_channel);
#endif
    }
    LCDML.OTHER_updateFunc();
  }

  return (ret_channel);
}

float midi_volume_transform(uint8_t midi_in) {
#ifdef DEBUG
  Serial.printf_P(PSTR("MIDI volume transform in=%3d, out=%3.1f\n"), midi_in, powf(midi_in / 127.0, VOLUME_TRANSFORM_EXP));
#endif
  return powf(midi_in / 127.0, VOLUME_TRANSFORM_EXP);
}

float volume_transform(float in) {
#ifdef DEBUG
  Serial.printf_P(PSTR("Volume transform in=%3.1f, out=%3.1f\n"), in, powf(in, VOLUME_TRANSFORM_EXP));
#endif
  return powf(in, VOLUME_TRANSFORM_EXP);
}

uint32_t crc32(byte* calc_start, uint16_t calc_bytes)  // base code from https://www.arduino.cc/en/Tutorial/EEPROMCrc
{
  const uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };
  uint32_t crc = ~0L;

  for (byte* index = calc_start; index < (calc_start + calc_bytes); ++index) {
    crc = crc_table[(crc ^ *index) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (*index >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return (crc);
}

void generate_version_string(char* buffer, uint8_t len) {
  char tmp[3];

  memset(buffer, 0, len);
  strlcat(buffer, VERSION, len);
#if defined(ARDUINO_TEENSY41)
  strlcat(buffer, "-4.1", 4);
#endif
  strlcat(buffer, "FX", 2);
#if defined(MAX_NOTES)
  strlcat(buffer, "-", 1);
  itoa(MAX_NOTES, tmp, 10);
  strlcat(buffer, tmp, 2);
#endif
}

FLASHMEM uint8_t check_sd_cards(void) {
  uint8_t ret = 0;

  memset(sd_string, 0, sizeof(sd_string));

  for (uint8_t i = 0; i < sizeof(cs_pins); i++) {
#ifdef DEBUG
    Serial.print(F("Checking CS pin "));
    Serial.print(cs_pins[i], DEC);
    Serial.println(F(" for SD card"));
#endif
    SPI.setMOSI(mosi_pins[i]);
    SPI.setSCK(sck_pins[i]);

    if (SD.begin(cs_pins[i]) == true) {
#ifdef DEBUG
      Serial.print(F("Found. Using pin "));
      Serial.println(cs_pins[i], DEC);
#endif
      ret = cs_pins[i];
      break;
    }
  }

  if (ret >= 0) {
    if (!card.init(SPI_HALF_SPEED, ret)) {
#ifdef DEBUG
      Serial.println(F("SD card initialization failed."));
#endif
      ret = -1;
    }
  }

  if (ret >= 0) {
#ifdef DEBUG
    Serial.print(F("Card type: "));
#endif
    switch (card.type()) {
      case SD_CARD_TYPE_SD1:
        snprintf_P(sd_string, sizeof(sd_string), PSTR("%-5s"), PSTR("SD1"));
#ifdef DEBUG
        Serial.println(F("SD1"));
#endif
        break;
      case SD_CARD_TYPE_SD2:
        snprintf_P(sd_string, sizeof(sd_string), PSTR("%-5s"), PSTR("SD2"));
#ifdef DEBUG
        Serial.println(F("SD2"));
#endif
        break;
      case SD_CARD_TYPE_SDHC:
        snprintf_P(sd_string, sizeof(sd_string), PSTR("%-5s"), PSTR("SD2"));
#ifdef DEBUG
        Serial.println(F("SDHC"));
#endif
        break;
      default:
        snprintf_P(sd_string, sizeof(sd_string), PSTR("%-5s"), PSTR("UKNW"));
#ifdef DEBUG
        Serial.println(F("Unknown"));
#endif
    }

    if (!volume.init(card)) {
#ifdef DEBUG
      Serial.println(F("Could not find FAT16/FAT32 partition."));
#endif
      ret = -1;
    }
  }

  if (ret >= 0) {
    uint32_t volumesize;

    volumesize = volume.blocksPerCluster() * volume.clusterCount() / 2097152;

    if (volumesize == 0)
      ret = -1;

#ifdef DEBUG
    Serial.print(F("Volume type is FAT"));
    Serial.println(volume.fatType(), DEC);
    Serial.print(F("Volume size (GB): "));
    Serial.println(volumesize);
#endif

    snprintf_P(sd_string + 5, sizeof(sd_string), PSTR("FAT%2d %02dGB"), volume.fatType(), int(volumesize));
  }

#ifdef DEBUG
  Serial.println(sd_string);
#endif

  return (ret);
}

FLASHMEM void check_and_create_directories(void) {
  if (sd_card > 0) {
    uint8_t i;
    char tmp[FILENAME_LEN];

#ifdef DEBUG
    Serial.println(F("Directory check... "));
#endif
    // create directories for banks
    for (i = 0; i < MAX_BANKS; i++) {
      snprintf_P(tmp, sizeof(tmp), PSTR("/%d"), i);
      if (!SD.exists(tmp)) {
#ifdef DEBUG
        Serial.print(F("Creating directory "));
        Serial.println(tmp);
#endif
        SD.mkdir(tmp);
      }
    }

    snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), PERFORMANCE_CONFIG_PATH);
    if (!SD.exists(tmp)) {
#ifdef DEBUG
      Serial.print(F("Creating directory "));
      Serial.println(tmp);
#endif
      SD.mkdir(tmp);
    }

    /*
        // create directories for configuration files
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), VOICE_CONFIG_PATH);
        if (!SD.exists(tmp))
        {
      #ifdef DEBUG
          Serial.print(F("Creating directory "));
          Serial.println(tmp);
      #endif
          SD.mkdir(tmp);
        }
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), PERFORMANCE_CONFIG_PATH);
        if (!SD.exists(tmp))
        {
      #ifdef DEBUG
          Serial.print(F("Creating directory "));
          Serial.println(tmp);
      #endif
          SD.mkdir(tmp);
        }
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), FX_CONFIG_PATH);
        if (!SD.exists(tmp))
        {
      #ifdef DEBUG
          Serial.print(F("Creating directory "));
          Serial.println(tmp);
      #endif
          SD.mkdir(tmp);
        }
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), DRUM_CONFIG_PATH);
        if (!SD.exists(tmp))
        {
      #ifdef DEBUG
          Serial.print(F("Creating directory "));
          Serial.println(tmp);
      #endif
          SD.mkdir(tmp);
        }
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), FAV_CONFIG_PATH);
        if (!SD.exists(tmp))
        {
      #ifdef DEBUG
          Serial.print(F("Creating directory "));
          Serial.println(tmp);
      #endif
          SD.mkdir(tmp);
        }
    */

    snprintf_P(tmp, sizeof(tmp), PSTR("/%s"), PERFORMANCE_CONFIG_PATH);
    if (!SD.exists(tmp)) {
#ifdef DEBUG
      Serial.print(F("Creating directory "));
      Serial.println(tmp);
#endif
      SD.mkdir(tmp);
    }

    //check if updated Fav-System is ready or if setup has to run once.
    snprintf_P(tmp, sizeof(tmp), PSTR("/%s/fav-v2"), FAV_CONFIG_PATH);
    if (!SD.exists(tmp)) {

      // Clear now obsolte marker files from Favs.
      // Only needs to run once.
      for (uint8_t i = 0; i < MAX_BANKS; i++) {
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/hasfav"), FAV_CONFIG_PATH, i);
#ifdef DEBUG
        Serial.print(F("Delete Marker File"));
        Serial.println(tmp);
#endif
        if (SD.exists(tmp))
          SD.remove(tmp);
      }
      // Remove empty Folders. rmdir will only remove strictly emtpy folders, which is the desired result.
      // Only needs to run once.
      for (uint8_t i = 0; i < MAX_BANKS; i++) {
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d"), FAV_CONFIG_PATH, i);
#ifdef DEBUG
        Serial.print(F("Delete empty folder "));
        Serial.println(tmp);
#endif
        if (SD.exists(tmp))
          SD.rmdir(tmp);
      }
      snprintf_P(tmp, sizeof(tmp), PSTR("/%s/fav-v2"), FAV_CONFIG_PATH);
      if (!SD.exists(tmp))
        SD.mkdir(tmp);  // Set Marker so that the Cleanup loops only run once.
    }
    /* #ifdef DEBUG
        else
          Serial.println(F("No SD card for directory check available."));
      #endif */
  }
}

/******************************************************************************
  DEBUG HELPER
******************************************************************************/
#if defined(DEBUG) && defined(SHOW_CPU_LOAD_MSEC)
void show_cpu_and_mem_usage(void) {
  uint32_t sum_xrun = 0;
  uint16_t sum_render_time_max = 0;

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    sum_xrun += MicroDexed[instance_id]->getXRun();
    sum_render_time_max += MicroDexed[instance_id]->getRenderTimeMax();
    MicroDexed[instance_id]->resetRenderTimeMax();
  }
  if (AudioProcessorUsageMax() > 99.9) {
    cpumax++;
#ifdef DEBUG
    Serial.print(F("*"));
#endif
  }
#ifdef DEBUG
  else
    Serial.print(F(" "));
  Serial.print(F("CPU:"));
  Serial.print(AudioProcessorUsage(), 2);
  Serial.print(F("%|CPUMAX:"));
  Serial.print(AudioProcessorUsageMax(), 2);
  Serial.print(F("%|CPUMAXCNT:"));
  Serial.print(cpumax, DEC);
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
  Serial.print(F("|CPUTEMP:"));
  Serial.print(tempmonGetTemp(), 2);
  Serial.print(F("C|MEM:"));
#else
  Serial.print(F("|MEM:"));
#endif
  Serial.print(AudioMemoryUsage(), DEC);
  Serial.print(F("|MEMMAX:"));
  Serial.print(AudioMemoryUsageMax(), DEC);
  Serial.print(F("|AUDIO_MEM_MAX:"));
  Serial.print(AUDIO_MEM, DEC);
  Serial.print(F("|RENDERTIMEMAX:"));
  Serial.print(sum_render_time_max, DEC);
  Serial.print(F("|XRUN:"));
  Serial.print(sum_xrun, DEC);
  Serial.print(F("|PEAKR:"));
  Serial.print(peak_r, DEC);
  Serial.print(F("|PEAKL:"));
  Serial.print(peak_l, DEC);
  Serial.print(F("|PEAKMD:"));
  Serial.print(peak_dexed, DEC);
  Serial.print(F("|ACTPEAKMD:"));
  Serial.print(peak_dexed_value, 1);
  Serial.print(F("|BLOCKSIZE:"));
  Serial.print(AUDIO_BLOCK_SAMPLES, DEC);
  Serial.print(F("|RAM:"));
  Serial.print(FreeMem(), DEC);

  Serial.print(F("|ACTVOICES:"));
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    Serial.print(instance_id, DEC);
    Serial.print(F("="));
    Serial.print(active_voices[instance_id], DEC);
    Serial.print(F("/"));
    Serial.print(MAX_NOTES / NUM_DEXED, DEC);
    if (instance_id != NUM_DEXED - 1)
      Serial.print(F(","));
  }
  Serial.println();
#endif
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
}
#endif

#ifdef DEBUG
void show_configuration(void) {
  Serial.println();
  Serial.println(F("CONFIGURATION:"));
  Serial.println(F("System"));
  Serial.print(F("  Volume              "));
  Serial.println(configuration.sys.vol, DEC);
  Serial.print(F("  Mono                "));
  Serial.println(configuration.sys.mono, DEC);
  Serial.print(F("  Soft MIDI Thru      "));
  Serial.println(configuration.sys.soft_midi_thru, DEC);
  Serial.print(F("  Favorites           "));
  Serial.println(configuration.sys.favorites, DEC);
  Serial.print(F("  Performance Number  "));
  Serial.println(configuration.sys.performance_number, DEC);
  Serial.print(F("  Load at startup     "));
  Serial.println(configuration.sys.load_at_startup, DEC);
  Serial.println(F("FX"));
  Serial.print(F("  Reverb Roomsize     "));
  Serial.println(configuration.fx.reverb_roomsize, DEC);
  Serial.print(F("  Reverb Level        "));
  Serial.println(configuration.fx.reverb_level, DEC);
#ifdef USE_PLATEREVERB
  Serial.print(F("  Reverb Lowpass      "));
  Serial.println(configuration.fx.reverb_lowpass, DEC);
  Serial.print(F("  Reverb Lodamp       "));
  Serial.println(configuration.fx.reverb_lodamp, DEC);
  Serial.print(F("  Reverb Hidamp       "));
  Serial.println(configuration.fx.reverb_hidamp, DEC);
  Serial.print(F("  Reverb Diffusion    "));
  Serial.println(configuration.fx.reverb_diffusion, DEC);
#else
  Serial.print(F("  Reverb Damping      "));
  Serial.println(configuration.fx.reverb_damping, DEC);
#endif

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++) {
    Serial.print(F("Dexed instance "));
    Serial.println(instance_id, DEC);
    Serial.print(F("  Bank                 "));
    Serial.println(configuration.dexed[instance_id].bank, DEC);
    Serial.print(F("  Voice                "));
    Serial.println(configuration.dexed[instance_id].voice, DEC);
    Serial.print(F("  MIDI-Channel         "));
    Serial.println(configuration.dexed[instance_id].midi_channel, DEC);
    Serial.print(F("  ENGINE               "));
    Serial.println(configuration.dexed[instance_id].engine, DEC);
    Serial.print(F("  Lowest Note          "));
    Serial.println(configuration.dexed[instance_id].lowest_note, DEC);
    Serial.print(F("  Highest Note         "));
    Serial.println(configuration.dexed[instance_id].highest_note, DEC);
    Serial.print(F("  Filter Cutoff        "));
    Serial.println(configuration.fx.filter_cutoff[instance_id], DEC);
    Serial.print(F("  Filter Resonance     "));
    Serial.println(configuration.fx.filter_resonance[instance_id], DEC);
    Serial.print(F("  Chorus Frequency     "));
    Serial.println(configuration.fx.chorus_frequency[instance_id], DEC);
    Serial.print(F("  Chorus Waveform      "));
    Serial.println(configuration.fx.chorus_waveform[instance_id], DEC);
    Serial.print(F("  Chorus Depth         "));
    Serial.println(configuration.fx.chorus_depth[instance_id], DEC);
    Serial.print(F("  Chorus Level         "));
    Serial.println(configuration.fx.chorus_level[instance_id], DEC);
    Serial.print(F("  Delay Time           "));
    Serial.println(configuration.fx.delay_time[instance_id], DEC);
    Serial.print(F("  Delay Feedback       "));
    Serial.println(configuration.fx.delay_feedback[instance_id], DEC);
    Serial.print(F("  Delay Level          "));
    Serial.println(configuration.fx.delay_level[instance_id], DEC);
    Serial.print(F("  Reverb Send          "));
    Serial.println(configuration.fx.reverb_send[instance_id], DEC);
    Serial.print(F("  Sound Intensity      "));
    Serial.println(configuration.dexed[instance_id].sound_intensity, DEC);
    Serial.print(F("  Panorama             "));
    Serial.println(configuration.dexed[instance_id].pan, DEC);
    Serial.print(F("  Transpose            "));
    Serial.println(configuration.dexed[instance_id].transpose, DEC);
    Serial.print(F("  Tune                 "));
    Serial.println(configuration.dexed[instance_id].tune, DEC);
    Serial.print(F("  Polyphony            "));
    Serial.println(configuration.dexed[instance_id].polyphony, DEC);
    Serial.print(F("  Mono/Poly            "));
    Serial.println(configuration.dexed[instance_id].monopoly, DEC);
    Serial.print(F("  Note Refresh         "));
    Serial.println(configuration.dexed[instance_id].note_refresh, DEC);
    Serial.print(F("  Pitchbend Range      "));
    Serial.println(configuration.dexed[instance_id].pb_range, DEC);
    Serial.print(F("  Pitchbend Step       "));
    Serial.println(configuration.dexed[instance_id].pb_step, DEC);
    Serial.print(F("  Modwheel Range       "));
    Serial.println(configuration.dexed[instance_id].mw_range, DEC);
    Serial.print(F("  Modwheel Assign      "));
    Serial.println(configuration.dexed[instance_id].mw_assign, DEC);
    Serial.print(F("  Modwheel Mode        "));
    Serial.println(configuration.dexed[instance_id].mw_mode, DEC);
    Serial.print(F("  Footctrl Range       "));
    Serial.println(configuration.dexed[instance_id].fc_range, DEC);
    Serial.print(F("  Footctrl Assign      "));
    Serial.println(configuration.dexed[instance_id].fc_assign, DEC);
    Serial.print(F("  Footctrl Mode        "));
    Serial.println(configuration.dexed[instance_id].fc_mode, DEC);
    Serial.print(F("  BreathCtrl Range     "));
    Serial.println(configuration.dexed[instance_id].bc_range, DEC);
    Serial.print(F("  Breathctrl Assign    "));
    Serial.println(configuration.dexed[instance_id].bc_assign, DEC);
    Serial.print(F("  Breathctrl Mode      "));
    Serial.println(configuration.dexed[instance_id].bc_mode, DEC);
    Serial.print(F("  Aftertouch Range     "));
    Serial.println(configuration.dexed[instance_id].at_range, DEC);
    Serial.print(F("  Aftertouch Assign    "));
    Serial.println(configuration.dexed[instance_id].at_assign, DEC);
    Serial.print(F("  Aftertouch Mode      "));
    Serial.println(configuration.dexed[instance_id].at_mode, DEC);
    Serial.print(F("  Portamento Mode      "));
    Serial.println(configuration.dexed[instance_id].portamento_mode, DEC);
    Serial.print(F("  Portamento Glissando "));
    Serial.println(configuration.dexed[instance_id].portamento_glissando, DEC);
    Serial.print(F("  Portamento Time      "));
    Serial.println(configuration.dexed[instance_id].portamento_time, DEC);
    Serial.print(F("  OP Enabled           "));
    Serial.println(configuration.dexed[instance_id].op_enabled, DEC);
    Serial.flush();
  }

  Serial.println();
  Serial.flush();
}

void show_patch(uint8_t instance_id) {
  char vn[VOICE_NAME_LEN];

  Serial.print(F("INSTANCE "));
  Serial.println(instance_id, DEC);

  memset(vn, 0, sizeof(vn));
  Serial.println(F("+==========================================================================================================+"));
  for (int8_t i = 5; i >= 0; --i) {
    Serial.println(F("+==========================================================================================================+"));
    Serial.print(F("| OP"));
    Serial.print(6 - i, DEC);
    Serial.println(F("                                                                                                      |"));
    Serial.println(F("+======+======+======+======+======+======+======+======+================+================+================+"));
    Serial.println(F("|  R1  |  R2  |  R3  |  R4  |  L1  |  L2  |  L3  |  L4  | LEV_SCL_BRK_PT | SCL_LEFT_DEPTH | SCL_RGHT_DEPTH |"));
    Serial.println(F("+------+------+------+------+------+------+------+------+----------------+----------------+----------------+"));
    Serial.print(F("| "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_R1));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_R2));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_R3));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_R4));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_L1));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_L2));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_L3));
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_EG_L4));
    Serial.print(F("  |           "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_LEV_SCL_BRK_PT));
    Serial.print(F("  |           "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_SCL_LEFT_DEPTH));
    Serial.print(F("  |           "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_SCL_RGHT_DEPTH));
    Serial.println(F("  |"));
    Serial.println(F("+======+======+======+======+======+===+==+==+===+======+====+========+==+====+=======+===+================+"));
    Serial.println(F("| SCL_L_CURVE | SCL_R_CURVE | RT_SCALE | AMS | KVS | OUT_LEV | OP_MOD | FRQ_C | FRQ_F | DETUNE             |"));
    Serial.println(F("+-------------+-------------+----------+-----+-----+---------+--------+-------+-------+--------------------+"));
    Serial.print(F("|        "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_SCL_LEFT_CURVE));
    Serial.print(F("  |        "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_SCL_RGHT_CURVE));
    Serial.print(F("  |     "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_OSC_RATE_SCALE));
    Serial.print(F("  |"));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_AMP_MOD_SENS));
    Serial.print(F("  |"));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_KEY_VEL_SENS));
    Serial.print(F("  |    "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_OUTPUT_LEV));
    Serial.print(F("  |   "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_OSC_MODE));
    Serial.print(F("  |  "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_FREQ_COARSE));
    Serial.print(F("  |  "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_FREQ_FINE));
    Serial.print(F("  |               "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement((i * 21) + DEXED_OP_OSC_DETUNE));
    Serial.println(F("  |"));
  }
  Serial.println(F("+=======+=====+=+=======+===+===+======++====+==+==+====+====+==+======+======+=====+=+====================+"));
  Serial.println(F("|  PR1  |  PR2  |  PR3  |  PR4  |  PL1  |  PL2  |  PL3  |  PL4  | ALG  |  FB  | OKS | TRANSPOSE            |"));
  Serial.println(F("+-------+-------+-------+-------+-------+-------+-------+-------+------+------+-----+----------------------+"));
  Serial.print(F("|  "));
  for (int8_t i = 0; i < 8; i++) {
    SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + i));
    Serial.print(F("  |  "));
  }
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_ALGORITHM));
  Serial.print(F(" | "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_FEEDBACK));
  Serial.print(F("  |"));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC));
  Serial.print(F("  |                 "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_TRANSPOSE));
  Serial.println(F("  |"));
  Serial.println(F("+=======+=+=====+===+===+=====+=+=======+=======+==+====+=====+=+======++=====+=====+======================+"));
  Serial.println(F("| LFO SPD | LFO DLY | LFO PMD | LFO AMD | LFO SYNC | LFO WAVE | LFO PMS | NAME                             |"));
  Serial.println(F("+---------+---------+---------+---------+----------+----------+---------+----------------------------------+"));
  Serial.print(F("|    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_SPEED));
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_DELAY));
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_DEP));
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_AMP_MOD_DEP));
  Serial.print(F("  |     "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_SYNC));
  Serial.print(F("  |     "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_WAVE));
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->getVoiceDataElement(DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS));
  Serial.print(F("  | "));
  MicroDexed[instance_id]->getName(vn);
  Serial.print(vn);
  Serial.println(F("                       |"));
  Serial.println(F("+=========+=========+=========+=========+==========+==========+=========+==================================+"));
  Serial.println(F("+==========================================================================================================+"));
}

void SerialPrintFormatInt3(uint8_t num) {
  char buf[4];
  memset(buf, 0, 4);
  snprintf_P(buf, sizeof(buf), PSTR("%3d"), num);
  Serial.print(buf);
}

const char* byte_to_binary(int x) {
  static char b[9];
  b[0] = '\0';

  int z;
  for (z = 128; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

#if defined(ARDUINO_TEENSY36)
/* From: https://forum.pjrc.com/threads/33443-How-to-display-free-ram */
extern "C" char* sbrk(int incr);
uint32_t FreeMem(void) {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#else
/* From: https://forum.pjrc.com/threads/33443-How-to-display-free-ram */
extern unsigned long _heap_end;
extern char* __brkval;
int FreeMem(void) {
  return (char*)&_heap_end - __brkval;
}
#endif
#endif
