/*
 * 2011 Eberhard Rensch, Pleasant Software
 *
 * Based on parts of the Makerbot firmware
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "Heater.h"
#include "Thermistor.h"
#include "WProgram.h"


// Offset to compensate for range clipping and bleed-off
#define HEATER_OFFSET_ADJUSTMENT 0

extern int heaterPin;
extern Thermistor thermistor;

Heater::Heater()
{
  reset();
}

void Heater::reset() {
	current_temperature = 0;
}

void Heater::set_target_temperature(int temp)
{
	pid.setTarget(temp);
}

int Heater::get_target_temperature()
{
	return pid.getTarget();
}

// We now define target hysteresis in absolute degrees.  The original
// implementation (+/-5%) was giving us swings of 10% in either direction
// *before* any artifacts of process instability came in.
#define TARGET_HYSTERESIS 2

bool Heater::hasReachedTargetTemperature()
{
  return (current_temperature >= (pid.getTarget() - TARGET_HYSTERESIS)) &&
	 (current_temperature <= (pid.getTarget() + TARGET_HYSTERESIS));
}

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */

/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void Heater::manage_temperature()
{
  thermistor.update();
		
  // update the temperature reading.
  current_temperature = thermistor.getTemperature();

  int mv = pid.calculate(current_temperature);
  // offset value to compensate for heat bleed-off.
  // There are probably more elegant ways to do this,
  // but this works pretty well.
  mv += HEATER_OFFSET_ADJUSTMENT;
  // clamp value
  if (mv < 0) { mv = 0; }
  if (mv >255) { mv = 255; }
  digitalWrite(heaterPin,(mv>0));
}

