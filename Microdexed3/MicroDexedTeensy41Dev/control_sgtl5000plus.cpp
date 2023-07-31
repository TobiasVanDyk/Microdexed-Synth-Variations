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

#include <Arduino.h>
#include <Audio.h>
#include "control_sgtl5000plus.h"

void AudioControlSGTL5000Plus::init_parametric_eq(uint8_t n)
{
  num_bands = constrain(n, 1, 7);

  eqSelect(PARAMETRIC_EQUALIZER);
  eqFilterCount(num_bands);

  filter_type = new uint8_t[num_bands];
  Fc = new float[num_bands];
  Q = new float[num_bands];
  peakGainDB = new float[num_bands];

  _enabled = true;

  setEQType(1, EQ_TYPE_0);
  setEQFc(1, EQ_CENTER_FRQ_0);
  setEQBandwidth(1, EQ_BW_0);
  setEQGain(1, 0.0);
  commitFilter(1);

  if (num_bands > 1)
  {
    setEQType(2, EQ_TYPE_1);
    setEQFc(2, EQ_CENTER_FRQ_1);
    setEQBandwidth(2, EQ_BW_1);
    setEQGain(2, 0.0);
    commitFilter(2);
  }

  if (num_bands > 2)
  {
    setEQType(3, EQ_TYPE_2);
    setEQFc(3, EQ_CENTER_FRQ_2);
    setEQBandwidth(3, EQ_BW_2);
    setEQGain(3, 0.0);
    commitFilter(3);
  }

  if (num_bands > 3)
  {
    setEQType(4, EQ_TYPE_3);
    setEQFc(4, EQ_CENTER_FRQ_3);
    setEQBandwidth(4, EQ_BW_3);
    setEQGain(4, 0.0);
    commitFilter(4);
  }

  if (num_bands > 4)
  {
    setEQType(5, EQ_TYPE_4);
    setEQFc(5, EQ_CENTER_FRQ_4);
    setEQBandwidth(5, EQ_BW_4);
    setEQGain(5, 0.0);
    commitFilter(5);
  }

  if (num_bands > 5)
  {
    setEQType(6, EQ_TYPE_5);
    setEQFc(6, EQ_CENTER_FRQ_5);
    setEQBandwidth(6, EQ_BW_5);
    setEQGain(6, 0.0);
    commitFilter(6);
  }

  if (num_bands > 6)
  {
    setEQType(7, EQ_TYPE_6);
    setEQFc(7, EQ_CENTER_FRQ_6);
    setEQBandwidth(7, EQ_BW_6);
    setEQGain(7, 0.0);
    commitFilter(7);
  }
}

void AudioControlSGTL5000Plus::setEQType(uint8_t band, uint8_t ft)
{
  if (filter_type && _enabled != false)
  {
    band = constrain(band, 1, num_bands);
    filter_type[band - 1] = ft;
  }
}

void AudioControlSGTL5000Plus::setEQFc(uint8_t band, float frq)
{
  if (Fc && _enabled != false)
  {
    band = constrain(band, 1, num_bands);
    Fc[band - 1] = frq;
  }
}

void AudioControlSGTL5000Plus::setEQQ(uint8_t band, float q)
{
  if (Q && _enabled != false)
  {
    band = constrain(band, 1, num_bands);
    Q[band - 1] = q;
  }
}

// Calculate Q: http://www.sengpielaudio.com/calculator-bandwidth.htm
//              http://jdm12.ch/Audio/EQ_BPF-Q-bandwidth.asp
void AudioControlSGTL5000Plus::setEQBandwidth(uint8_t band, float bw)
{
  if (Q && Fc && _enabled != false)
  {
    band = constrain(band, 1, num_bands);
    Q[band - 1] = sqrt(pow(2, bw)) / (pow(2, bw) - 1);
  }
}

void AudioControlSGTL5000Plus::setEQGain(uint8_t band, float gain)
{
  if (peakGainDB && _enabled != false)
  {
    band = constrain(band, 1, num_bands);
    peakGainDB[band - 1] = gain;
  }
}

void AudioControlSGTL5000Plus::commitFilter(uint8_t band)
{
  int filter[5] = {0, 0, 0, 0, 0};

  if (_enabled == false)
    return;

  band = constrain(band, 1, num_bands);

  calcBiquad(filter_type[band - 1], Fc[band - 1], peakGainDB[band - 1],  Q[band - 1],  0x80000,  AUDIO_SAMPLE_RATE, filter);
  eqFilter(band, filter);
}

void AudioControlSGTL5000Plus::show_params(uint8_t band)
{
  if (_enabled == false)
  {
    Serial.print(F("Parametric-EQ not initialized!"));
  }
  else
  {
    band = constrain(band, 1, num_bands);

    Serial.print(F("[SGTL5000 EQ] Band: "));
    Serial.print(band, DEC);
    Serial.print(F(" Type:"));
    Serial.print(filter_type[band - 1], DEC);
    Serial.print(F(" Fc:"));
    Serial.print(Fc[band - 1], 1);
    Serial.print(F(" Q:"));
    Serial.print(Q[band - 1], 1);
    Serial.print(F(" peakGainDB:"));
    Serial.print(peakGainDB[band - 1], 1);
    Serial.println();
  }
}
