/*
   MicroDexed

   MicroDexed is a port of the Dexed sound engine
   (https://github.com/asb2m10/dexed) for the Teensy-3.5/3.6/4.x/4.x with audio shield.
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

#define STRING_BUFFER_SIZE LCD_cols + 1

template<class T>
class Disp_Plus : public T {
public:

  using T::T;

  void show(uint8_t y, uint8_t x, uint8_t fs, const char *str) {
    _show(y, x, fs, str, false, false);
  }

  void show(uint8_t y, uint8_t x, uint8_t fs, long num) {
    char _buf10[STRING_BUFFER_SIZE];

    _show(y, x, fs, itoa(num, _buf10, 10), true, true);
  }

private:
  void _show(uint8_t pos_y, uint8_t pos_x, uint8_t field_size, const char *str, bool justify_right, bool fill_zero) {
    {
      char tmp[STRING_BUFFER_SIZE];
      char *s = tmp;
      uint8_t l = strlen(str);

      memset(tmp, 0, sizeof(tmp));
      if (fill_zero == true)
        memset(tmp, '0', field_size);
      else
        memset(tmp, 0x20, field_size);  // blank

      if (l > field_size)
        l = field_size;

      if (justify_right == true)
        s += field_size - l;

      strncpy(s, str, l);
      //s[l] = '\0';

      //setCursor(pos_x * getMaxCharWidth(), pos_y * getMaxCharHeight());
      this->setCursor(pos_x, pos_y);
      this->print(tmp);

#ifdef DEBUG
      Serial.print(pos_y, DEC);
      Serial.print(F("/"));
      Serial.print(pos_x, DEC);
      Serial.print(F(": ["));
      Serial.print(tmp);
      Serial.println(F("]"));
#endif
    }
  }
};
