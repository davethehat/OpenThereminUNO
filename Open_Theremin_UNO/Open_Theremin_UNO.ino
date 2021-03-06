/*
 *  Open.Theremin.UNO control software for Arduino.UNO
 *  Version 2.0
 *  Copyright (C) 2010-2013 by Urs Gaudenz
 *
 *  Open.Theremin.UNO control software is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Open.Theremin.UNO control software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  the Open.Theremin.UNO control software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Version 1.2A - minor 'arduino-izing' - Michael Margolis 20 Oct 2013 
 */
 
/**
Building the code
=================
build.h contains #defines that control the compilation of the code

EXTENDED - if non-0, build for the extended theremin (with potentiometers 
           controlling volume and wavetable)

DEFAULT_WAVETABLE - if EXTENDED is zero, use this to choose which of the
           eight provided wavetables will be used

ENABLE_SERIAL - if non-0, the build will include code to write the detected
           pitch to the serial connection every 100 milliseconds. Set serial
           receive baud to 115200

ENABLE_CV - if non-0, emit cv output on pin 6 (EXPERIMENTAL!)
           
Structure of the code
=====================
** Open_Theremin_UNI.ino **
This file. Creates and hooks up the application object to the arduino setup()
and loop() callbacks.

** application.h/application.cpp **
Main application object. Holds the state of the app (playing, calibrating), deals
with initialisation and the app main loop, reads pitch and volume changed flags
from the interrupt handlers and sets pitch and volume values which the timer
interrupt sends to the DAC.

** OTPinDefs.h **
Pin definitions for the DAC.

** build.h **
Preprocessor definitions for build (see above).

** hw.h **
Definitions for hardware button and LED.

** ihandlers.h/ihandlers.cpp
Interrupt handler code and volatile variables implementing the communication between
the app and its input/output.

** theremin_sinetable<N>.c **
Wavetable data for a variety of sounds. Switchable via the potentiometer in the 
extended build, a sound for a standard build can be selected by setting DEFAULT_WAVETABLE 
in build.h to the desired index.

** timer.h/timer.cpp **
Definitions and functions for setting delays in tics and in milliseconds

*/

#include "application.h"

Application app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}

