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

#include "application.h"

Application app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}

