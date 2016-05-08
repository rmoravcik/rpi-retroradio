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
	STATE_RPI_OFF_FROM_LINUX,
	STATE_RPI_RUNNING,
	STATE_POWER_OFF_REQUESTED,
	STATE_POWER_OFF_REQUESTED_FROM_LINUX,
	STATE_POWER_ON_REQUESTED,
};

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

static uint8_t is_power_on_req(void)
{
	uint8_t counter = 0;

	while (!(PINB & _BV(POWER_SWITCH)) && counter < 10) {
		_delay_ms(20);
		counter++;
	}

	if (counter == 10) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static uint8_t is_power_off_req(void)
{
	uint8_t counter = 0;

	while ((PINB & _BV(POWER_SWITCH)) && counter < 10) {
		_delay_ms(20);
		counter++;
	}

	if (counter == 10) {
		return TRUE;
	} else {
		return FALSE;
	}
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

static uint8_t state_rpi_off(uint8_t state)
{
	uint8_t next_state = state;

	if (!is_rpi_running()) {
		if (is_power_on_req()) {
			// RPi request to power on via power switch
			next_state = STATE_POWER_ON_REQUESTED;
		}
	}

	return next_state;
}

static uint8_t state_rpi_off_from_linux(uint8_t state)
{
	uint8_t next_state = state;

	if (is_power_off_req()) {
		// RPi already powered off from linux
		next_state = STATE_RPI_OFF;
	}

	return next_state;
}

static uint8_t state_rpi_running(uint8_t state)
{
	uint8_t next_state = state;

	if (is_rpi_running()) {
		if (is_power_off_req()) {
			// RPi request to power off via power switch
			next_state = STATE_POWER_OFF_REQUESTED;
		}
	} else {
		// RPi was halted from Web/Linux
		next_state = STATE_POWER_OFF_REQUESTED_FROM_LINUX;
	}

	return next_state;
}

static uint8_t state_power_off_requested(uint8_t state)
{
	uint8_t next_state = state;

	do_power_off();

	// Move to power off state
	next_state = STATE_RPI_OFF;

	return next_state;
}

static uint8_t state_power_off_requested_from_linux(uint8_t state)
{
	uint8_t next_state = state;

	do_power_off();

	// Move to power off from linux state
	next_state = STATE_RPI_OFF_FROM_LINUX;

	return next_state;
}

static uint8_t state_power_on_requested(uint8_t state)
{
	uint8_t next_state = state;

	do_power_on();

	// Move to running state
	next_state = STATE_RPI_RUNNING;

	return next_state;
}

static void init(void)
{
	OSCCAL = 0x93;

	DDRB = 0xFF;

	// Set pins as inputs
	DDRB &= ~(_BV(RPI_RUNNING_IND) | _BV(POWER_SWITCH));

	// Enable pull-up on Power switch
	PORTB |= _BV(POWER_SWITCH);

	sei();
}

int main(void)
{
	uint8_t state = STATE_RPI_OFF;

	init();

	// Mute PA
	pa_mute();

	// Disable PSU
	psu_off();

	_delay_ms(2000);

	while(1) {
		switch (state) {
			case STATE_RPI_OFF:
				state = state_rpi_off(state);
				break;

			case STATE_RPI_OFF_FROM_LINUX:
				state = state_rpi_off_from_linux(state);
				break;

			case STATE_RPI_RUNNING:
				state = state_rpi_running(state);
				break;

			case STATE_POWER_OFF_REQUESTED:
				state = state_power_off_requested(state);
				break;

			case STATE_POWER_OFF_REQUESTED_FROM_LINUX:
				state = state_power_off_requested_from_linux(state);
				break;

			case STATE_POWER_ON_REQUESTED:
				state = state_power_on_requested(state);
				break;
		}
	}

	return 0;
}
