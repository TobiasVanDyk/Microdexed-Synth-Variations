/*
   MicroDexed

   MicroDexed is a port of the Dexed sound engine
   (https://github.com/asb2m10/dexed) for the Teensy-3.5/3.6/4.x with audio shield.
   Dexed ist heavily based on https://github.com/google/music-synthesizer-for-android

   (c)2018-2023 H. Wirtz <wirtz@parasitstudio.de>
   (c)2021-2022 H. Wirtz <wirtz@parasitstudio.de>, M. Koslowski <positionhigh@gmx.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#pragma once

#include "config.h"
#include <LCDMenuLib2.h>
#include <MD_REncoder.h>
#include <effect_modulated_delay.h>
#include <effect_stereo_mono.h>
#include <effect_mono_stereo.h>
#include <effect_platervbstereo.h>
#include <template_mixer.hpp>
#include "disp_plus.h"
#include "midi_devices.hpp"
#include "synth_dexed.h"
#include "synth_mda_epiano.h"
#include "drumset.h"
#include "dexed_sd.h"
#include <effect_stereo_panorama.h>
#if defined(USE_DELAY_8M)
#include <effect_delay_ext8.h>
#endif

#define _LCDML_DISP_cols LCD_cols
#define _LCDML_DISP_rows LCD_rows

#ifdef I2C_DISPLAY
#define _LCDML_DISP_cfg_cursor 0x7E  // cursor Symbol
#else
#define _LCDML_DISP_cfg_cursor 0x8d  // cursor Symbol
#endif
#define _LCDML_DISP_cfg_scrollbar 1  // enable a scrollbar

extern uint8_t midi_learn_mode;
extern uint8_t active_sample;

extern config_t configuration;
extern void set_volume(uint8_t v, uint8_t m);
extern bool load_sysex(uint8_t b, uint8_t v);
extern void generate_version_string(char* buffer, uint8_t len);
extern void _softRestart(void);
extern float midi_volume_transform(uint8_t midi_amp);
extern float volume_transform(float amp);
extern uint8_t selected_instance_id;
extern char receive_bank_filename[FILENAME_LEN];
extern void eeprom_update(void);

#if NUM_DRUMS > 0
#include "midinotes.h"
#include "drumset.h"
extern void get_sd_performance_name_json(uint8_t number, char* name, uint8_t len);
extern bool save_sd_performance_json(uint8_t p);
extern uint8_t drum_midi_channel;
uint8_t activesample = 0;
#endif

#ifdef SGTL5000_AUDIO_ENHANCE
#include "control_sgtl5000plus.h"
extern AudioControlSGTL5000Plus sgtl5000;
#else
extern AudioControlSGTL5000 sgtl5000;
#endif

extern AudioSynthDexed* MicroDexed[NUM_DEXED];

extern AudioSynthWaveform* chorus_modulator[NUM_DEXED];
extern AudioEffectModulatedDelay* modchorus[NUM_DEXED];
extern AudioMixer<2>* chorus_mixer[NUM_DEXED];
extern AudioMixer<2>* delay_fb_mixer[NUM_DEXED];
#if defined(USE_DELAY_8M)
extern AudioEffectDelayExternal8* delay_fx[NUM_DEXED];
#else
extern AudioEffectDelay* delay_fx[NUM_DEXED];
#endif
extern AudioMixer<2>* delay_mixer[NUM_DEXED];
extern AudioEffectMonoStereo* mono2stereo[NUM_DEXED];
extern AudioMixer<2> microdexed_peak_mixer;
extern AudioAnalyzePeak microdexed_peak;
extern AudioSynthEPiano ep;
extern AudioMixer<4> reverb_mixer_r;
extern AudioMixer<4> reverb_mixer_l;
extern AudioEffectPlateReverb reverb;
extern AudioEffectStereoPanorama ep_stereo_panorama;
extern AudioSynthWaveform ep_chorus_modulator;
#if MOD_FILTER_OUTPUT != MOD_NO_FILTER_OUTPUT
extern AudioFilterBiquad ep_modchorus_filter;
#endif
extern AudioEffectModulatedDelayStereo ep_modchorus;
extern AudioMixer<2> ep_chorus_mixer_r;
extern AudioMixer<2> ep_chorus_mixer_l;
extern AudioMixer<2> ep_delay_fb_mixer_r;
extern AudioMixer<2> ep_delay_fb_mixer_l;
extern AudioEffectDelay ep_delay_fx_r;
extern AudioEffectDelay ep_delay_fx_l;
extern AudioMixer<2> ep_delay_mixer_r;
extern AudioMixer<2> ep_delay_mixer_l;
extern AudioMixer<5> master_mixer_r;
extern AudioMixer<5> master_mixer_l;
extern AudioEffectStereoMono stereo2mono;
extern AudioAnalyzePeak master_peak_r;
extern AudioAnalyzePeak master_peak_l;

extern char sd_string[LCD_cols + 1];
extern char g_voice_name[NUM_DEXED][VOICE_NAME_LEN];
extern char g_bank_name[NUM_DEXED][BANK_NAME_LEN];
extern int perform_attack_mod[NUM_DEXED];
extern int perform_release_mod[NUM_DEXED];
extern const float midi_ticks_factor[10];
extern uint8_t midi_bpm;
extern bool save_sys_flag;
extern elapsedMillis save_sys;
extern bool save_sys_flag;
extern uint8_t midinote_old[NUM_DRUMSET_CONFIG];

/***********************************************************************
   GLOBAL
************************************************************************/
elapsedMillis back_from_volume;
uint8_t instance_num[8][8];
const char accepted_chars[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-abcdefghijklmnopqrstuvwxyz";
//const char noteNames[12][3] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
uint8_t active_perform_page = 1;
uint8_t orig_attack_values[2][7];
uint8_t orig_release_values[2][7];
int temp_int;
float temp_float;
bool ask_before_quit = false;

#ifdef I2C_DISPLAY
#include <LiquidCrystal_I2C.h>
Disp_Plus<LiquidCrystal_I2C> display(LCD_I2C_ADDRESS, _LCDML_DISP_cols, _LCDML_DISP_rows);
#endif

#ifdef U8X8_DISPLAY
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
Disp_Plus<U8X8_DISPLAY_CLASS> lcd(/* cs=*/U8X8_CS_PIN, /* dc=*/U8X8_DC_PIN, /* reset=*/U8X8_RESET_PIN);
//Disp_Plus<U8X8_DISPLAY_CLASS> lcd(U8X8_PIN_NONE);
#endif

const uint8_t scroll_bar[5][8] = {
  { B10001, B10001, B10001, B10001, B10001, B10001, B10001, B10001 },  // scrollbar top
  { B11111, B11111, B10001, B10001, B10001, B10001, B10001, B10001 },  // scroll state 1
  { B10001, B10001, B11111, B11111, B10001, B10001, B10001, B10001 },  // scroll state 2
  { B10001, B10001, B10001, B10001, B11111, B11111, B10001, B10001 },  // scroll state 3
  { B10001, B10001, B10001, B10001, B10001, B10001, B11111, B11111 }   // scrollbar bottom
};

const uint8_t block_bar[5][8] = {
  { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 },
  { B11000, B11000, B11000, B11000, B11000, B11000, B11000, B11000 },
  { B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100 },
  { B11110, B11110, B11110, B11110, B11110, B11110, B11110, B11110 },
  { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }
};

const uint8_t meter_bar[5][8] = {
  { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 },
  { B01000, B01000, B01000, B01000, B01000, B01000, B01000, B01000 },
  { B00100, B00100, B00100, B00100, B00100, B00100, B00100, B00100 },
  { B00010, B00010, B00010, B00010, B00010, B00010, B00010, B00010 },
  { B00001, B00001, B00001, B00001, B00001, B00001, B00001, B00001 }
};

const uint8_t special_chars[20][8] = {
  { B11111, B11011, B10011, B11011, B11011, B11011, B11011, B11111 },  //  [0] 1 small invers
  { B11111, B11011, B10101, B11101, B11011, B10111, B10001, B11111 },  //  [1] 2 small invers
  { B11111, B11011, B10011, B11011, B11011, B11011, B11011, B11111 },  //  [2] 1 OP invers
  { B11111, B11011, B10101, B11101, B11011, B10111, B10001, B11111 },  //  [3] 2 OP invers
  { B11111, B10001, B11101, B11011, B11101, B10101, B11011, B11111 },  //  [4] 3 OP invers
  { B11111, B10111, B10111, B10101, B10001, B11101, B11101, B11111 },  //  [5] 4 OP invers
  { B11111, B10001, B10111, B10011, B11101, B11101, B10011, B11111 },  //  [6] 5 OP invers
  { B11111, B11001, B10111, B10011, B10101, B10101, B11011, B11111 },  //  [7] 6 OP invers
  { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B11111 },  //  [8] Level 1
  { B00000, B00000, B00000, B00000, B00000, B00000, B11111, B11111 },  //  [9] Level 2
  { B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111 },  // [10] Level 3
  { B00000, B00000, B00000, B00000, B11111, B11111, B11111, B11111 },  // [11] Level 4
  { B00000, B00000, B00000, B11111, B11111, B11111, B11111, B11111 },  // [12] Level 5
  { B00000, B00000, B11111, B11111, B11111, B11111, B11111, B11111 },  // [13] Level 6
  { B00000, B11111, B11111, B11111, B11111, B11111, B11111, B11111 },  // [14] Level 7
  { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 },  // [15] Level 8
  { B00100, B00110, B00101, B00101, B01101, B11101, B11100, B11000 },  // [16] Note
  { B01110, B10001, B10001, B01110, B00100, B00100, B00110, B00110 },  // [17] Disabled 2nd instance symbol
  { B11111, B10001, B10111, B10001, B10111, B10111, B10111, B11111 },  // [18] Favorites Icon
  { B11111, B10001, B10111, B10001, B10111, B10001, B11111, B00000 }   // [19] Edit Icon
};

enum { SCROLLBAR,
       BLOCKBAR,
       METERBAR
};
enum { ENC_R,
       ENC_L
};
enum { MENU_VOICE_BANK,
       MENU_VOICE_SOUND
};

void lcdml_menu_display(void);
void lcdml_menu_clear(void);
void lcdml_menu_control(void);
void UI_func_reverb_roomsize(uint8_t param);
void UI_func_reverb_damping(uint8_t param);
void UI_func_reverb_lowpass(uint8_t param);
void UI_func_reverb_lodamp(uint8_t param);
void UI_func_reverb_hidamp(uint8_t param);
void UI_func_reverb_diffusion(uint8_t param);
void UI_func_reverb_level(uint8_t param);
void UI_func_chorus_frequency(uint8_t param);
void UI_func_chorus_waveform(uint8_t param);
void UI_func_chorus_depth(uint8_t param);
void UI_func_chorus_level(uint8_t param);
void UI_func_delay_time(uint8_t param);
void UI_func_delay_feedback(uint8_t param);
void UI_func_delay_level(uint8_t param);
void UI_func_reverb_send(uint8_t param);
void UI_func_filter_cutoff(uint8_t param);
void UI_func_filter_resonance(uint8_t param);
void UI_func_drum_reverb_send(uint8_t param);
void UI_func_transpose(uint8_t param);
void UI_func_tune(uint8_t param);
void UI_func_midi_channel(uint8_t param);
void UI_func_lowest_note(uint8_t param);
void UI_func_highest_note(uint8_t param);
void UI_func_sound_intensity(uint8_t param);
void UI_func_panorama(uint8_t param);
void UI_func_stereo_mono(uint8_t param);
void UI_func_note_refresh(uint8_t param);
void UI_func_polyphony(uint8_t param);
void UI_func_mono_poly(uint8_t param);
void UI_func_pb_range(uint8_t param);
void UI_func_pb_step(uint8_t param);
void UI_func_mw_range(uint8_t param);
void UI_func_mw_assign(uint8_t param);
void UI_func_mw_mode(uint8_t param);
void UI_func_fc_range(uint8_t param);
void UI_func_fc_assign(uint8_t param);
void UI_func_fc_mode(uint8_t param);
void UI_func_bc_range(uint8_t param);
void UI_func_bc_assign(uint8_t param);
void UI_func_bc_mode(uint8_t param);
void UI_func_at_range(uint8_t param);
void UI_func_at_assign(uint8_t param);
void UI_func_at_mode(uint8_t param);
void UI_func_portamento_mode(uint8_t param);
void UI_func_portamento_glissando(uint8_t param);
void UI_func_portamento_time(uint8_t param);
void UI_handle_OP(uint8_t param);
void UI_func_information(uint8_t param);
void UI_func_set_performance_name(uint8_t param);
void UI_func_volume(uint8_t param);
void UI_func_smart_filter(uint8_t param);
void UI_func_load_performance(uint8_t param);
void UI_func_save_performance(uint8_t param);
void UI_func_save_voice(uint8_t param);
void UI_func_midi_soft_thru(uint8_t param);
void UI_func_velocity_level(uint8_t param);
void UI_func_engine(uint8_t param);
void UI_func_voice_select(uint8_t param);
void UI_func_sysex_send_voice(uint8_t param);
void UI_func_sysex_receive_bank(uint8_t param);
void UI_func_sysex_send_bank(uint8_t param);
void UI_func_eq_1(uint8_t param);
void UI_func_eq_2(uint8_t param);
void UI_func_eq_3(uint8_t param);
void UI_func_eq_4(uint8_t param);
void UI_func_eq_5(uint8_t param);
void UI_func_eq_6(uint8_t param);
void UI_func_eq_7(uint8_t param);
void UI_func_startup(uint8_t param);
void UI_function_not_enabled(void);
void UI_function_not_implemented(uint8_t param);
void UI_func_favorites(uint8_t param);
void UI_func_epiano_sound_intensity(uint8_t param);
void UI_func_epiano_panorama(uint8_t param);
void UI_func_epiano_decay(uint8_t param);
void UI_func_epiano_release(uint8_t param);
void UI_func_epiano_hardness(uint8_t param);
void UI_func_epiano_treble(uint8_t param);
void UI_func_epiano_stereo(uint8_t param);
void UI_func_epiano_tune(uint8_t param);
void UI_func_epiano_detune(uint8_t param);
void UI_func_epiano_pan_tremolo(uint8_t param);
void UI_func_epiano_pan_lfo(uint8_t param);
void UI_func_epiano_overdrive(uint8_t param);
void UI_func_epiano_reverb_send(uint8_t param);
void UI_func_epiano_midi_channel(uint8_t param);
void UI_func_epiano_lowest_note(uint8_t param);
void UI_func_epiano_highest_note(uint8_t param);
void UI_func_epiano_transpose(uint8_t param);
void UI_func_epiano_polyphony(uint8_t param);
void UI_func_epiano_velocity_sense(uint8_t param);
void UI_func_epiano_chorus_frequency(uint8_t param);
void UI_func_epiano_chorus_waveform(uint8_t param);
void UI_func_epiano_chorus_depth(uint8_t param);
void UI_func_epiano_chorus_level(uint8_t param);
void UI_func_drum_midi_channel(uint8_t param);
void UI_func_drum_main_volume(uint8_t param);
void UI_func_drum_vol_min_max(uint8_t param);
void UI_func_drum_pan(uint8_t param);
void UI_func_drum_pitch(uint8_t param);
void UI_func_drum_midi_note(uint8_t param);
void UI_update_instance_icons();
bool UI_select_name(uint8_t y, uint8_t x, char* edit_string, uint8_t len, bool init);
uint8_t search_accepted_char(uint8_t c);
void display_int(int16_t var, uint8_t size, bool zeros, bool brackets, bool sign);
void display_float(float var, uint8_t size_number, uint8_t size_fraction, bool zeros, bool brackets, bool sign);
void display_bar_int(const char* title, uint32_t value, float factor, int32_t min_value, int32_t max_value, uint8_t size, bool zeros, bool sign, bool init);
void display_bar_float(const char* title, float value, float factor, int32_t min_value, int32_t max_value, uint8_t size_number, uint8_t size_fraction, bool zeros, bool sign, bool init);
void display_meter_int(const char* title, uint32_t value, float factor, float offset, int32_t min_value, int32_t max_value, uint8_t size, bool zeros, bool sign, bool init);
void display_meter_float(const char* title, float value, float factor, float offset, int32_t min_value, int32_t max_value, uint8_t size_number, uint8_t size_fraction, bool zeros, bool sign, bool init);
void lcd_active_instance_number(uint8_t instance_id);
void lcd_OP_active_instance_number(uint8_t instance_id, uint8_t op);
void lcd_special_chars(uint8_t mode);
void string_trim(char* s);
void save_favorite(uint8_t b, uint8_t v, uint8_t instance_id);
void draw_favorite_icon(uint8_t b, uint8_t v, uint8_t instance_id);
bool check_favorite(uint8_t b, uint8_t v, uint8_t instance_id);
bool quick_check_favorites_in_bank(uint8_t b, uint8_t instance_id);
void locate_previous_non_favorite();
void locate_previous_favorite();
void locate_next_favorite();
void locate_next_non_favorite();
void locate_random_non_favorite();

char* basename(const char* filename);
char* strip_extension(char* filename);

// normal menu
LCDMenuLib2_menu LCDML_0(255, 0, 0, NULL, NULL);  // normal root menu element (do not change)
LCDMenuLib2 LCDML(LCDML_0, _LCDML_DISP_rows, _LCDML_DISP_cols, lcdml_menu_display, lcdml_menu_clear, lcdml_menu_control);

#include "UI.h"

int favsearcher = 0;

// create menu
LCDML_createMenu(_LCDML_DISP_cnt);

/***********************************************************************
   CONTROL
 ***********************************************************************/
class EncoderDirection {
public:
  EncoderDirection(void) {
    reset();
  }

  void reset(void) {
    button_short = false;
    button_long = false;
    button_pressed = false;
    left = false;
    right = false;
    up = false;
    down = false;
  }

  void ButtonShort(bool state) {
    button_short = state;
  }

  bool ButtonShort(void) {
    if (button_short == true) {
      button_short = false;
      return (true);
    }
    return (false);
  }

  void ButtonLong(bool state) {
    button_long = state;
  }

  bool ButtonLong(void) {
    if (button_long == true) {
      button_long = false;
      return (true);
    }
    return (false);
  }

  void ButtonPressed(bool state) {
    button_pressed = state;
  }

  bool ButtonPressed(void) {
    return (button_pressed);
  }

  void Left(bool state) {
    left = state;
  }

  bool Left(void) {
    if (left == true) {
      left = false;
      return (true);
    }
    return (false);
  }

  void Right(bool state) {
    right = state;
  }

  bool Right(void) {
    if (right == true) {
      right = false;
      return (true);
    }
    return (false);
  }

  void Up(bool state) {
    up = state;
  }

  bool Up(void) {
    if (up == true) {
      up = false;
      return (true);
    }
    return (false);
  }

  void Down(bool state) {
    down = state;
  }

  bool Down(void) {
    if (down == true) {
      down = false;
      return (true);
    }
    return (false);
  }

private:
  bool button_short;
  bool button_long;
  bool button_pressed;
  bool left;
  bool right;
  bool up;
  bool down;
};

MD_REncoder ENCODER[NUM_ENCODER] = { MD_REncoder(ENC_R_PIN_B, ENC_R_PIN_A), MD_REncoder(ENC_L_PIN_B, ENC_L_PIN_A) };
EncoderDirection encoderDir[NUM_ENCODER];

long g_LCDML_CONTROL_button_press_time[NUM_ENCODER] = { 0, 0 };
bool g_LCDML_CONTROL_button_prev[NUM_ENCODER] = { HIGH, HIGH };
uint8_t g_LCDML_CONTROL_prev[NUM_ENCODER] = { 0, 0 };
bool menu_init = true;

#ifdef U8X8_DISPLAY
const uint8_t* flipped_scroll_bar[5];
const uint8_t* flipped_block_bar[7];
const uint8_t* flipped_meter_bar[7];

uint8_t* rotTile(const uint8_t* tile) {
  uint8_t* newt = new uint8_t[8];
  for (int x = 0; x < 8; x++) {
    uint8_t newb = 0;
    for (int y = 0; y < 8; y++) {
      newb |= (tile[y] << x) & 0x80;
      newb >>= 1;
    }
    newt[x] = newb;
  }
  return newt;
}
#endif

FLASHMEM void setup_ui(void) {
  // LCD Begin
#ifdef I2C_DISPLAY
  display.init();
  display.backlight();
  display.clear();
  display.noCursor();
#else
  display.begin();
  display.clear();
  display.setFont(u8x8_font_amstrad_cpc_extended_f);
#endif
  display.setCursor(3, 0);
  display.print(F("MicroDexed"));
  display.setCursor(0, 1);
  display.print(F("(c)parasiTstudio"));

  lcd_special_chars(SCROLLBAR);
  // LCDMenuLib Setup
  LCDML_setup(_LCDML_DISP_cnt);
}

#ifdef DEBUG
FLASHMEM void setup_debug_message(void) {
  // LCD Begin
  display.clear();
  display.setCursor(1, 0);
  display.print(F("* DEBUG MODE *"));
  display.setCursor(1, 1);
  display.print(F("ENABLE CONSOLE"));
  delay(300);
  display.setCursor(1, 1);
  display.print(_LCDML_VERSION);
  display.print(F("  "));
}
#endif

/***********************************************************************
   MENU CONTROL
 ***********************************************************************/
uint8_t get_current_cursor_id(void) {
  LCDMenuLib2_menu* tmp;

  if ((tmp = LCDML.MENU_getCurrentObj()) != NULL)
    return (tmp->getChild(LCDML.MENU_getCursorPosAbs())->getID());
  else
    return (0);
}

void lcdml_menu_control(void) {
  // If something must init, put in in the setup condition
  if (LCDML.BT_setup()) {
    pinMode(BUT_R_PIN, INPUT_PULLUP);
    pinMode(BUT_L_PIN, INPUT_PULLUP);

    ENCODER[ENC_R].begin();
    ENCODER[ENC_L].begin();
  }

  if (back_from_volume > BACK_FROM_VOLUME_MS && LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_volume)) {
    encoderDir[ENC_L].reset();
    encoderDir[ENC_R].reset();

    if (LCDML.MENU_getLastActiveFunctionID() < 0xff)
      LCDML.OTHER_jumpToID(LCDML.MENU_getLastActiveFunctionID());
    else
      LCDML.OTHER_setCursorToID(LCDML.MENU_getLastCursorPositionID());
    //LCDML.FUNC_goBackToMenu();
  }

  // Volatile Variables
  long g_LCDML_CONTROL_Encoder_position[NUM_ENCODER] = { ENCODER[ENC_R].read(), ENCODER[ENC_L].read() };
  bool button[NUM_ENCODER] = { bool(digitalRead(BUT_R_PIN)), bool(digitalRead(BUT_L_PIN)) };

  /************************************************************************************
    Basic encoder handling (from LCDMenuLib2)
   ************************************************************************************/

  // RIGHT
  if (g_LCDML_CONTROL_Encoder_position[ENC_R] <= -3) {
    if (!button[ENC_R]) {
      LCDML.BT_left();
#ifdef DEBUG
      Serial.println(F("ENC-R left"));
#endif
      encoderDir[ENC_R].Left(true);
      g_LCDML_CONTROL_button_prev[ENC_R] = LOW;
      g_LCDML_CONTROL_button_press_time[ENC_R] = -1;
    } else {
#ifdef DEBUG
      Serial.println(F("ENC-R down"));
#endif
      encoderDir[ENC_R].Down(true);
      LCDML.BT_down();
    }
    ENCODER[ENC_R].write(g_LCDML_CONTROL_Encoder_position[ENC_R] + 4);
  } else if (g_LCDML_CONTROL_Encoder_position[ENC_R] >= 3) {
    if (!button[ENC_R]) {
#ifdef DEBUG
      Serial.println(F("ENC-R right"));
#endif
      encoderDir[ENC_R].Right(true);
      LCDML.BT_right();
      g_LCDML_CONTROL_button_prev[ENC_R] = LOW;
      g_LCDML_CONTROL_button_press_time[ENC_R] = -1;
    } else {
#ifdef DEBUG
      Serial.println(F("ENC-R up"));
#endif
      encoderDir[ENC_R].Up(true);
      LCDML.BT_up();
    }
    ENCODER[ENC_R].write(g_LCDML_CONTROL_Encoder_position[ENC_R] - 4);
  } else {
    if (!button[ENC_R] && g_LCDML_CONTROL_button_prev[ENC_R])  //falling edge, button pressed
    {
      encoderDir[ENC_R].ButtonPressed(true);
      g_LCDML_CONTROL_button_prev[ENC_R] = LOW;
      g_LCDML_CONTROL_button_press_time[ENC_R] = millis();
    } else if (button[ENC_R] && !g_LCDML_CONTROL_button_prev[ENC_R])  //rising edge, button not active
    {
      encoderDir[ENC_R].ButtonPressed(false);
      g_LCDML_CONTROL_button_prev[ENC_R] = HIGH;

      if (g_LCDML_CONTROL_button_press_time[ENC_R] < 0) {
        g_LCDML_CONTROL_button_press_time[ENC_R] = millis();
        //Reset for left right action
      } else if ((millis() - g_LCDML_CONTROL_button_press_time[ENC_R]) >= LONG_BUTTON_PRESS) {
#ifdef DEBUG
        Serial.println(F("ENC-R long released"));
#endif
        //LCDML.BT_quit();
        encoderDir[ENC_R].ButtonLong(true);
      } else if ((millis() - g_LCDML_CONTROL_button_press_time[ENC_R]) >= BUT_DEBOUNCE_MS) {
#ifdef DEBUG
        Serial.println(F("ENC-R short"));
#endif
        encoderDir[ENC_R].ButtonShort(true);

        LCDML.BT_enter();
      }
    }
  }

  if (encoderDir[ENC_R].ButtonPressed() == true && (millis() - g_LCDML_CONTROL_button_press_time[ENC_R]) >= LONG_BUTTON_PRESS) {
#ifdef DEBUG
    Serial.println(F("ENC-R long recognized"));
#endif
    encoderDir[ENC_R].ButtonLong(true);

    if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select)) {
      LCDML.BT_enter();
      LCDML.OTHER_updateFunc();
      LCDML.loop_menu();
      encoderDir[ENC_R].ButtonPressed(false);
      encoderDir[ENC_R].ButtonLong(false);
    } else {
      if (LCDML.FUNC_getID() < 0xff)
        LCDML.FUNC_setGBAToLastFunc();
      else
        LCDML.FUNC_setGBAToLastCursorPos();

      LCDML.OTHER_jumpToFunc(UI_func_voice_select);
      encoderDir[ENC_R].reset();
    }
  }

  // LEFT
  if (g_LCDML_CONTROL_Encoder_position[ENC_L] <= -3) {
    if (!button[ENC_L]) {
#ifdef DEBUG
      Serial.println(F("ENC-L left"));
#endif
      encoderDir[ENC_L].Left(true);
      LCDML.BT_left();
      g_LCDML_CONTROL_button_prev[ENC_L] = LOW;
      g_LCDML_CONTROL_button_press_time[ENC_L] = -1;
    } else {
#ifdef DEBUG
      Serial.println(F("ENC-L down"));
#endif
      encoderDir[ENC_L].Down(true);
      LCDML.BT_down();
      if (LCDML.FUNC_getID() != LCDML.OTHER_getIDFromFunction(UI_func_volume)) {
        LCDML.OTHER_jumpToFunc(UI_func_volume);
      }
    }
    ENCODER[ENC_L].write(g_LCDML_CONTROL_Encoder_position[ENC_L] + 4);
  } else if (g_LCDML_CONTROL_Encoder_position[ENC_L] >= 3) {
    if (!button[ENC_L]) {
#ifdef DEBUG
      Serial.println(F("ENC-L right"));
#endif
      encoderDir[ENC_L].Right(true);
      LCDML.BT_right();
      g_LCDML_CONTROL_button_prev[ENC_L] = LOW;
      g_LCDML_CONTROL_button_press_time[ENC_L] = -1;
    } else {
#ifdef DEBUG
      Serial.println(F("ENC-L up"));
#endif
      encoderDir[ENC_L].Up(true);
      LCDML.BT_up();
      if (LCDML.FUNC_getID() != LCDML.OTHER_getIDFromFunction(UI_func_volume)) {
        LCDML.OTHER_jumpToFunc(UI_func_volume);
      }
    }
    ENCODER[ENC_L].write(g_LCDML_CONTROL_Encoder_position[ENC_L] - 4);
  } else {
    if (!button[ENC_L] && g_LCDML_CONTROL_button_prev[ENC_L])  //falling edge, button pressed
    {
      encoderDir[ENC_L].ButtonPressed(true);
      g_LCDML_CONTROL_button_prev[ENC_L] = LOW;
      g_LCDML_CONTROL_button_press_time[ENC_L] = millis();
    } else if (button[ENC_L] && !g_LCDML_CONTROL_button_prev[ENC_L])  //rising edge, button not active
    {
      encoderDir[ENC_L].ButtonPressed(false);
      g_LCDML_CONTROL_button_prev[ENC_L] = HIGH;

      if (g_LCDML_CONTROL_button_press_time[ENC_L] < 0) {
        g_LCDML_CONTROL_button_press_time[ENC_L] = millis();
        //Reset for left right action
      } else if ((millis() - g_LCDML_CONTROL_button_press_time[ENC_L]) >= LONG_BUTTON_PRESS) {
#ifdef DEBUG
        Serial.println(F("ENC-L long released"));
#endif
        //encoderDir[ENC_L].ButtonLong(true);
        //LCDML.BT_quit();
      } else if ((millis() - g_LCDML_CONTROL_button_press_time[ENC_L]) >= BUT_DEBOUNCE_MS) {
        //LCDML.BT_enter();
#ifdef DEBUG
        Serial.println(F("ENC-L short"));
#endif
        encoderDir[ENC_L].ButtonShort(true);

        if ((LCDML.MENU_getLastActiveFunctionID() == 0xff && LCDML.MENU_getLastCursorPositionID() == 0) || menu_init == true) {
          LCDML.MENU_goRoot();
          menu_init = false;
        } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_volume)) {
          encoderDir[ENC_L].reset();
          encoderDir[ENC_R].reset();

          if (LCDML.MENU_getLastActiveFunctionID() < 0xff)
            LCDML.OTHER_jumpToID(LCDML.MENU_getLastActiveFunctionID());
          else
            LCDML.OTHER_setCursorToID(LCDML.MENU_getLastCursorPositionID());
        } else if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_drum_midi_note) && ask_before_quit == false) {
          ask_before_quit = true;
          Serial.printf("ask_before_quit = %d\n", ask_before_quit);
          encoderDir[ENC_L].reset();
          encoderDir[ENC_R].reset();
          LCDML.OTHER_updateFunc();
          LCDML.loop_menu();
        } else {
          ask_before_quit = false;
          Serial.printf("ask_before_quit = %d\n", ask_before_quit);
          LCDML.BT_quit();
        }
      }
    }
  }

  if (encoderDir[ENC_L].ButtonPressed() == true && (millis() - g_LCDML_CONTROL_button_press_time[ENC_L]) >= LONG_BUTTON_PRESS) {
#ifdef DEBUG
    Serial.println(F("ENC-L long recognized"));
#endif

    // when in Voice select Menu, long left-press sets/unsets favorite
    if (LCDML.FUNC_getID() == LCDML.OTHER_getIDFromFunction(UI_func_voice_select))
      save_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);

    //for (uint8_t i = 0; i < NUM_DEXED; i++)
    //  MicroDexed[i]->panic();

    encoderDir[ENC_L].reset();
    encoderDir[ENC_R].reset();
  }
}

/***********************************************************************
   MENU DISPLAY
 ***********************************************************************/
void lcdml_menu_clear(void) {
  display.clear();
  display.setCursor(0, 0);
}

void lcdml_menu_display(void) {
  // update content
  // ***************
  if (LCDML.DISP_checkMenuUpdate()) {
    // clear menu
    // ***************
    LCDML.DISP_clear();

    // declaration of some variables
    // ***************
    // content variable
    char content_text[_LCDML_DISP_cols];  // save the content text of every menu element
    // menu element object
    LCDMenuLib2_menu* tmp;
    // some limit values
    uint8_t i = LCDML.MENU_getScroll();
    uint8_t maxi = _LCDML_DISP_rows + i;
    uint8_t n = 0;

    // check if this element has children
    if ((tmp = LCDML.MENU_getDisplayedObj()) != NULL) {
      // loop to display lines
      do {
        // check if a menu element has a condition and if the condition be true
        if (tmp->checkCondition()) {
          // check the type off a menu element
          if (tmp->checkType_menu() == true) {
            // display normal content
            LCDML_getContent(content_text, tmp->getID());
            display.setCursor(1, n);
            display.print(content_text);
          } else {
            if (tmp->checkType_dynParam()) {
              tmp->callback(n);
            }
          }
          // increment some values
          i++;
          n++;
        }
        // try to go to the next sibling and check the number of displayed rows
      } while (((tmp = tmp->getSibling(1)) != NULL) && (i < maxi));
    }
  }

  if (LCDML.DISP_checkMenuCursorUpdate()) {
    // init vars
    uint8_t n_max = (LCDML.MENU_getChilds() >= _LCDML_DISP_rows) ? _LCDML_DISP_rows : (LCDML.MENU_getChilds());
    uint8_t scrollbar_min = 0;
    uint8_t scrollbar_max = LCDML.MENU_getChilds();
    uint8_t scrollbar_cur_pos = LCDML.MENU_getCursorPosAbs();
    uint8_t scroll_pos = ((1. * n_max * _LCDML_DISP_rows) / (scrollbar_max - 1) * scrollbar_cur_pos);

    // display rows
    for (uint8_t n = 0; n < n_max; n++) {
      //set cursor
      display.setCursor(0, n);

      //set cursor char
      if (n == LCDML.MENU_getCursorPos()) {
        display.write(_LCDML_DISP_cfg_cursor);
      } else {
        display.write(' ');
      }

      // delete or reset scrollbar
      if (_LCDML_DISP_cfg_scrollbar == 1) {
        if (scrollbar_max > n_max) {
#ifdef I2C_DISPLAY
          display.setCursor((_LCDML_DISP_cols - 1), n);
          display.write((uint8_t)0);
#else
          display.drawTile((_LCDML_DISP_cols - 1), n, 1, flipped_scroll_bar[0]);
          display.setCursor((_LCDML_DISP_cols), n + 1);
#endif
        } else {
          display.setCursor((_LCDML_DISP_cols - 1), n);
          display.print(F(" "));
        }
      }
    }

    // display scrollbar
    if (_LCDML_DISP_cfg_scrollbar == 1) {
      if (scrollbar_max > n_max) {
        //set scroll position
        if (scrollbar_cur_pos == scrollbar_min) {
          // min pos
#ifdef I2C_DISPLAY
          display.setCursor((_LCDML_DISP_cols - 1), 0);
          display.write((uint8_t)1);
#else
          display.drawTile((_LCDML_DISP_cols - 1), 0, 1, flipped_scroll_bar[1]);
          display.setCursor((_LCDML_DISP_cols), 1);
#endif
        } else if (scrollbar_cur_pos == (scrollbar_max - 1)) {
          // max pos
#ifdef I2C_DISPLAY
          display.setCursor((_LCDML_DISP_cols - 1), (n_max - 1));
          display.write((uint8_t)4);
#else
          display.drawTile((_LCDML_DISP_cols - 1), (n_max - 1), 1, flipped_scroll_bar[4]);
          display.setCursor((_LCDML_DISP_cols), (n_max));
#endif
        } else {
          // between
#ifdef I2C_DISPLAY
          display.setCursor((_LCDML_DISP_cols - 1), scroll_pos / n_max);
          display.write((uint8_t)(scroll_pos % n_max) + 1);
#else
          display.drawTile((_LCDML_DISP_cols - 1), scroll_pos / n_max, 1, flipped_scroll_bar[(scroll_pos % n_max) + 1]);
          display.setCursor((_LCDML_DISP_cols), (scroll_pos / n_max) + 1);
#endif
        }
      }
    }
  }
}

//####################################################################################################################################################################################################

/***********************************************************************
   MENU
 ***********************************************************************/

void UI_func_reverb_roomsize(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Room", configuration.fx.reverb_roomsize, 1.0, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.reverb_roomsize = constrain(configuration.fx.reverb_roomsize + ENCODER[ENC_R].speed(), REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.reverb_roomsize = constrain(configuration.fx.reverb_roomsize - ENCODER[ENC_R].speed(), REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX);
    }
    display_bar_int("Reverb Room", configuration.fx.reverb_roomsize, 1.0, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 3, false, false, false);

    reverb.size(mapfloat(configuration.fx.reverb_roomsize, REVERB_ROOMSIZE_MIN, REVERB_ROOMSIZE_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_reverb_lowpass(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Lowpass", configuration.fx.reverb_lowpass, 1.0, REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.reverb_lowpass = constrain(configuration.fx.reverb_lowpass + ENCODER[ENC_R].speed(), REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.reverb_lowpass = constrain(configuration.fx.reverb_lowpass - ENCODER[ENC_R].speed(), REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX);
    }

    display_bar_int("Reverb Lowpass", configuration.fx.reverb_lowpass, 1.0, REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX, 3, false, false, false);

    reverb.lowpass(mapfloat(configuration.fx.reverb_lowpass, REVERB_LOWPASS_MIN, REVERB_LOWPASS_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_reverb_lodamp(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Lodamp.", configuration.fx.reverb_lodamp, 1.0, REVERB_LODAMP_MIN, REVERB_LODAMP_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.reverb_lodamp = constrain(configuration.fx.reverb_lodamp + ENCODER[ENC_R].speed(), REVERB_LODAMP_MIN, REVERB_LODAMP_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.reverb_lodamp = constrain(configuration.fx.reverb_lodamp - ENCODER[ENC_R].speed(), REVERB_LODAMP_MIN, REVERB_LODAMP_MAX);
    }

    display_bar_int("Reverb Lodamp.", configuration.fx.reverb_lodamp, 1.0, REVERB_LODAMP_MIN, REVERB_LODAMP_MAX, 3, false, false, false);

    reverb.lodamp(mapfloat(configuration.fx.reverb_lodamp, REVERB_LODAMP_MIN, REVERB_LODAMP_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_reverb_hidamp(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Hidamp.", configuration.fx.reverb_hidamp, 1.0, REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.reverb_hidamp = constrain(configuration.fx.reverb_hidamp + ENCODER[ENC_R].speed(), REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.reverb_hidamp = constrain(configuration.fx.reverb_hidamp - ENCODER[ENC_R].speed(), REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX);
    }

    display_bar_int("Reverb Hidamp.", configuration.fx.reverb_hidamp, 1.0, REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX, 3, false, false, false);

    reverb.hidamp(mapfloat(configuration.fx.reverb_hidamp, REVERB_HIDAMP_MIN, REVERB_HIDAMP_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_reverb_diffusion(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Diff.", configuration.fx.reverb_diffusion, 1.0, REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.reverb_diffusion = constrain(configuration.fx.reverb_diffusion + ENCODER[ENC_R].speed(), REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.reverb_diffusion = constrain(configuration.fx.reverb_diffusion - ENCODER[ENC_R].speed(), REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX);
    }

    display_bar_int("Reverb Diff.", configuration.fx.reverb_diffusion, 1.0, REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX, 3, false, false, false);

    reverb.diffusion(mapfloat(configuration.fx.reverb_diffusion, REVERB_DIFFUSION_MIN, REVERB_DIFFUSION_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_reverb_level(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Level", configuration.fx.reverb_level, 1.0, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.reverb_level = constrain(configuration.fx.reverb_level + ENCODER[ENC_R].speed(), REVERB_LEVEL_MIN, REVERB_LEVEL_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.reverb_level = constrain(configuration.fx.reverb_level - ENCODER[ENC_R].speed(), REVERB_LEVEL_MIN, REVERB_LEVEL_MAX);
    }

    display_bar_int("Reverb Level", configuration.fx.reverb_level, 1.0, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 3, false, false, true);

    //master_mixer_r.gain(MASTER_MIX_CH_REVERB, pseudo_log_curve(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, 1.0)));
    //master_mixer_l.gain(MASTER_MIX_CH_REVERB, pseudo_log_curve(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, 1.0)));
    master_mixer_r.gain(MASTER_MIX_CH_REVERB, volume_transform(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, VOL_MAX_FLOAT)));
    master_mixer_l.gain(MASTER_MIX_CH_REVERB, volume_transform(mapfloat(configuration.fx.reverb_level, REVERB_LEVEL_MIN, REVERB_LEVEL_MAX, 0.0, VOL_MAX_FLOAT)));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_chorus_frequency(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_float("Chorus Frq.", configuration.fx.chorus_frequency[selected_instance_id], 0.1, CHORUS_FREQUENCY_MIN, CHORUS_FREQUENCY_MAX, 2, 1, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.fx.chorus_frequency[selected_instance_id] = constrain(configuration.fx.chorus_frequency[selected_instance_id] + ENCODER[ENC_R].speed(), CHORUS_FREQUENCY_MIN, CHORUS_FREQUENCY_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.chorus_frequency[selected_instance_id] = constrain(configuration.fx.chorus_frequency[selected_instance_id] - ENCODER[ENC_R].speed(), CHORUS_FREQUENCY_MIN, CHORUS_FREQUENCY_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }
    display_bar_float("Chorus Frq.", configuration.fx.chorus_frequency[selected_instance_id], 0.1, CHORUS_FREQUENCY_MIN, CHORUS_FREQUENCY_MAX, 2, 1, false, false, false);

    chorus_modulator[selected_instance_id]->frequency(configuration.fx.chorus_frequency[selected_instance_id] / 10.0);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_chorus_waveform(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Chorus Wavefrm"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down())
      configuration.fx.chorus_waveform[selected_instance_id] = constrain(configuration.fx.chorus_waveform[selected_instance_id] + 1, CHORUS_WAVEFORM_MIN, CHORUS_WAVEFORM_MAX);
    else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())
      configuration.fx.chorus_waveform[selected_instance_id] = constrain(configuration.fx.chorus_waveform[selected_instance_id] - 1, CHORUS_WAVEFORM_MIN, CHORUS_WAVEFORM_MAX);
#if NUM_DEXED > 1
    else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
#endif

    display.setCursor(0, 1);
    switch (configuration.fx.chorus_waveform[selected_instance_id]) {
      case 0:
        chorus_modulator[selected_instance_id]->begin(WAVEFORM_TRIANGLE);
        display.print(F("[TRIANGLE]"));
        break;
      case 1:
        chorus_modulator[selected_instance_id]->begin(WAVEFORM_SINE);
        display.print(F("[SINE    ]"));
        break;
      default:
        chorus_modulator[selected_instance_id]->begin(WAVEFORM_TRIANGLE);
        display.print(F("[TRIANGLE]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_chorus_depth(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Chorus Dpt.", configuration.fx.chorus_depth[selected_instance_id], 1.0, CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.fx.chorus_depth[selected_instance_id] = constrain(configuration.fx.chorus_depth[selected_instance_id] + ENCODER[ENC_R].speed(), CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.chorus_depth[selected_instance_id] = constrain(configuration.fx.chorus_depth[selected_instance_id] - ENCODER[ENC_R].speed(), CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Chorus Dpt.", configuration.fx.chorus_depth[selected_instance_id], 1.0, CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX, 3, false, false, false);

    chorus_modulator[selected_instance_id]->amplitude(configuration.fx.chorus_depth[selected_instance_id] / 100.0);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_chorus_level(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Chorus Lvl.", configuration.fx.chorus_level[selected_instance_id], 1.0, CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.chorus_level[selected_instance_id] = constrain(configuration.fx.chorus_level[selected_instance_id] + ENCODER[ENC_R].speed(), CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 93, configuration.fx.chorus_level[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.chorus_level[selected_instance_id] = constrain(configuration.fx.chorus_level[selected_instance_id] - ENCODER[ENC_R].speed(), CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 93, configuration.fx.chorus_level[selected_instance_id]);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Chorus Lvl.", configuration.fx.chorus_level[selected_instance_id], 1.0, CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 3, false, false, false);

    //chorus_mixer[selected_instance_id]->gain(0, pseudo_log_curve(1.0 - mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5)));
    //chorus_mixer[selected_instance_id]->gain(1, pseudo_log_curve(mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5)));
    chorus_mixer[selected_instance_id]->gain(0, 1.0 - mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5));
    //chorus_mixer[selected_instance_id]->gain(1, mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_delay_time(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);

#if DELAY_TIME_MAX >= 1000
    display_bar_int("Delay Time", configuration.fx.delay_time[selected_instance_id], 10.0, DELAY_TIME_MIN, DELAY_TIME_MAX, 5, false, false, true);
#elif DELAY_TIME_MAX >= 100
    display_bar_int("Delay Time", configuration.fx.delay_time[selected_instance_id], 10.0, DELAY_TIME_MIN, DELAY_TIME_MAX, 4, false, false, true);
#else
    display_bar_int("Delay Time", configuration.fx.delay_time[selected_instance_id], 10.0, DELAY_TIME_MIN, DELAY_TIME_MAX, 3, false, false, true);
#endif

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.delay_time[selected_instance_id] = constrain(configuration.fx.delay_time[selected_instance_id] + ENCODER[ENC_R].speed(), DELAY_TIME_MIN, DELAY_TIME_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 105, configuration.fx.delay_time[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.delay_time[selected_instance_id] = constrain(configuration.fx.delay_time[selected_instance_id] - ENCODER[ENC_R].speed(), DELAY_TIME_MIN, DELAY_TIME_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 105, configuration.fx.delay_time[selected_instance_id]);
      }
    }

#if NUM_DEXED > 1
    if (LCDML.BT_checkEnter()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
#endif

#if DELAY_TIME_MAX >= 1000
    display_bar_int("Delay Time", configuration.fx.delay_time[selected_instance_id], 10.0, DELAY_TIME_MIN, DELAY_TIME_MAX, 5, false, false, true);
#elif DELAY_TIME_MAX >= 100
    display_bar_int("Delay Time", configuration.fx.delay_time[selected_instance_id], 10.0, DELAY_TIME_MIN, DELAY_TIME_MAX, 4, false, false, true);
#else
    display_bar_int("Delay Time", configuration.fx.delay_time[selected_instance_id], 10.0, DELAY_TIME_MIN, DELAY_TIME_MAX, 3, false, false, true);
#endif
    delay_fx[selected_instance_id]->delay(0, constrain(configuration.fx.delay_time[selected_instance_id], DELAY_TIME_MIN, DELAY_TIME_MAX) * 10);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_delay_feedback(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Delay Feedb.", configuration.fx.delay_feedback[selected_instance_id], 1.0, DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.delay_feedback[selected_instance_id] = constrain(configuration.fx.delay_feedback[selected_instance_id] + ENCODER[ENC_R].speed(), DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 106, configuration.fx.delay_feedback[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.delay_feedback[selected_instance_id] = constrain(configuration.fx.delay_feedback[selected_instance_id] - ENCODER[ENC_R].speed(), DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 106, configuration.fx.delay_feedback[selected_instance_id]);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Delay Feedb.", configuration.fx.delay_feedback[selected_instance_id], 1.0, DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 3, false, false, false);

    delay_fb_mixer[selected_instance_id]->gain(1, midi_volume_transform(map(configuration.fx.delay_feedback[selected_instance_id], DELAY_FEEDBACK_MIN, DELAY_FEEDBACK_MAX, 0, 127)));  // amount of feedback
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_delay_level(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Delay Lvl.", configuration.fx.delay_level[selected_instance_id], 1.0, DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.delay_level[selected_instance_id] = constrain(configuration.fx.delay_level[selected_instance_id] + ENCODER[ENC_R].speed(), DELAY_LEVEL_MIN, DELAY_LEVEL_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 107, configuration.fx.delay_level[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.delay_level[selected_instance_id] = constrain(configuration.fx.delay_level[selected_instance_id] - ENCODER[ENC_R].speed(), DELAY_LEVEL_MIN, DELAY_LEVEL_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 107, configuration.fx.delay_level[selected_instance_id]);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Delay Lvl.", configuration.fx.delay_level[selected_instance_id], 1.0, DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 3, false, false, false);

    delay_mixer[selected_instance_id]->gain(1, midi_volume_transform(map(configuration.fx.delay_level[selected_instance_id], DELAY_LEVEL_MIN, DELAY_LEVEL_MAX, 0, 127)));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_reverb_send(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Reverb Send", configuration.fx.reverb_send[selected_instance_id], 1.0, REVERB_SEND_MIN, REVERB_SEND_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.reverb_send[selected_instance_id] = constrain(configuration.fx.reverb_send[selected_instance_id] + ENCODER[ENC_R].speed(), REVERB_SEND_MIN, REVERB_SEND_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 91, configuration.fx.reverb_send[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.reverb_send[selected_instance_id] = constrain(configuration.fx.reverb_send[selected_instance_id] - ENCODER[ENC_R].speed(), REVERB_SEND_MIN, REVERB_SEND_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 91, configuration.fx.reverb_send[selected_instance_id]);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Reverb Send", configuration.fx.reverb_send[selected_instance_id], 1.0, REVERB_SEND_MIN, REVERB_SEND_MAX, 3, false, false, false);

    reverb_mixer_r.gain(selected_instance_id, volume_transform(mapfloat(configuration.fx.reverb_send[selected_instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
    reverb_mixer_l.gain(selected_instance_id, volume_transform(mapfloat(configuration.fx.reverb_send[selected_instance_id], REVERB_SEND_MIN, REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_filter_cutoff(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Filter Cutoff", configuration.fx.filter_cutoff[selected_instance_id], 1.0, FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.filter_cutoff[selected_instance_id] = constrain(configuration.fx.filter_cutoff[selected_instance_id] + ENCODER[ENC_R].speed(), FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 104, configuration.fx.filter_cutoff[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.filter_cutoff[selected_instance_id] = constrain(configuration.fx.filter_cutoff[selected_instance_id] - ENCODER[ENC_R].speed(), FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 104, configuration.fx.filter_cutoff[selected_instance_id]);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Filter Cutoff", configuration.fx.filter_cutoff[selected_instance_id], 1.0, FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 3, false, false, false);

    MicroDexed[selected_instance_id]->setFilterCutoff(mapfloat(configuration.fx.filter_cutoff[selected_instance_id], FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX, 1.0, 0.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_filter_resonance(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Filter Reso.", configuration.fx.filter_resonance[selected_instance_id], 1.0, FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.filter_resonance[selected_instance_id] = constrain(configuration.fx.filter_resonance[selected_instance_id] + ENCODER[ENC_R].speed(), FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 103, configuration.fx.filter_resonance[selected_instance_id]);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.filter_resonance[selected_instance_id] = constrain(configuration.fx.filter_resonance[selected_instance_id] - ENCODER[ENC_R].speed(), FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 103, configuration.fx.filter_resonance[selected_instance_id]);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Filter Reso.", configuration.fx.filter_resonance[selected_instance_id], 1.0, FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 3, false, false, false);

    MicroDexed[selected_instance_id]->setFilterResonance(mapfloat(configuration.fx.filter_resonance[selected_instance_id], FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX, 1.0, 0.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_transpose(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(METERBAR);
    display_meter_int("Transpose", configuration.dexed[selected_instance_id].transpose, 1.0, -24.0, TRANSPOSE_MIN, TRANSPOSE_MAX, 2, false, true, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].transpose = constrain(configuration.dexed[selected_instance_id].transpose + ENCODER[ENC_R].speed(), TRANSPOSE_MIN, TRANSPOSE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].transpose = constrain(configuration.dexed[selected_instance_id].transpose - ENCODER[ENC_R].speed(), TRANSPOSE_MIN, TRANSPOSE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_meter_int("Transpose", configuration.dexed[selected_instance_id].transpose, 1.0, -24.0, TRANSPOSE_MIN, TRANSPOSE_MAX, 2, false, true, true);

    MicroDexed[selected_instance_id]->setTranspose(configuration.dexed[selected_instance_id].transpose);
    MicroDexed[selected_instance_id]->notesOff();
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 144, configuration.dexed[selected_instance_id].transpose, 0);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_tune(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(METERBAR);
    display_meter_int("Fine Tune", configuration.dexed[selected_instance_id].tune, 1.0, -100.0, TUNE_MIN, TUNE_MAX, 3, false, true, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.dexed[selected_instance_id].tune = constrain(configuration.dexed[selected_instance_id].tune + ENCODER[ENC_R].speed(), TUNE_MIN, TUNE_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 94, configuration.dexed[selected_instance_id].tune);
      } else if (LCDML.BT_checkUp()) {
        configuration.dexed[selected_instance_id].tune = constrain(configuration.dexed[selected_instance_id].tune - ENCODER[ENC_R].speed(), TUNE_MIN, TUNE_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 94, configuration.dexed[selected_instance_id].tune);
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_meter_int("Fine Tune", configuration.dexed[selected_instance_id].tune, 1.0, -100.0, TUNE_MIN, TUNE_MAX, 3, false, true, false);

    MicroDexed[selected_instance_id]->setMasterTune((int((configuration.dexed[selected_instance_id].tune - 100) / 100.0 * 0x4000) << 11) * (1.0 / 12));
    MicroDexed[selected_instance_id]->doRefreshVoice();
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_midi_channel(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("MIDI Channel"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down())
      configuration.dexed[selected_instance_id].midi_channel = constrain(configuration.dexed[selected_instance_id].midi_channel + ENCODER[ENC_R].speed(), MIDI_CHANNEL_MIN, MIDI_CHANNEL_MAX);
    else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())
      configuration.dexed[selected_instance_id].midi_channel = constrain(configuration.dexed[selected_instance_id].midi_channel - ENCODER[ENC_R].speed(), MIDI_CHANNEL_MIN, MIDI_CHANNEL_MAX);
#if NUM_DEXED > 1
    else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
#endif

    display.setCursor(0, 1);
    if (configuration.dexed[selected_instance_id].midi_channel == 0) {
      display.print(F("[OMNI]"));
    } else {
      display_int(configuration.dexed[selected_instance_id].midi_channel, 4, false, true, false);
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void getNoteName(char* noteName, uint8_t noteNumber) {
  char notes[12][3] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
  uint8_t oct_index = noteNumber - 12;

  noteNumber -= 21;
  snprintf_P(noteName, sizeof(noteName), PSTR("%2s%1d"), notes[noteNumber % 12], oct_index / 12);
}

void UI_func_lowest_note(uint8_t param) {
  char note_name[4];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    getNoteName(note_name, configuration.dexed[selected_instance_id].lowest_note);
    display.setCursor(0, 0);
    display.print(F("Lowest Note"));
    display.setCursor(0, 1);
    display.print(F("["));
    display.print(note_name);
    display.print(F("]"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();

    midi_learn_mode = MIDI_LEARN_MODE_ON;
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].lowest_note = constrain(configuration.dexed[selected_instance_id].lowest_note + ENCODER[ENC_R].speed(), INSTANCE_LOWEST_NOTE_MIN, INSTANCE_LOWEST_NOTE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].lowest_note = constrain(configuration.dexed[selected_instance_id].lowest_note - ENCODER[ENC_R].speed(), INSTANCE_LOWEST_NOTE_MIN, INSTANCE_LOWEST_NOTE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    getNoteName(note_name, configuration.dexed[selected_instance_id].lowest_note);
    display.setCursor(1, 1);
    display.print(note_name);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_highest_note(uint8_t param) {
  char note_name[4];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    getNoteName(note_name, configuration.dexed[selected_instance_id].highest_note);
    display.setCursor(0, 0);
    display.print(F("Highest Note"));
    display.setCursor(0, 1);
    display.print(F("["));
    display.print(note_name);
    display.print(F("]"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
    midi_learn_mode = MIDI_LEARN_MODE_ON;
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].highest_note = constrain(configuration.dexed[selected_instance_id].highest_note + ENCODER[ENC_R].speed(), INSTANCE_HIGHEST_NOTE_MIN, INSTANCE_HIGHEST_NOTE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].highest_note = constrain(configuration.dexed[selected_instance_id].highest_note - ENCODER[ENC_R].speed(), INSTANCE_HIGHEST_NOTE_MIN, INSTANCE_HIGHEST_NOTE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    getNoteName(note_name, configuration.dexed[selected_instance_id].highest_note);
    display.setCursor(1, 1);
    display.print(note_name);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_sound_intensity(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Voice Level", configuration.dexed[selected_instance_id].sound_intensity, 1.0, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.dexed[selected_instance_id].sound_intensity = constrain(configuration.dexed[selected_instance_id].sound_intensity + ENCODER[ENC_R].speed(), SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 7, configuration.dexed[selected_instance_id].sound_intensity);
      } else if (LCDML.BT_checkUp()) {
        configuration.dexed[selected_instance_id].sound_intensity = constrain(configuration.dexed[selected_instance_id].sound_intensity - ENCODER[ENC_R].speed(), SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX);
        MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 7, configuration.dexed[selected_instance_id].sound_intensity);
      }

#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Voice Level", configuration.dexed[selected_instance_id].sound_intensity, 1.0, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 3, false, false, false);
    MicroDexed[selected_instance_id]->setGain(midi_volume_transform(map(configuration.dexed[selected_instance_id].sound_intensity, SOUND_INTENSITY_MIN, SOUND_INTENSITY_MAX, 0, 127)));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_panorama(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    if (configuration.sys.mono > 0) {
      display.setCursor(0, 0);
      display.print(F("Panorama"));
      display.setCursor(0, 1);
      display.print(F("MONO-disabled"));
      return;
    }
    lcd_special_chars(METERBAR);
    display_meter_float("Panorama", configuration.dexed[selected_instance_id].pan, 0.05, -20.0, PANORAMA_MIN, PANORAMA_MAX, 1, 1, false, true, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down() && configuration.sys.mono == 0) {
      configuration.dexed[selected_instance_id].pan = constrain(configuration.dexed[selected_instance_id].pan + ENCODER[ENC_R].speed(), PANORAMA_MIN, PANORAMA_MAX);
      MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 10, map(configuration.dexed[selected_instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, 0, 127));
    } else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up() && configuration.sys.mono == 0) {
      configuration.dexed[selected_instance_id].pan = constrain(configuration.dexed[selected_instance_id].pan - ENCODER[ENC_R].speed(), PANORAMA_MIN, PANORAMA_MAX);
      MD_sendControlChange(configuration.dexed[selected_instance_id].midi_channel, 10, map(configuration.dexed[selected_instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, 0, 127));
    }
#if NUM_DEXED > 1
    else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
#endif

    if (configuration.sys.mono == 0) {
      display_meter_float("Panorama", configuration.dexed[selected_instance_id].pan, 0.05, -20.0, PANORAMA_MIN, PANORAMA_MAX, 1, 1, false, true, false);
      mono2stereo[selected_instance_id]->panorama(mapfloat(configuration.dexed[selected_instance_id].pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_favorites(uint8_t param) {
  static uint8_t old_favorites;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_L].reset();
    old_favorites = configuration.sys.favorites;

    display.setCursor(0, 0);
    display.print(F("Favorites"));
    display.setCursor(0, 1);
    switch (configuration.sys.favorites) {
      case 0:
        display.print(F("[ All  presets ]"));
        break;
      case 1:
        display.print(F("[  FAVs. only  ]"));
        break;
      case 2:
        display.print(F("[non-FAVs. only]"));
        break;
      case 3:
        display.print(F("[random non-FAV]"));
        break;
    }
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown())
      configuration.sys.favorites = constrain(configuration.sys.favorites + 1, 0, 3);
    else if (LCDML.BT_checkUp())
      configuration.sys.favorites = constrain(configuration.sys.favorites - 1, 0, 3);

    display.setCursor(0, 1);
    switch (configuration.sys.favorites) {
      case 0:
        display.print(F("[ All  presets ]"));
        break;
      case 1:
        display.print(F("[  FAVs. only  ]"));
        break;
      case 2:
        display.print(F("[non-FAVs. only]"));
        break;
      case 3:
        display.print(F("[random non-FAV]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_L].reset();
    if (old_favorites != configuration.sys.favorites) {
      save_sys_flag = true;
      save_sys = 0;
    }
  }
}

void UI_func_epiano_midi_channel(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("EP MIDI Channel"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down())
      configuration.epiano.midi_channel = constrain(configuration.epiano.midi_channel + ENCODER[ENC_R].speed(), EP_MIDI_CHANNEL_MIN, EP_MIDI_CHANNEL_MAX);
    else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())
      configuration.epiano.midi_channel = constrain(configuration.epiano.midi_channel - ENCODER[ENC_R].speed(), EP_MIDI_CHANNEL_MIN, EP_MIDI_CHANNEL_MAX);

    display.setCursor(0, 1);
    if (configuration.epiano.midi_channel == 0) {
      display.print(F("[OMNI]"));
    } else {
      display_int(configuration.epiano.midi_channel, 4, false, true, false);
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_lowest_note(uint8_t param) {
  char note_name[4];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    midi_learn_mode = MIDI_LEARN_MODE_ON;

    getNoteName(note_name, configuration.epiano.lowest_note);
    display.setCursor(0, 0);
    display.print(F("EP Lowest Note"));
    display.setCursor(0, 1);
    display.print(F("["));
    display.print(note_name);
    display.print(F("]"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.epiano.lowest_note = constrain(configuration.epiano.lowest_note + ENCODER[ENC_R].speed(), EP_LOWEST_NOTE_MIN, EP_LOWEST_NOTE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.epiano.lowest_note = constrain(configuration.epiano.lowest_note - ENCODER[ENC_R].speed(), EP_LOWEST_NOTE_MIN, EP_LOWEST_NOTE_MAX);
    }

    getNoteName(note_name, configuration.epiano.lowest_note);
    display.setCursor(1, 1);
    display.print(note_name);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_epiano_highest_note(uint8_t param) {
  char note_name[4];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    midi_learn_mode = MIDI_LEARN_MODE_ON;

    getNoteName(note_name, configuration.dexed[selected_instance_id].highest_note);
    display.setCursor(0, 0);
    display.print(F("EP Highest Note"));
    display.setCursor(0, 1);
    display.print(F("["));
    display.print(note_name);
    display.print(F("]"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.epiano.highest_note = constrain(configuration.epiano.highest_note + ENCODER[ENC_R].speed(), EP_HIGHEST_NOTE_MIN, EP_HIGHEST_NOTE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.epiano.highest_note = constrain(configuration.epiano.highest_note - ENCODER[ENC_R].speed(), EP_HIGHEST_NOTE_MIN, EP_HIGHEST_NOTE_MAX);
    }

    getNoteName(note_name, configuration.epiano.highest_note);
    display.setCursor(1, 1);
    display.print(note_name);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_epiano_sound_intensity(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Level", configuration.epiano.sound_intensity, 1.0, EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.sound_intensity = constrain(configuration.epiano.sound_intensity + ENCODER[ENC_R].speed(), EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 7, configuration.epiano.sound_intensity);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.sound_intensity = constrain(configuration.epiano.sound_intensity - ENCODER[ENC_R].speed(), EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 7, configuration.epiano.sound_intensity);
      }
    }

    display_bar_int("EP Level", configuration.epiano.sound_intensity, 1.0, EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX, 3, false, false, false);
    ep.setVolume(mapfloat(configuration.epiano.sound_intensity, EP_SOUND_INTENSITY_MIN, EP_SOUND_INTENSITY_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_panorama(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    if (configuration.sys.mono > 0) {
      display.setCursor(0, 0);
      display.print(F("EP Panorama"));
      display.setCursor(0, 1);
      display.print(F("MONO-disabled"));
      return;
    }

    lcd_special_chars(METERBAR);
    display_meter_float("EP Panorama", configuration.epiano.pan, 0.05, -20.0, EP_PANORAMA_MIN, EP_PANORAMA_MAX, 1, 1, false, true, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down() && configuration.sys.mono == 0) {
      configuration.epiano.pan = constrain(configuration.epiano.pan + ENCODER[ENC_R].speed(), EP_PANORAMA_MIN, EP_PANORAMA_MAX);
      MD_sendControlChange(configuration.epiano.midi_channel, 10, map(configuration.epiano.pan, EP_PANORAMA_MIN, EP_PANORAMA_MAX, 0, 127));
    } else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up() && configuration.sys.mono == 0) {
      configuration.epiano.pan = constrain(configuration.epiano.pan - ENCODER[ENC_R].speed(), EP_PANORAMA_MIN, EP_PANORAMA_MAX);
      MD_sendControlChange(configuration.epiano.midi_channel, 10, map(configuration.epiano.pan, EP_PANORAMA_MIN, EP_PANORAMA_MAX, 0, 127));
    }

    if (configuration.sys.mono == 0) {
      display_meter_float("EP Panorama", configuration.epiano.pan, 0.05, -20.0, EP_PANORAMA_MIN, EP_PANORAMA_MAX, 1, 1, false, true, false);
      ep_stereo_panorama.panorama(mapfloat(configuration.epiano.pan, PANORAMA_MIN, PANORAMA_MAX, -1.0, 1.0));
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_decay(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Decay", configuration.epiano.decay, 1.0, EP_DECAY_MIN, EP_DECAY_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.decay = constrain(configuration.epiano.decay + ENCODER[ENC_R].speed(), EP_DECAY_MIN, EP_DECAY_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.decay = constrain(configuration.epiano.decay - ENCODER[ENC_R].speed(), EP_DECAY_MIN, EP_DECAY_MAX);
      }
    }

    display_bar_int("EP Decay", configuration.epiano.decay, 1.0, EP_DECAY_MIN, EP_DECAY_MAX, 3, false, false, false);
    ep.setDecay(mapfloat(configuration.epiano.decay, EP_DECAY_MIN, EP_DECAY_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_release(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Release", configuration.epiano.release, 1.0, EP_RELEASE_MIN, EP_RELEASE_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.release = constrain(configuration.epiano.release + ENCODER[ENC_R].speed(), EP_RELEASE_MIN, EP_RELEASE_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.release = constrain(configuration.epiano.release - ENCODER[ENC_R].speed(), EP_RELEASE_MIN, EP_RELEASE_MAX);
      }
    }

    display_bar_int("EP Release", configuration.epiano.release, 1.0, EP_RELEASE_MIN, EP_RELEASE_MAX, 3, false, false, false);
    ep.setRelease(mapfloat(configuration.epiano.release, EP_RELEASE_MIN, EP_RELEASE_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_hardness(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Hardness", configuration.epiano.hardness, 1.0, EP_HARDNESS_MIN, EP_HARDNESS_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.hardness = constrain(configuration.epiano.hardness + ENCODER[ENC_R].speed(), EP_HARDNESS_MIN, EP_HARDNESS_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.hardness = constrain(configuration.epiano.hardness - ENCODER[ENC_R].speed(), EP_HARDNESS_MIN, EP_HARDNESS_MAX);
      }
    }

    display_bar_int("EP Hardness", configuration.epiano.hardness, 1.0, EP_HARDNESS_MIN, EP_HARDNESS_MAX, 3, false, false, false);
    ep.setHardness(mapfloat(configuration.epiano.hardness, EP_HARDNESS_MIN, EP_HARDNESS_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_treble(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Treble", configuration.epiano.treble, 1.0, EP_TREBLE_MIN, EP_TREBLE_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.treble = constrain(configuration.epiano.treble + ENCODER[ENC_R].speed(), EP_TREBLE_MIN, EP_TREBLE_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.treble = constrain(configuration.epiano.treble - ENCODER[ENC_R].speed(), EP_TREBLE_MIN, EP_TREBLE_MAX);
      }
    }

    display_bar_int("EP Treble", configuration.epiano.treble, 1.0, EP_TREBLE_MIN, EP_TREBLE_MAX, 3, false, false, false);
    ep.setTreble(mapfloat(configuration.epiano.treble, EP_TREBLE_MIN, EP_TREBLE_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_stereo(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Stereo", configuration.epiano.stereo, 1.0, EP_STEREO_MIN, EP_STEREO_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.stereo = constrain(configuration.epiano.stereo + ENCODER[ENC_R].speed(), EP_STEREO_MIN, EP_STEREO_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.stereo = constrain(configuration.epiano.stereo - ENCODER[ENC_R].speed(), EP_STEREO_MIN, EP_STEREO_MAX);
      }
    }

    display_bar_int("EP Stereo", configuration.epiano.stereo, 1.0, EP_STEREO_MIN, EP_STEREO_MAX, 3, false, false, false);
    ep.setStereo(mapfloat(configuration.epiano.stereo, EP_STEREO_MIN, EP_STEREO_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_tune(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(METERBAR);
    display_bar_int("EP Tune", configuration.epiano.tune, 1.0, EP_TUNE_MIN, EP_TUNE_MAX, 3, false, true, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.epiano.tune = constrain(configuration.epiano.tune + ENCODER[ENC_R].speed(), EP_TUNE_MIN, EP_TUNE_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 94, configuration.epiano.tune);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.tune = constrain(configuration.epiano.tune - ENCODER[ENC_R].speed(), EP_TUNE_MIN, EP_TUNE_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 94, configuration.epiano.tune);
      }
    }

    display_bar_int("EP Tune", configuration.epiano.tune, 1.0, EP_TUNE_MIN, EP_TUNE_MAX, 3, false, true, false);
    ep.setTune(mapfloat(configuration.epiano.tune, EP_TUNE_MIN, EP_TUNE_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_detune(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Detune", configuration.epiano.detune, 1.0, EP_DETUNE_MIN, EP_DETUNE_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.detune = constrain(configuration.epiano.detune + ENCODER[ENC_R].speed(), EP_DETUNE_MIN, EP_DETUNE_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.detune = constrain(configuration.epiano.detune - ENCODER[ENC_R].speed(), EP_DETUNE_MIN, EP_DETUNE_MAX);
      }
    }

    display_bar_int("EP Detune", configuration.epiano.detune, 1.0, EP_DETUNE_MIN, EP_DETUNE_MAX, 3, false, false, false);
    ep.setDetune(mapfloat(configuration.epiano.detune, EP_DETUNE_MIN, EP_DETUNE_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_pan_tremolo(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Trem. Width", configuration.epiano.pan_tremolo, 1.0, EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.pan_tremolo = constrain(configuration.epiano.pan_tremolo + ENCODER[ENC_R].speed(), EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.pan_tremolo = constrain(configuration.epiano.pan_tremolo - ENCODER[ENC_R].speed(), EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX);
      }
    }

    display_bar_int("EP Trem. Width", configuration.epiano.pan_tremolo, 1.0, EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX, 3, false, false, false);
    if (configuration.epiano.pan_tremolo == 0)
      ep.setPanTremolo(0.0);
    else
      ep.setPanTremolo(mapfloat(configuration.epiano.pan_tremolo, EP_PAN_TREMOLO_MIN, EP_PAN_TREMOLO_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_pan_lfo(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP LFO", configuration.epiano.pan_lfo, 1.0, EP_PAN_LFO_MIN, EP_PAN_LFO_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.pan_lfo = constrain(configuration.epiano.pan_lfo + ENCODER[ENC_R].speed(), EP_PAN_LFO_MIN, EP_PAN_LFO_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.pan_lfo = constrain(configuration.epiano.pan_lfo - ENCODER[ENC_R].speed(), EP_PAN_LFO_MIN, EP_PAN_LFO_MAX);
      }
    }

    display_bar_int("EP LFO", configuration.epiano.pan_lfo, 1.0, EP_PAN_LFO_MIN, EP_PAN_LFO_MAX, 3, false, false, false);
    if (configuration.epiano.pan_lfo == 0)
      ep.setPanLFO(0.0);
    else
      ep.setPanLFO(mapfloat(configuration.epiano.pan_lfo, EP_PAN_LFO_MIN, EP_PAN_LFO_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_overdrive(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Overdrive", configuration.epiano.overdrive, 1.0, EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.overdrive = constrain(configuration.epiano.overdrive + ENCODER[ENC_R].speed(), EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.overdrive = constrain(configuration.epiano.overdrive - ENCODER[ENC_R].speed(), EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX);
      }
    }

    display_bar_int("EP Overdrive", configuration.epiano.overdrive, 1.0, EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX, 3, false, false, false);
    ep.setOverdrive(mapfloat(configuration.epiano.overdrive, EP_OVERDRIVE_MIN, EP_OVERDRIVE_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_transpose(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(METERBAR);
    display_meter_int("EP Transpose", configuration.epiano.transpose, 1.0, -24.0, EP_TRANSPOSE_MIN, EP_TRANSPOSE_MAX, 2, false, true, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.epiano.transpose = constrain(configuration.epiano.transpose + ENCODER[ENC_R].speed(), EP_TRANSPOSE_MIN, EP_TRANSPOSE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.epiano.transpose = constrain(configuration.epiano.transpose - ENCODER[ENC_R].speed(), EP_TRANSPOSE_MIN, EP_TRANSPOSE_MAX);
    }

    display_meter_int("EP Transpose", configuration.epiano.transpose, 1.0, -24.0, EP_TRANSPOSE_MIN, EP_TRANSPOSE_MAX, 2, false, true, true);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_polyphony(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Polyphony", configuration.epiano.polyphony, 1.0, EP_POLYPHONY_MIN, EP_POLYPHONY_MAX, 2, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.epiano.polyphony = constrain(configuration.epiano.polyphony + 1, EP_POLYPHONY_MIN, EP_POLYPHONY_MAX);
      } else if (LCDML.BT_checkUp()) {
        if (configuration.epiano.polyphony - 1 < 1)
          configuration.epiano.polyphony = 1;
        else {
          configuration.epiano.polyphony = constrain(configuration.epiano.polyphony - 1, EP_POLYPHONY_MIN, EP_POLYPHONY_MAX);
        }
      }
    }

    display_bar_int("EP Polyphony", configuration.epiano.polyphony, 1.0, EP_POLYPHONY_MIN, EP_POLYPHONY_MAX, 2, false, false, false);
    ep.setPolyphony(configuration.epiano.polyphony);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_velocity_sense(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Vel. Sense", configuration.epiano.velocity_sense, 1.0, EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      encoderDir[ENC_R].reset();

      if (LCDML.BT_checkDown()) {
        configuration.epiano.velocity_sense = constrain(configuration.epiano.velocity_sense + ENCODER[ENC_R].speed(), EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.epiano.velocity_sense = constrain(configuration.epiano.velocity_sense - ENCODER[ENC_R].speed(), EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX);
      }
    }

    display_bar_int("EP Vel. Sense", configuration.epiano.velocity_sense, 1.0, EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX, 3, false, false, false);
    ep.setVelocitySense(mapfloat(configuration.epiano.velocity_sense, EP_VELOCITY_SENSE_MIN, EP_VELOCITY_SENSE_MAX, 0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_reverb_send(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Reverb Send", configuration.fx.ep_reverb_send, 1.0, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.ep_reverb_send = constrain(configuration.fx.ep_reverb_send + ENCODER[ENC_R].speed(), EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 91, configuration.fx.ep_reverb_send);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.ep_reverb_send = constrain(configuration.fx.ep_reverb_send - ENCODER[ENC_R].speed(), EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 91, configuration.fx.ep_reverb_send);
      }
    }

    display_bar_int("EP Reverb Send", configuration.fx.ep_reverb_send, 1.0, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX, 3, false, false, false);

    reverb_mixer_r.gain(REVERB_MIX_CH_EPIANO, volume_transform(mapfloat(configuration.fx.ep_reverb_send, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
    reverb_mixer_l.gain(REVERB_MIX_CH_EPIANO, volume_transform(mapfloat(configuration.fx.ep_reverb_send, EP_REVERB_SEND_MIN, EP_REVERB_SEND_MAX, 0.0, VOL_MAX_FLOAT)));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_chorus_frequency(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_float("EP Chorus Frq.", configuration.fx.ep_chorus_frequency, 0.1, EP_CHORUS_FREQUENCY_MIN, EP_CHORUS_FREQUENCY_MAX, 2, 1, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.ep_chorus_frequency = constrain(configuration.fx.ep_chorus_frequency + ENCODER[ENC_R].speed(), EP_CHORUS_FREQUENCY_MIN, EP_CHORUS_FREQUENCY_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.ep_chorus_frequency = constrain(configuration.fx.ep_chorus_frequency - ENCODER[ENC_R].speed(), EP_CHORUS_FREQUENCY_MIN, EP_CHORUS_FREQUENCY_MAX);
    }
    display_bar_float("EP Chorus Frq.", configuration.fx.ep_chorus_frequency, 0.1, EP_CHORUS_FREQUENCY_MIN, EP_CHORUS_FREQUENCY_MAX, 2, 1, false, false, false);
    ep_chorus_modulator.frequency(configuration.fx.ep_chorus_frequency / 10.0);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_chorus_waveform(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("EP Ch. Wavefrm"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down())
      configuration.fx.ep_chorus_waveform = constrain(configuration.fx.ep_chorus_waveform + 1, EP_CHORUS_WAVEFORM_MIN, EP_CHORUS_WAVEFORM_MAX);
    else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())
      configuration.fx.ep_chorus_waveform = constrain(configuration.fx.ep_chorus_waveform - 1, EP_CHORUS_WAVEFORM_MIN, EP_CHORUS_WAVEFORM_MAX);

    display.setCursor(0, 1);
    switch (configuration.fx.ep_chorus_waveform) {
      case 0:
        ep_chorus_modulator.begin(WAVEFORM_TRIANGLE);
        display.print(F("[TRIANGLE]"));
        break;
      case 1:
        ep_chorus_modulator.begin(WAVEFORM_SINE);
        display.print(F("[SINE    ]"));
        break;
      default:
        ep_chorus_modulator.begin(WAVEFORM_TRIANGLE);
        display.print(F("[TRIANGLE]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_chorus_depth(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Ch. Depth", configuration.fx.ep_chorus_depth, 1.0, EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.fx.ep_chorus_depth = constrain(configuration.fx.ep_chorus_depth + ENCODER[ENC_R].speed(), EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX);
      else if (LCDML.BT_checkUp())
        configuration.fx.ep_chorus_depth = constrain(configuration.fx.ep_chorus_depth - ENCODER[ENC_R].speed(), EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX);
    }

    display_bar_int("EP Ch. Depth", configuration.fx.ep_chorus_depth, 1.0, EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX, 3, false, false, false);
    ep_chorus_modulator.amplitude(mapfloat(configuration.fx.ep_chorus_depth, EP_CHORUS_DEPTH_MIN, EP_CHORUS_DEPTH_MAX, 0.0, 1.0));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_epiano_chorus_level(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("EP Ch. Level", configuration.fx.ep_chorus_level, 1.0, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.ep_chorus_level = constrain(configuration.fx.ep_chorus_level + ENCODER[ENC_R].speed(), EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 93, configuration.fx.ep_chorus_level);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.ep_chorus_level = constrain(configuration.fx.ep_chorus_level - ENCODER[ENC_R].speed(), EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX);
        MD_sendControlChange(configuration.epiano.midi_channel, 93, configuration.fx.ep_chorus_level);
      }
    }

    display_bar_int("EP Ch. Level", configuration.fx.ep_chorus_level, 1.0, EP_CHORUS_LEVEL_MIN, EP_CHORUS_LEVEL_MAX, 3, false, false, false);
    ep_chorus_mixer_l.gain(0, 1.0 - mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5));
    ep_chorus_mixer_r.gain(0, 1.0 - mapfloat(configuration.fx.chorus_level[selected_instance_id], CHORUS_LEVEL_MIN, CHORUS_LEVEL_MAX, 0.0, 0.5));
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_stereo_mono(uint8_t param) {
  static uint8_t old_mono;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    old_mono = configuration.sys.mono;

    display.setCursor(0, 0);
    display.print(F("Stereo/Mono"));
    display.setCursor(0, 1);
    switch (configuration.sys.mono) {
      case 0:
        display.print(F("[STEREO]"));
        stereo2mono.stereo(true);
        break;
      case 1:
        display.print(F("[MONO  ]"));
        stereo2mono.stereo(false);
        break;
      case 2:
        display.print(F("[MONO-R]"));
        stereo2mono.stereo(false);
        break;
      case 3:
        display.print(F("[MONO-L]"));
        stereo2mono.stereo(false);
        break;
    }
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown())
      configuration.sys.mono = constrain(configuration.sys.mono + 1, MONO_MIN, MONO_MAX);
    else if (LCDML.BT_checkUp())
      configuration.sys.mono = constrain(configuration.sys.mono - 1, MONO_MIN, MONO_MAX);

    display.setCursor(0, 1);
    switch (configuration.sys.mono) {
      case 0:
        display.print(F("[STEREO]"));
        stereo2mono.stereo(true);
        break;
      case 1:
        display.print(F("[MONO  ]"));
        stereo2mono.stereo(false);
        break;
      case 2:
        display.print(F("[MONO-R]"));
        stereo2mono.stereo(false);
        break;
      case 3:
        display.print(F("[MONO-L]"));
        stereo2mono.stereo(false);
        break;
    }
    set_volume(configuration.sys.vol, configuration.sys.mono);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_L].reset();

    if (old_mono != configuration.sys.mono) {
      save_sys_flag = true;
      save_sys = 0;
    }
  }
}

void UI_func_polyphony(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
#if NUM_DEXED > 1
    display_bar_int("Polyphony", configuration.dexed[selected_instance_id].polyphony, 1.0, POLYPHONY_MIN, POLYPHONY_MAX - configuration.dexed[(selected_instance_id + 1) % NUM_DEXED].polyphony, 2, false, false, true);
#else
    display_bar_int("Polyphony", configuration.dexed[selected_instance_id].polyphony, 1.0, POLYPHONY_MIN, POLYPHONY_MAX, 2, false, false, true);
#endif
    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
#if NUM_DEXED > 1
        if (configuration.dexed[selected_instance_id].polyphony < POLYPHONY_MAX - configuration.dexed[(selected_instance_id + 1) % NUM_DEXED].polyphony)
          configuration.dexed[selected_instance_id].polyphony = constrain(configuration.dexed[selected_instance_id].polyphony + 1, POLYPHONY_MIN, POLYPHONY_MAX - configuration.dexed[(selected_instance_id + 1) % NUM_DEXED].polyphony);
#else
        configuration.dexed[selected_instance_id].polyphony = constrain(configuration.dexed[selected_instance_id].polyphony + 1, POLYPHONY_MIN, POLYPHONY_MAX);
#endif
      } else if (LCDML.BT_checkUp()) {
        if (configuration.dexed[selected_instance_id].polyphony - 1 < 0)
          configuration.dexed[selected_instance_id].polyphony = 0;
        else {
#if NUM_DEXED > 1
          configuration.dexed[selected_instance_id].polyphony = constrain(configuration.dexed[selected_instance_id].polyphony - 1, POLYPHONY_MIN, POLYPHONY_MAX - configuration.dexed[(selected_instance_id + 1) % NUM_DEXED].polyphony);
#else
          configuration.dexed[selected_instance_id].polyphony = constrain(configuration.dexed[selected_instance_id].polyphony - 1, POLYPHONY_MIN, POLYPHONY_MAX);
#endif
        }
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
      lcd_active_instance_number(selected_instance_id);
#endif
    }

#if NUM_DEXED > 1
    display_bar_int("Polyphony", configuration.dexed[selected_instance_id].polyphony, 1.0, POLYPHONY_MIN, POLYPHONY_MAX - configuration.dexed[(selected_instance_id + 1) % NUM_DEXED].polyphony, 2, false, false, false);
#else
    display_bar_int("Polyphony", configuration.dexed[selected_instance_id].polyphony, 1.0, POLYPHONY_MIN, POLYPHONY_MAX, 2, false, false, false);
#endif
    MicroDexed[selected_instance_id]->setMaxNotes(configuration.dexed[selected_instance_id].polyphony);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_mono_poly(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Mono/Poly"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].monopoly = constrain(configuration.dexed[selected_instance_id].monopoly + 1, MONOPOLY_MIN, MONOPOLY_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].monopoly = constrain(configuration.dexed[selected_instance_id].monopoly - 1, MONOPOLY_MIN, MONOPOLY_MAX);

      MicroDexed[selected_instance_id]->setMonoMode(!configuration.dexed[selected_instance_id].monopoly);
      configuration.dexed[selected_instance_id].monopoly = MicroDexed[selected_instance_id]->getMonoMode();
    }
#if NUM_DEXED > 1
    if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
    lcd_active_instance_number(selected_instance_id);
#endif
  }

  display.setCursor(0, 1);
  switch (configuration.dexed[selected_instance_id].monopoly) {
    case 1:
      display.print(F("[MONOPHONIC]"));
      break;
    case 0:
      display.print(F("[POLYPHONIC]"));
      break;
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_note_refresh(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Note Refresh"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].note_refresh = constrain(configuration.dexed[selected_instance_id].note_refresh + 1, NOTE_REFRESH_MIN, NOTE_REFRESH_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].note_refresh = constrain(configuration.dexed[selected_instance_id].note_refresh - 1, NOTE_REFRESH_MIN, NOTE_REFRESH_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setNoteRefreshMode(configuration.dexed[selected_instance_id].note_refresh);
#ifdef DEBUG
    Serial.printf_P(PSTR("Note refresh: %d\n"), configuration.dexed[selected_instance_id].note_refresh);
#endif

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].note_refresh) {
      case 0:
        display.print(F("[NORMAL     ]"));
        break;
      case 1:
        display.print(F("[RETRIGGERED]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_pb_range(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("PB Range", configuration.dexed[selected_instance_id].pb_range, 1.0, PB_RANGE_MIN, PB_RANGE_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].pb_range = constrain(configuration.dexed[selected_instance_id].pb_range + ENCODER[ENC_R].speed(), PB_RANGE_MIN, PB_RANGE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].pb_range = constrain(configuration.dexed[selected_instance_id].pb_range - ENCODER[ENC_R].speed(), PB_RANGE_MIN, PB_RANGE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("PB Range", configuration.dexed[selected_instance_id].pb_range, 1.0, PB_RANGE_MIN, PB_RANGE_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setPBController(configuration.dexed[selected_instance_id].pb_range, configuration.dexed[selected_instance_id].pb_step);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 65, configuration.dexed[selected_instance_id].pb_range, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_pb_step(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("PB Step", configuration.dexed[selected_instance_id].pb_step, 1.0, PB_STEP_MIN, PB_STEP_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].pb_step = constrain(configuration.dexed[selected_instance_id].pb_step + ENCODER[ENC_R].speed(), PB_STEP_MIN, PB_STEP_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].pb_step = constrain(configuration.dexed[selected_instance_id].pb_step - ENCODER[ENC_R].speed(), PB_STEP_MIN, PB_STEP_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("PB Step", configuration.dexed[selected_instance_id].pb_step, 1.0, PB_STEP_MIN, PB_STEP_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setPBController(configuration.dexed[selected_instance_id].pb_range, configuration.dexed[selected_instance_id].pb_step);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 66, configuration.dexed[selected_instance_id].pb_step, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_mw_range(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("MW Range", configuration.dexed[selected_instance_id].mw_range, 1.0, MW_RANGE_MIN, MW_RANGE_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].mw_range = constrain(configuration.dexed[selected_instance_id].mw_range + ENCODER[ENC_R].speed(), MW_RANGE_MIN, MW_RANGE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].mw_range = constrain(configuration.dexed[selected_instance_id].mw_range - ENCODER[ENC_R].speed(), MW_RANGE_MIN, MW_RANGE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("MW Range", configuration.dexed[selected_instance_id].mw_range, 1.0, MW_RANGE_MIN, MW_RANGE_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setMWController(configuration.dexed[selected_instance_id].mw_range, configuration.dexed[selected_instance_id].mw_assign, configuration.dexed[selected_instance_id].mw_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 70, configuration.dexed[selected_instance_id].mw_range, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_mw_assign(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("MW Assign"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].mw_assign = constrain(configuration.dexed[selected_instance_id].mw_assign + 1, MW_ASSIGN_MIN, MW_ASSIGN_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].mw_assign = constrain(configuration.dexed[selected_instance_id].mw_assign - 1, MW_ASSIGN_MIN, MW_ASSIGN_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setMWController(configuration.dexed[selected_instance_id].mw_range, configuration.dexed[selected_instance_id].mw_assign, configuration.dexed[selected_instance_id].mw_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 71, configuration.dexed[selected_instance_id].mw_assign, 2);

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].mw_assign) {
      case 0:
        display.print(F("[   NONE    ]"));
        break;
      case 1:
        display.print(F("[PTCH       ]"));
        break;
      case 2:
        display.print(F("[     AMP   ]"));
        break;
      case 3:
        display.print(F("[PTCH AMP   ]"));
        break;
      case 4:
        display.print(F("[         EG]"));
        break;
      case 5:
        display.print(F("[PTCH     EG]"));
        break;
      case 6:
        display.print(F("[     AMP EG]"));
        break;
      case 7:
        display.print(F("[PTCH AMP EG]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_mw_mode(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("MW Mode"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].mw_mode = constrain(configuration.dexed[selected_instance_id].mw_mode + 1, MW_MODE_MIN, MW_MODE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].mw_mode = constrain(configuration.dexed[selected_instance_id].mw_mode - 1, MW_MODE_MIN, MW_MODE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setMWController(configuration.dexed[selected_instance_id].mw_range, configuration.dexed[selected_instance_id].mw_assign, configuration.dexed[selected_instance_id].mw_mode);
    MicroDexed[selected_instance_id]->ControllersRefresh();

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].mw_mode) {
      case 0:
        display.print(F("[LINEAR      ]"));
        break;
      case 1:
        display.print(F("[REVERSE LIN.]"));
        break;
      case 2:
        display.print(F("[DIRECT      ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_fc_range(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("FC Range", configuration.dexed[selected_instance_id].fc_range, 1.0, FC_RANGE_MIN, FC_RANGE_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].fc_range = constrain(configuration.dexed[selected_instance_id].fc_range + ENCODER[ENC_R].speed(), FC_RANGE_MIN, FC_RANGE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].fc_range = constrain(configuration.dexed[selected_instance_id].fc_range - ENCODER[ENC_R].speed(), FC_RANGE_MIN, FC_RANGE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("FC Range", configuration.dexed[selected_instance_id].fc_range, 1.0, FC_RANGE_MIN, FC_RANGE_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setFCController(configuration.dexed[selected_instance_id].fc_range, configuration.dexed[selected_instance_id].fc_assign, configuration.dexed[selected_instance_id].fc_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 72, configuration.dexed[selected_instance_id].fc_range, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_fc_assign(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("FC Assign"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].fc_assign = constrain(configuration.dexed[selected_instance_id].fc_assign + 1, FC_ASSIGN_MIN, FC_ASSIGN_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].fc_assign = constrain(configuration.dexed[selected_instance_id].fc_assign - 1, FC_ASSIGN_MIN, FC_ASSIGN_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setFCController(configuration.dexed[selected_instance_id].fc_range, configuration.dexed[selected_instance_id].fc_assign, configuration.dexed[selected_instance_id].fc_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 73, configuration.dexed[selected_instance_id].fc_assign, 2);

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].fc_assign) {
      case 0:
        display.print(F("[   NONE    ]"));
        break;
      case 1:
        display.print(F("[PTCH       ]"));
        break;
      case 2:
        display.print(F("[     AMP   ]"));
        break;
      case 3:
        display.print(F("[PTCH AMP   ]"));
        break;
      case 4:
        display.print(F("[         EG]"));
        break;
      case 5:
        display.print(F("[PTCH     EG]"));
        break;
      case 6:
        display.print(F("[     AMP EG]"));
        break;
      case 7:
        display.print(F("[PTCH AMP EG]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_fc_mode(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("FC Mode"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].fc_mode = constrain(configuration.dexed[selected_instance_id].fc_mode + 1, FC_MODE_MIN, FC_MODE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].fc_mode = constrain(configuration.dexed[selected_instance_id].fc_mode - 1, FC_MODE_MIN, FC_MODE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setFCController(configuration.dexed[selected_instance_id].fc_range, configuration.dexed[selected_instance_id].fc_assign, configuration.dexed[selected_instance_id].fc_mode);
    MicroDexed[selected_instance_id]->ControllersRefresh();

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].fc_mode) {
      case 0:
        display.print(F("[LINEAR      ]"));
        break;
      case 1:
        display.print(F("[REVERSE LIN.]"));
        break;
      case 2:
        display.print(F("[DIRECT      ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_bc_range(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("BC Range", configuration.dexed[selected_instance_id].bc_range, 1.0, BC_RANGE_MIN, BC_RANGE_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].bc_range = constrain(configuration.dexed[selected_instance_id].bc_range + ENCODER[ENC_R].speed(), BC_RANGE_MIN, BC_RANGE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].bc_range = constrain(configuration.dexed[selected_instance_id].bc_range - ENCODER[ENC_R].speed(), BC_RANGE_MIN, BC_RANGE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("BC Range", configuration.dexed[selected_instance_id].bc_range, 1.0, BC_RANGE_MIN, BC_RANGE_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setBCController(configuration.dexed[selected_instance_id].bc_range, configuration.dexed[selected_instance_id].bc_assign, configuration.dexed[selected_instance_id].bc_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 74, configuration.dexed[selected_instance_id].bc_range, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_bc_assign(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("BC Assign"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].bc_assign = constrain(configuration.dexed[selected_instance_id].bc_assign + 1, BC_ASSIGN_MIN, BC_ASSIGN_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].bc_assign = constrain(configuration.dexed[selected_instance_id].bc_assign - 1, BC_ASSIGN_MIN, BC_ASSIGN_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setBCController(configuration.dexed[selected_instance_id].bc_range, configuration.dexed[selected_instance_id].bc_assign, configuration.dexed[selected_instance_id].bc_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 75, configuration.dexed[selected_instance_id].bc_assign, 2);

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].bc_assign) {
      case 0:
        display.print(F("[   NONE    ]"));
        break;
      case 1:
        display.print(F("[PTCH       ]"));
        break;
      case 2:
        display.print(F("[     AMP   ]"));
        break;
      case 3:
        display.print(F("[PTCH AMP   ]"));
        break;
      case 4:
        display.print(F("[         EG]"));
        break;
      case 5:
        display.print(F("[PTCH     EG]"));
        break;
      case 6:
        display.print(F("[     AMP EG]"));
        break;
      case 7:
        display.print(F("[PTCH AMP EG]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_bc_mode(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("BC Mode"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].bc_mode = constrain(configuration.dexed[selected_instance_id].bc_mode + 1, BC_MODE_MIN, BC_MODE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].bc_mode = constrain(configuration.dexed[selected_instance_id].bc_mode - 1, BC_MODE_MIN, BC_MODE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setBCController(configuration.dexed[selected_instance_id].bc_range, configuration.dexed[selected_instance_id].bc_assign, configuration.dexed[selected_instance_id].bc_mode);
    MicroDexed[selected_instance_id]->ControllersRefresh();

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].bc_mode) {
      case 0:
        display.print(F("[LINEAR      ]"));
        break;
      case 1:
        display.print(F("[REVERSE LIN.]"));
        break;
      case 2:
        display.print(F("[DIRECT      ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_at_range(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("AT Range", configuration.dexed[selected_instance_id].at_range, 1.0, AT_RANGE_MIN, AT_RANGE_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].at_range = constrain(configuration.dexed[selected_instance_id].at_range + ENCODER[ENC_R].speed(), AT_RANGE_MIN, AT_RANGE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].at_range = constrain(configuration.dexed[selected_instance_id].at_range - ENCODER[ENC_R].speed(), AT_RANGE_MIN, AT_RANGE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("AT Range", configuration.dexed[selected_instance_id].at_range, 1.0, AT_RANGE_MIN, AT_RANGE_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setATController(configuration.dexed[selected_instance_id].at_range, configuration.dexed[selected_instance_id].at_assign, configuration.dexed[selected_instance_id].at_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 76, configuration.dexed[selected_instance_id].at_range, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_at_assign(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("AT Assign"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].at_assign = constrain(configuration.dexed[selected_instance_id].at_assign + 1, AT_ASSIGN_MIN, AT_ASSIGN_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].at_assign = constrain(configuration.dexed[selected_instance_id].at_assign - 1, AT_ASSIGN_MIN, AT_ASSIGN_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setATController(configuration.dexed[selected_instance_id].at_range, configuration.dexed[selected_instance_id].at_assign, configuration.dexed[selected_instance_id].at_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 77, configuration.dexed[selected_instance_id].at_assign, 2);

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].at_assign) {
      case 0:
        display.print(F("[   NONE    ]"));
        break;
      case 1:
        display.print(F("[PTCH       ]"));
        break;
      case 2:
        display.print(F("[     AMP   ]"));
        break;
      case 3:
        display.print(F("[PTCH AMP   ]"));
        break;
      case 4:
        display.print(F("[         EG]"));
        break;
      case 5:
        display.print(F("[PTCH     EG]"));
        break;
      case 6:
        display.print(F("[     AMP EG]"));
        break;
      case 7:
        display.print(F("[PTCH AMP EG]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_at_mode(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("AT Mode"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].at_mode = constrain(configuration.dexed[selected_instance_id].at_mode + 1, AT_MODE_MIN, AT_MODE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].at_mode = constrain(configuration.dexed[selected_instance_id].at_mode - 1, AT_MODE_MIN, AT_MODE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setATController(configuration.dexed[selected_instance_id].at_range, configuration.dexed[selected_instance_id].at_assign, configuration.dexed[selected_instance_id].at_mode);
    MicroDexed[selected_instance_id]->ControllersRefresh();

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].at_mode) {
      case 0:
        display.print(F("[LINEAR      ]"));
        break;
      case 1:
        display.print(F("[REVERSE LIN.]"));
        break;
      case 2:
        display.print(F("[DIRECT      ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_portamento_mode(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Port. Mode"));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].portamento_mode = constrain(configuration.dexed[selected_instance_id].portamento_mode + 1, PORTAMENTO_MODE_MIN, PORTAMENTO_MODE_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].portamento_mode = constrain(configuration.dexed[selected_instance_id].portamento_mode - 1, PORTAMENTO_MODE_MIN, PORTAMENTO_MODE_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setPortamentoMode(configuration.dexed[selected_instance_id].portamento_mode);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 67, configuration.dexed[selected_instance_id].portamento_mode, 2);

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].portamento_mode) {
      case 0:
        if (configuration.dexed[selected_instance_id].monopoly == 1)
          display.print(F("[RETAIN  ]"));
        else
          display.print(F("[FINGERED]"));
        break;
      case 1:
        if (configuration.dexed[selected_instance_id].monopoly == 1)
          display.print(F("[FOLLOW  ]"));
        else
          display.print(F("[FULL    ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_portamento_glissando(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Port. Gliss."));

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].portamento_glissando = constrain(configuration.dexed[selected_instance_id].portamento_glissando + 1, PORTAMENTO_GLISSANDO_MIN, PORTAMENTO_GLISSANDO_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].portamento_glissando = constrain(configuration.dexed[selected_instance_id].portamento_glissando - 1, PORTAMENTO_GLISSANDO_MIN, PORTAMENTO_GLISSANDO_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    MicroDexed[selected_instance_id]->setPortamentoGlissando(configuration.dexed[selected_instance_id].portamento_glissando);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 68, configuration.dexed[selected_instance_id].portamento_glissando, 2);

    display.setCursor(0, 1);
    switch (configuration.dexed[selected_instance_id].portamento_glissando) {
      case 0:
        display.print(F("[OFF]"));
        break;
      case 1:
        display.print(F("[ON ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_portamento_time(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Port. Time", configuration.dexed[selected_instance_id].portamento_time, 1.0, PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX, 2, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].portamento_time = constrain(configuration.dexed[selected_instance_id].portamento_time + ENCODER[ENC_R].speed(), PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].portamento_time = constrain(configuration.dexed[selected_instance_id].portamento_time - ENCODER[ENC_R].speed(), PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX);
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    display_bar_int("Portam. Time", configuration.dexed[selected_instance_id].portamento_time, 1.0, PORTAMENTO_TIME_MIN, PORTAMENTO_TIME_MAX, 2, false, false, false);

    MicroDexed[selected_instance_id]->setPortamentoTime(configuration.dexed[selected_instance_id].portamento_time);
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 69, configuration.dexed[selected_instance_id].portamento_time, 2);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_handle_OP(uint8_t param) {
  static uint8_t op_selected;

  lcd_OP_active_instance_number(selected_instance_id, configuration.dexed[selected_instance_id].op_enabled);

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("OP Enable"));
    display.setCursor(0, 1);
    for (uint8_t i = 2; i < 8; i++)
      display.write(i);

    UI_update_instance_icons();

    display.setCursor(op_selected, 1);
    display.blink();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) {
#if NUM_DEXED > 1
      if (op_selected == 0) {
        selected_instance_id = !selected_instance_id;
        op_selected = 5;
        lcd_OP_active_instance_number(selected_instance_id, configuration.dexed[selected_instance_id].op_enabled);
      } else
#endif
        op_selected = constrain(op_selected - 1, 0, 5);
    } else if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) {
#if NUM_DEXED > 1
      if (op_selected == 5) {
        selected_instance_id = !selected_instance_id;
        op_selected = 0;
        lcd_OP_active_instance_number(selected_instance_id, configuration.dexed[selected_instance_id].op_enabled);
      } else
#endif
        op_selected = constrain(op_selected + 1, 0, 5);
    } else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      if (bitRead(configuration.dexed[selected_instance_id].op_enabled, op_selected))
        bitClear(configuration.dexed[selected_instance_id].op_enabled, op_selected);
      else
        bitSet(configuration.dexed[selected_instance_id].op_enabled, op_selected);

      lcd_OP_active_instance_number(selected_instance_id, configuration.dexed[selected_instance_id].op_enabled);
    }

    display.setCursor(op_selected, 1);

    MicroDexed[selected_instance_id]->setOPAll(configuration.dexed[selected_instance_id].op_enabled);
    MicroDexed[selected_instance_id]->doRefreshVoice();
    send_sysex_param(configuration.dexed[selected_instance_id].midi_channel, 155, configuration.dexed[selected_instance_id].op_enabled, 0);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    display.noBlink();
    display.noCursor();
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

#if NUM_DRUMS > 0
void _check_yes_no_back(uint8_t state) {
  if (state == 0) {  // BACK
    display.setCursor(0, 1);
    display.print(F(" "));
    display.setCursor(4, 1);
    display.print(F(" "));
    display.setCursor(5, 1);
    display.print(F(" "));
    display.setCursor(8, 1);
    display.print(F(" "));
    display.setCursor(9, 1);
    display.print(F("["));
    display.setCursor(14, 1);
    display.print(F("] "));
  } else if (state == 1) {  // NO
    display.setCursor(0, 1);
    display.print(F("["));
    display.setCursor(4, 1);
    display.print(F("]"));
    display.setCursor(5, 1);
    display.print(F(" "));
    display.setCursor(8, 1);
    display.print(F(" "));
    display.setCursor(9, 1);
    display.print(F(" "));
    display.setCursor(14, 1);
    display.print(F("  "));
  } else {  // YES
    display.setCursor(0, 1);
    display.print(F(" "));
    display.setCursor(4, 1);
    display.print(F(" "));
    display.setCursor(5, 1);
    display.print(F("["));
    display.setCursor(8, 1);
    display.print(F("]"));
    display.setCursor(9, 1);
    display.print(F(" "));
    display.setCursor(14, 1);
    display.print(F("  "));
  }
}

void _check_display_name(bool display_name, uint8_t digits) {
  if (display_name == true) {
    display.setCursor(0, 1);
    display.print(F("["));
    display.setCursor(9, 1);
    display.print(F("] "));
    display.setCursor(LCD_cols - digits - 2, 1);
    display.print(F(" "));
    display.setCursor(LCD_cols - 1, 1);
    display.print(F(" "));
  } else {
    display.setCursor(0, 1);
    display.print(F(" "));
    display.setCursor(9, 1);
    display.print(F("  "));
    display.setCursor(LCD_cols - digits - 2, 1);
    display.print(F("["));
    display.setCursor(LCD_cols - 1, 1);
    display.print(F("]"));
  }
}

void _check_display_name_min_max(uint8_t input_mode, uint8_t input_type, uint8_t digits) {
  switch (input_mode) {
    case 0:
      display.setCursor(7, 0);
      if (input_type == 0)
        display.print(F("[MIN]"));
      else
        display.print(F("[MAX]"));
      display.setCursor(0, 1);
      display.print(F(" "));
      display.setCursor(9, 1);
      display.print(F(" "));
      display.setCursor(LCD_cols - digits - 2, 1);
      display.print(F(" "));
      display.setCursor(LCD_cols - 1, 1);
      display.print(F(" "));
      break;
    case 1:
      display.setCursor(7, 0);
      if (input_type == 0)
        display.print(F(" MIN "));
      else
        display.print(F(" MAX "));
      display.setCursor(0, 1);
      display.print(F("["));
      display.setCursor(9, 1);
      display.print(F("]"));
      display.setCursor(LCD_cols - digits - 2, 1);
      display.print(F(" "));
      display.setCursor(LCD_cols - 1, 1);
      display.print(F(" "));
      break;
    case 2:
      display.setCursor(7, 0);
      if (input_type == 0)
        display.print(F(" MIN "));
      else
        display.print(F(" MAX "));
      display.setCursor(0, 1);
      display.print(F(" "));
      display.setCursor(9, 1);
      display.print(F(" "));
      display.setCursor(LCD_cols - digits - 2, 1);
      display.print(F("["));
      display.setCursor(LCD_cols - 1, 1);
      display.print(F("]"));
      break;
  }
}
#endif

void UI_func_drum_midi_channel(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("MIDI Channel"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    display.setCursor(0, 0);
    display.print(F("MIDI Channel"));

    display.setCursor(0, 1);
    if (configuration.drums.midi_channel == 0) {
      display.print(F("[OMNI]"));
    } else {
      display_int(configuration.drums.midi_channel, 4, false, true, false);
    }
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      int8_t tmp_midi_channel = 0;
      if (LCDML.BT_checkDown())
        tmp_midi_channel = configuration.drums.midi_channel + ENCODER[ENC_R].speed();
      else if (LCDML.BT_checkUp())
        tmp_midi_channel = configuration.drums.midi_channel - ENCODER[ENC_R].speed();
      configuration.drums.midi_channel = constrain(tmp_midi_channel, MIDI_CHANNEL_MIN, MIDI_CHANNEL_MAX);

      display.setCursor(0, 1);
      if (configuration.drums.midi_channel == 0) {
        display.print(F("[OMNI]"));
      } else {
        display_int(configuration.drums.midi_channel, 4, false, true, false);
      }
#ifdef DEBUG
      Serial.printf_P(PSTR("Changing drum MIDI channel to %d\n"), configuration.drums.midi_channel);
#endif
    }
  }
#endif

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();
  }
}

void UI_func_drum_main_volume(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Main Volume"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    lcd_special_chars(BLOCKBAR);
    display_bar_int("Main Volume", configuration.drums.main_vol, 1.0, VOLUME_MIN, VOLUME_MAX, 3, false, false, true);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.drums.main_vol = constrain(configuration.drums.main_vol + ENCODER[ENC_L].speed(), DRUMS_MAIN_VOL_MIN, DRUMS_MAIN_VOL_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.drums.main_vol = constrain(configuration.drums.main_vol - ENCODER[ENC_L].speed(), DRUMS_MAIN_VOL_MIN, DRUMS_MAIN_VOL_MAX);
      }
      display_bar_int("DRM Main Vol", configuration.drums.main_vol, 1.0, DRUMS_MAIN_VOL_MIN, DRUMS_MAIN_VOL_MAX, 3, false, false, false);
      float tmp_vol = configuration.drums.main_vol / 100.0;
      master_mixer_r.gain(MASTER_MIX_CH_DRUMS, tmp_vol);
      master_mixer_l.gain(MASTER_MIX_CH_DRUMS, tmp_vol);
    }
  }
#endif

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_L].reset();
  }
}

void UI_func_drum_pitch(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Pitch"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  static bool display_name;
  char tmp_val[5];
  char tmp_name[9];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    midi_learn_mode = MIDI_LEARN_MODE_ON;

    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(0, 0);
    display.print(F("Pitch"));
    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 5, 1);
    snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%4.1f"), configuration.drums.pitch[active_sample] / 10.0);
    display.print(tmp_val);
    _check_display_name(display_name, 4);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        if (display_name == true) {
          if (active_sample < NUM_DRUMSET_CONFIG - 2)
            active_sample++;
        } else {
          configuration.drums.pitch[active_sample] = constrain(configuration.drums.pitch[active_sample] + ENCODER[ENC_L].speed(), DRUMS_PITCH_MIN, DRUMS_PITCH_MAX);
        }
      } else if (LCDML.BT_checkUp()) {
        if (display_name == true) {
          if (active_sample > 0)
            active_sample--;
        } else {
          configuration.drums.pitch[active_sample] = constrain(configuration.drums.pitch[active_sample] - ENCODER[ENC_L].speed(), DRUMS_PITCH_MIN, DRUMS_PITCH_MAX);
        }
      } else if (LCDML.BT_checkEnter()) {
        display_name = !display_name;
      }
    }
#ifdef DEBUG
    Serial.printf("Drum pitch for active_sample=%d [%s]=%d\n", active_sample, drum_config[active_sample].name, configuration.drums.pitch[active_sample]);
#endif
    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 5, 1);
    snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%4.1f"), configuration.drums.pitch[active_sample] / 10.0);
    display.print(tmp_val);

    _check_display_name(display_name, 4);
  }
#endif
  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_L].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_drum_vol_min_max(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Volume  MIN MAX"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  static uint8_t input_mode;
  static uint8_t input_type;
  char tmp_val[5];
  char tmp_name[9];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    midi_learn_mode = MIDI_LEARN_MODE_ON;

    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(0, 0);
    display.print(F("Volume"));
    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 3, 1);
    if (input_type == 0)
      snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%2d"), configuration.drums.vol_min[active_sample]);
    else
      snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%2d"), configuration.drums.vol_max[active_sample]);
    display.print(tmp_val);
    _check_display_name_min_max(input_mode, input_type, 2);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        switch (input_mode) {
          case 0:
            if (input_type == 0)
              input_type = 1;
            break;
          case 1:
            if (active_sample < NUM_DRUMSET_CONFIG - 2)
              active_sample++;
            break;
          case 2:
            if (input_type == 0)
              configuration.drums.vol_min[active_sample] = constrain(configuration.drums.vol_min[active_sample] + ENCODER[ENC_L].speed(), DRUMS_VOL_MIN, DRUMS_VOL_MAX);
            else
              configuration.drums.vol_max[active_sample] = constrain(configuration.drums.vol_max[active_sample] + ENCODER[ENC_L].speed(), DRUMS_VOL_MIN, DRUMS_VOL_MAX);
            break;
        }
      } else if (LCDML.BT_checkUp()) {
        switch (input_mode) {
          case 0:
            if (input_type == 1)
              input_type = 0;
            break;
          case 1:
            if (active_sample > 0)
              active_sample--;
            break;
          case 2:
            if (input_type == 0)
              configuration.drums.vol_min[active_sample] = constrain(configuration.drums.vol_min[active_sample] - ENCODER[ENC_L].speed(), DRUMS_VOL_MIN, DRUMS_VOL_MAX);
            else
              configuration.drums.vol_max[active_sample] = constrain(configuration.drums.vol_max[active_sample] - ENCODER[ENC_L].speed(), DRUMS_VOL_MIN, DRUMS_VOL_MAX);
            break;
        }
      } else if (LCDML.BT_checkEnter()) {
        input_mode++;
        input_mode %= 3;
      }
    }
#ifdef DEBUG
    if (input_type == 0)
      Serial.printf_P(PSTR("Drum volume MIN for active_sample=%d [%s]=%d\n"), active_sample, drum_config[active_sample].name, configuration.drums.vol_min[active_sample]);
    else
      Serial.printf_P(PSTR("Drum volume MAX for active_sample=%d [%s]=%d\n"), active_sample, drum_config[active_sample].name, configuration.drums.vol_max[active_sample]);
#endif
    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 3, 1);
    if (input_type == 0)
      snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%2d"), configuration.drums.vol_min[active_sample]);
    else
      snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%2d"), configuration.drums.vol_max[active_sample]);
    display.print(tmp_val);

    _check_display_name_min_max(input_mode, input_type, 2);
  }
#endif

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_L].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_drum_pan(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Panorama"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  static bool display_name;
  char tmp_val[5];
  char tmp_name[9];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    midi_learn_mode = MIDI_LEARN_MODE_ON;

    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(0, 0);
    display.print(F("Panorama"));
    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 5, 1);
    snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%4.1f"), configuration.drums.pan[active_sample] / 10.0);
    display.print(tmp_val);
    _check_display_name(display_name, 4);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        if (display_name == true) {
          if (active_sample < NUM_DRUMSET_CONFIG - 2)
            active_sample++;
        } else {
          configuration.drums.pan[active_sample] = constrain(configuration.drums.pan[active_sample] + ENCODER[ENC_L].speed(), DRUMS_PANORAMA_MIN, DRUMS_PANORAMA_MAX);
        }
      } else if (LCDML.BT_checkUp()) {
        if (display_name == true) {
          if (active_sample > 0)
            active_sample--;
        } else {
          configuration.drums.pan[active_sample] = constrain(configuration.drums.pan[active_sample] - ENCODER[ENC_L].speed(), DRUMS_PANORAMA_MIN, DRUMS_PANORAMA_MAX);
        }
      } else if (LCDML.BT_checkEnter()) {
        display_name = !display_name;
      }
    }
#ifdef DEBUG
    Serial.printf("Drum panorama for active_sample=%d [%s]=%d\n", active_sample, drum_config[active_sample].name, configuration.drums.pan[active_sample]);
#endif
    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 5, 1);
    snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%4.1f"), configuration.drums.pan[active_sample] / 10.0);
    display.print(tmp_val);

    _check_display_name(display_name, 4);
  }
#endif

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_L].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_drum_reverb_send(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Reverb Send"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  static bool display_name;
  char tmp_val[5];
  char tmp_name[9];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    midi_learn_mode = MIDI_LEARN_MODE_ON;

    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(0, 0);
    display.print(F("Reverb Send"));
    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 3, 1);
    snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%2d"), configuration.drums.reverb_send[active_sample]);
    display.print(tmp_val);
    _check_display_name(display_name, 2);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        if (display_name == true) {
          if (active_sample < NUM_DRUMSET_CONFIG - 2)
            active_sample++;
        } else {
          configuration.drums.reverb_send[active_sample] = constrain(configuration.drums.reverb_send[active_sample] + ENCODER[ENC_L].speed(), DRUMS_REVERB_SEND_MIN, DRUMS_REVERB_SEND_MAX);
        }
      } else if (LCDML.BT_checkUp()) {
        if (display_name == true) {
          if (active_sample > 0)
            active_sample--;
        } else {
          configuration.drums.reverb_send[active_sample] = constrain(configuration.drums.reverb_send[active_sample] - ENCODER[ENC_L].speed(), DRUMS_REVERB_SEND_MIN, DRUMS_REVERB_SEND_MAX);
        }
      } else if (LCDML.BT_checkEnter()) {
        display_name = !display_name;
      }
    }
#ifdef DEBUG
    Serial.printf("Reverb send for active_sample=%d [%s]=%d\n", active_sample, drum_config[active_sample].name, configuration.drums.reverb_send[active_sample]);
#endif
    memset(tmp_name, ' ', 8);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    tmp_name[8] = '\0';

    display.setCursor(1, 1);
    display.print(tmp_name);
    display.setCursor(LCD_cols - 3, 1);
    snprintf_P(tmp_val, sizeof(tmp_val), PSTR("%2d"), configuration.drums.reverb_send[active_sample]);
    display.print(tmp_val);

    _check_display_name(display_name, 2);
  }
#endif

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_L].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
  }
}

void UI_func_drum_midi_note(uint8_t param) {
#if NUM_DRUMS == 0
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("MIDI Note"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  static bool display_name;
  static int8_t ask_before_quit_mode;

  char tmp_val[4];
  char tmp_name[9];

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    ask_before_quit_mode = -1;

    for (uint8_t i = 0; i < NUM_DRUMSET_CONFIG; i++)
      midinote_old[i] = configuration.drums.midinote[i];

    if (display_name)
      midi_learn_mode = MIDI_LEARN_MODE_ON | 0x80;
    else
      midi_learn_mode = MIDI_LEARN_MODE_ON;

    getNoteName(tmp_val, configuration.drums.midinote[active_sample]);
    strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
    display.setCursor(0, 0);
    display.print(F("MIDI Note"));
    display.show(1, 1, 8, tmp_name);
    display.show(1, LCD_cols - strlen(tmp_val) - 1, strlen(tmp_val), tmp_val);
    _check_display_name(display_name, 3);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (ask_before_quit == true && ask_before_quit_mode <= 0) {
      uint8_t num_changes = 0;
      for (uint8_t i = 0; i < NUM_DRUMSET_CONFIG; i++) {
        if (midinote_old[i] != configuration.drums.midinote[i]) {
          num_changes++;
          break;
        }
      }
      if (num_changes > 0) {
        ask_before_quit_mode = 0;
        midi_learn_mode = MIDI_LEARN_MODE_OFF;
        display.show(0, 0, 16, "Use this setup?");
        display.show(1, 0, 16, " YES  NO [BACK]");
        _check_yes_no_back(ask_before_quit_mode);
      } else {
        ask_before_quit = false;
        LCDML.FUNC_goBackToMenu();
      }
    }

    // if (midi_learn_mode & 0x80)
    //  configuration.drums.midinote[active_sample] = midi_learn_mode & 0x7f;

    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        if (ask_before_quit_mode >= 0) {
          ++ask_before_quit_mode %= 3;
          _check_yes_no_back(ask_before_quit_mode);
          LCDML.OTHER_updateFunc();
          LCDML.loop_menu();
        } else {
          if (display_name == true) {
            if (active_sample < NUM_DRUMSET_CONFIG - 2) {
              configuration.drums.midinote[active_sample] = midinote_old[active_sample];
              active_sample++;
            }
          } else
            configuration.drums.midinote[active_sample] = constrain(configuration.drums.midinote[active_sample] + ENCODER[ENC_L].speed(), DRUMS_MIDI_NOTE_MIN, DRUMS_MIDI_NOTE_MAX);
        }
      } else if (LCDML.BT_checkUp()) {
        if (ask_before_quit_mode >= 0) {
          if (ask_before_quit_mode > 0)
            ask_before_quit_mode--;
          else
            ask_before_quit_mode = 2;
          _check_yes_no_back(ask_before_quit_mode);
          LCDML.OTHER_updateFunc();
          LCDML.loop_menu();
        } else {
          if (display_name == true) {
            if (active_sample > 0) {
              configuration.drums.midinote[active_sample] = midinote_old[active_sample];
              active_sample--;
            }
          } else
            configuration.drums.midinote[active_sample] = constrain(configuration.drums.midinote[active_sample] - ENCODER[ENC_L].speed(), DRUMS_MIDI_NOTE_MIN, DRUMS_MIDI_NOTE_MAX);
        }
      } else if (LCDML.BT_checkEnter()) {
        if (ask_before_quit_mode >= 0) {
          if (ask_before_quit_mode == 2) {  // NO
            for (uint8_t i = 0; i < NUM_DRUMSET_CONFIG; i++)
              configuration.drums.midinote[i] = midinote_old[i];
            ask_before_quit = false;
            display.show(1, 0, 16, "Canceled.");
            delay(MESSAGE_WAIT_TIME);
            LCDML.FUNC_goBackToMenu();
          } else if (ask_before_quit_mode == 1) {  // YES
            ask_before_quit = false;
            display.show(1, 0, 16, "Done.");
            delay(MESSAGE_WAIT_TIME);
            LCDML.FUNC_goBackToMenu();
          } else if (ask_before_quit_mode == 0) {  // BACK
            ask_before_quit_mode = -1;
            ask_before_quit = false;
            if (display_name)
              midi_learn_mode = MIDI_LEARN_MODE_ON | 0x80;
            else
              midi_learn_mode = MIDI_LEARN_MODE_ON;
            LCDML.OTHER_updateFunc();
            LCDML.loop_menu();
          }
        } else
          display_name = !display_name;
      }
    }

    if (ask_before_quit_mode < 0) {
      getNoteName(tmp_val, configuration.drums.midinote[active_sample]);
      strlcpy(tmp_name, drum_config[active_sample].name, sizeof(drum_config[active_sample].name));
      display.show(1, 1, 8, tmp_name);
      display.show(1, LCD_cols - strlen(tmp_val) - 1, strlen(tmp_val), tmp_val);

#ifdef DEBUG
      Serial.printf("Drum midinote for active_sample=%d [%s]=%d (%s)\n", active_sample, drum_config[active_sample].name, configuration.drums.midinote[active_sample], tmp_val);
#endif

      _check_display_name(display_name, 3);
      if (display_name)
        midi_learn_mode = MIDI_LEARN_MODE_ON | 0x80;
      else
        midi_learn_mode = MIDI_LEARN_MODE_ON;
    }
  }
#endif

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_L].reset();
    midi_learn_mode = MIDI_LEARN_MODE_OFF;
    ask_before_quit = false;
  }
}

void UI_func_save_performance(uint8_t param) {
  static bool overwrite;
  static bool yesno;
  static uint8_t mode;
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    char tmp[CONFIG_FILENAME_LEN];
    yesno = false;
    temp_int = 0;
    mode = 0;
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Save Performance"));
    display.setCursor(0, 1);
    snprintf_P(tmp, sizeof(tmp), PSTR("[%2d]"), temp_int);
    display.print(tmp);
    snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/%s.json"), PERFORMANCE_CONFIG_PATH, temp_int, PERFORMANCE_CONFIG_NAME);
    if (SD.exists(tmp))
      overwrite = true;
    else
      overwrite = false;
    if (check_sd_performance_exists(temp_int)) {
      char tmp_confname[PERFORMANCE_NAME_LEN];
      get_sd_performance_name_json(temp_int, tmp_confname, sizeof(tmp_confname));
      if (tmp_confname[0] != 0)
        display.show(1, 5, 11, tmp_confname);
      else
        display.print("-- DATA -- ");
    } else
      display.print("-- EMPTY --");
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        if (mode == 0)
          temp_int = constrain(temp_int + ENCODER[ENC_L].speed(), 0, 99);
        else
          yesno = true;
      } else if (LCDML.BT_checkUp()) {
        if (mode == 0)
          temp_int = constrain(temp_int - ENCODER[ENC_L].speed(), 0, 99);
        else
          yesno = false;
      } else if (LCDML.BT_checkEnter()) {
        if (mode == 0 && overwrite == true) {
          mode = 1;
          display.setCursor(0, 1);
          display.print(F("Overwrite: [   ]"));
        } else {
          mode = 0xff;
          if (overwrite == false || yesno == true) {
            display.show(1, 0, 16, "Writing...");
            LCDML.FUNC_goBackToMenu();
          } else if (overwrite == true && yesno == false) {
            char tmp[10];

            mode = 0;
            display.setCursor(0, 1);
            snprintf_P(tmp, sizeof(tmp), PSTR("[%2d]   "), temp_int);
            display.print(tmp);
          }
        }
      }
      if (mode == 0) {
        char tmp[CONFIG_FILENAME_LEN];
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/%s.json"), PERFORMANCE_CONFIG_PATH, temp_int, PERFORMANCE_CONFIG_NAME);

        if (SD.exists(tmp))
          overwrite = true;
        else
          overwrite = false;

        display.setCursor(0, 1);
        snprintf_P(tmp, sizeof(tmp), PSTR("[%2d]"), temp_int);
        display.print(tmp);
        display.setCursor(5, 1);
        if (overwrite == false) {
          display.print("-- EMPTY --");
        } else if (check_sd_performance_exists(temp_int)) {
          char tmp_confname[PERFORMANCE_NAME_LEN];
          get_sd_performance_name_json(temp_int, tmp_confname, sizeof(tmp_confname));
          if (tmp_confname[0] != 0)
            display.show(1, 5, 11, tmp_confname);
          else
            display.print("-- DATA -- ");
        } else
          display.print("-- EMPTY --");
      } else if (mode < 0xff) {
        display.setCursor(12, 1);
        if (yesno == true) {
          display.print(F("YES"));
        } else
          display.print(F("NO "));
      }
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    if (mode < 0xff) {
      display.show(1, 0, 16, "Canceled.");
      delay(MESSAGE_WAIT_TIME);
    } else
      save_sd_performance_json(temp_int);

    encoderDir[ENC_R].reset();
  }
}

void UI_func_load_performance(uint8_t param) {
  static uint8_t mode;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    char tmp[CONFIG_FILENAME_LEN];
    temp_int = 0;
    mode = 0;
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("Load Performance "));

    display.setCursor(0, 1);
    snprintf_P(tmp, sizeof(tmp), PSTR("[%2d]"), temp_int);
    display.print(tmp);
    if (check_sd_performance_exists(temp_int)) {
      get_sd_performance_name_json(temp_int, configuration.performance.name, sizeof(configuration.performance.name));
      if (configuration.performance.name[0] != 0)
        display.show(1, 5, 11, configuration.performance.name);
      else
        display.print(" -- DATA -- ");
    } else display.print(" -- EMPTY --");
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        temp_int = constrain(temp_int + ENCODER[ENC_L].speed(), 0, 99);
      } else if (LCDML.BT_checkUp()) {
        temp_int = constrain(temp_int - ENCODER[ENC_L].speed(), 0, 99);
      } else if (LCDML.BT_checkEnter()) {
        mode = 0xff;
        display.setCursor(0, 1);
        if (load_sd_performance_json(temp_int) == false)
          display.print("Does not exist. ");
        else {
          display.print("Loading...      ");
          load_sd_performance_json(temp_int);
        }
        delay(MESSAGE_WAIT_TIME);
        LCDML.FUNC_goBackToMenu();
      }

      display.setCursor(0, 1);
      char tmp[10];
      snprintf_P(tmp, sizeof(tmp), PSTR("[%2d]"), temp_int);
      display.print(tmp);
      if (check_sd_performance_exists(temp_int)) {
        get_sd_performance_name_json(temp_int, configuration.performance.name, sizeof(configuration.performance.name));
        if (configuration.performance.name[0] != 0)
          display.show(1, 5, 11, configuration.performance.name);
        else
          display.print(" -- DATA -- ");
      } else display.print(" -- EMPTY --");
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    if (mode < 0xff) {
      display.show(1, 0, 16, "Canceled.");
      delay(MESSAGE_WAIT_TIME);
    } else {
      configuration.sys.performance_number = temp_int;
      encoderDir[ENC_R].reset();
    }
  }
}

void UI_func_information(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    char version_string[LCD_cols + 1];

    encoderDir[ENC_R].reset();

    generate_version_string(version_string, sizeof(version_string));

    // setup function
    display.setCursor(0, 0);
    display.print(version_string);
    display.setCursor(0, 1);
    display.print(sd_string);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    ;
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();
  }
}

void UI_func_midi_soft_thru(uint8_t param) {
  static uint8_t old_soft_midi_thru;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("MIDI Soft THRU"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.sys.soft_midi_thru = constrain(configuration.sys.soft_midi_thru + 1, SOFT_MIDI_THRU_MIN, SOFT_MIDI_THRU_MAX);
      else if (LCDML.BT_checkUp())
        configuration.sys.soft_midi_thru = constrain(configuration.sys.soft_midi_thru - 1, SOFT_MIDI_THRU_MIN, SOFT_MIDI_THRU_MAX);
    }

    display.setCursor(0, 1);
    switch (configuration.sys.soft_midi_thru) {
      case 0:
        display.print(F("[OFF]"));
        break;
      case 1:
        display.print(F("[ON ]"));
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();

    if (old_soft_midi_thru != configuration.sys.soft_midi_thru) {
      save_sys_flag = true;
      save_sys = 0;
    }
  }
}

void UI_func_velocity_level(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_special_chars(BLOCKBAR);
    display_bar_int("Velocity Lvl", configuration.dexed[selected_instance_id].velocity_level, 1.0, VELOCITY_LEVEL_MIN, VELOCITY_LEVEL_MAX, 3, false, false, true);

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].velocity_level = constrain(configuration.dexed[selected_instance_id].velocity_level + ENCODER[ENC_R].speed(), VELOCITY_LEVEL_MIN, VELOCITY_LEVEL_MAX);
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].velocity_level = constrain(configuration.dexed[selected_instance_id].velocity_level - ENCODER[ENC_R].speed(), VELOCITY_LEVEL_MIN, VELOCITY_LEVEL_MAX);
    }
#if NUM_DEXED > 1
    else if (LCDML.BT_checkEnter()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
#endif

    display_bar_int("Velocity Lvl", configuration.dexed[selected_instance_id].velocity_level, 1.0, VELOCITY_LEVEL_MIN, VELOCITY_LEVEL_MAX, 3, false, false, false);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_engine(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.clear();
    display.show(0, 0, 16, "Engine");
    display.show(1, 0, 16, "[      ]");
    switch (configuration.dexed[selected_instance_id].engine) {
      case 0:
        display.show(1, 1, 6, "Modern");
        break;
      case 1:
        display.show(1, 1, 6, "Mark I");
        break;
      case 2:
        display.show(1, 1, 6, "OPL");
        break;
    }

    lcd_active_instance_number(selected_instance_id);
    UI_update_instance_icons();
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown())
        configuration.dexed[selected_instance_id].engine = (configuration.dexed[selected_instance_id].engine + 1) % 3;
      else if (LCDML.BT_checkUp())
        configuration.dexed[selected_instance_id].engine = (configuration.dexed[selected_instance_id].engine + 1) % 3;
    }
#if NUM_DEXED > 1
    else if (LCDML.BT_checkEnter()) {
      selected_instance_id = !selected_instance_id;
      lcd_active_instance_number(selected_instance_id);
      UI_update_instance_icons();
    }
#endif

    switch (configuration.dexed[selected_instance_id].engine) {
      case 0:
        display.show(1, 0, 16, "[Modern]");
        break;
      case 1:
        display.show(1, 0, 16, "[Mark I]");
        break;
      case 2:
        display.show(1, 0, 16, "[OPL]");
        break;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_update_instance_icons() {
  display.setCursor(14, 0);
  display.write(0);  //Icon for first instance
  display.setCursor(15, 0);
  display.write(1);  //Icon for second instance
}

void UI_func_voice_select(uint8_t param) {
  static uint8_t menu_voice_select = MENU_VOICE_SOUND;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    lcd_active_instance_number(selected_instance_id);

    char bank_name[BANK_NAME_LEN];
    char voice_name[VOICE_NAME_LEN];

    if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
      strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
    if (!get_voice_by_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, configuration.dexed[selected_instance_id].voice, voice_name, sizeof(voice_name)))
      strlcpy(voice_name, "*ERROR*", sizeof(bank_name));

    UI_update_instance_icons();

    display.createChar(2, (uint8_t*)special_chars[18]);  // favorites symbol

#if defined(TEENSY36)
    display.createChar(6, (uint8_t*)special_chars[16]);  // MIDI activity note symbol
    display.createChar(7, (uint8_t*)special_chars[16]);  // MIDI activity note symbol
#endif
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    char bank_name[BANK_NAME_LEN];
    char voice_name[VOICE_NAME_LEN];

    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && (encoderDir[ENC_R].ButtonShort() || encoderDir[ENC_R].ButtonLong()))) {
      uint8_t bank_tmp;
      int8_t voice_tmp;

      // Reset Performance Modifiers to 0 after every preset change
      for (uint8_t count_tmp = 0; count_tmp < NUM_DEXED; count_tmp++) {
        perform_attack_mod[count_tmp] = 0;
        perform_release_mod[count_tmp] = 0;
      }
      active_perform_page = 1;

      if (LCDML.BT_checkUp()) {
        //start : show all presets
        if (configuration.sys.favorites == 0) {
          switch (menu_voice_select) {
            case MENU_VOICE_BANK:
              memset(g_bank_name[selected_instance_id], 0, BANK_NAME_LEN);
              bank_tmp = constrain(configuration.dexed[selected_instance_id].bank - ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
              configuration.dexed[selected_instance_id].bank = bank_tmp;
              load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              break;
            case MENU_VOICE_SOUND:
              memset(g_voice_name[selected_instance_id], 0, VOICE_NAME_LEN);
              voice_tmp = configuration.dexed[selected_instance_id].voice - ENCODER[ENC_R].speed();
              if (voice_tmp < 0 && configuration.dexed[selected_instance_id].bank - 1 >= 0) {
                configuration.dexed[selected_instance_id].bank--;
                configuration.dexed[selected_instance_id].bank = constrain(configuration.dexed[selected_instance_id].bank, 0, MAX_BANKS - 1);
              } else if (voice_tmp < 0 && configuration.dexed[selected_instance_id].bank - 1 <= 0) {
                voice_tmp = 0;
              }
              if (voice_tmp < 0)
                voice_tmp = MAX_VOICES + voice_tmp;
              configuration.dexed[selected_instance_id].voice = constrain(voice_tmp, 0, MAX_VOICES - 1);
              load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              break;
          }
        } else  //only Favs
          if (configuration.sys.favorites == 1) {
            locate_previous_favorite();
            load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
          } else  //only non-Favs
            if (configuration.sys.favorites == 2) {
              locate_previous_non_favorite();
              load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              //break;

            } else  //random non-Favs
              if (configuration.sys.favorites == 3) {
                locate_random_non_favorite();
                load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              }
      }  //end UP
      else if (LCDML.BT_checkDown()) {
        //start : show all presets
        if (configuration.sys.favorites == 0) {
          switch (menu_voice_select) {
            case MENU_VOICE_BANK:
              memset(g_bank_name[selected_instance_id], 0, BANK_NAME_LEN);
              bank_tmp = constrain(configuration.dexed[selected_instance_id].bank + ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
              configuration.dexed[selected_instance_id].bank = bank_tmp;
              load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              break;
            case MENU_VOICE_SOUND:
              memset(g_voice_name[selected_instance_id], 0, VOICE_NAME_LEN);
              voice_tmp = configuration.dexed[selected_instance_id].voice + ENCODER[ENC_R].speed();
              if (voice_tmp >= MAX_VOICES && configuration.dexed[selected_instance_id].bank + 1 < MAX_BANKS) {
                voice_tmp %= MAX_VOICES;
                configuration.dexed[selected_instance_id].bank++;
                configuration.dexed[selected_instance_id].bank = constrain(configuration.dexed[selected_instance_id].bank, 0, MAX_BANKS - 1);
              } else if (voice_tmp >= MAX_VOICES && configuration.dexed[selected_instance_id].bank + 1 >= MAX_BANKS) {
                voice_tmp = MAX_VOICES - 1;
              }
              configuration.dexed[selected_instance_id].voice = constrain(voice_tmp, 0, MAX_VOICES - 1);
              load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              break;
          }
        } else  //only Favs
          if (configuration.sys.favorites == 1) {

            locate_next_favorite();
            load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
            //break;
          } else  //only non-Favs
            if (configuration.sys.favorites == 2) {
              locate_next_non_favorite();
              load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              //break;
            } else  //random non-Favs
              if (configuration.sys.favorites == 3) {
                locate_random_non_favorite();
                load_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
              }

      } else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonPressed()) {
        if (menu_voice_select == MENU_VOICE_BANK)
          menu_voice_select = MENU_VOICE_SOUND;
        else
          menu_voice_select = MENU_VOICE_BANK;
      }
#if NUM_DEXED > 1
      else if (LCDML.BT_checkEnter()) {
        selected_instance_id = !selected_instance_id;
        lcd_active_instance_number(selected_instance_id);
        UI_update_instance_icons();
      }
#endif
    }

    if (strlen(g_bank_name[selected_instance_id]) > 0) {
      strlcpy(bank_name, g_bank_name[selected_instance_id], sizeof(bank_name));
    } else {
      if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
        strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
    }

    if (strlen(g_voice_name[selected_instance_id]) > 0) {
      strlcpy(voice_name, g_voice_name[selected_instance_id], sizeof(voice_name));
    } else {
      if (!get_voice_by_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, configuration.dexed[selected_instance_id].voice, voice_name, sizeof(voice_name)))
        strlcpy(voice_name, "*ERROR*", sizeof(voice_name));
    }

    display.show(0, 0, 2, configuration.dexed[selected_instance_id].bank);
    display.show(1, 0, 2, configuration.dexed[selected_instance_id].voice + 1);

    string_toupper(bank_name);
    display.show(0, 3, 8, bank_name);
    display.show(0, 12, 1, " ");  //forced because this char does not clear after fav-search (because the bank name is one char to short to do it).
    string_toupper(voice_name);
    display.show(1, 3, 10, voice_name);

    switch (menu_voice_select) {
      case MENU_VOICE_BANK:
        display.show(0, 2, 1, "[");
        display.show(0, 11, 1, "]");
        display.show(1, 2, 1, " ");
        display.show(1, 13, 1, " ");
        break;
      case MENU_VOICE_SOUND:
        display.show(0, 2, 1, " ");
        display.show(0, 11, 1, " ");
        display.show(1, 2, 1, "[");
        display.show(1, 13, 1, "]");
        break;
    }

    draw_favorite_icon(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
}

void UI_func_volume(uint8_t param) {
  char tmp[6];
  static uint8_t old_volume;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    old_volume = configuration.sys.vol;

    encoderDir[ENC_L].reset();

    if (active_perform_page == 1) {  //Master Volume
      lcd_special_chars(BLOCKBAR);
      display_bar_int("Master Vol.", configuration.sys.vol, 1.0, VOLUME_MIN, VOLUME_MAX, 3, false, false, true);
      back_from_volume = 0;
    }

    else if (active_perform_page == 2) {  // Live Performance Mod - Attack
      display.setCursor(0, 0);
      display.print("Live Modify");
      display.setCursor(0, 1);
      display.print("Attack =      ");
      display.setCursor(13, 1);
      snprintf_P(tmp, sizeof(tmp), PSTR("%03d"), perform_attack_mod[selected_instance_id]);
      display.print(tmp);
      back_from_volume = 0;
    }

    else if (active_perform_page == 3) {  // Live Performance Mod - Release
      display.setCursor(0, 0);
      display.print("Live Modify");
      display.setCursor(11, 1);
      display.print("Release =    ");
      display.setCursor(13, 1);
      snprintf_P(tmp, sizeof(tmp), PSTR("%03d"), perform_release_mod[selected_instance_id]);
      display.print(tmp);
      back_from_volume = 0;
    }

    display.setCursor(12, 0);
    display.print("P");
    display.setCursor(13, 0);
    display.print(active_perform_page);
    display.setCursor(14, 0);
    display.print("/3");
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {

    if (LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) {
      back_from_volume = 0;
      active_perform_page++;
      if (active_perform_page > 3) active_perform_page = 1;
    } else if (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) {
      back_from_volume = 0;
      active_perform_page--;
      if (active_perform_page < 1) active_perform_page = 3;
    }

    if ((LCDML.BT_checkDown() && encoderDir[ENC_L].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_L].Up())) {
      if (active_perform_page == 1) {
        back_from_volume = 0;

        if (LCDML.BT_checkDown()) {
          configuration.sys.vol = constrain(configuration.sys.vol + ENCODER[ENC_L].speed(), VOLUME_MIN, VOLUME_MAX);
        } else if (LCDML.BT_checkUp()) {
          configuration.sys.vol = constrain(configuration.sys.vol - ENCODER[ENC_L].speed(), VOLUME_MIN, VOLUME_MAX);
        }
      }

      else if (active_perform_page == 2) {  //Attack

        if (LCDML.BT_checkDown()) {
          if (perform_attack_mod[selected_instance_id] == 0)
            for (uint8_t i = 0; i < 6; i++) {
              orig_attack_values[selected_instance_id][i] = MicroDexed[selected_instance_id]->getOPRate(i, ATTACK);
            }
          perform_attack_mod[selected_instance_id] = constrain(perform_attack_mod[selected_instance_id] + ENCODER[ENC_L].speed(), -MAX_PERF_MOD, MAX_PERF_MOD);
          for (uint8_t i = 0; i < 6; i++)
            MicroDexed[selected_instance_id]->setOPRate(i, ATTACK, orig_attack_values[selected_instance_id][i] - perform_attack_mod[selected_instance_id]);
        } else if (LCDML.BT_checkUp()) {
          if (perform_attack_mod[selected_instance_id] == 0)  // Save initial Values
            for (uint8_t i = 0; i < 6; i++) {
              orig_attack_values[selected_instance_id][i] = MicroDexed[selected_instance_id]->getOPRate(i, ATTACK);
            }

          perform_attack_mod[selected_instance_id] = constrain(perform_attack_mod[selected_instance_id] - ENCODER[ENC_L].speed(), -MAX_PERF_MOD, MAX_PERF_MOD);
          for (uint8_t i = 0; i < 6; i++)
            MicroDexed[selected_instance_id]->setOPRate(i, ATTACK, orig_attack_values[selected_instance_id][i] - perform_attack_mod[selected_instance_id]);
        }

      } else if (active_perform_page == 3) {  //Release

        if (LCDML.BT_checkDown()) {
          if (perform_release_mod[selected_instance_id] == 0)  // Save initial Values
            for (uint8_t i = 0; i < 6; i++) {
              orig_release_values[selected_instance_id][i] = MicroDexed[selected_instance_id]->getOPRate(i, RELEASE);
            }
          perform_release_mod[selected_instance_id] = constrain(perform_release_mod[selected_instance_id] + ENCODER[ENC_L].speed(), -MAX_PERF_MOD, MAX_PERF_MOD);
          for (uint8_t i = 0; i < 6; i++)
            MicroDexed[selected_instance_id]->setOPRate(i, RELEASE, orig_release_values[selected_instance_id][i] - perform_release_mod[selected_instance_id]);
        } else if (LCDML.BT_checkUp()) {
          if (perform_release_mod[selected_instance_id] == 0)
            for (uint8_t i = 0; i < 6; i++) {
              orig_release_values[selected_instance_id][i] = MicroDexed[selected_instance_id]->getOPRate(i, RELEASE);
            }
          perform_release_mod[selected_instance_id] = constrain(perform_release_mod[selected_instance_id] - ENCODER[ENC_L].speed(), -MAX_PERF_MOD, MAX_PERF_MOD);
          for (uint8_t i = 0; i < 6; i++)
            MicroDexed[selected_instance_id]->setOPRate(i, RELEASE, orig_release_values[selected_instance_id][i] - perform_release_mod[selected_instance_id]);
        }
      }
    }

    display.setCursor(13, 0);
    display.print(active_perform_page);

    if (active_perform_page == 1) {  //Master Volume
      display.setCursor(0, 0);
      display.print("Master Vol.  ");
      lcd_special_chars(BLOCKBAR);
      display_bar_int("Master Vol.", configuration.sys.vol, 1.0, VOLUME_MIN, VOLUME_MAX, 3, false, false, false);
      set_volume(configuration.sys.vol, configuration.sys.mono);
    } else if (active_perform_page == 2) {  //Attack
      display.setCursor(0, 0);
      display.print("Live Modify");
      display.setCursor(0, 1);
      display.print("Attack =     ");
      display.setCursor(13, 1);
      snprintf_P(tmp, sizeof(tmp), PSTR("%03d"), perform_attack_mod[selected_instance_id]);
      display.print(tmp);
      back_from_volume = 0;
    } else if (active_perform_page == 3) {  //Release
      display.setCursor(0, 0);
      display.print("Live Modify");
      display.setCursor(0, 1);
      display.print("Release =    ");
      display.setCursor(13, 1);
      snprintf_P(tmp, sizeof(tmp), PSTR("%03d"), perform_release_mod[selected_instance_id]);
      display.print(tmp);
      back_from_volume = 0;
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_L].reset();

    if (old_volume != configuration.sys.vol) {
      eeprom_update();
      save_sys_flag = true;
      save_sys = 0;
    }
  }
}

void UI_func_save_voice(uint8_t param) {
  static bool yesno;
  static uint8_t mode;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    yesno = false;
#if NUM_DEXED == 1
    mode = 1;
#else
    mode = 0;
#endif

#if NUM_DEXED == 1
    char bank_name[BANK_NAME_LEN];

    if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
      strlcpy(bank_name, "*ERROR*");

    display.setCursor(0, 0);
    display.print(F("Save to Bank"));
    display.show(1, 0, 2, configuration.dexed[selected_instance_id].bank);
    display.show(1, 3, 10, bank_name);
    display.show(1, 2, 1, "[");
    display.show(1, 13, 1, "]");
#else
    display.setCursor(0, 0);
    display.print(F("Save Instance"));
    lcd_active_instance_number(selected_instance_id);
    display.setCursor(5, 1);
    display.write(0);
    display.setCursor(10, 1);
    display.write(1);
#endif
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    char bank_name[BANK_NAME_LEN];
    char voice_name[VOICE_NAME_LEN];

    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      switch (mode) {
        case 0:  // Instance selection
          if (LCDML.BT_checkDown() || LCDML.BT_checkUp())
            selected_instance_id = !selected_instance_id;

          lcd_active_instance_number(selected_instance_id);
          display.setCursor(5, 1);
          display.write(0);
          display.setCursor(10, 1);
          display.write(1);
          break;
        case 1:  // Bank selection
          if (LCDML.BT_checkDown())
            configuration.dexed[selected_instance_id].bank = constrain(configuration.dexed[selected_instance_id].bank + ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
          else if (LCDML.BT_checkUp() && configuration.dexed[selected_instance_id].voice > 0)
            configuration.dexed[selected_instance_id].bank = constrain(configuration.dexed[selected_instance_id].bank - ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);

          if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));

          display.show(1, 0, 2, configuration.dexed[selected_instance_id].bank);
          display.show(1, 3, 10, bank_name);
          break;
        case 2:  // Voice selection
          if (LCDML.BT_checkDown() && configuration.dexed[selected_instance_id].voice < MAX_VOICES - 1)
            configuration.dexed[selected_instance_id].voice = constrain(configuration.dexed[selected_instance_id].voice + ENCODER[ENC_R].speed(), 0, MAX_VOICES - 1);
          else if (LCDML.BT_checkUp() && configuration.dexed[selected_instance_id].voice > 0)
            configuration.dexed[selected_instance_id].voice = constrain(configuration.dexed[selected_instance_id].voice - ENCODER[ENC_R].speed(), 0, MAX_VOICES - 1);

          if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
          if (!get_voice_by_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, configuration.dexed[selected_instance_id].voice, voice_name, sizeof(voice_name)))
            strlcpy(voice_name, "*ERROR*", sizeof(voice_name));

          display.show(1, 0, 2, configuration.dexed[selected_instance_id].voice + 1);
          display.show(1, 3, 10, voice_name);
          break;
        case 3:  // Yes/No selection
          yesno = !yesno;
          if (yesno == true) {
            display.show(1, 1, 3, "YES");
          } else {
            display.show(1, 1, 3, "NO");
          }
          break;
      }
    } else if (LCDML.BT_checkEnter()) {
      if (encoderDir[ENC_R].ButtonShort())
        mode++;
      switch (mode) {
        case 1:
          if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
          display.setCursor(0, 0);
          display.print(F("Save to Bank"));
          display.show(1, 0, 2, configuration.dexed[selected_instance_id].bank);
          display.show(1, 3, 10, bank_name);
          display.show(1, 2, 2, " [");
          display.show(1, 14, 1, "]");
          break;
        case 2:
          if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
          if (!get_voice_by_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, configuration.dexed[selected_instance_id].voice, voice_name, sizeof(voice_name)))
            strlcpy(voice_name, "*ERROR*", sizeof(voice_name));

          display.show(0, 0, 16, "Save to Bank");
          display.show(0, 13, 2, configuration.dexed[selected_instance_id].bank);
          display.show(1, 0, 2, configuration.dexed[selected_instance_id].voice + 1);
          display.show(1, 3, 10, voice_name);
          break;
        case 3:
          display.show(0, 0, 16, "Overwrite?");
          display.show(1, 0, 15, "[NO");
          display.show(1, 4, 1, "]");
          break;
        default:
          if (yesno == true) {
#ifdef DEBUG
            bool ret = save_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);

            if (ret == true)
              Serial.println(F("Saving voice OK."));
            else
              Serial.println(F("Error while saving voice."));
#else
            save_sd_voice(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice, selected_instance_id);
#endif

            display.show(1, 0, 16, "Done.");
            delay(MESSAGE_WAIT_TIME);

            mode = 0xff;
            break;
          }

          LCDML.FUNC_goBackToMenu();
      }
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);

    if (mode < 0xff) {
      display.show(1, 0, 16, "Canceled.");
      delay(MESSAGE_WAIT_TIME);
    }
    encoderDir[ENC_R].reset();
  }
}

void UI_func_sysex_receive_bank(uint8_t param) {
  static bool yesno;
  static uint8_t mode;
  static uint8_t bank_number;
  static uint8_t ui_select_name_state;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    yesno = false;
    mode = 0;
    bank_number = configuration.dexed[selected_instance_id].bank;
    memset(receive_bank_filename, 0, sizeof(receive_bank_filename));

    display.setCursor(0, 0);
    display.print(F("MIDI Recv Bank"));
    display.setCursor(2, 1);
    display.print(F("["));
    display.setCursor(14, 1);
    display.print(F("]"));
    if (!get_bank_name(configuration.dexed[selected_instance_id].bank, receive_bank_filename, sizeof(receive_bank_filename)))
      strlcpy(receive_bank_filename, "*ERROR*", sizeof(receive_bank_filename));
    display.show(1, 0, 2, bank_number);
    display.show(1, 3, 10, receive_bank_filename);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        switch (mode) {
          case 0:
            bank_number = constrain(bank_number + ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
            if (!get_bank_name(bank_number, receive_bank_filename, sizeof(receive_bank_filename)))
              strlcpy(receive_bank_filename, "*ERROR*", sizeof(receive_bank_filename));
            display.show(1, 0, 2, bank_number);
            display.show(1, 3, 10, receive_bank_filename);
            break;
          case 1:
            yesno = !yesno;
            if (yesno)
              display.show(1, 12, 3, "YES");
            else
              display.show(1, 12, 3, "NO");
            break;
          case 2:
            ui_select_name_state = UI_select_name(1, 1, receive_bank_filename, BANK_NAME_LEN - 1, false);
            break;
        }
      } else if (LCDML.BT_checkUp()) {
        switch (mode) {
          case 0:
            bank_number = constrain(bank_number - ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
            if (!get_bank_name(bank_number, receive_bank_filename, sizeof(receive_bank_filename)))
              strlcpy(receive_bank_filename, "*ERROR*", sizeof(receive_bank_filename));
            display.show(1, 0, 2, bank_number);
            display.show(1, 3, 10, receive_bank_filename);
            break;
          case 1:
            yesno = !yesno;
            if (yesno)
              display.show(1, 12, 3, "YES");
            else
              display.show(1, 12, 3, "NO");
            break;
          case 2:
            ui_select_name_state = UI_select_name(1, 1, receive_bank_filename, BANK_NAME_LEN - 1, false);
            break;
        }
      }
    } else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      if (mode == 0) {
        if (!strcmp(receive_bank_filename, "*ERROR*")) {
          yesno = true;
          strlcpy(receive_bank_filename, "NONAME", sizeof(receive_bank_filename));
          mode = 2;
          display.setCursor(0, 1);
          display.print(F("[          ]    "));
          ui_select_name_state = UI_select_name(1, 1, receive_bank_filename, BANK_NAME_LEN - 1, true);
          display.blink();
        } else {
          mode = 1;
          display.setCursor(0, 1);
          display.print(F("Overwrite: [NO ]"));
        }
      } else if (mode == 1 && yesno == true) {
        mode = 2;
        display.setCursor(0, 1);
        display.print(F("[          ]    "));
        ui_select_name_state = UI_select_name(1, 1, receive_bank_filename, BANK_NAME_LEN - 1, true);
        display.blink();
      } else if (mode == 2) {
        ui_select_name_state = UI_select_name(1, 1, receive_bank_filename, BANK_NAME_LEN - 1, false);
        if (ui_select_name_state == true) {
          if (yesno == true) {
#ifdef DEBUG
            Serial.print(F("Bank name: ["));
            Serial.print(receive_bank_filename);
            Serial.println(F("]"));
#endif
            char tmp[CONFIG_FILENAME_LEN];
            strlcpy(tmp, receive_bank_filename, sizeof(tmp));
            snprintf_P(receive_bank_filename, sizeof(receive_bank_filename), PSTR("/%d/%s.syx"), bank_number, tmp);
#ifdef DEBUG
            Serial.print(F("Receiving into bank "));
            Serial.print(bank_number);
            Serial.print(F(" as filename "));
            Serial.print(receive_bank_filename);
            Serial.println(F("."));
#endif
            mode = 0xff;
            display.noBlink();
            display.setCursor(0, 1);
            display.print(F("Waiting...      "));
            /// Storing is done in SYSEX code
          }
        }
      } else if (mode >= 1 && yesno == false) {
        Serial.println(mode, DEC);
        memset(receive_bank_filename, 0, sizeof(receive_bank_filename));
        mode = 0xff;
        display.noBlink();
        display.setCursor(0, 1);
        display.print(F("Canceled.       "));
        delay(MESSAGE_WAIT_TIME);
        LCDML.FUNC_goBackToMenu();
      }
    }
    encoderDir[ENC_R].reset();
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();

    memset(receive_bank_filename, 0, sizeof(receive_bank_filename));
    display.noBlink();

    if (mode < 0xff) {
      display.setCursor(0, 1);
      display.print(F("Canceled.       "));
      delay(MESSAGE_WAIT_TIME);
    }
  }
}

void UI_func_set_performance_name(uint8_t param) {
  static uint8_t mode;
  static uint8_t ui_select_name_state;
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    display.createChar(0, (uint8_t*)special_chars[19]);  // edit symbol

    encoderDir[ENC_R].reset();
    mode = 0;
    display.setCursor(0, 0);
    display.print(F("Perf. Name"));

    bool found = false;
    for (uint8_t i = 0; i < sizeof(configuration.performance.name) - 1; i++) {
      if (configuration.performance.name[i] == 0 && found == false)
        found = true;
      if (found == true) {
        configuration.performance.name[i] = ' ';
        configuration.performance.name[i + 1] = 0;
      }
    }
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        if (mode == 1) ui_select_name_state = UI_select_name(1, 1, configuration.performance.name, sizeof(configuration.performance.name), false);
      } else if (LCDML.BT_checkUp()) {
        if (mode == 1) ui_select_name_state = UI_select_name(1, 1, configuration.performance.name, sizeof(configuration.performance.name), false);
      }
    } else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {

      if (mode == 1) {
        ui_select_name_state = UI_select_name(1, 1, configuration.performance.name, sizeof(configuration.performance.name), false);
        if (ui_select_name_state == true) {
          mode = 0xff;
          display.noBlink();
          display.setCursor(0, 1);
          display.print(F("OK.             "));
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();
        }
      }
    }
    if (mode == 0) {
      mode = 1;
      display.setCursor(0, 1);
      display.print(F("[          ]    "));
      ui_select_name_state = UI_select_name(1, 1, configuration.performance.name, sizeof(configuration.performance.name), true);
      display.blink();
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
    display.noBlink();
  }
}

void UI_func_sysex_send_bank(uint8_t param) {
  char bank_name[BANK_NAME_LEN];
  static uint8_t bank_number;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    bank_number = configuration.dexed[selected_instance_id].bank;
    display.setCursor(0, 0);
    display.print(F("MIDI Send Bank"));
    if (!get_bank_name(configuration.dexed[selected_instance_id].bank, bank_name, sizeof(bank_name)))
      strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
    display.show(1, 2, 1, "[");
    display.show(1, 14, 1, "]");
    display.show(1, 0, 2, configuration.dexed[selected_instance_id].bank);
    display.show(1, 3, 10, bank_name);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        bank_number = constrain(bank_number + ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);

      } else if (LCDML.BT_checkUp()) {
        bank_number = constrain(bank_number - ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
      }
      if (!get_bank_name(bank_number, bank_name, sizeof(bank_name)))
        strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
      display.show(1, 0, 2, bank_number);
      display.show(1, 3, 10, bank_name);
    } else if (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort()) {
      File sysex;
      char filename[FILENAME_LEN];

      if (get_bank_name(bank_number, bank_name, sizeof(bank_name))) {
        snprintf_P(filename, sizeof(filename), PSTR("/%d/%s.syx"), bank_number, bank_name);
#ifdef DEBUG
        Serial.print(F("Send bank "));
        Serial.print(filename);
        Serial.println(F(" from SD."));
#endif
        sysex = SD.open(filename);
        if (!sysex) {
#ifdef DEBUG
          Serial.println(F("Connot read from SD."));
#endif
          display.show(1, 0, 16, "Read error.");
          bank_number = 0xff;
        } else {
          uint8_t bank_data[4104];

          sysex.read(bank_data, 4104);
          sysex.close();

          display.show(1, 0, 16, "Sending Ch");
          if (configuration.dexed[selected_instance_id].midi_channel == MIDI_CHANNEL_OMNI) {
            display.show(1, 11, 2, "01");
            send_sysex_bank(1, bank_data);
          } else {
            display.show(1, 11, 2, configuration.dexed[selected_instance_id].midi_channel + 1);
            send_sysex_bank(configuration.dexed[selected_instance_id].midi_channel, bank_data);
          }
          display.show(1, 0, 16, "Done.");
          bank_number = 0xff;
        }
      } else {
        display.show(1, 0, 16, "No bank.");
        bank_number = 0xff;
      }

      delay(MESSAGE_WAIT_TIME);
      LCDML.FUNC_goBackToMenu();
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();

    if (bank_number < 0xff) {
      display.setCursor(0, 1);
      display.print(F("Canceled.       "));
      delay(MESSAGE_WAIT_TIME);
    }
  }
}

void UI_func_sysex_send_voice(uint8_t param) {
  static uint8_t mode;
  static uint8_t bank_number;
  static uint8_t voice_number;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    mode = 0;
    bank_number = configuration.dexed[selected_instance_id].bank;
    voice_number = configuration.dexed[selected_instance_id].voice;

    char bank_name[BANK_NAME_LEN];

    if (!get_bank_name(bank_number, bank_name, sizeof(bank_name)))
      strlcpy(bank_name, "*ERROR*", sizeof(bank_name));

    display.setCursor(0, 0);
    display.print(F("MIDI Send Voice"));
    display.show(1, 0, 2, bank_number);
    display.show(1, 3, 10, bank_name);
    display.show(1, 2, 1, "[");
    display.show(1, 13, 1, "]");
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    char bank_name[BANK_NAME_LEN];
    char voice_name[VOICE_NAME_LEN];

    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      switch (mode) {
        case 0:  // Bank selection
          if (LCDML.BT_checkDown())
            bank_number = constrain(bank_number + ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);
          else if (LCDML.BT_checkUp() && bank_number > 0)
            bank_number = constrain(bank_number - ENCODER[ENC_R].speed(), 0, MAX_BANKS - 1);

          if (!get_bank_name(bank_number, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));

          display.show(1, 0, 2, bank_number);
          display.show(1, 3, 10, bank_name);
          break;
        case 1:  // Voice selection
          if (LCDML.BT_checkDown() && voice_number < MAX_VOICES - 1)
            voice_number = constrain(voice_number + ENCODER[ENC_R].speed(), 0, MAX_VOICES - 1);
          else if (LCDML.BT_checkUp() && voice_number > 0)
            voice_number = constrain(voice_number - ENCODER[ENC_R].speed(), 0, MAX_VOICES - 1);
          if (!get_bank_name(bank_number, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
          if (!get_voice_by_bank_name(bank_number, bank_name, voice_number, voice_name, sizeof(voice_name)))
            strlcpy(voice_name, "*ERROR*", sizeof(voice_name));

          display.show(1, 0, 2, voice_number + 1);
          display.show(1, 3, 10, voice_name);
          break;
      }
    } else if (LCDML.BT_checkEnter()) {
      if (encoderDir[ENC_R].ButtonShort())
        mode++;
      switch (mode) {
        case 1:
          if (!get_bank_name(bank_number, bank_name, sizeof(bank_name)))
            strlcpy(bank_name, "*ERROR*", sizeof(bank_name));
          if (!get_voice_by_bank_name(bank_number, bank_name, voice_number, voice_name, sizeof(voice_name)))
            strlcpy(voice_name, "*ERROR*", sizeof(voice_name));

          display.show(1, 0, 2, voice_number + 1);
          display.show(1, 3, 10, voice_name);
          break;
        case 2:
          File sysex;
          char filename[FILENAME_LEN];

          if (get_bank_name(bank_number, bank_name, sizeof(bank_name))) {
            snprintf_P(filename, sizeof(filename), PSTR("/%d/%s.syx"), bank_number, bank_name);
#ifdef DEBUG
            Serial.print(F("Send voice "));
            Serial.print(voice_number);
            Serial.print(F(" of "));
            Serial.print(filename);
            Serial.println(F(" from SD."));
#endif
            sysex = SD.open(filename);
            if (!sysex) {
#ifdef DEBUG
              Serial.println(F("Connot read from SD."));
#endif
              display.show(1, 0, 16, "Read error.");
              bank_number = 0xff;
            } else {
              uint8_t voice_data[155];
              uint8_t encoded_voice_data[128];

              sysex.seek(6 + (voice_number * 128));
              sysex.read(encoded_voice_data, 128);

              MicroDexed[selected_instance_id]->decodeVoice(voice_data, encoded_voice_data);

              display.show(1, 0, 16, "Sending Ch");
              if (configuration.dexed[selected_instance_id].midi_channel == MIDI_CHANNEL_OMNI) {
                display.show(1, 11, 2, "01");
                send_sysex_voice(1, voice_data);
              } else {
                display.show(1, 11, 2, configuration.dexed[selected_instance_id].midi_channel + 1);
                send_sysex_voice(configuration.dexed[selected_instance_id].midi_channel, voice_data);
              }
              delay(MESSAGE_WAIT_TIME);
              display.show(1, 0, 16, "Done.");
              sysex.close();

              bank_number = 0xff;
            }
          } else {
            display.show(1, 0, 16, "No voice.");
            bank_number = 0xff;
          }

          mode = 0xff;
          delay(MESSAGE_WAIT_TIME);
          LCDML.FUNC_goBackToMenu();

          break;
      }
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    if (mode < 0xff) {
      display.show(1, 0, 16, "Canceled.");
      delay(MESSAGE_WAIT_TIME);
    }
    encoderDir[ENC_R].reset();
  }
}

void UI_func_eq_1(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ Low-Cut"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_1 = constrain(configuration.fx.eq_1 + ENCODER[ENC_R].speed(), EQ_1_MIN, EQ_1_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_1 = constrain(configuration.fx.eq_1 - ENCODER[ENC_R].speed(), EQ_1_MIN, EQ_1_MAX);
      }
    }
    display_meter_int("EQ Low-Cut [Hz]", configuration.fx.eq_1, 1.0, 0.0, EQ_1_MIN, EQ_1_MAX, 3, false, false, true);
    sgtl5000.setEQFc(1, float(configuration.fx.eq_1));
    sgtl5000.setEQGain(1, 6.0);
    sgtl5000.commitFilter(1);
#ifdef DEBUG
    sgtl5000.show_params(1);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_eq_2(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ 120Hz"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_2 = constrain(configuration.fx.eq_2 + ENCODER[ENC_R].speed(), EQ_2_MIN, EQ_2_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_2 = constrain(configuration.fx.eq_2 - ENCODER[ENC_R].speed(), EQ_2_MIN, EQ_2_MAX);
      }
    }
    display_meter_float("EQ 120Hz [dB]", configuration.fx.eq_2, 0.1, 0.0, EQ_2_MIN, EQ_2_MAX, 1, 1, false, true, true);
    sgtl5000.setEQGain(2, mapfloat(configuration.fx.eq_2, EQ_2_MIN, EQ_2_MAX, -9.9, 9.9));
    sgtl5000.commitFilter(2);
#ifdef DEBUG
    sgtl5000.show_params(2);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_eq_3(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ 220Hz"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_3 = constrain(configuration.fx.eq_3 + ENCODER[ENC_R].speed(), EQ_3_MIN, EQ_3_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_3 = constrain(configuration.fx.eq_3 - ENCODER[ENC_R].speed(), EQ_3_MIN, EQ_3_MAX);
      }
    }
    display_meter_float("EQ 220Hz [dB]", configuration.fx.eq_3, 0.1, 0.0, EQ_3_MIN, EQ_3_MAX, 1, 1, false, true, true);
    sgtl5000.setEQGain(3, mapfloat(configuration.fx.eq_3, EQ_3_MIN, EQ_3_MAX, -9.9, 9.9));
    sgtl5000.commitFilter(3);
#ifdef DEBUG
    sgtl5000.show_params(3);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_eq_4(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ 1000Hz"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_4 = constrain(configuration.fx.eq_4 + ENCODER[ENC_R].speed(), EQ_4_MIN, EQ_4_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_4 = constrain(configuration.fx.eq_4 - ENCODER[ENC_R].speed(), EQ_4_MIN, EQ_4_MAX);
      }
    }
    display_meter_float("EQ 1000Hz [dB]", configuration.fx.eq_4, 0.1, 0.0, EQ_4_MIN, EQ_4_MAX, 1, 1, false, true, true);
    sgtl5000.setEQGain(4, mapfloat(configuration.fx.eq_4, EQ_4_MIN, EQ_4_MAX, -9.9, 9.9));
    sgtl5000.commitFilter(4);
#ifdef DEBUG
    sgtl5000.show_params(4);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_eq_5(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ 2000Hz"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_5 = constrain(configuration.fx.eq_5 + ENCODER[ENC_R].speed(), EQ_5_MIN, EQ_5_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_5 = constrain(configuration.fx.eq_5 - ENCODER[ENC_R].speed(), EQ_5_MIN, EQ_5_MAX);
      }
    }
    display_meter_float("EQ 2000Hz [dB]", configuration.fx.eq_5, 0.1, 0.0, EQ_5_MIN, EQ_5_MAX, 1, 1, false, true, true);
    sgtl5000.setEQGain(5, mapfloat(configuration.fx.eq_5, EQ_5_MIN, EQ_5_MAX, -9.9, 9.9));
    sgtl5000.commitFilter(5);
#ifdef DEBUG
    sgtl5000.show_params(5);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_eq_6(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ 7000Hz"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_6 = constrain(configuration.fx.eq_6 + ENCODER[ENC_R].speed(), EQ_6_MIN, EQ_6_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_6 = constrain(configuration.fx.eq_6 - ENCODER[ENC_R].speed(), EQ_6_MIN, EQ_6_MAX);
      }
    }
    display_meter_float("EQ 7000Hz [dB]", configuration.fx.eq_6, 0.1, 0.0, EQ_6_MIN, EQ_6_MAX, 1, 1, false, true, true);
    sgtl5000.setEQGain(6, mapfloat(configuration.fx.eq_6, EQ_6_MIN, EQ_6_MAX, -9.9, 9.9));
    sgtl5000.commitFilter(6);
#ifdef DEBUG
    sgtl5000.show_params(6);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_eq_7(uint8_t param) {
#ifndef SGTL5000_AUDIO_ENHANCE
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.print(F("EQ High-Cut"));
    display.setCursor(0, 1);
    display.print(F("Not implemented."));
  }
#else
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();
    lcd_special_chars(METERBAR);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up())) {
      if (LCDML.BT_checkDown()) {
        configuration.fx.eq_7 = constrain(configuration.fx.eq_7 + ENCODER[ENC_R].speed(), EQ_7_MIN, EQ_7_MAX);
      } else if (LCDML.BT_checkUp()) {
        configuration.fx.eq_7 = constrain(configuration.fx.eq_7 - ENCODER[ENC_R].speed(), EQ_7_MIN, EQ_7_MAX);
      }
    }
    display_meter_float("EQ High-Cut[kHz]", configuration.fx.eq_7, 1.0, 0.0, EQ_7_MIN, EQ_7_MAX, 3, 1, false, false, true);
    sgtl5000.setEQFc(7, float(configuration.fx.eq_7) * 1000.0);
    sgtl5000.commitFilter(7);
#ifdef DEBUG
    sgtl5000.show_params(7);
#endif
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd_special_chars(SCROLLBAR);
    encoderDir[ENC_R].reset();
  }
#endif
}

void UI_func_startup(uint8_t param) {
  bool stored = false;
  static uint8_t old_load_at_startup;

  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    old_load_at_startup = configuration.sys.load_at_startup;

    encoderDir[ENC_R].reset();
    display.setCursor(0, 0);
    display.show(0, 0, 16, "Load at startup");
    if (configuration.sys.load_at_startup == 255)
      display.show(1, 0, 16, "Last Perf.");
    else if (configuration.sys.load_at_startup <= PERFORMANCE_NUM_MAX) {
      display.show(1, 0, 16, "Fixed Perf. [");
      display.show(1, 13, 2, configuration.sys.load_at_startup);
      display.show(1, 15, 1, "]");
    }
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if ((LCDML.BT_checkDown() && encoderDir[ENC_R].Down()) || (LCDML.BT_checkUp() && encoderDir[ENC_R].Up()) || (LCDML.BT_checkEnter() && encoderDir[ENC_R].ButtonShort())) {
      if (LCDML.BT_checkDown()) {
        if (configuration.sys.load_at_startup == 255)
          configuration.sys.load_at_startup = PERFORMANCE_NUM_MIN;
        else if (configuration.sys.load_at_startup >= 0 && configuration.sys.load_at_startup <= PERFORMANCE_NUM_MAX)
          configuration.sys.load_at_startup++;
        if (configuration.sys.load_at_startup > PERFORMANCE_NUM_MAX)
          configuration.sys.load_at_startup = 255;
      } else if (LCDML.BT_checkUp()) {
        if (configuration.sys.load_at_startup == 255)
          configuration.sys.load_at_startup = PERFORMANCE_NUM_MAX;
        else if (configuration.sys.load_at_startup >= PERFORMANCE_NUM_MIN && configuration.sys.load_at_startup <= PERFORMANCE_NUM_MAX)
          configuration.sys.load_at_startup--;
      } else if (LCDML.BT_checkEnter()) {
        stored = true;
        display.show(1, 0, 16, "Done.");
        save_sd_sys_json();
        if (configuration.sys.load_at_startup <= PERFORMANCE_NUM_MAX && configuration.sys.load_at_startup != configuration.sys.performance_number)
          load_sd_performance_json(configuration.sys.load_at_startup);
        delay(MESSAGE_WAIT_TIME);
        LCDML.FUNC_goBackToMenu();
      }

      display.setCursor(0, 1);
      if (configuration.sys.load_at_startup == 255)
        display.show(1, 0, 16, "Last Perf.");
      else if (configuration.sys.load_at_startup <= PERFORMANCE_NUM_MAX) {
        display.show(1, 0, 16, "Fixed Perf. [");
        display.show(1, 13, 2, configuration.sys.load_at_startup);
        display.show(1, 15, 1, "]");
      }
    }
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    //lcd_special_chars(SCROLLBAR);
    if (stored == false) {
      display.show(1, 0, 16, "Canceled.");
      configuration.sys.load_at_startup = old_load_at_startup;
      delay(MESSAGE_WAIT_TIME);
    }
    encoderDir[ENC_R].reset();
  }
}

void UI_function_not_enabled(void) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Function not"));
    display.setCursor(0, 1);
    display.print(F("enbaled!"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    ;
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();
  }
}

void UI_function_not_implemented(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    encoderDir[ENC_R].reset();

    display.setCursor(0, 0);
    display.print(F("Function not"));
    display.setCursor(0, 1);
    display.print(F("implemented!"));
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    ;
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    encoderDir[ENC_R].reset();
  }
}

bool UI_select_name(uint8_t y, uint8_t x, char* edit_string, uint8_t len, bool init) {
  static int8_t edit_pos;
  static bool edit_mode;
  static uint8_t edit_value;

  if (init == true) {
    edit_mode = false;
    edit_pos = 0;
    edit_value = search_accepted_char(edit_string[edit_pos]);

    display.setCursor(x, y);
    display.print(edit_string);
    display.setCursor(x, y);

    return (false);
  }

  if (LCDML.BT_checkDown() || LCDML.BT_checkUp() || LCDML.BT_checkEnter()) {
    if (edit_mode) {
      edit_value = search_accepted_char(edit_string[edit_pos]);
      if (LCDML.BT_checkDown()) {
        if (edit_value < sizeof(accepted_chars) - 2)
          edit_value++;
      } else if (LCDML.BT_checkUp()) {
        if (edit_value > 0)
          edit_value--;
      } else if (LCDML.BT_checkEnter()) {
        edit_mode = !edit_mode;
        display.setCursor(LCD_cols - 1, 0);
        display.print(F(" "));
      }

      edit_string[edit_pos] = accepted_chars[edit_value];
      display.setCursor(x + edit_pos, y);
      display.print(edit_string[edit_pos]);
    } else {
      if (LCDML.BT_checkDown()) {
        if (edit_pos < len - 1)
          edit_pos++;
      } else if (LCDML.BT_checkUp()) {
        if (edit_pos > 0)
          edit_pos--;
      } else if (LCDML.BT_checkEnter()) {
        if (edit_pos == len - 1) {  // OK pressed
          edit_pos = 0;
          edit_mode = false;
          string_trim(edit_string);
          return (true);
        } else {
          edit_mode = !edit_mode;
          display.setCursor(LCD_cols - 1, 0);
          display.write(0);
        }
      }

      if (edit_pos > len - 2) {
        display.noBlink();
        display.setCursor(x - 1, y);
        display.print(F(" "));
        display.setCursor(x + len - 1, y);
        display.print(F(" "));
        display.setCursor(LCD_cols - 4, y);
        display.print(F("[OK]"));
      } else if (edit_pos == len - 2) {
        display.setCursor(x - 1, y);
        display.print(F("["));
        display.setCursor(x + len - 1, y);
        display.print(F("]"));
        display.setCursor(LCD_cols - 4, y);
        display.print(F("    "));
        display.blink();
      }
    }
  }

  display.setCursor(x + edit_pos, y);
  encoderDir[ENC_R].reset();

  return (false);
}

uint8_t search_accepted_char(uint8_t c) {
  //if (c == 0)
  //  c = 32;

  for (uint8_t i = 0; i < sizeof(accepted_chars) - 1; i++) {
    Serial.print(i, DEC);
    Serial.print(F(":"));
    Serial.print(c);
    Serial.print(F("=="));
    Serial.println(accepted_chars[i], DEC);
    if (c == accepted_chars[i])
      return (i);
  }
  return (0);
}

void display_int(int16_t var, uint8_t size, bool zeros, bool brackets, bool sign) {
  display_float(float(var), size, 0, zeros, brackets, sign);
}

void display_float(float var, uint8_t size_number, uint8_t size_fraction, bool zeros, bool brackets, bool sign) {
  char s[LCD_cols + 1];

  if (size_fraction > 0) {
    if (zeros == true && sign == true)
      snprintf_P(s, sizeof(s), PSTR("%+0*.*f"), size_number + size_fraction + 2, size_fraction, var);
    else if (zeros == true && sign == false)
      snprintf_P(s, sizeof(s), PSTR("%0*.*f"), size_number + size_fraction + 1, size_fraction, var);
    else if (zeros == false && sign == true)
      snprintf_P(s, sizeof(s), PSTR("%+*.*f"), size_number + size_fraction + 2, size_fraction, var);
    else if (zeros == false && sign == false)
      snprintf_P(s, sizeof(s), PSTR("%*.*f"), size_number + size_fraction + 1, size_fraction, var);
  } else {
    if (zeros == true && sign == true)
      snprintf_P(s, sizeof(s), PSTR("%+0*d"), size_number + 1, int(var));
    else if (zeros == true && sign == false)
      snprintf_P(s, sizeof(s), PSTR("%0*d"), size_number, int(var));
    else if (zeros == false && sign == true)
      snprintf_P(s, sizeof(s), PSTR("%+*d"), size_number + 1, int(var));
    else if (zeros == false && sign == false)
      snprintf_P(s, sizeof(s), PSTR("%*d"), size_number, int(var));
  }

  if (brackets == true) {
    char tmp[LCD_cols + 1];

    strlcpy(tmp, s, sizeof(tmp));
    snprintf_P(s, sizeof(s), PSTR("[%s]"), tmp);
  }

  Serial.println(var);
  Serial.println(s);

  display.print(s);
}

inline void display_bar_int(const char* title, uint32_t value, float factor, int32_t min_value, int32_t max_value, uint8_t size, bool zeros, bool sign, bool init) {
  display_bar_float(title, float(value), factor, min_value, max_value, size, 0, zeros, sign, init);
}

void display_bar_float(const char* title, float value, float factor, int32_t min_value, int32_t max_value, uint8_t size_number, uint8_t size_fraction, bool zeros, bool sign, bool init) {
  uint8_t size;
  float v;
  float _vi = 0.0;
  uint8_t vf;
  uint8_t vi;

  if (size_fraction == 0)
    size = size_number;
  else
    size = size_number + size_fraction + 1;
  if (sign == true)
    size++;

  v = float((value - min_value) * (LCD_cols - size)) / (max_value - min_value);
  vf = uint8_t(modff(v, &_vi) * 10.0 + 0.5);
  vi = uint8_t(_vi);

  if (sign == true)
    size += 1;

  // Title
  if (init == true)
    display.show(0, 0, LCD_cols - 3, title);

  // Value
  display.setCursor(LCD_cols - size, 1);
  display_float(value * factor, size_number, size_fraction, zeros, false, sign);  // does not work with "Smallest code" optimizer
  /* char s[LCD_cols + 1];
    snprintf_P(s, sizeof(s), PSTR("%+1.1f"), value * factor); // not so good solution, but works with optimizer
    display.print(s); */

  // Bar
  display.setCursor(0, 1);

  if (vi == 0) {
    display.write((uint8_t)(vf / 2.0 - 0.5) + 2);
    for (uint8_t i = vi + 1; i < LCD_cols - size; i++)
      display.print(F(" "));  // empty block
  } else {
    for (uint8_t i = 0; i < vi; i++)
      display.write((uint8_t)4 + 2);  // full block
    if (vi < LCD_cols - size)
      display.write((uint8_t)(vf / 2.0 - 0.5) + 2);
    for (uint8_t i = vi + 1; i < LCD_cols - size; i++)
      display.print(F(" "));  // empty block
  }
}

inline void display_meter_int(const char* title, uint32_t value, float factor, float offset, int32_t min_value, int32_t max_value, uint8_t size, bool zeros, bool sign, bool init) {
  display_meter_float(title, float(value), factor, offset, min_value, max_value, size, 0, zeros, sign, init);
}

void display_meter_float(const char* title, float value, float factor, float offset, int32_t min_value, int32_t max_value, uint8_t size_number, uint8_t size_fraction, bool zeros, bool sign, bool init) {
  uint8_t size = 0;
  float v;
  float _vi = 0.0;
  uint8_t vf;
  uint8_t vi;

  if (size_fraction == 0)
    size = size_number;
  else
    size = size_number + size_fraction + 1;
  if (sign == true)
    size++;

  v = float((value - min_value) * (LCD_cols - size)) / (max_value - min_value);
  vf = uint8_t(modff(v, &_vi) * 10.0 + 0.5);
  vi = uint8_t(_vi);

  if (init == true) {
    // Title
    display.setCursor(0, 0);
    display.print(title);
  }

  // Value
  display.setCursor(LCD_cols - size, 1);
  display_float((value + offset) * factor, size_number, size_fraction, zeros, false, sign);  // does not work with "Smallest code" optimizer
  /* char s[LCD_cols + 1];
    snprintf_P(s, sizeof(s), PSTR("%+1.1f"), (value + offset) * factor); // not so good solution, but works with optimizer
    display.print(s); */

  // Bar
  display.setCursor(0, 1);

  if (vi == 0) {
    display.write((uint8_t)(vf / 2.0) + 2);
    for (uint8_t i = 1; i < LCD_cols - size; i++)
      display.print(F(" "));  // empty block
  } else if (vi == LCD_cols - size) {
    for (uint8_t i = 0; i < LCD_cols - size - 1; i++)
      display.print(F(" "));  // empty block
    display.write(4 + 2);
  } else {
    for (uint8_t i = 0; i < LCD_cols - size; i++)
      display.print(F(" "));  // empty block
    display.setCursor(vi, 1);
    display.write((uint8_t)(vf / 2.0) + 2);
    for (uint8_t i = vi + 1; i < LCD_cols - size; i++)
      display.print(F(" "));  // empty block
  }
}

uint8_t bit_reverse8(uint8_t v) {
  uint8_t result = 0;
  for (; v > 0; v >>= 1)
    (result <<= 1) |= (v & 1);
  return (result);
}

void lcd_active_instance_number(uint8_t instance_id) {
  for (uint8_t i = 0; i < 8; i++) {
    if (instance_id == 0) {
      if (configuration.dexed[instance_id].polyphony == 0)
        instance_num[0][i] = bit_reverse8(special_chars[0][i]);
      else
        instance_num[0][i] = special_chars[0][i];

      if (configuration.dexed[!instance_id].polyphony == 0) {
        instance_num[1][i] = bit_reverse8(special_chars[1][i]);
        instance_num[1][i] = ~instance_num[1][i];
      } else
        instance_num[1][i] = ~special_chars[1][i];
    } else {
      if (configuration.dexed[!instance_id].polyphony == 0) {
        instance_num[0][i] = bit_reverse8(special_chars[0][i]);
        instance_num[0][i] = ~instance_num[0][i];
      } else
        instance_num[0][i] = ~special_chars[0][i];

      if (configuration.dexed[instance_id].polyphony == 0)
        instance_num[1][i] = bit_reverse8(special_chars[1][i]);
      else
        instance_num[1][i] = special_chars[1][i];
    }
  }

#if NUM_DEXED == 1
  display.createChar(0, instance_num[0]);
  display.createChar(1, (uint8_t*)special_chars[17]);
#else
  display.createChar(0, instance_num[0]);
  display.createChar(1, instance_num[1]);
#endif
}

void lcd_OP_active_instance_number(uint8_t instance_id, uint8_t op) {
  uint8_t i, n;

  for (n = 2; n < 8; n++) {
    for (i = 0; i < 8; i++) {
      if (bitRead(op, n - 2))
        instance_num[n][i] = special_chars[n][i];
      else
        instance_num[n][i] = ~special_chars[n][i];
    }
    display.createChar(n, instance_num[n]);
  }

  for (i = 0; i < 8; i++) {
    if (instance_id == 0) {
      if (configuration.dexed[instance_id].polyphony == 0)
        instance_num[0][i] = bit_reverse8(special_chars[0][i]);
      else
        instance_num[0][i] = special_chars[0][i];

      if (configuration.dexed[!instance_id].polyphony == 0) {
        instance_num[1][i] = bit_reverse8(special_chars[1][i]);
        instance_num[1][i] = ~instance_num[1][i];
      } else
        instance_num[1][i] = ~special_chars[1][i];
    } else {
      if (configuration.dexed[!instance_id].polyphony == 0) {
        instance_num[0][i] = bit_reverse8(special_chars[0][i]);
        instance_num[0][i] = ~instance_num[0][i];
      } else
        instance_num[0][i] = ~special_chars[0][i];

      if (configuration.dexed[instance_id].polyphony == 0)
        instance_num[1][i] = bit_reverse8(special_chars[1][i]);
      else
        instance_num[1][i] = special_chars[1][i];
    }
  }

  display.createChar(0, instance_num[0]);
#if NUM_DEXED > 1
  display.createChar(1, instance_num[1]);
#else
  display.createChar(1, (uint8_t*)special_chars[17]);
#endif
}

void lcd_special_chars(uint8_t mode) {
  switch (mode) {
    case SCROLLBAR:
      // set special chars for scrollbar
      for (uint8_t i = 0; i < 5; i++) {
#ifdef I2C_DISPLAY
        display.createChar(i, (uint8_t*)scroll_bar[i]);
#else
        flipped_scroll_bar[i] = rotTile(scroll_bar[i]);
#endif
      }
      break;
    case BLOCKBAR:
      // set special chars for volume-bar
      for (uint8_t i = 0; i < 7; i++) {
#ifdef I2C_DISPLAY
        display.createChar(i + 2, (uint8_t*)block_bar[i]);
#else
        flipped_block_bar[i] = rotTile(block_bar[i]);
#endif
      }
      break;
    case METERBAR:
      // set special chars for panorama-bar
      for (uint8_t i = 0; i < 7; i++) {
#ifdef I2C_DISPLAY
        display.createChar(i + 2, (uint8_t*)meter_bar[i]);
#else
        flipped_meter_bar[i] = rotTile(meter_bar[i]);
#endif
      }
      break;
  }
}

void string_trim(char* s) {
  int i;

  while (isspace(*s)) s++;  // skip left side white spaces
  for (i = strlen(s) - 1; (isspace(s[i])); i--)
    ;  // skip right side white spaces
  s[i + 1] = '\0';
}

void locate_previous_non_favorite() {
  //find prev. non fav in current bank
  display.setCursor(3, 0);
  display.print("<SEARCHING");
  do {
    if (configuration.dexed[selected_instance_id].voice == 0) {
      configuration.dexed[selected_instance_id].voice = 32;  //+1
      if (configuration.dexed[selected_instance_id].bank < 1)
        configuration.dexed[selected_instance_id].bank = MAX_BANKS - 1;

      do {  //seek for previous bank
        configuration.dexed[selected_instance_id].bank--;
        if (configuration.dexed[selected_instance_id].bank < 1)
          configuration.dexed[selected_instance_id].bank = MAX_BANKS - 1;
        favsearcher++;
      } while (quick_check_favorites_in_bank(configuration.dexed[selected_instance_id].bank, selected_instance_id) == true && favsearcher < 132);
    }
    configuration.dexed[selected_instance_id].voice--;
    favsearcher++;
  } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                          selected_instance_id)
             == true
           && favsearcher < 170);
  favsearcher = 0;
}

void locate_previous_favorite() {

  // worst case, nothing found below voice 0 /  bank 0 - start loop at last bank
  if (configuration.dexed[selected_instance_id].voice < 2 && configuration.dexed[selected_instance_id].bank == 0 && favsearcher < 170) {
    configuration.dexed[selected_instance_id].bank = MAX_BANKS - 1;
    configuration.dexed[selected_instance_id].voice = 32;
  } else

    if (configuration.dexed[selected_instance_id].voice == 0 && configuration.dexed[selected_instance_id].bank < MAX_BANKS - 1) {  //if at begin of any other bank
    configuration.dexed[selected_instance_id].bank--;
    configuration.dexed[selected_instance_id].voice = 32;
  }

  if (configuration.dexed[selected_instance_id].voice >= 0 && configuration.dexed[selected_instance_id].bank >= 0) {

    display.setCursor(3, 0);
    display.print("<SEARCHING");

    do {  //first find previous fav in current bank

      if (configuration.dexed[selected_instance_id].voice == 0) {

        if (configuration.dexed[selected_instance_id].bank == 0) {
          configuration.dexed[selected_instance_id].bank = MAX_BANKS - 1;
          configuration.dexed[selected_instance_id].voice = 32;
        } else
          configuration.dexed[selected_instance_id].bank--;
        configuration.dexed[selected_instance_id].voice = 32;

      } else

        configuration.dexed[selected_instance_id].voice--;
      favsearcher++;

    } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                            selected_instance_id)
               == false
             && configuration.dexed[selected_instance_id].voice >= 1 && favsearcher < 36);

    // if found, we are done. else quick check in previous banks

    if (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                       selected_instance_id)
          == false
        && configuration.dexed[selected_instance_id].voice >= 0 && configuration.dexed[selected_instance_id].bank >= 0 && favsearcher < 170) {
      configuration.dexed[selected_instance_id].voice = 32;

      do {  //seek for previous bank
        configuration.dexed[selected_instance_id].bank--;
        favsearcher++;
      } while (quick_check_favorites_in_bank(configuration.dexed[selected_instance_id].bank, selected_instance_id) == false && favsearcher < 132 && configuration.dexed[selected_instance_id].bank >= 0);

      do {  //last try to search if a bank with fav was found

        configuration.dexed[selected_instance_id].voice--;
        favsearcher++;
      } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                              selected_instance_id)
                 == false
               && configuration.dexed[selected_instance_id].voice >= 1 && favsearcher < 170);
    }
  }
  favsearcher = 0;
}

void locate_next_favorite()

{

  bool RollOver = false;
  if (configuration.dexed[selected_instance_id].voice > 30 && configuration.dexed[selected_instance_id].bank >= MAX_BANKS - 1) {  //if at end of all banks
    configuration.dexed[selected_instance_id].bank = 0;
    configuration.dexed[selected_instance_id].voice = 0;
    RollOver = true;

  } else if (configuration.dexed[selected_instance_id].voice > 30 && configuration.dexed[selected_instance_id].bank < MAX_BANKS - 1) {  //if at end of any other bank
    configuration.dexed[selected_instance_id].bank++;
    configuration.dexed[selected_instance_id].voice = 0;
  }

  if (configuration.dexed[selected_instance_id].voice <= 30 && configuration.dexed[selected_instance_id].bank <= MAX_BANKS) {

    display.setCursor(3, 0);
    display.print(">SEARCHING");

    do {  //first find next fav in current bank

      if (RollOver == false) configuration.dexed[selected_instance_id].voice++;
      else RollOver = true;
      favsearcher++;

    } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                            selected_instance_id)
               == false
             && configuration.dexed[selected_instance_id].voice <= 32 && favsearcher < 36);

    // if found, we are done. else quick check in next banks

    if (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                       selected_instance_id)
          == false
        && configuration.dexed[selected_instance_id].bank < MAX_BANKS && favsearcher < 170) {
      configuration.dexed[selected_instance_id].voice = 0;

      do {  //seek in next bank

        configuration.dexed[selected_instance_id].bank++;
        if (configuration.dexed[selected_instance_id].bank > MAX_BANKS - 1 && favsearcher < 190) {
          configuration.dexed[selected_instance_id].bank = 0;
          configuration.dexed[selected_instance_id].voice = 0;
        }
        favsearcher++;
      } while (quick_check_favorites_in_bank(configuration.dexed[selected_instance_id].bank, selected_instance_id) == false && favsearcher < 132);

      if (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                         selected_instance_id)
            == false
          && configuration.dexed[selected_instance_id].voice <= 32 && favsearcher < 190) {
        do {  //last bank to search if a fav can be found

          configuration.dexed[selected_instance_id].voice++;
          favsearcher++;
        } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                                selected_instance_id)
                   == false
                 && favsearcher < 170);
      }
    }
  }
  favsearcher = 0;
}

void locate_next_non_favorite() {
  //find next non-fav in current bank
  display.setCursor(3, 0);
  display.print(">SEARCHING");
  do {
    configuration.dexed[selected_instance_id].voice++;
    if (configuration.dexed[selected_instance_id].voice > 31) {
      configuration.dexed[selected_instance_id].voice = 0;
      //configuration.dexed[selected_instance_id].bank++;
      if (configuration.dexed[selected_instance_id].bank > MAX_BANKS - 1)
        configuration.dexed[selected_instance_id].bank = 0;
      do {  //seek for next bank
        configuration.dexed[selected_instance_id].bank++;
        if (configuration.dexed[selected_instance_id].bank > MAX_BANKS - 1)
          configuration.dexed[selected_instance_id].bank = 0;
        favsearcher++;
      } while (quick_check_favorites_in_bank(configuration.dexed[selected_instance_id].bank, selected_instance_id) == true && favsearcher < 132);
    }
    favsearcher++;
  } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                          selected_instance_id)
             == true
           && favsearcher < 170);
  favsearcher = 0;
}

void locate_random_non_favorite() {
  //find random non-fav
  do {
    configuration.dexed[selected_instance_id].voice = random(32);
    configuration.dexed[selected_instance_id].bank = random(MAX_BANKS - 1);
    favsearcher++;
  } while (check_favorite(configuration.dexed[selected_instance_id].bank, configuration.dexed[selected_instance_id].voice,
                          selected_instance_id)
             == true
           && favsearcher < 100);
  favsearcher = 0;
}

bool check_favorite(uint8_t b, uint8_t v, uint8_t instance_id) {
  b = constrain(b, 0, MAX_BANKS - 1);
  v = constrain(v, 0, MAX_VOICES - 1);
  char tmp[18];
  File myFav;
  if (sd_card > 0) {
    snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/%d.fav"), FAV_CONFIG_PATH, b, v);
#ifdef DEBUG
    Serial.print(F("check if Voice is a Favorite: "));
    Serial.print(tmp);
    Serial.println();
#endif
    if (SD.exists(tmp)) {  //is Favorite
#ifdef DEBUG
      Serial.println(F(" - It is in Favorites."));
#endif
      return true;
    } else {  // it was not a favorite

#ifdef DEBUG
      Serial.println(F(" - It is not in Favorites."));
#endif
      return false;
    }
  } else
    return false;
}

void draw_favorite_icon(uint8_t b, uint8_t v, uint8_t instance_id) {
  b = constrain(b, 0, MAX_BANKS - 1);
  v = constrain(v, 0, MAX_VOICES - 1);
  char tmp[18];
  File myFav;
  if (sd_card > 0) {
    snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/%d.fav"), FAV_CONFIG_PATH, b, v);
    if (SD.exists(tmp)) {  //is Favorite
      display.setCursor(13, 0);
      display.write(2);  //fav symbol
    } else {             // it was not a favorite
      display.setCursor(13, 0);
      display.print(" ");
    }
  }
}

bool quick_check_favorites_in_bank(uint8_t b, uint8_t instance_id) {
  b = constrain(b, 0, MAX_BANKS - 1);
  char tmp[18];

  if (sd_card > 0) {
    snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d"), FAV_CONFIG_PATH, b);
#ifdef DEBUG
    Serial.print(F("check if there is a Favorite in Bank: "));
    Serial.print(tmp);
    Serial.println();
#endif
    if (SD.exists(tmp)) {  // this bank HAS at least 1 favorite(s)
#ifdef DEBUG
      Serial.println(F("quickcheck found a FAV in bank!"));
#endif
      return (true);
    } else {  // no favorites in bank stored
      return (false);
#ifdef DEBUG
      Serial.println(F(" - It is no Favorite in current Bank."));
#endif
    }
  } else
    return false;
}

void save_favorite(uint8_t b, uint8_t v, uint8_t instance_id) {
#ifdef DEBUG
  Serial.println(F("Starting saving Favorite."));
#endif
  b = constrain(b, 0, MAX_BANKS - 1);
  v = constrain(v, 0, MAX_VOICES - 1);
  char tmp[18];
  char tmpfolder[18];
  File myFav;
  uint8_t i = 0, countfavs = 0;
  if (sd_card > 0) {
    snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/%d.fav"), FAV_CONFIG_PATH, b, v);
    snprintf_P(tmpfolder, sizeof(tmpfolder), PSTR("/%s/%d"), FAV_CONFIG_PATH, b);
#ifdef DEBUG
    Serial.println(F("Save Favorite to SD card..."));
    Serial.println(tmp);
#endif
    if (!SD.exists(tmp)) {  //create Favorite Semaphore
      if (!SD.exists(tmpfolder)) {
        SD.mkdir(tmpfolder);
      }
      myFav = SD.open(tmp, FILE_WRITE);
      myFav.close();
      Serial.println(F("Favorite saved..."));
      display.setCursor(13, 0);
      display.write(2);  //fav symbol
#ifdef DEBUG
      Serial.println(F("Added to Favorites..."));
#endif
    } else {  // delete the file, is no longer a favorite
      SD.remove(tmp);
#ifdef DEBUG
      Serial.println(F("Removed from Favorites..."));
#endif
      for (i = 0; i < 32; i++) {  //if no other favs exist in current bank, remove folder
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d/%d.fav"), FAV_CONFIG_PATH, b, i);
        if (SD.exists(tmp)) countfavs++;
      }
      if (countfavs == 0) {
        snprintf_P(tmp, sizeof(tmp), PSTR("/%s/%d"), FAV_CONFIG_PATH, b);
        SD.rmdir(tmp);
#ifdef DEBUG
        Serial.println(F("Fav count in bank:"));
        Serial.print(countfavs);
        Serial.println(F("Removed folder since no voice in bank flagged as favorite any more"));
#endif
      }
      display.setCursor(13, 0);
      display.print(" ");  //remove fav symbol
#ifdef DEBUG
      Serial.println(F("Removed from Favorites..."));
#endif
    }
  }
}

char* basename(const char* filename) {
  char* p = strrchr(filename, '/');
  return p ? p + 1 : (char*)filename;
}
