/*
   MicroDexed

   MicroDexed is a port of the Dexed sound engine
   Dexed ist heavily based on https://github.com/google/music-synthesizer-for-android

   (c)2018-2021 H. Wirtz <wirtz@parasitstudio.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include "midi_devices.hpp"
#include "dexed.h"
#include "dexed_sd.h"
#include "effect_modulated_delay.h"
#include "effect_stereo_mono.h"
#include "effect_mono_stereo.h"
#ifdef USE_PLATEREVERB
#include "effect_platervbstereo.h"
#else
#include "effect_freeverbf.h"
#endif
#include "PluginFx.h"
#include "UI.hpp"
#include "source_microdexed.h"

#include "ILI9341_t3.h"
#include <XPT2046_Touchscreen.h>
#include "font_Arial.h"
#define CS_PIN      14   // 8
#define TFT_DC      9    // 15
#define TFT_CS      15   // 22
#define TFT_RST     255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12
XPT2046_Touchscreen ts(CS_PIN);
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

// Audio engines
AudioSourceMicroDexed*     MicroDexed[NUM_DEXED];
#if defined(USE_FX)
AudioSynthWaveform*        chorus_modulator[NUM_DEXED];
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
AudioFilterBiquad*         modchorus_filter[NUM_DEXED];
#endif
AudioEffectModulatedDelay* modchorus[NUM_DEXED];
AudioMixer4*               chorus_mixer[NUM_DEXED];
AudioMixer4*               delay_fb_mixer[NUM_DEXED];
AudioEffectDelay*          delay_fx[NUM_DEXED];
AudioMixer4*               delay_mixer[NUM_DEXED];
#endif
AudioEffectMonoStereo*     mono2stereo[NUM_DEXED];

AudioMixer4                microdexed_peak_mixer;
AudioAnalyzePeak           microdexed_peak;
#if defined(USE_FX)
AudioMixer4                reverb_mixer_r;
AudioMixer4                reverb_mixer_l;
#ifdef USE_PLATEREVERB
AudioEffectPlateReverb     reverb;
#else
AudioEffectFreeverb        freeverb_r;
AudioEffectFreeverb        freeverb_l;
#endif
#endif
AudioMixer4                master_mixer_r;
AudioMixer4                master_mixer_l;
AudioAmplifier             volume_r;
AudioAmplifier             volume_l;
AudioEffectStereoMono      stereo2mono;
AudioAnalyzePeak           master_peak_r;
AudioAnalyzePeak           master_peak_l;
#if defined(ANTIALIAS_FRQ)
AudioFilterBiquad          antialias_r;
AudioFilterBiquad          antialias_l;
#endif
#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
AudioMixer4                audio_thru_mixer_r;
AudioMixer4                audio_thru_mixer_l;
#endif

// Audio chain tail
#if defined(USE_FX)
#ifdef USE_PLATEREVERB
AudioConnection          patchCord0(reverb_mixer_r, 0, reverb, 0);
AudioConnection          patchCord1(reverb_mixer_l, 0, reverb, 1);
AudioConnection          patchCord2(reverb, 0, master_mixer_r, 3);
AudioConnection          patchCord3(reverb, 1, master_mixer_l, 3);
#else
AudioConnection          patchCord0(reverb_mixer_r, freeverb_r);
AudioConnection          patchCord1(reverb_mixer_l, freeverb_l);
AudioConnection          patchCord2(freeverb_r, 0, master_mixer_r, 3);
AudioConnection          patchCord3(freeverb_l, 0, master_mixer_l, 3);
#endif
#endif
#if defined(ANTIALIAS_FRQ)
AudioConnection          patchCord6(master_mixer_r, antialias_r);
AudioConnection          patchCord7(master_mixer_l, antialias_l);
AudioConnection          patchCord8(antialias_r, volume_r);
AudioConnection          patchCord9(antialias_l, volume_l);
#else
AudioConnection          patchCord6(master_mixer_r, volume_r);
AudioConnection          patchCord7(master_mixer_l, volume_l);
#endif
AudioConnection          patchCord10(volume_r, 0, stereo2mono, 0);
AudioConnection          patchCord11(volume_l, 0, stereo2mono, 1);
AudioConnection          patchCord12(stereo2mono, 0, master_peak_r, 0);
AudioConnection          patchCord13(stereo2mono, 0, master_peak_l, 0);

// Outputs
#if defined(TEENSY_AUDIO_BOARD)
AudioOutputI2S           i2s1;
#ifndef SGTL5000_AUDIO_THRU
AudioConnection          patchCord14(stereo2mono, 0, i2s1, 0);
AudioConnection          patchCord15(stereo2mono, 1, i2s1, 1);
#endif
AudioControlSGTL5000     sgtl5000_1;
#elif defined (I2S_AUDIO_ONLY)
AudioOutputI2S           i2s1;
AudioConnection          patchCord14(stereo2mono, 0, i2s1, 0);
AudioConnection          patchCord15(stereo2mono, 1, i2s1, 1);
#elif defined(TGA_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioConnection          patchCord14(stereo2mono, 0, i2s1, 0);
AudioConnection          patchCord15(stereo2mono, 1, i2s1, 1);
AudioControlWM8731master wm8731_1;
#elif defined(PT8211_AUDIO)
AudioOutputPT8211        pt8211_1;
AudioConnection          patchCord14(stereo2mono, 0, pt8211_1, 0);
AudioConnection          patchCord15(stereo2mono, 1, pt8211_1, 1);
#elif defined(TEENSY_DAC_SYMMETRIC)
AudioOutputAnalogStereo  dacOut;
AudioMixer4              invMixer;
AudioConnection          patchCord14(stereo2mono, 0, dacOut  , 0);
AudioConnection          patchCord15(stereo2mono, 1, invMixer, 0);
AudioConnection          patchCord16(invMixer, 0, dacOut  , 1);
#elif defined(TEENSY_DAC)
AudioOutputAnalogStereo  dacOut;
AudioConnection          patchCord14(stereo2mono, 0, dacOut, 0);
AudioConnection          patchCord15(stereo2mono, 1, dacOut, 1);
#endif
#ifdef AUDIO_DEVICE_USB
AudioOutputUSB           usb1;
AudioConnection          patchCord17(stereo2mono, 0, usb1, 0);
AudioConnection          patchCord18(stereo2mono, 1, usb1, 1);
#endif

#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
AudioInputI2S            i2s1in;
AudioConnection          patchCord19(stereo2mono, 0, audio_thru_mixer_r, 0);
AudioConnection          patchCord20(stereo2mono, 1, audio_thru_mixer_l, 0);
AudioConnection          patchCord21(i2s1in, 0, audio_thru_mixer_r, 1);
AudioConnection          patchCord22(i2s1in, 1, audio_thru_mixer_l, 1);
AudioConnection          patchCord23(audio_thru_mixer_r, 0, i2s1, 0);
AudioConnection          patchCord24(audio_thru_mixer_l, 0, i2s1, 1);
#endif

//
// Dynamic patching of MicroDexed objects
//
uint16_t nDynamic = 0;
#if defined(USE_FX) && MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
AudioConnection * dynamicConnections[NUM_DEXED * 16];
#elif defined(USE_FX) && MOD_FILTER_OUTPUT == MOD_NO_FILTER_OUTPUT
AudioConnection * dynamicConnections[NUM_DEXED * 15];
#else
AudioConnection * dynamicConnections[NUM_DEXED * 4];
#endif
void create_audio_engine_chain(uint8_t instance_id)
{
  MicroDexed[instance_id] = new AudioSourceMicroDexed(SAMPLE_RATE);
  mono2stereo[instance_id] = new AudioEffectMonoStereo();
#if defined(USE_FX)
  chorus_modulator[instance_id] = new AudioSynthWaveform();
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
  modchorus_filter[instance_id] = new AudioFilterBiquad();
#endif
  modchorus[instance_id] = new AudioEffectModulatedDelay();
  chorus_mixer[instance_id] = new AudioMixer4();
  delay_fb_mixer[instance_id] = new AudioMixer4();
  delay_fx[instance_id] = new AudioEffectDelay();
  delay_mixer[instance_id] = new AudioMixer4();
#endif

  dynamicConnections[nDynamic++] = new AudioConnection(*MicroDexed[instance_id], 0, microdexed_peak_mixer, instance_id);
#if defined(USE_FX)
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
#else
  dynamicConnections[nDynamic++] = new AudioConnection(*MicroDexed[instance_id], 0, *mono2stereo[instance_id], 0);
#endif
  dynamicConnections[nDynamic++] = new AudioConnection(*mono2stereo[instance_id], 0, master_mixer_r, instance_id);
  dynamicConnections[nDynamic++] = new AudioConnection(*mono2stereo[instance_id], 1, master_mixer_l, instance_id);
}

uint8_t sd_card = 0;
Sd2Card card;
SdVolume volume;
uint8_t midi_timing_counter = 0; // 24 per qarter
elapsedMillis midi_timing_timestep;
uint16_t midi_timing_quarter = 0;
elapsedMillis long_button_pressed;
elapsedMillis control_rate;
uint8_t active_voices[NUM_DEXED];
uint8_t midi_voices[NUM_DEXED];
#ifdef SHOW_CPU_LOAD_MSEC
elapsedMillis cpu_mem_millis;
#endif
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
#ifdef TEENSY4
#if NUM_DEXED>1
int8_t midi_decay[NUM_DEXED] = { -1, -1};
#else
int8_t midi_decay[NUM_DEXED] = { -1};
#endif
elapsedMillis midi_decay_timer;
#endif

#if defined(USE_FX)
// Allocate the delay lines for chorus
int16_t delayline[NUM_DEXED][MOD_DELAY_SAMPLE_BUFFER];
#endif

#ifdef ENABLE_LCD_UI
extern LCDMenuLib2 LCDML;
#endif

extern void getNoteName(char* noteName, uint8_t noteNumber);

/***********************************************************************
   SETUP
 ***********************************************************************/
void setup()
{
  // Start audio system
  //AudioNoInterrupts();
  AudioMemory(AUDIO_MEM);

#ifdef DISPLAY_LCD_SPI
  pinMode(SDCARD_CS_PIN, OUTPUT);
  pinMode(U8X8_CS_PIN, OUTPUT);
#endif

#ifdef ENABLE_LCD_UI
  setup_ui();
#endif
#ifdef DEBUG
#ifdef ENABLE_LCD_UI
  setup_debug_message();
#endif
  Serial.begin(SERIAL_SPEED);
#endif
#ifndef ENABLE_LCD_UI
#ifdef DEBUG
  Serial.println(F("NO LCD DISPLAY ENABLED!"));
#endif
#endif


#ifdef DEBUG
  generate_version_string(version_string, sizeof(version_string));

  Serial.println(F("MicroDexed based on https://github.com/asb2m10/dexed"));
  Serial.println(F("(c)2018-2021 H. Wirtz <wirtz@parasitstudio.de>"));
  Serial.println(F("https://codeberg.org/dcoredump/MicroDexed"));
  Serial.print(F("Version: "));
  Serial.println(version_string);
  Serial.print(F("CPU-Speed: "));
  Serial.print(F_CPU / 1000000.0, 1);
  Serial.print(F(" MHz / "));
  Serial.print(NUM_DEXED, DEC);
  Serial.print(F(" Instances with "));
  Serial.print(MAX_NOTES);
  Serial.println(F(" notes for each instance"));
  Serial.println(F("<setup start>"));
  Serial.flush();
#endif

  setup_midi_devices();

#if defined(TEENSY_AUDIO_BOARD)
  sgtl5000_1.enable();
  sgtl5000_1.lineOutLevel(SGTL5000_LINEOUT_LEVEL);
  sgtl5000_1.dacVolumeRamp();
  sgtl5000_1.dacVolume(1.0);
  //sgtl5000_1.dacVolumeRampLinear();
  //sgtl5000_1.dacVolumeRampDisable();
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.unmuteLineout();
  sgtl5000_1.volume(SGTL5000_HEADPHONE_VOLUME, SGTL5000_HEADPHONE_VOLUME); // Headphone volume
#ifdef SGTL5000_AUDIO_THRU
  //sgtl5000_1.audioPreProcessorEnable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(5);
  //sgtl5000_1.adcHighPassFilterEnable();
#endif
#ifdef SGTL5000_AUDIO_ENHANCE
  sgtl5000_1.audioPostProcessorEnable();
  sgtl5000_1.enhanceBassEnable();
  sgtl5000_1.enhanceBass(1.0, 1.5, 0, 5); // enhanceBass(1.0, 1.0, 1, 2); // Configures the bass enhancement by setting the levels of the original stereo signal and the bass-enhanced mono level which will be mixed together. The high-pass filter may be enabled (0) or bypassed (1).
  sgtl5000_1.surroundSoundEnable();
  sgtl5000_1.surroundSound(7, 3); // Configures virtual surround width from 0 (mono) to 7 (widest). select may be set to 1 (disable), 2 (mono input) or 3 (stereo input).
  sgtl5000_1.autoVolumeEnable();
  sgtl5000_1.autoVolumeControl(1, 1, 1, 0.9, 0.01, 0.05);
  sgtl5000_1.eqSelect(2); // Tone Control
  sgtl5000_1.eqBands(EQ_BASS_DEFAULT, EQ_TREBLE_DEFAULT);
#else
  sgtl5000_1.audioProcessorDisable();
  sgtl5000_1.autoVolumeDisable();
  sgtl5000_1.surroundSoundDisable();
  sgtl5000_1.enhanceBassDisable();
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
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
#ifdef DEBUG
    Serial.print(F("Creating MicroDexed instance "));
    Serial.println(instance_id, DEC);
#endif
    create_audio_engine_chain(instance_id);

  }

#if defined(USE_FX)
  // Init effects
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    memset(delayline[instance_id], 0, sizeof(delayline[instance_id]));
    if (!modchorus[instance_id]->begin(delayline[instance_id], MOD_DELAY_SAMPLE_BUFFER)) {
#ifdef DEBUG
      Serial.print(F("AudioEffectModulatedDelay - begin failed ["));
      Serial.print(instance_id);
      Serial.println(F("]"));
#endif
      while (1);
    }
  }
#ifdef DEBUG
  Serial.print(F("MOD_DELAY_SAMPLE_BUFFER="));
  Serial.print(MOD_DELAY_SAMPLE_BUFFER, DEC);
  Serial.println(F(" samples"));
#endif
#endif

#if defined(ANTIALIAS_FRQ)
  antialias_r.setLowpass(0, ANTIALIAS_FRQ, 0.54);
  antialias_r.setLowpass(1, ANTIALIAS_FRQ, 1.3);
  antialias_r.setLowpass(2, ANTIALIAS_FRQ, 0.54);
  antialias_r.setLowpass(3, ANTIALIAS_FRQ, 1.3);
  antialias_l.setLowpass(0, ANTIALIAS_FRQ, 0.54);
  antialias_l.setLowpass(1, ANTIALIAS_FRQ, 1.3);
  antialias_l.setLowpass(2, ANTIALIAS_FRQ, 0.54);
  antialias_l.setLowpass(3, ANTIALIAS_FRQ, 1.3);
#endif

  initial_values_from_eeprom(false);

  // start SD card
#ifdef DISPLAY_LCD_SPI
  change_disp_sd(false);
#endif
  sd_card = check_sd_cards();

  if (sd_card < 1)
  {
#ifdef DEBUG
    Serial.println(F("SD card not accessable."));
#endif
  }
  else
  {
    check_and_create_directories();

    for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
    {
      // load default SYSEX data
      load_sd_voice(configuration.performance.bank[instance_id], configuration.performance.voice[instance_id], instance_id);
      memset(g_voice_name[instance_id], 0, VOICE_NAME_LEN);
      memset(g_bank_name[instance_id], 0, BANK_NAME_LEN);
      memset(receive_bank_filename, 0, FILENAME_LEN);
    }
  }

#ifdef DISPLAY_LCD_SPI
  change_disp_sd(true);
#endif

  tft.begin();
  tft.setRotation(3);
  ts.begin();
  ts.setRotation(3);
// Note: you can now set the SPI speed to any value
// the default value is 30Mhz, but most ILI9341 displays
// can handle at least 60Mhz and as much as 100Mhz
//tft.setClock(10000000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  //if (ts.touched()) {tft.println("Hello...");} 


  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

#ifdef DEBUG
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    Serial.print(F("Dexed instance "));
    Serial.print(instance_id);
    Serial.println(F(":"));
    Serial.print(F("Bank/Voice from EEPROM ["));
    Serial.print(configuration.performance.bank[instance_id], DEC);
    Serial.print(F("/"));
    Serial.print(configuration.performance.voice[instance_id], DEC);
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

#if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
  show_cpu_and_mem_usage();
#endif

  // init master_mixer
#if NUM_DEXED > 1
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    master_mixer_r.gain(instance_id, 1.0);
    master_mixer_l.gain(instance_id, 1.0);
  }
#else
  master_mixer_r.gain(0, 1.0);
  master_mixer_l.gain(0, 1.0);
  master_mixer_r.gain(1, 0.0);
  master_mixer_l.gain(1, 0.0);
#endif
  master_mixer_r.gain(2, 0.0);
  master_mixer_l.gain(2, 0.0);
  master_mixer_r.gain(3, 0.0);
  master_mixer_l.gain(3, 0.0);

#if defined(TEENSY_AUDIO_BOARD) && defined(SGTL5000_AUDIO_THRU)
  audio_thru_mixer_r.gain(0, 1.0); // MD signal sum
  audio_thru_mixer_l.gain(0, 1.0); // MD signal sum
#ifdef TEENSY_AUDIO_BOARD
  audio_thru_mixer_r.gain(1, 1.0); // I2S nput
  audio_thru_mixer_l.gain(1, 1.0); // I2S input
#else
  audio_thru_mixer_r.gain(1, 0.0);
  audio_thru_mixer_l.gain(1, 0.0);
#endif
  audio_thru_mixer_r.gain(2, 0.0);
  audio_thru_mixer_l.gain(2, 0.0);
  audio_thru_mixer_r.gain(3, 0.0);
  audio_thru_mixer_l.gain(3, 0.0);
#endif

  //AudioInterrupts();

#ifdef DEBUG
  Serial.println(F("<setup end>"));
#endif

  LCDML.OTHER_jumpToFunc(UI_func_voice_select);
}

void loop()
{
  if (ts.touched()) {tft.println("Hello...");} 
  
  // MIDI input handling
  check_midi_devices();

  // check encoder
  ENCODER[ENC_L].update();
  ENCODER[ENC_R].update();

#ifdef ENABLE_LCD_UI
  LCDML.loop();
#endif

  // CONTROL-RATE-EVENT-HANDLING
  if (control_rate > CONTROL_RATE_MS)
  {
    control_rate = 0;

    // check for value changes, unused voices and CPU overload
    for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
    {
      active_voices[instance_id] = MicroDexed[instance_id]->getNumNotesPlaying();
      if (active_voices[instance_id] == 0)
        midi_voices[instance_id] = 0;
    }

    if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
    {
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
      {
#ifdef TEENSY4
        if (midi_decay_timer > MIDI_DECAY_TIMER && midi_decay[instance_id] > 0)
        {
          midi_decay[instance_id]--;
          lcd.createChar(6 + instance_id, (uint8_t*)special_chars[15 - (7 - midi_decay[instance_id])]);
          lcd.setCursor(14 + instance_id, 1);
          lcd.write(6 + instance_id);
        }
        else if (midi_voices[instance_id] == 0 && midi_decay[instance_id] == 0 && !MicroDexed[instance_id]->getSustain())
        {
          midi_decay[instance_id]--;
          lcd.setCursor(14 + instance_id, 1);
          lcd.write(20); // blank
        }
#else
        static bool midi_playing[NUM_DEXED];
        if (midi_voices[instance_id] > 0 && midi_playing[instance_id] == false)
        {
          midi_playing[instance_id] = true;
          lcd.setCursor(14 + instance_id, 1);
          lcd.write(6 + instance_id);
        }
        else if (midi_voices[instance_id] == 0 && !MicroDexed[instance_id]->getSustain())
        {
          midi_playing[instance_id] = false;
          lcd.setCursor(14 + instance_id, 1);
          lcd.write(20); // blank
        }
#endif
      }
#ifdef TEENSY4
      if (midi_decay_timer > 250)
      {
        midi_decay_timer = 0;
      }
#endif
    }
  }
  else
    yield();

#if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
  if (cpu_mem_millis >= SHOW_CPU_LOAD_MSEC)
  {
    if (master_peak_r.available())
      if (master_peak_r.read() == 1.0)
        peak_r++;
    if (master_peak_l.available())
      if (master_peak_l.read() == 1.0)
        peak_l++;
    if (microdexed_peak.available())
    {
      peak_dexed_value = microdexed_peak.read();
      if (peak_dexed_value > 0.99)
        peak_dexed++;
    }
    cpu_mem_millis -= SHOW_CPU_LOAD_MSEC;
    show_cpu_and_mem_usage();
  }
#endif
}

/******************************************************************************
   MIDI MESSAGE HANDLER
 ******************************************************************************/
void handleNoteOn(byte inChannel, byte inNumber, byte inVelocity)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (checkMidiChannel(inChannel, instance_id))
    {
      if (inNumber >= configuration.dexed[instance_id].lowest_note && inNumber <= configuration.dexed[instance_id].highest_note)
      {
        if (configuration.dexed[instance_id].polyphony > 0)
          MicroDexed[instance_id]->keydown(inNumber, uint8_t(float(configuration.dexed[instance_id].velocity_level / 127.0)*inVelocity + 0.5));

        midi_voices[instance_id]++;
#ifdef TEENSY4
        if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
        {
          midi_decay_timer = 0;
          midi_decay[instance_id] = min(inVelocity / 5, 7);
        }
#endif
#ifdef DEBUG
        char note_name[4];
        getNoteName(note_name, inNumber);
        Serial.print(F("Keydown "));
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
}

void handleNoteOff(byte inChannel, byte inNumber, byte inVelocity)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (checkMidiChannel(inChannel, instance_id))
    {
      if (inNumber >= configuration.dexed[instance_id].lowest_note && inNumber <= configuration.dexed[instance_id].highest_note)
      {
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
}

void handleControlChange(byte inChannel, byte inCtrl, byte inValue)
{
  inCtrl = constrain(inCtrl, 0, 127);
  inValue = constrain(inValue, 0, 127);

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (checkMidiChannel(inChannel, instance_id))
    {
#ifdef DEBUG
      Serial.print(F("INSTANCE "));
      Serial.print(instance_id, DEC);
      Serial.print(F(": CC#"));
      Serial.print(inCtrl, DEC);
      Serial.print(F(":"));
      Serial.println(inValue, DEC);
#endif

      switch (inCtrl) {
        case 0: // BankSelect MSB
#ifdef DEBUG
          Serial.println(F("BANK-SELECT MSB CC"));
#endif
          configuration.performance.bank[instance_id] = constrain((inValue << 7)&configuration.performance.bank[instance_id], 0, MAX_BANKS - 1);
          /* load_sd_voice(configuration.performance.bank[instance_id], configuration.performance.voice[instance_id], instance_id);
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
          MicroDexed[instance_id]->controllers.modwheel_cc = inValue;
          MicroDexed[instance_id]->controllers.refresh();
          break;
        case 2:
#ifdef DEBUG
          Serial.println(F("BREATH CC"));
#endif
          MicroDexed[instance_id]->controllers.breath_cc = inValue;
          MicroDexed[instance_id]->controllers.refresh();
          break;
        case 4:
#ifdef DEBUG
          Serial.println(F("FOOT CC"));
#endif
          MicroDexed[instance_id]->controllers.foot_cc = inValue;
          MicroDexed[instance_id]->controllers.refresh();
          break;
        case 5: // Portamento time
          configuration.dexed[instance_id].portamento_time = inValue;
          MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode, configuration.dexed[instance_id].portamento_glissando, configuration.dexed[instance_id].portamento_time);
          break;
        case 7: // Instance Volume
#ifdef DEBUG
          Serial.println(F("VOLUME CC"));
#endif
          configuration.dexed[instance_id].sound_intensity = map(inValue, 0, 0x7f, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX);
          MicroDexed[instance_id]->fx.Gain = pseudo_log_curve(mapfloat(configuration.dexed[instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 0.0, SOUND_INTENSITY_AMP_MAX));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_sound_intensity))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 10: // Pan
#ifdef DEBUG
          Serial.println(F("PANORAMA CC"));
#endif
          configuration.dexed[instance_id].pan = map(inValue, 0, 0x7f, PANORAMA_MIN, PANORAMA_MAX);
          mono2stereo[instance_id]->panorama(mapfloat(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_panorama))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 32: // BankSelect LSB
#ifdef DEBUG
          Serial.println(F("BANK-SELECT LSB CC"));
#endif
          configuration.performance.bank[instance_id] = constrain(inValue, 0, MAX_BANKS - 1);
          /*load_sd_voice(configuration.performance.bank[instance_id], configuration.performance.voice[instance_id], instance_id);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
            {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
            }*/
          break;
        case 64:
          MicroDexed[instance_id]->setSustain(inValue > 63);
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
          break;
        case 65:
          MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode, configuration.dexed[instance_id].portamento_glissando, configuration.dexed[instance_id].portamento_time);
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_mode))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 94:  // CC 94: (de)tune
          configuration.dexed[selected_instance_id].tune = map(inValue, 0, 0x7f, TUNE_MIN, TUNE_MAX);
          MicroDexed[selected_instance_id]->controllers.masterTune = (int((configuration.dexed[selected_instance_id].tune - 100) / 100.0 * 0x4000) << 11) * (1.0 / 12);
          MicroDexed[selected_instance_id]->doRefreshVoice();
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_tune))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
#if defined(USE_FX)
        case 91:  // CC 91: reverb send
          configuration.fx.reverb_send[selected_instance_id] = map(inValue, 0, 0x7f, REVERB_SEND_MIN, REVERB_SEND_MAX);
          reverb_mixer_r.gain(selected_instance_id, pseudo_log_curve(mapfloat(configuration.fx.reverb_send[selected_instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, 1.0)));
          reverb_mixer_l.gain(selected_instance_id, pseudo_log_curve(mapfloat(configuration.fx.reverb_send[selected_instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, 1.0)));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_reverb_send))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 93:  // CC 93: chorus level
          configuration.fx.chorus_level[selected_instance_id] = map(inValue, 0, 0x7f, CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX);
          chorus_mixer[selected_instance_id]->gain(1, mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_chorus_level))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 103:  // CC 103: filter resonance
          configuration.fx.filter_resonance[instance_id] = map(inValue, 0, 0x7f, FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX);
          MicroDexed[instance_id]->fx.Reso = mapfloat(configuration.fx.filter_resonance[instance_id], FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 1.0, 0.0);
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_filter_resonance))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 104:  // CC 104: filter cutoff
          configuration.fx.filter_cutoff[instance_id] = map(inValue, 0, 0x7f, FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX);
          MicroDexed[instance_id]->fx.Cutoff = mapfloat(configuration.fx.filter_cutoff[instance_id], FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 1.0, 0.0);
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_filter_cutoff))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 105:  // CC 105: delay time
          configuration.fx.delay_time[instance_id] = map(inValue, 0, 0x7f, DELAY_TIME_MIN, DELAY_TIME_MAX);
          delay_fx[instance_id]->delay(0, constrain(configuration.fx.delay_time[instance_id] * 10, DELAY_TIME_MIN * 10, DELAY_TIME_MAX * 10));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_delay_time))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 106:  // CC 106: delay feedback
          configuration.fx.delay_feedback[instance_id] = map(inValue, 0, 0x7f, DELAY_FEEDBACK_MIN , DELAY_FEEDBACK_MAX);
          delay_fb_mixer[instance_id]->gain(1, pseudo_log_curve(mapfloat(configuration.fx.delay_feedback[instance_id], DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 0.0, 1.0))); // amount of feedback
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_delay_feedback))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
        case 107:  // CC 107: delay volume
          configuration.fx.delay_level[instance_id] = map(inValue, 0, 0x7f, DELAY_LEVEL_MIN, DELAY_LEVEL_MAX);
          delay_mixer[instance_id]->gain(1, pseudo_log_curve(mapfloat(configuration.fx.delay_level[instance_id], DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 0.0, 1.0)));
          if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_delay_level))
          {
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
          break;
#endif
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

void handleAfterTouch(byte inChannel, byte inPressure)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (checkMidiChannel(inChannel, instance_id))
    {
      MicroDexed[instance_id]->controllers.aftertouch_cc = inPressure;
      MicroDexed[instance_id]->controllers.refresh();
    }
  }
}

void handlePitchBend(byte inChannel, int inPitch)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (checkMidiChannel(inChannel, instance_id))
    {
      MicroDexed[instance_id]->controllers.values_[kControllerPitch] = inPitch + 0x2000; // -8192 to +8191 --> 0 to 16383
    }
  }
}

void handleProgramChange(byte inChannel, byte inProgram)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (checkMidiChannel(inChannel, instance_id))
    {
      configuration.performance.voice[instance_id] = constrain(inProgram, 0, MAX_VOICES - 1);
#ifdef DISPLAY_LCD_SPI
      change_disp_sd(false);
#endif
      load_sd_voice(configuration.performance.bank[instance_id], configuration.performance.voice[instance_id], instance_id);
#ifdef DISPLAY_LCD_SPI
      change_disp_sd(true);
#endif
      if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
      {
        LCDML.OTHER_updateFunc();
        LCDML.loop_menu();
      }
    }
  }
}

void handleSystemExclusive(byte * sysex, uint len)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    if (!checkMidiChannel((sysex[2] & 0x0f) + 1 , instance_id))
    {
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
    for (uint16_t i = 0; i < len; i++)
    {
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

    // Check for SYSEX end byte
    if (sysex[len - 1] != 0xf7)
    {
#ifdef DEBUG
      Serial.println(F("E: SysEx end status byte not detected."));
#endif
      return;
    }

    if (sysex[1] != 0x43) // check for Yamaha sysex
    {
#ifdef DEBUG
      Serial.println(F("E: SysEx vendor not Yamaha."));
#endif
      return;
    }

#ifdef DEBUG
    Serial.print(F("Substatus: ["));
    Serial.print((sysex[2] & 0x70) >> 4);
    Serial.println(F("]"));
#endif

    // parse parameter change
    if (len == 7)
    {
      if (((sysex[3] & 0x7c) >> 2) != 0 && ((sysex[3] & 0x7c) >> 2) != 2)
      {
#ifdef DEBUG
        Serial.println(F("E: Not a SysEx parameter or function parameter change."));
#endif
        return;
      }

      sysex[4] &= 0x7f;
      sysex[5] &= 0x7f;

      if ((sysex[3] & 0x7c) >> 2 == 0)
      {
#ifdef DEBUG
        Serial.println(F("SysEx Voice parameter:"));
        Serial.print("Parameter #");
        Serial.print(sysex[4] + ((sysex[3] & 0x03) * 128), DEC);
        Serial.print(" Value: ");
        Serial.println(sysex[5], DEC);
#endif
        MicroDexed[instance_id]->data[sysex[4] + ((sysex[3] & 0x03) * 128)] = sysex[5];
      }
      else if ((sysex[3] & 0x7c) >> 2 == 2)
      {
#ifdef DEBUG
        Serial.println(F("SysEx Function parameter:"));
        Serial.print("Parameter #");
        Serial.print(sysex[4], DEC);
        Serial.print(" Value: ");
        Serial.println(sysex[5], DEC);
#endif
        switch (sysex[4])
        {
          case 65:
            configuration.dexed[instance_id].pb_range = constrain(sysex[5], PB_RANGE_MIN, PB_RANGE_MAX);
            MicroDexed[instance_id]->controllers.values_[kControllerPitchRange] = configuration.dexed[instance_id].pb_range;
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_pb_range))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 66:
            configuration.dexed[instance_id].pb_step = constrain(sysex[5], PB_STEP_MIN, PB_STEP_MAX);
            MicroDexed[instance_id]->controllers.values_[kControllerPitchStep] = configuration.dexed[instance_id].pb_step;
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_pb_step))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 67:
            configuration.dexed[instance_id].portamento_mode = constrain(sysex[5], PORTAMENTO_MODE_MIN, PORTAMENTO_MODE_MAX);
            MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode, configuration.dexed[instance_id].portamento_glissando, configuration.dexed[instance_id].portamento_time);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_mode))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 68:
            configuration.dexed[instance_id].portamento_glissando = constrain(sysex[5], PORTAMENTO_GLISSANDO_MIN, PORTAMENTO_GLISSANDO_MAX);
            MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode, configuration.dexed[instance_id].portamento_glissando, configuration.dexed[instance_id].portamento_time);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_glissando))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 69:
            configuration.dexed[instance_id].portamento_time = constrain(sysex[5], PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX);
            MicroDexed[instance_id]->setPortamentoMode(configuration.dexed[instance_id].portamento_mode, configuration.dexed[instance_id].portamento_glissando, configuration.dexed[instance_id].portamento_time);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_portamento_time))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 70:
            configuration.dexed[instance_id].mw_range = constrain(sysex[5], MW_RANGE_MIN, MW_RANGE_MAX);
            MicroDexed[instance_id]->controllers.wheel.setRange(configuration.dexed[instance_id].mw_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_mw_range))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 71:
            configuration.dexed[instance_id].mw_assign = constrain(sysex[5], MW_ASSIGN_MIN, MW_ASSIGN_MAX);
            MicroDexed[instance_id]->controllers.wheel.setTarget(configuration.dexed[instance_id].mw_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_mw_assign))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 72:
            configuration.dexed[instance_id].fc_range = constrain(sysex[5], FC_RANGE_MIN, FC_RANGE_MAX);
            MicroDexed[instance_id]->controllers.foot.setRange(configuration.dexed[instance_id].fc_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_fc_range))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 73:
            configuration.dexed[instance_id].fc_assign = constrain(sysex[5], FC_ASSIGN_MIN, FC_ASSIGN_MAX);
            MicroDexed[instance_id]->controllers.foot.setTarget(configuration.dexed[instance_id].fc_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_fc_assign))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 74:
            configuration.dexed[instance_id].bc_range = constrain(sysex[5], BC_RANGE_MIN, BC_RANGE_MAX);
            MicroDexed[instance_id]->controllers.breath.setRange(configuration.dexed[instance_id].bc_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_bc_range))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 75:
            configuration.dexed[instance_id].bc_assign = constrain(sysex[5], BC_ASSIGN_MIN, BC_ASSIGN_MAX);
            MicroDexed[instance_id]->controllers.breath.setTarget(configuration.dexed[instance_id].bc_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_bc_assign))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 76:
            configuration.dexed[instance_id].at_range = constrain(sysex[5], AT_RANGE_MIN, AT_RANGE_MAX);
            MicroDexed[instance_id]->controllers.at.setRange(configuration.dexed[instance_id].at_range);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_at_range))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          case 77:
            configuration.dexed[instance_id].at_assign = constrain(sysex[5], AT_ASSIGN_MIN, AT_ASSIGN_MAX);
            MicroDexed[instance_id]->controllers.at.setTarget(configuration.dexed[instance_id].at_assign);
            if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_at_assign))
            {
              LCDML.OTHER_updateFunc();
              LCDML.loop_menu();
            }
            break;
          default:
            MicroDexed[instance_id]->data[sysex[4]] = sysex[5]; // set function parameter
            break;
        }
        MicroDexed[instance_id]->controllers.refresh();
      }
#ifdef DEBUG
      else
      {
        Serial.println(F("E: Unknown SysEx voice or function."));
      }
#endif
    }
    else if (len == 163)
    {
      int32_t bulk_checksum_calc = 0;
      int8_t bulk_checksum = sysex[161];

      // 1 Voice bulk upload
#ifdef DEBUG
      Serial.println(F("One Voice bulk upload"));
#endif

      if ((sysex[3] & 0x7f) != 0)
      {
#ifdef DEBUG
        Serial.println(F("E: Not a SysEx voice bulk upload."));
#endif
        return;
      }

      if (((sysex[4] << 7) | sysex[5]) != 0x9b)
      {
#ifdef DEBUG
        Serial.println(F("E: Wrong length for SysEx voice bulk upload (not 155)."));
#endif
        return;
      }

      // checksum calculation
      for (uint8_t i = 0; i < 155 ; i++)
      {
        bulk_checksum_calc -= sysex[i + 6];
      }
      bulk_checksum_calc &= 0x7f;

      if (bulk_checksum_calc != bulk_checksum)
      {
#ifdef DEBUG
        Serial.print(F("E: Checksum error for one voice [0x"));
        Serial.print(bulk_checksum, HEX);
        Serial.print(F("/0x"));
        Serial.print(bulk_checksum_calc, HEX);
        Serial.println(F("]"));
#endif
        return;
      }

      // fix voice name
      for (uint8_t i = 0; i < 10; i++)
      {
        if (sysex[151 + i] > 126) // filter characters
          sysex[151 + i] = 32;
      }

      // load sysex-data into voice memory
      MicroDexed[instance_id]->loadVoiceParameters(&sysex[6]);

#ifdef DEBUG
      show_patch(instance_id);
#endif

      // show voice name
      strncpy(g_voice_name[instance_id], (char*)&sysex[151], VOICE_NAME_LEN - 1);

      if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
      {
        LCDML.OTHER_updateFunc();
        LCDML.loop_menu();
      }
    }
    else if (len == 4104)
    {
      if (strlen(receive_bank_filename) > 0 && LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_sysex_receive_bank))
      {
        int32_t bulk_checksum_calc = 0;
        int8_t bulk_checksum = sysex[4102];

        // 1 Bank bulk upload
        if ((sysex[3] & 0x7f) != 9)
        {
#ifdef DEBUG
          Serial.println(F("E: Not a SysEx bank bulk upload."));
#endif
          lcd.setCursor(0, 1);
          lcd.print(F("Error (TYPE)      "));
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();
          return;
        }

#ifdef DEBUG
        Serial.println(F("Bank bulk upload."));
#endif

        if (((sysex[4] << 7) | sysex[5]) != 0x1000)
        {
#ifdef DEBUG
          Serial.println(F("E: Wrong length for SysEx bank bulk upload (not 4096)."));
#endif
          lcd.setCursor(0, 1);
          lcd.print(F("Error (SIZE)     "));
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();
          return;
        }

#ifdef DEBUG
        Serial.println(F("Bank type ok"));
#endif

        // checksum calculation
        for (uint16_t i = 0; i < 4096 ; i++)
        {
          bulk_checksum_calc -= sysex[i + 6];
        }
        bulk_checksum_calc &= 0x7f;

        if (bulk_checksum_calc != bulk_checksum)
        {
#ifdef DEBUG
          Serial.print(F("E: Checksum error for bank [0x"));
          Serial.print(bulk_checksum, HEX);
          Serial.print(F("/0x"));
          Serial.print(bulk_checksum_calc, HEX);
          Serial.println(F("]"));
#endif
          lcd.setCursor(0, 1);
          lcd.print(F("Error (CHECKSUM)"));
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();
          return;
        }

#ifdef DEBUG
        Serial.println(F("Bank checksum ok"));
#endif

        if (save_sd_bank(receive_bank_filename, sysex))
        {
#ifdef DEBUG
          Serial.print(F("Bank saved as ["));
          Serial.print(receive_bank_filename);
          Serial.println(F("]"));
#endif
          lcd.setCursor(0, 1);
          lcd.print(F("Done.           "));
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();
        }
        else
        {
#ifdef DEBUG
          Serial.println(F("Error during saving bank as ["));
          Serial.print(receive_bank_filename);
          Serial.println(F("]"));
#endif
          lcd.setCursor(0, 1);
          lcd.print(F("Error.          "));
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();
        }
        memset(receive_bank_filename, 0, FILENAME_LEN);
      }
#ifdef DEBUG
      else
        Serial.println(F("E: Not in MIDI receive bank mode."));
#endif
    }
#ifdef DEBUG
    else
      Serial.println(F("E: SysEx parameter length wrong."));
#endif
  }
}

void handleTimeCodeQuarterFrame(byte data)
{
  ;
}

void handleAfterTouchPoly(byte inChannel, byte inNumber, byte inVelocity)
{
  ;
}

void handleSongSelect(byte inSong)
{
  ;
}

void handleTuneRequest(void)
{
  ;
}

void handleClock(void)
{
  midi_timing_counter++;
  if (midi_timing_counter % 24 == 0)
  {
    midi_timing_quarter = midi_timing_timestep;
    midi_timing_counter = 0;
    midi_timing_timestep = 0;
    // Adjust delay control here
#ifdef DEBUG
    Serial.print(F("MIDI Clock: "));
    Serial.print(60000 / midi_timing_quarter, DEC);
    Serial.print(F("bpm ("));
    Serial.print(midi_timing_quarter, DEC);
    Serial.println(F("ms per quarter)"));
#endif
  }
}

void handleStart(void)
{
  ;
}

void handleContinue(void)
{
  ;
}

void handleStop(void)
{
  ;
}

void handleActiveSensing(void)
{
  ;
}

void handleSystemReset(void)
{
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
#ifdef DEBUG
    Serial.println(F("MIDI SYSEX RESET"));
#endif
    MicroDexed[instance_id]->notesOff();
    MicroDexed[instance_id]->panic();
    MicroDexed[instance_id]->resetControllers();
  }
}

/******************************************************************************
   MIDI HELPER
 ******************************************************************************/
bool checkMidiChannel(byte inChannel, uint8_t instance_id)
{
  // check for MIDI channel
  if (configuration.dexed[instance_id].midi_channel == MIDI_CHANNEL_OMNI)
  {
    return (true);
  }
  else if (inChannel != configuration.dexed[instance_id].midi_channel)
  {
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

void init_MIDI_send_CC(void)
{
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
   VOLUME HELPER
 ******************************************************************************/

void set_volume(uint8_t v, uint8_t m)
{
  configuration.sys.vol = v;

  if (configuration.sys.vol > 100)
    configuration.sys.vol = 100;

  configuration.sys.mono = m;

#ifdef DEBUG
  Serial.print(F("Setting volume: VOL="));
  Serial.println(v, DEC);
#endif

  volume_r.gain(pseudo_log_curve(v / 100.0));
  volume_l.gain(pseudo_log_curve(v / 100.0));

  switch (m)
  {
    case 0: // stereo
      stereo2mono.stereo(true);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
    case 1: // mono both
      stereo2mono.stereo(false);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(PANORAMA_DEFAULT, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
    case 2: // mono right
      volume_l.gain(0.0);
      stereo2mono.stereo(false);
      for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
        mono2stereo[instance_id]->panorama(mapfloat(PANORAMA_MAX, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
      break;
    case 3: // mono left
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

void initial_values_from_eeprom(bool init)
{
  uint16_t _m_;

  if (init == true)
    init_configuration();
  else
  {
    _m_ = (EEPROM[EEPROM_START_ADDRESS + offsetof(configuration_s, _marker_)] << 8) | EEPROM[EEPROM_START_ADDRESS + offsetof(configuration_s, _marker_) + 1];
    if (_m_ != EEPROM_MARKER)
    {
#ifdef DEBUG
      Serial.println(F("Found wrong EEPROM marker, initializing EEPROM..."));
#endif
      configuration._marker_ = EEPROM_MARKER;
      init_configuration();
    }

#ifdef DEBUG
    Serial.println(F("Loading inital system data from EEPROM."));
#endif

    eeprom_get_performance();
    eeprom_get_sys();
    eeprom_get_fx();
    for (uint8_t i = 0; i < NUM_DEXED; i++)
    {
      eeprom_get_dexed(i);
    }

#ifdef DEBUG
    Serial.println(F("OK, loaded!"));
#endif
  }

  check_configuration();

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    set_voiceconfig_params(instance_id);
  }
  set_fx_params();
  set_sys_params();
  set_volume(configuration.sys.vol, configuration.sys.mono);

#ifdef DEBUG
  show_configuration();
#endif
}

void check_configuration(void)
{
  configuration.sys.instances = constrain(configuration.sys.instances, INSTANCES_MIN, INSTANCES_MAX);
  configuration.sys.vol = constrain(configuration.sys.vol, VOLUME_MIN, VOLUME_MAX);
  configuration.sys.mono = constrain(configuration.sys.mono, MONO_MIN, MONO_MAX);
  configuration.sys.soft_midi_thru = constrain(configuration.sys.soft_midi_thru, SOFT_MIDI_THRU_MIN, SOFT_MIDI_THRU_MAX);
  configuration.sys.performance_number = constrain(configuration.sys.performance_number, PERFORMANCE_NUM_MIN, PERFORMANCE_NUM_MAX);

  configuration.performance.fx_number = constrain(configuration.performance.fx_number, FX_NUM_MIN, FX_NUM_MAX);

  configuration.fx.reverb_roomsize = constrain(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX);
  configuration.fx.reverb_damping = constrain(configuration.fx.reverb_damping, REVERB_DAMPING_MIN, REVERB_DAMPING_MAX);
  configuration.fx.reverb_level = constrain(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX);

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    configuration.performance.bank[instance_id] = constrain(configuration.performance.bank[instance_id], 0, MAX_BANKS - 1);
    configuration.performance.voice[instance_id] = constrain(configuration.performance.voice[instance_id], 0, MAX_VOICES - 1);
    configuration.performance.voiceconfig_number[instance_id] = constrain(configuration.performance.voiceconfig_number[instance_id], VOICECONFIG_NUM_MIN, VOICECONFIG_NUM_MAX);

    configuration.dexed[instance_id].midi_channel = constrain(configuration.dexed[instance_id].midi_channel, MIDI_CHANNEL_MIN, MIDI_CHANNEL_MAX);
    configuration.dexed[instance_id].lowest_note = constrain(configuration.dexed[instance_id].lowest_note, INSTANCE_LOWEST_NOTE_MIN, INSTANCE_LOWEST_NOTE_MAX);
    configuration.dexed[instance_id].highest_note = constrain(configuration.dexed[instance_id].highest_note, INSTANCE_HIGHEST_NOTE_MIN, INSTANCE_HIGHEST_NOTE_MAX);
    configuration.dexed[instance_id].sound_intensity = constrain(configuration.dexed[instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX);
    configuration.dexed[instance_id].pan = constrain(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX);
    configuration.dexed[instance_id].transpose = constrain(configuration.dexed[instance_id].transpose, TRANSPOSE_MIN, TRANSPOSE_MAX);
    configuration.dexed[instance_id].tune = constrain(configuration.dexed[instance_id].tune, TUNE_MIN, TUNE_MAX);
    configuration.dexed[instance_id].polyphony = constrain(configuration.dexed[instance_id].polyphony, POLYPHONY_MIN, POLYPHONY_MAX);
    configuration.dexed[instance_id].velocity_level = constrain(configuration.dexed[instance_id].velocity_level, VELOCITY_LEVEL_MIN, VELOCITY_LEVEL_MAX);
    configuration.dexed[instance_id].engine = constrain(configuration.dexed[instance_id].engine, ENGINE_MIN, ENGINE_MAX);
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
}

void init_configuration(void)
{
#ifdef DEBUG
  Serial.println(F("INITIALIZING CONFIGURATION"));
#endif

  configuration.sys.instances = INSTANCES_DEFAULT;
  configuration.sys.vol = VOLUME_DEFAULT;
  configuration.sys.mono = MONO_DEFAULT;
  configuration.sys.soft_midi_thru = SOFT_MIDI_THRU_DEFAULT;
  configuration.sys.performance_number = PERFORMANCE_NUM_DEFAULT;

  configuration.fx.reverb_roomsize = REVERB_ROOMSIZE_DEFAULT;
  configuration.fx.reverb_damping = REVERB_DAMPING_DEFAULT;
  configuration.fx.reverb_level = REVERB_LEVEL_DEFAULT;

  configuration.performance.fx_number = FX_NUM_DEFAULT;

  for (uint8_t instance_id = 0; instance_id < MAX_DEXED; instance_id++)
  {
    configuration.performance.bank[instance_id] = SYSEXBANK_DEFAULT;
    configuration.performance.voice[instance_id] = SYSEXSOUND_DEFAULT;
    configuration.performance.voiceconfig_number[instance_id] = VOICECONFIG_NUM_DEFAULT;

    configuration.dexed[instance_id].midi_channel = DEFAULT_MIDI_CHANNEL;
    configuration.dexed[instance_id].lowest_note = INSTANCE_LOWEST_NOTE_MIN;
    configuration.dexed[instance_id].highest_note = INSTANCE_HIGHEST_NOTE_MAX;
    configuration.dexed[instance_id].sound_intensity = SOUND_INTENSITY_DEFAULT;
    configuration.dexed[instance_id].pan = PANORAMA_DEFAULT;
    configuration.dexed[instance_id].transpose = TRANSPOSE_DEFAULT;
    configuration.dexed[instance_id].tune = TUNE_DEFAULT;
    configuration.dexed[instance_id].polyphony = POLYPHONY_DEFAULT;
    configuration.dexed[instance_id].velocity_level = VELOCITY_LEVEL_DEFAULT;
    configuration.dexed[instance_id].engine = ENGINE_DEFAULT;
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

    configuration.performance.bank[instance_id] = SYSEXBANK_DEFAULT;
    configuration.performance.voice[instance_id] = SYSEXSOUND_DEFAULT;

    configuration.dexed[instance_id].polyphony = POLYPHONY_DEFAULT;


#if NUM_DEXED > 1
    MicroDexed[instance_id]->controllers.refresh();
#else
    if (instance_id == 0)
      MicroDexed[instance_id]->controllers.refresh();
#endif
  }

  set_volume(configuration.sys.vol, configuration.sys.mono);

  eeprom_update();
}

void eeprom_update(void)
{
  uint8_t* c = (uint8_t*)&configuration;
  for (uint16_t i = 0; i < sizeof(configuration); i++)
    EEPROM.update(EEPROM_START_ADDRESS + i, c[i]);
}

void eeprom_update_sys(void)
{
  uint8_t* c = (uint8_t*)&configuration.sys;

  for (uint16_t i = 0; i < sizeof(configuration.sys); i++)
    EEPROM.update(EEPROM_START_ADDRESS + offsetof(configuration_s, sys) + i, c[i]);

#ifdef DEBUG
  Serial.println(F("Updating EEPROM sys."));
#endif
}

bool eeprom_get_sys(void)
{
  EEPROM.get(EEPROM_START_ADDRESS + offsetof(configuration_s, sys), configuration.sys);
  return (true);
}

void eeprom_update_fx(void)
{
  uint8_t* c = (uint8_t*)&configuration.fx;

  for (uint16_t i = 0; i < sizeof(configuration.fx); i++)
    EEPROM.update(EEPROM_START_ADDRESS + offsetof(configuration_s, fx) + i, c[i]);

#ifdef DEBUG
  Serial.println(F("Updating EEPROM fx."));
#endif
}

bool eeprom_get_fx(void)
{
  EEPROM.get(EEPROM_START_ADDRESS + offsetof(configuration_s, fx), configuration.fx);
  return (true);
}

void eeprom_update_dexed(uint8_t instance_id)
{
#if NUM_DEXED == 1
  uint8_t* c = (uint8_t*)&configuration.dexed[0];

  for (uint16_t i = 0; i < sizeof(configuration.dexed[0]); i++)
    EEPROM.update(EEPROM_START_ADDRESS + offsetof(configuration_s, dexed[0]) + i, c[i]);
#else
  uint8_t* c;

  if (instance_id == 0)
    c = (uint8_t*)&configuration.dexed[0];
  else
    c = (uint8_t*)&configuration.dexed[1];

  for (uint16_t i = 0; i < sizeof(configuration.dexed[instance_id]); i++)
  {
    if (instance_id == 0)
      EEPROM.update(EEPROM_START_ADDRESS + offsetof(configuration_s, dexed[0]) + i, c[i]);
    else
      EEPROM.update(EEPROM_START_ADDRESS + offsetof(configuration_s, dexed[1]) + i, c[i]);
  }
#endif

#ifdef DEBUG
  Serial.print(F("Updating EEPROM dexed (instance "));
  Serial.print(instance_id);
  Serial.println(F(")."));
#endif
}

bool eeprom_get_dexed(uint8_t instance_id)
{
  for (uint8_t instance_id = 0; instance_id < MAX_DEXED; instance_id++)
  {
    if (instance_id == 0)
      EEPROM.get(EEPROM_START_ADDRESS + offsetof(configuration_s, dexed[0]), configuration.dexed[0]);
    else
      EEPROM.get(EEPROM_START_ADDRESS + offsetof(configuration_s, dexed[1]), configuration.dexed[1]);
  }
  return (true);
}

void eeprom_update_performance()
{
  EEPROM.put(EEPROM_START_ADDRESS + offsetof(configuration_s, performance), configuration.performance);
#ifdef DEBUG
  Serial.println(F("Updating EEPROM performance."));
#endif
}

bool eeprom_get_performance()
{
  EEPROM.get(EEPROM_START_ADDRESS + offsetof(configuration_s, performance), configuration.performance);
#ifdef DEBUG
  Serial.println(F("Getting EEPROM performance."));
#endif
  return (true);
}

/******************************************************************************
  PARAMETER-HELPERS
******************************************************************************/

void set_fx_params(void)
{
#if defined(USE_FX)
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    // CHORUS
    switch (configuration.fx.chorus_waveform[instance_id])
    {
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
    delay_mixer[instance_id]->gain(1, pseudo_log_curve(mapfloat(configuration.fx.delay_level[instance_id], DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 0.0, 1.0)));
    delay_fb_mixer[instance_id]->gain(0, 1.0);
    delay_fb_mixer[instance_id]->gain(1, pseudo_log_curve(mapfloat(configuration.fx.delay_feedback[instance_id], DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 0.0, 1.0)));
    if (configuration.fx.delay_level[selected_instance_id] <= DELAY_LEVEL_MIN)
      delay_fx[instance_id]->disable(0);
    else
      delay_fx[instance_id]->delay(0, constrain(configuration.fx.delay_time[instance_id], DELAY_TIME_MIN, DELAY_TIME_MAX) * 10);

    // REVERB SEND
    reverb_mixer_r.gain(instance_id, pseudo_log_curve(mapfloat(configuration.fx.reverb_send[instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, 1.0)));
    reverb_mixer_l.gain(instance_id, pseudo_log_curve(mapfloat(configuration.fx.reverb_send[instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, 1.0)));

    // DEXED FILTER
    MicroDexed[instance_id]->fx.Reso = mapfloat(configuration.fx.filter_resonance[instance_id], FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 1.0, 0.0);
    MicroDexed[instance_id]->fx.Cutoff = mapfloat(configuration.fx.filter_cutoff[instance_id], FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 1.0, 0.0);
    MicroDexed[instance_id]->doRefreshVoice();
  }

  // REVERB
#ifdef USE_PLATEREVERB
  reverb.size(mapfloat(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 0.0, 1.0));     // max reverb length
  reverb.lowpass(0.3);  // sets the reverb master lowpass filter
  reverb.lodamp(0.1);   // amount of low end loss in the reverb tail
  reverb.hidamp(mapfloat(configuration.fx.reverb_damping, REVERB_DAMPING_MIN, REVERB_DAMPING_MAX, 0.0, 1.0));   // amount of treble loss in the reverb tail
  reverb.diffusion(1.0);  // 1.0 is the detault setting, lower it to create more "echoey" reverb
#else
  freeverb_r.roomsize(mapfloat(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 0.0, 1.0));
  freeverb_r.damping(mapfloat(configuration.fx.reverb_damping, REVERB_DAMPING_MIN, REVERB_DAMPING_MAX, 0.0, 1.0));
  freeverb_l.roomsize(mapfloat(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 0.0, 1.0));
  freeverb_l.damping(mapfloat(configuration.fx.reverb_damping, REVERB_DAMPING_MIN, REVERB_DAMPING_MAX, 0.0, 1.0));
#endif

  master_mixer_r.gain(3, pseudo_log_curve(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, 1.0)));
  master_mixer_l.gain(3, pseudo_log_curve(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, 1.0)));
#endif
#ifdef SGTL5000_AUDIO_ENHANCE
  sgtl5000_1.eqBands(mapfloat(configuration.fx.eq_bass, EQ_BASS_MIN, EQ_BASS_MAX, -1.0, 1.0), mapfloat(configuration.fx.eq_treble, EQ_TREBLE_MIN, EQ_TREBLE_MAX, -1.0, 1.0));
#endif

  init_MIDI_send_CC();
}

void set_voiceconfig_params(uint8_t instance_id)
{
  // INIT PEAK MIXER
  microdexed_peak_mixer.gain(instance_id, 1.0);

  // Controller
  MicroDexed[instance_id]->setPBController(configuration.dexed[instance_id].pb_range, configuration.dexed[instance_id].pb_step);
  MicroDexed[instance_id]->setMWController(configuration.dexed[instance_id].mw_range, configuration.dexed[instance_id].mw_assign, configuration.dexed[instance_id].mw_mode);
  MicroDexed[instance_id]->setFCController(configuration.dexed[instance_id].fc_range, configuration.dexed[instance_id].fc_assign, configuration.dexed[instance_id].fc_mode);
  MicroDexed[instance_id]->setBCController(configuration.dexed[instance_id].bc_range, configuration.dexed[instance_id].bc_assign, configuration.dexed[instance_id].bc_mode);
  MicroDexed[instance_id]->setATController(configuration.dexed[instance_id].at_range, configuration.dexed[instance_id].at_assign, configuration.dexed[instance_id].at_mode);
  MicroDexed[instance_id]->controllers.refresh();
  MicroDexed[instance_id]->setOPs(configuration.dexed[instance_id].op_enabled);
  MicroDexed[instance_id]->doRefreshVoice();
  MicroDexed[instance_id]->setMaxNotes(configuration.dexed[instance_id].polyphony);
  MicroDexed[instance_id]->setMonoMode(configuration.sys.mono);

  // Dexed output level
  MicroDexed[instance_id]->fx.Gain = mapfloat(configuration.dexed[instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 0.0, SOUND_INTENSITY_AMP_MAX);

  // PANORAMA
  mono2stereo[instance_id]->panorama(mapfloat(configuration.dexed[instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
}

void set_sys_params(void)
{
  // set initial volume
  set_volume(configuration.sys.vol, configuration.sys.mono);
}

/******************************************************************************
   HELPERS
 ******************************************************************************/

// https://www.reddit.com/r/Teensy/comments/7r19uk/reset_and_reboot_teensy_lc_via_code/
#define SCB_AIRCR (*(volatile uint32_t *)0xE000ED0C) // Application Interrupt and Reset Control location
void _softRestart(void)
{
  Serial.end();  //clears the serial monitor  if used
  SCB_AIRCR = 0x05FA0004;  //write value for restart
}

float pseudo_log_curve(float value)
{
  //return (mapfloat(_pseudo_log * arm_sin_f32(value), 0.0, _pseudo_log * arm_sin_f32(1.0), 0.0, 1.0));
  //return (1 - sqrt(1 - value * value));
  //return (pow(2, value) - 1);
  return (pow(value, 2.2));
}

uint32_t crc32(byte * calc_start, uint16_t calc_bytes) // base code from https://www.arduino.cc/en/Tutorial/EEPROMCrc
{
  const uint32_t crc_table[16] =
  {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };
  uint32_t crc = ~0L;

  for (byte* index = calc_start ; index < (calc_start + calc_bytes) ; ++index)
  {
    crc = crc_table[(crc ^ *index) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (*index >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return (crc);
}

void generate_version_string(char* buffer, uint8_t len)
{
  char tmp[3];

  memset(buffer, 0, len);
  strncat(buffer, VERSION, len);
#if defined(TEENSY3_5)
  strncat(buffer, "-3.5", 4);
#elif defined(TEENSY3_6)
  strncat(buffer, "-3.6", 4);
#elif defined(TEENSY4)
  strncat(buffer, "-4.0", 4);
#endif
#if defined(USE_FX)
  strncat(buffer, "FX", 2);
#endif
#if defined(MAX_NOTES)
  strncat(buffer, "-", 1);
  itoa (MAX_NOTES, tmp, 10);
  strncat(buffer, tmp, 2);
#endif
}

#ifdef DISPLAY_LCD_SPI
void change_disp_sd(bool disp)
{
  if (sd_card > 0)
  {
    digitalWrite(sd_card, disp);
    digitalWrite(U8X8_CS_PIN, !disp);
  }
}
#endif

uint8_t check_sd_cards(void)
{
  uint8_t ret = 0;

  memset(sd_string, 0, sizeof(sd_string));

  for (uint8_t i = 0; i < sizeof(cs_pins); i++)
  {
#ifdef DEBUG
    Serial.print(F("Checking CS pin "));
    Serial.print(cs_pins[i], DEC);
    Serial.println(F(" for SD card"));
#endif
    SPI.setMOSI(mosi_pins[i]);
    SPI.setSCK(sck_pins[i]);

    if (SD.begin(cs_pins[i]) == true)
    {
#ifdef DEBUG
      Serial.print(F("Found. Using pin "));
      Serial.println(cs_pins[i], DEC);
#endif
      ret = cs_pins[i];
      break;
    }
  }

  if (ret >= 0)
  {
    if (!card.init(SPI_HALF_SPEED, ret))
    {
#ifdef DEBUG
      Serial.println(F("SD card initialization failed."));
#endif
      ret = -1;
    }
  }

  if (ret >= 0)
  {
#ifdef DEBUG
    Serial.print(F("Card type: "));
#endif
    switch (card.type()) {
      case SD_CARD_TYPE_SD1:
        sprintf(sd_string, "%-5s", "SD1");
#ifdef DEBUG
        Serial.println(F("SD1"));
#endif
        break;
      case SD_CARD_TYPE_SD2:
        sprintf(sd_string, "%-5s", "SD2");
#ifdef DEBUG
        Serial.println(F("SD2"));
#endif
        break;
      case SD_CARD_TYPE_SDHC:
        sprintf(sd_string, "%-5s", "SD2");
#ifdef DEBUG
        Serial.println(F("SDHC"));
#endif
        break;
      default:
        sprintf(sd_string, "%-5s", "UKNW");
#ifdef DEBUG
        Serial.println(F("Unknown"));
#endif
    }

    if (!volume.init(card))
    {
#ifdef DEBUG
      Serial.println(F("Could not find FAT16/FAT32 partition."));
#endif
      ret = -1;
    }
  }

  if (ret >= 0)
  {
    uint32_t volumesize;

    volumesize = volume.blocksPerCluster() * volume.clusterCount() / 2097152;

#ifdef DEBUG
    Serial.print(F("Volume type is FAT"));
    Serial.println(volume.fatType(), DEC);
    Serial.print(F("Volume size (GB): "));
    Serial.println(volumesize);
#endif

    sprintf(sd_string + 5, "FAT%2d %02dGB", volume.fatType(), int(volumesize));
  }

#ifdef DEBUG
  Serial.println(sd_string);
#endif

  return (ret);
}

void check_and_create_directories(void)
{
  if (sd_card > 0)
  {
    uint8_t i;
    char tmp[FILENAME_LEN];

#ifdef DEBUG
    Serial.println(F("Directory check... "));
#endif
    // create directories for banks
    for (i = 0; i < MAX_BANKS; i++)
    {
      sprintf(tmp, "/%d", i);
      if (!SD.exists(tmp))
      {
#ifdef DEBUG
        Serial.print(F("Creating directory "));
        Serial.println(tmp);
#endif
        SD.mkdir(tmp);
      }
    }

    // create directories for confgiuration files
    sprintf(tmp, "/%s", VOICE_CONFIG_PATH);
    if (!SD.exists(tmp))
    {
#ifdef DEBUG
      Serial.print(F("Creating directory "));
      Serial.println(tmp);
#endif
      SD.mkdir(tmp);
    }
    sprintf(tmp, "/%s", PERFORMANCE_CONFIG_PATH);
    if (!SD.exists(tmp))
    {
#ifdef DEBUG
      Serial.print(F("Creating directory "));
      Serial.println(tmp);
#endif
      SD.mkdir(tmp);
    }
    sprintf(tmp, "/%s", FX_CONFIG_PATH);
    if (!SD.exists(tmp))
    {
#ifdef DEBUG
      Serial.print(F("Creating directory "));
      Serial.println(tmp);
#endif
      SD.mkdir(tmp);
    }
  }
#ifdef DEBUG
  else
    Serial.println(F("No SD card for directory check available."));
#endif
}

/******************************************************************************
   DEBUG HELPER
 ******************************************************************************/
#if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
void show_cpu_and_mem_usage(void)
{
  uint32_t sum_xrun = 0;
  uint16_t sum_render_time_max = 0;

  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    sum_xrun += MicroDexed[instance_id]->xrun;
    sum_render_time_max += MicroDexed[instance_id]->render_time_max;
    MicroDexed[instance_id]->render_time_max = 0;
  }
  if (AudioProcessorUsageMax() > 99.9)
  {
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
  Serial.print(F("|MEM:"));
  Serial.print(AudioMemoryUsage(), DEC);
  Serial.print(F("|MEMMAX:"));
  Serial.print(AudioMemoryUsageMax(), DEC);
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
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    Serial.print(instance_id, DEC);
    Serial.print(F("="));
    Serial.print(active_voices[instance_id], DEC);
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
void show_configuration(void)
{
  Serial.println();
  Serial.println(F("CONFIGURATION:"));
  Serial.println(F("System"));
  Serial.print(F("  Instances           ")); Serial.println(configuration.sys.instances, DEC);
  Serial.print(F("  Volume              ")); Serial.println(configuration.sys.vol, DEC);
  Serial.print(F("  Mono                ")); Serial.println(configuration.sys.mono, DEC);
  Serial.print(F("  Soft MIDI Thru      ")); Serial.println(configuration.sys.soft_midi_thru, DEC);
  Serial.println(F("FX"));
  Serial.print(F("  Reverb Roomsize     ")); Serial.println(configuration.fx.reverb_roomsize, DEC);
  Serial.print(F("  Reverb Damping      ")); Serial.println(configuration.fx.reverb_damping, DEC);
  Serial.print(F("  Reverb Level        ")); Serial.println(configuration.fx.reverb_level, DEC);


  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    Serial.print(F("Dexed instance "));
    Serial.println(instance_id, DEC);
    Serial.print(F("  MIDI-Channel         ")); Serial.println(configuration.dexed[instance_id].midi_channel, DEC);
    Serial.print(F("  Lowest Note          ")); Serial.println(configuration.dexed[instance_id].lowest_note, DEC);
    Serial.print(F("  Highest Note         ")); Serial.println(configuration.dexed[instance_id].highest_note, DEC);
    Serial.print(F("  Filter Cutoff        ")); Serial.println(configuration.fx.filter_cutoff[instance_id], DEC);
    Serial.print(F("  Filter Resonance     ")); Serial.println(configuration.fx.filter_resonance[instance_id], DEC);
    Serial.print(F("  Chorus Frequency     ")); Serial.println(configuration.fx.chorus_frequency[instance_id], DEC);
    Serial.print(F("  Chorus Waveform      ")); Serial.println(configuration.fx.chorus_waveform[instance_id], DEC);
    Serial.print(F("  Chorus Depth         ")); Serial.println(configuration.fx.chorus_depth[instance_id], DEC);
    Serial.print(F("  Chorus Level         ")); Serial.println(configuration.fx.chorus_level[instance_id], DEC);
    Serial.print(F("  Delay Time           ")); Serial.println(configuration.fx.delay_time[instance_id], DEC);
    Serial.print(F("  Delay Feedback       ")); Serial.println(configuration.fx.delay_feedback[instance_id], DEC);
    Serial.print(F("  Delay Level          ")); Serial.println(configuration.fx.delay_level[instance_id], DEC);
    Serial.print(F("  Reverb Send          ")); Serial.println(configuration.fx.reverb_send[instance_id], DEC);
    Serial.print(F("  Sound Intensity      ")); Serial.println(configuration.dexed[instance_id].sound_intensity, DEC);
    Serial.print(F("  Panorama             ")); Serial.println(configuration.dexed[instance_id].pan, DEC);
    Serial.print(F("  Transpose            ")); Serial.println(configuration.dexed[instance_id].transpose, DEC);
    Serial.print(F("  Tune                 ")); Serial.println(configuration.dexed[instance_id].tune, DEC);
    Serial.print(F("  Polyphony            ")); Serial.println(configuration.dexed[instance_id].polyphony, DEC);
    Serial.print(F("  Engine               ")); Serial.println(configuration.dexed[instance_id].engine, DEC);
    Serial.print(F("  Mono/Poly            ")); Serial.println(configuration.dexed[instance_id].monopoly, DEC);
    Serial.print(F("  Note Refresh         ")); Serial.println(configuration.dexed[instance_id].note_refresh, DEC);
    Serial.print(F("  Pitchbend Range      ")); Serial.println(configuration.dexed[instance_id].pb_range, DEC);
    Serial.print(F("  Pitchbend Step       ")); Serial.println(configuration.dexed[instance_id].pb_step, DEC);
    Serial.print(F("  Modwheel Range       ")); Serial.println(configuration.dexed[instance_id].mw_range, DEC);
    Serial.print(F("  Modwheel Assign      ")); Serial.println(configuration.dexed[instance_id].mw_assign, DEC);
    Serial.print(F("  Modwheel Mode        ")); Serial.println(configuration.dexed[instance_id].mw_mode, DEC);
    Serial.print(F("  Footctrl Range       ")); Serial.println(configuration.dexed[instance_id].fc_range, DEC);
    Serial.print(F("  Footctrl Assign      ")); Serial.println(configuration.dexed[instance_id].fc_assign, DEC);
    Serial.print(F("  Footctrl Mode        ")); Serial.println(configuration.dexed[instance_id].fc_mode, DEC);
    Serial.print(F("  BreathCtrl Range     ")); Serial.println(configuration.dexed[instance_id].bc_range, DEC);
    Serial.print(F("  Breathctrl Assign    ")); Serial.println(configuration.dexed[instance_id].bc_assign, DEC);
    Serial.print(F("  Breathctrl Mode      ")); Serial.println(configuration.dexed[instance_id].bc_mode, DEC);
    Serial.print(F("  Aftertouch Range     ")); Serial.println(configuration.dexed[instance_id].at_range, DEC);
    Serial.print(F("  Aftertouch Assign    ")); Serial.println(configuration.dexed[instance_id].at_assign, DEC);
    Serial.print(F("  Aftertouch Mode      ")); Serial.println(configuration.dexed[instance_id].at_mode, DEC);
    Serial.print(F("  Portamento Mode      ")); Serial.println(configuration.dexed[instance_id].portamento_mode, DEC);
    Serial.print(F("  Portamento Glissando ")); Serial.println(configuration.dexed[instance_id].portamento_glissando, DEC);
    Serial.print(F("  Portamento Time      ")); Serial.println(configuration.dexed[instance_id].portamento_time, DEC);
    Serial.print(F("  OP Enabled           ")); Serial.println(configuration.dexed[instance_id].op_enabled, DEC);
    Serial.flush();
  }

  Serial.println(F("Performance"));
  for (uint8_t instance_id = 0; instance_id < NUM_DEXED; instance_id++)
  {
    Serial.print(F("  Bank ")); Serial.print(instance_id, DEC); Serial.print("               "); Serial.println(configuration.performance.bank[instance_id], DEC);
    Serial.print(F("  Voice ")); Serial.print(instance_id, DEC); Serial.print("              "); Serial.println(configuration.performance.voice[instance_id], DEC);
  }
  Serial.print(F("  FX-Number            ")); Serial.println(configuration.performance.fx_number, DEC);

  Serial.println();
  Serial.flush();
}

void show_patch(uint8_t instance_id)
{
  char vn[VOICE_NAME_LEN];

  Serial.print(F("INSTANCE "));
  Serial.println(instance_id, DEC);

  memset(vn, 0, sizeof(vn));
  Serial.println(F("+==========================================================================================================+"));
  for (int8_t i = 5; i >= 0; --i)
  {
    Serial.println(F("+==========================================================================================================+"));
    Serial.print(F("| OP"));
    Serial.print(6 - i, DEC);
    Serial.println(F("                                                                                                      |"));
    Serial.println(F("+======+======+======+======+======+======+======+======+================+================+================+"));
    Serial.println(F("|  R1  |  R2  |  R3  |  R4  |  L1  |  L2  |  L3  |  L4  | LEV_SCL_BRK_PT | SCL_LEFT_DEPTH | SCL_RGHT_DEPTH |"));
    Serial.println(F("+------+------+------+------+------+------+------+------+----------------+----------------+----------------+"));
    Serial.print("| ");
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_R1]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_R2]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_R3]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_R4]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_L1]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_L2]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_L3]);
    Serial.print(F("  | "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_EG_L4]);
    Serial.print(F("  |           "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_LEV_SCL_BRK_PT]);
    Serial.print(F("  |           "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_SCL_LEFT_DEPTH]);
    Serial.print(F("  |           "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_SCL_RGHT_DEPTH]);
    Serial.println(F("  |"));
    Serial.println(F("+======+======+======+======+======+===+==+==+===+======+====+========+==+====+=======+===+================+"));
    Serial.println(F("| SCL_L_CURVE | SCL_R_CURVE | RT_SCALE | AMS | KVS | OUT_LEV | OP_MOD | FRQ_C | FRQ_F | DETUNE             |"));
    Serial.println(F("+-------------+-------------+----------+-----+-----+---------+--------+-------+-------+--------------------+"));
    Serial.print(F("|        "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_SCL_LEFT_CURVE]);
    Serial.print(F("  |        "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_SCL_RGHT_CURVE]);
    Serial.print(F("  |     "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_OSC_RATE_SCALE]);
    Serial.print(F("  |"));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_AMP_MOD_SENS]);
    Serial.print(F("  |"));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_KEY_VEL_SENS]);
    Serial.print(F("  |    "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_OUTPUT_LEV]);
    Serial.print(F("  |   "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_OSC_MODE]);
    Serial.print(F("  |  "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_FREQ_COARSE]);
    Serial.print(F("  |  "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_FREQ_FINE]);
    Serial.print(F("  |               "));
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[(i * 21) + DEXED_OP_OSC_DETUNE]);
    Serial.println(F("  |"));
  }
  Serial.println(F("+=======+=====+=+=======+===+===+======++====+==+==+====+====+==+======+======+=====+=+====================+"));
  Serial.println(F("|  PR1  |  PR2  |  PR3  |  PR4  |  PL1  |  PL2  |  PL3  |  PL4  | ALG  |  FB  | OKS | TRANSPOSE            |"));
  Serial.println(F("+-------+-------+-------+-------+-------+-------+-------+-------+------+------+-----+----------------------+"));
  Serial.print(F("|  "));
  for (int8_t i = 0; i < 8; i++)
  {
    SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + i]);
    Serial.print(F("  |  "));
  }
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_ALGORITHM]);
  Serial.print(F(" | "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_FEEDBACK]);
  Serial.print(F("  |"));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC]);
  Serial.print(F("  |                 "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_TRANSPOSE]);
  Serial.println(F("  |"));
  Serial.println(F("+=======+=+=====+===+===+=====+=+=======+=======+==+====+=====+=+======++=====+=====+======================+"));
  Serial.println(F("| LFO SPD | LFO DLY | LFO PMD | LFO AMD | LFO SYNC | LFO WAVE | LFO PMS | NAME                             |"));
  Serial.println(F("+---------+---------+---------+---------+----------+----------+---------+----------------------------------+"));
  Serial.print(F("|    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_SPEED]);
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_DELAY]);
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_DEP]);
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_AMP_MOD_DEP]);
  Serial.print(F("  |     "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_SYNC]);
  Serial.print(F("  |     "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_WAVE]);
  Serial.print(F("  |    "));
  SerialPrintFormatInt3(MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS]);
  Serial.print(F("  | "));
  strncpy(vn, (char *)&MicroDexed[instance_id]->data[DEXED_VOICE_OFFSET + DEXED_NAME], VOICE_NAME_LEN - 1);
  Serial.print(vn);
  Serial.println(F("                       |"));
  Serial.println(F("+=========+=========+=========+=========+==========+==========+=========+==================================+"));
  Serial.println(F("+==========================================================================================================+"));
}

void SerialPrintFormatInt3(uint8_t num)
{
  char buf[4];
  sprintf(buf, "%3d", num);
  Serial.print(buf);
}

/* From: https://forum.pjrc.com/threads/33443-How-to-display-free-ram */
extern "C" char* sbrk(int incr);
uint32_t FreeMem(void)
{
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
/*
  uint32_t FreeMem(void) { // for Teensy 3.0
  uint32_t stackTop;
  uint32_t heapTop;

  // current position of the stack.
  stackTop = (uint32_t) &stackTop;

  // current position of heap.
  void* hTop = malloc(1);
  heapTop = (uint32_t) hTop;
  free(hTop);

  // The difference is (approximately) the free, available ram.
  return stackTop - heapTop;
  }
*/
#endif
