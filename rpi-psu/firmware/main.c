/*
 *  rpi-psu-1.0
 *  Copyright (C) 2015 Roman Moravcik
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/delay.h>

#include "common.h"

enum {
	STATE_RPI_OFF = 0,
	STATE_RPI_RUNNING,
	STATE_POWER_OFF_REQUESTED,
	STATE_POWER_ON_REQUESTED,
};

volatile uint8_t state = STATE_RPI_OFF;

static void pa_mute(void)
{
	PORTB |= _BV(PA_CONTROL);
}

static void pa_unmute(void)
{
	PORTB &= ~(_BV(PA_CONTROL));
}

static void psu_on(void)
{
	PORTB |= _BV(RELAY);
}

static void psu_off(void)
{
	PORTB &= ~(_BV(RELAY));
}

static uint8_t is_rpi_running(void)
{
	if (PINB & _BV(RPI_RUNNING_IND)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static void do_power_on(void)
{
	// Enable PSU
	psu_on();

	// Un-mute PA
	pa_unmute();

	// Wait 3 sec to start-up RPi
	_delay_ms(3000);
}

static void do_power_off(void)
{
	// Indicate RPi a power off request
	PORTB |= _BV(RPI_POWER_OFF_REQ);

	// Wait till RPi is off
	while (is_rpi_running()) {
	}

	// Mute PA
	pa_mute();

	// Wait 500 msec here
	_delay_ms(500);

	// Disable PSU
	psu_off();

	// Clear a power off request to RPi
	PORTB &= ~(_BV(RPI_POWER_OFF_REQ));
}

static void state_rpi_off(void)
{
	// Nothing to do here
}

static void state_rpi_running(void)
{
	if (!is_rpi_running()) {
		// RPi was halted from Web/Linux
		state = STATE_POWER_OFF_REQUESTED;
	}
}

static void state_power_on_requested(void)
{
	do_power_on();

	// Move to running state
	state = STATE_RPI_RUNNING;
}

static void state_power_off_requested(void)
{
	do_power_off();

	// Move to power off state
	state = STATE_RPI_OFF;
}

static void init(void)
{
	OSCCAL = 0x93;

	DDRB = 0xFF;

	// Set pins as inputs
	DDRB &= ~(_BV(RPI_RUNNING_IND) | _BV(POWER_SWITCH));

	// Enable pull-up on Power switch
	PORTB |= _BV(POWER_SWITCH);

	// Enable interrupt for Power switch
	GIFR |= _BV(PCIF);
	GIMSK |= _BV(PCIE);
	PCMSK = _BV(POWER_SWITCH);

	state = STATE_RPI_OFF;

	sei();
}

ISR(PCINT0_vect, ISR_BLOCK)
{
	uint8_t counter = 0;

	if (!(PINB & _BV(POWER_SWITCH))) {
		while (!(PINB & _BV(POWER_SWITCH)) && counter < 10) {
			_delay_ms(20);
			counter++;
		}

		if (counter == 10) {
			if (state == STATE_RPI_OFF) {
				// RPi request to power on via power switch
					state = STATE_POWER_ON_REQUESTED;
			}
		}
	} else {
		while ((PINB & _BV(POWER_SWITCH)) && counter < 10) {
			_delay_ms(20);
			counter++;
		}

		if (counter == 10) {
			if (state == STATE_RPI_RUNNING) {
				// RPi request to power off via power switch
				state = STATE_POWER_OFF_REQUESTED;
			}
		}
	}

	GIFR |= _BV(PCIF);
}

int main(void)
{
	init();

	// Mute PA
	pa_mute();

	// Disable PSU
	psu_off();

	_delay_ms(2000);

	while(1) {
		switch (state) {
			case STATE_RPI_OFF:
				state_rpi_off();
				break;

			case STATE_RPI_RUNNING:
				state_rpi_running();
				break;

			case STATE_POWER_ON_REQUESTED:
				state_power_on_requested();
				break;

			case STATE_POWER_OFF_REQUESTED:
				state_power_off_requested();
				break;
		}
	}

	return 0;
}
