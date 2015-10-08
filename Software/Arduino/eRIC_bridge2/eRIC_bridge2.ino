/*
 * Part of Elektor project 150308 eRIC Nitro.
 *
 * Put S2 in position 'X'.
 * Software serial port on pins 2 & 3.
 * 3.3 V FTDI compatible USB-to-serial converter connected to K1 (D2, D3, GND),
 * +5V connected to K4 pin 4.
 * 
 * Sends all known queries - one per second - to eRIC module,
 * responses are forwarded to the PC.
 *
 * MCU communicates with eRIC module through hardware UART.
 * MCU communicates with PC through software UART.
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
/* 
 * Put S2 in position 'X' (close to D1).
 * Software serial port on pins 2 & 3.
 * 
 * Sends all known queries to eRIC module, one per second,
 * and forwards response to PC.
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
  Serial.print(eric_commands[n]);
  delay(50);
  Serial.print("ACK");
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
  
  soft_serial.begin(19200);
  soft_serial.println("eRIC Nitro bridge");

  // Software serial port on pins 2 & 3.
  // Put S2 in position 'X'.
  Serial.begin(19200);
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
    soft_serial.println();
    if (eric_cmd<10) soft_serial.print(" ");
    soft_serial.print(eric_cmd);
    soft_serial.print(": ");
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

