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

#include "Thermistor.h"
#include "WProgram.h"

// Default thermistor table.  If no thermistor table is loaded into eeprom,
// this will be copied in by the initTable() method.
//
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4066 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4066
// max adc: 1023
typedef int16_t TempTable[THERM_TABLE_SIZE][2];
TempTable default_table = {
  {1, 841},
  {54, 255},
  {107, 209},
  {160, 184},
  {213, 166},
  {266, 153},
  {319, 142},
  {372, 132},
  {425, 124},
  {478, 116},
  {531, 108},
  {584, 101},
  {637, 93},
  {690, 86},
  {743, 78},
  {796, 70},
  {849, 61},
  {902, 50},
  {955, 34},
  {1008, 3}
};

int16_t thermistorToCelsius(int16_t reading, int8_t table_idx) {
  int16_t celsius = 0;
  int8_t i;
  for (i=1; i<THERM_TABLE_SIZE; i++)
  {
	  if (default_table[i][0] > reading)
	  {
		  celsius  = default_table[i-1][1] +
				  (reading - default_table[i-1][0]) *
				  (default_table[i][1] - default_table[i-1][1]) /
				  (default_table[i][0] - default_table[i-1][0]);
		  if (celsius > 255)
			  celsius = 255;
		  break;
	  }
  }
  // Overflow: We just clamp to 255 degrees celsius to ensure
  // that the heater gets shut down if something goes wrong.
  if (i == THERM_TABLE_SIZE) {
    celsius = 255;
  }
  return celsius;
}

Thermistor::Thermistor(uint8_t analog_pin_in, uint8_t table_index_in) :
analog_pin(analog_pin_in), next_sample(0), table_index(table_index_in) {
	for (int i = 0; i < SAMPLE_COUNT; i++) { sample_buffer[i] = 0; }
}

bool Thermistor::update() {
	int16_t temp;
	temp = analogRead(analog_pin);

	sample_buffer[next_sample] = temp;
	next_sample = (next_sample+1) % SAMPLE_COUNT;

	// average
	int16_t cumulative = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		cumulative += sample_buffer[i];
	}
	int16_t avg = cumulative / SAMPLE_COUNT;

	//current_temp = thermistorToCelsius(avg,table_index);
	current_temp = thermistorToCelsius(temp,table_index);
	return true;
}
