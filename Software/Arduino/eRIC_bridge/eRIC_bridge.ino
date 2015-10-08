/*
 * Part of Elektor project 150308 eRIC Nitro.
 *
 * Put S2 in position 'D'.
 * Software serial port on pins 2 & 3.
 * 3.3 V FTDI compatible USB-to-serial converter connected to K4
 * 
 * Sends all known queries - one per second - to eRIC module,
 * responses are forwarded to the PC.
 *
 * MCU communicates with eRIC module through software UART.
 * MCU communicates with PC through hardware UART.
 * Serial port settings: 19200n81
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

#include <SoftwareSerial.h>

SoftwareSerial soft_serial(2,3); // RX, TX

int led1 = 9;
int led2 = 13;
uint32_t t_prev;
int eric_cmd;

#define ERIC_COMMANDS  (13)
char *eric_commands[ERIC_COMMANDS] =
{
  "ER_CMD#U?",
  "ER_CMD#P?",
  "ER_CMD#C?",
  "ER_CMD#F?",
  "ER_CMD#f?",
  "ER_CMD#B?",
  "ER_CMD#L7?",
  "ER_CMD#L8?",
  "ER_CMD#L4xx?",
  "ER_CMD#A1?",
  "ER_CMD#A2?",
  "ER_CMD#A5?",
  "ER_CMD#Z0?",
};

void eric_send_command(int n)
{
  soft_serial.print(eric_commands[n]);
  delay(50);
  soft_serial.print("ACK");
}

void led_on(int led, boolean on)
{
  digitalWrite(led,on==true?HIGH:LOW);
}

void led_toggle(int led)
{
  if (digitalRead(led)!=0)
  {
    digitalWrite(led,LOW);
  }
  else
  {
    digitalWrite(led,HIGH);
  }
}

void setup(void)
{
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  led_on(led1,false);
  led_on(led2,false);
  
  Serial.begin(19200);
  Serial.println("eRIC Nitro bridge");

  // Software serial port on pins 2 & 3.
  // Put S2 in position 'D'.
  soft_serial.begin(19200);
  t_prev = 0;
  eric_cmd = 0;

  // Give everybody some time to initialise.
  delay(500);
}

void loop(void)
{
  if (millis()>=t_prev+1000)
  {
    t_prev = millis();
    Serial.println();
    if (eric_cmd<10) Serial.print(" ");
    Serial.print(eric_cmd);
    Serial.print(": ");
    eric_send_command(eric_cmd);
    eric_cmd += 1;
    if (eric_cmd>=ERIC_COMMANDS) eric_cmd = 0;
  }
  
  if (soft_serial.available()) 
  {
    Serial.write(soft_serial.read());
    led_toggle(led1);
  }
  if (Serial.available()) 
  {
    soft_serial.write(Serial.read());
    led_toggle(led2);
  }
}
