/*
 * Part of Elektor project 150308 eRIC Nitro.
 *
 * Blinky sketch to see if the on-board LED (pin 9) is working.
 * Also blinks an LED on pin 13.
 *
 * CPV, 8/10/2015
 *
 * Copyright (c) 2015 Elektor (www.elektor.com, www.elektor-labs.com).
 * All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

int led = 9;
int led2 = 13;

void setup(void)
{
  pinMode(led,OUTPUT);
  pinMode(led2,OUTPUT);
}

void loop(void)
{
  digitalWrite(led,HIGH);
  digitalWrite(led2,LOW);
  delay(250);
  digitalWrite(led,LOW);
  digitalWrite(led2,HIGH);
  delay(250);
}
