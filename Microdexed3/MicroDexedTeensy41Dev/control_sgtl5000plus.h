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

#include <Arduino.h>
#include <Audio.h>

#define EQ_TYPE_0 FILTER_HIPASS
#define EQ_CENTER_FRQ_0 15.0
#define EQ_BW_0 2

#define EQ_TYPE_1 FILTER_LOSHELF
#define EQ_CENTER_FRQ_1 120.0
#define EQ_BW_1 2

#define EQ_TYPE_2 FILTER_PARAEQ
#define EQ_CENTER_FRQ_2 220.0
#define EQ_BW_2 2

#define EQ_TYPE_3 FILTER_PARAEQ
#define EQ_CENTER_FRQ_3 1000.0
#define EQ_BW_3 2

#define EQ_TYPE_4 FILTER_PARAEQ
#define EQ_CENTER_FRQ_4 2000.0
#define EQ_BW_4 2

#define EQ_TYPE_5 FILTER_HISHELF
#define EQ_CENTER_FRQ_5 7000.0
#define EQ_BW_5 2

#define EQ_TYPE_6 FILTER_LOPASS
#define EQ_CENTER_FRQ_6 18000.0
#define EQ_BW_6 2

class AudioControlSGTL5000Plus : public AudioControlSGTL5000
{
  public:
    AudioControlSGTL5000Plus(uint8_t n = 7) {
      _enabled = false;
      num_bands = constrain(n, 1, 7);
    };
    void setEQType(uint8_t band, uint8_t ft);
    void setEQFc(uint8_t band, float frq);
    void setEQQ(uint8_t band, float q);
    void setEQBandwidth(uint8_t band, float bw);
    void setEQGain(uint8_t band, float gain);
    void commitFilter(uint8_t band);
    void show_params(uint8_t band);
    void init_parametric_eq(uint8_t n = 7);

  private:
    bool _enabled;
    uint8_t num_bands;
    uint8_t* filter_type;
    float* Fc;
    float* Q;
    float* peakGainDB;
};
