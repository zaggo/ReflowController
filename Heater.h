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

#ifndef HEATER_H
#define HEATER_H

#include "Thermistor.h"
#include "PID.h"

class Heater
{
  private:
    int current_temperature;
    PID pid;

  public:
    Heater();

    void set_target_temperature(int temp);
    int  get_target_temperature();
    bool hasReachedTargetTemperature();

    // Call once each temperature interval
    void manage_temperature();


    // Reset to board-on state
    void reset();
};

#endif // HEATER_H
