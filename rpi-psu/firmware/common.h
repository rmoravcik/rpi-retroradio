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

#ifndef COMMON_H
#define COMMON_H

#include <avr/io.h>

#define TRUE				1
#define FALSE				0

#define RPI_RUNNING_IND			PB0
#define RPI_POWER_OFF_REQ		PB1
#define RELAY				PB2
#define PA_CONTROL			PB3
#define POWER_SWITCH			PB4

#endif // COMMON_H
