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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/

#pragma once

#include "config.h"
#include "synth_dexed.h"

extern uint8_t sd_card;
extern AudioSynthDexed* MicroDexed[NUM_DEXED];
extern void show_patch(uint8_t instance_id);

extern void send_sysex_voice(uint8_t midi_channel, uint8_t* data);

extern uint8_t bank;
extern uint8_t voice;
extern uint8_t ui_state;
extern uint8_t ui_main_state;
extern config_t configuration;
extern uint32_t crc32(byte* calc_start, uint16_t calc_bytes);
extern void set_fx_params(void);
extern void set_voiceconfig_params(uint8_t instance_id);
extern void set_sys_params(void);
extern void set_epiano_params(void);

bool load_sd_voice(uint8_t b, uint8_t v, uint8_t instance_id);
bool save_sd_voice(uint8_t b, uint8_t v, uint8_t instance_id);
bool get_sd_voice(File sysex, uint8_t voice_number, uint8_t* data);
bool put_sd_voice(File sysex, uint8_t voice_number, uint8_t* data);
bool save_sd_bank(const char* bank_filename, uint8_t* data);

bool load_sd_voiceconfig_json(uint8_t vc, uint8_t instance_id);
bool save_sd_voiceconfig_json(uint8_t vc, uint8_t instance_id);

bool load_sd_fx_json(uint8_t fx);
bool save_sd_fx_json(uint8_t fx);

bool load_sd_sys_json(void);
bool save_sd_sys_json(void);

bool load_sd_performance_json(uint8_t p);
bool save_sd_performance_json(uint8_t p);

bool load_sd_drumsettings_json(uint8_t number);
bool save_sd_drumsettings_json(uint8_t number);

bool check_performance_directory(uint8_t seq_number);
bool check_sd_performance_exists(uint8_t number);

bool get_sd_data(File sysex, uint8_t format, uint8_t* conf);
bool write_sd_data(File sysex, uint8_t format, uint8_t* data, uint16_t len);

uint8_t calc_checksum(uint8_t* data, uint16_t len);
void strip_extension(const char* s, char* target);

bool get_bank_name(uint8_t b, char* name, uint8_t len);
bool get_voice_name(uint8_t b, uint8_t v, char* name, uint8_t len);
bool get_voice_by_bank_name(uint8_t b, const char* bank_name, uint8_t v, char* voice_name, uint8_t len);

void string_toupper(char* s);
