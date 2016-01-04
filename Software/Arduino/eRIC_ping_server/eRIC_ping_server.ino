/*
 * Part of Elektor project 150308 eRIC Nitro.
 *
 * Put S2 in position 'D'.
 * Software serial port on pins 2 & 3.
 * 3.3 V FTDI compatible USB-to-serial converter connected to K4
 * 
 * Receives ping messages from eRIC Nitro client and replies to client with
 * received message.
 * Does not use any hardware flow control between MCU and eRIC module.
 *
 * MCU communicates with eRIC module through software UART.
 * MCU communicates with PC through hardware UART.
 * Serial port settings: 19200n81
 *
 * AR, 03/01/2016
 *
 * Copyright (c) 2016 Elektor (www.elektor.com, www.elektor-labs.com).
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

#define TIMEOUT                1000 
  // Timeout value - 1000milliseconds
#define ERIC_CMD_FIRMWARE      "ER_CMD#T3"
  // eRIC OS command to read module firmware version
#define ACK                    1
#define NO_ACK                 0
#define MAX_RF_PAYLOAD_SIZE    32 
  // We only want to use a small RF payload.
  // eRIC module can handle much larger RF payloads than this
#define ERROR_OK               1
#define ERROR_TIMEOUT          2

// Declare local RF data receive and transmit buffers
char abTxData[MAX_RF_PAYLOAD_SIZE];
char abRxData[MAX_RF_PAYLOAD_SIZE];

// Declare SoftwareSerial object. Will be used by MCU to 
// communicate with eRIC module
SoftwareSerial soft_serial(2,3); // RX, TX

void setup(void)
{
  int iStatus;
 
  // Give everybody some time to initialise.
  delay(500);
  
  // Initialise hardware usart with 19200 bps baud rate
  Serial.begin(19200);
  Serial.println();
  Serial.println("eRIC Nitro ping server");
  
  // Software serial port on pins 2 & 3.
  // Put S2 in position 'D'.
  soft_serial.begin(19200);
  
  // Send command to eRIC modudel to read module firmware
  // version
  iStatus = eRIC_SendCommand(ERIC_CMD_FIRMWARE, ACK, abRxData, sizeof(abRxData));
  
  // Was the command processed correctly by the eRIC module?
  if (iStatus == ERROR_OK)
  {
    // Command processed correctly. Print firmware version
    Serial.print(F("eRIC firmware version  : "));
    Serial.println(abRxData);
  }
  else
  {
    // Command was not processed correctly. 
    // Print error message and hang the MCU
    Serial.println(F("Can't communicate with eRIC module. Program halted."));
    Serial.println(F("Check wiring and eRIC baud rate."));
    while (1)
    {
      ;
    }
  }
}

void loop(void)
{
  Serial.println(F("Waiting for ping..."));
  
  // Clear local receive buffer
  memset(abRxData, '\0', sizeof(abRxData));
  
  // Wait for eRIC module to receive  data over RF interface
  while(eRIC_GetData(abRxData, sizeof(abRxData)) == 0)
  {
    ;
  }
  
  // eRIC has received data over RF interface.
  Serial.println(F("Got ping"));
  Serial.print(F("Sending reply... "));
  
  // transmit received data back to sender over RF interface
  soft_serial.print(abRxData);
  Serial.print(abRxData);
  Serial.println();
}


// Get received RF data from eRIC module
int eRIC_GetData(char *abRxData, int iRxLength)
{
  int iCount;
    
  if(iRxLength <= 0) 
  {
    // Return length of 0
    return 0;
  }
  
  if(!soft_serial.available()) 
  {
    // Return length of 0
    return 0;
  }
  
  // If eRIC has received data over RF interface, read it 
  iCount = 0;
  while(soft_serial.available() && (iCount < iRxLength)) 
  {
    // Save RF data byte received by eRIC module into abRxData[] 
    abRxData[iCount++] = soft_serial.read();
  }
  
  // Return number of bytes received
  return iCount;
}

// Send eRIC OS command to eRIC module
// Returns : ERROR_OK - command processed correctly by eRIC OS
//           ERROR_TIMEOUT - eRIC OS did not reply withing timeout period
int eRIC_SendCommand(char *abCommand, int iACKrequired, char *abRxData, int iRxLength)
{
  int iPosition = 0;
  unsigned long ulSendStartTime;
  char abBuffer[MAX_RF_PAYLOAD_SIZE];
 
  // Initialise local variables
  memset(abBuffer, 0, sizeof(abBuffer));  
    
  // Send command to eRIC
  soft_serial.print(abCommand);
  
  ulSendStartTime = millis();
  while(!soft_serial.available())
  {   
    if (millis() - ulSendStartTime > TIMEOUT) 
    {
      // No response from eRIC so return with diagnostic message
      // Serial.println("Cannot communicate with eRIC module.");
      return ERROR_TIMEOUT;
    }
  }
  
  // Read response from eRIC module 
  iPosition = 0;
  while(soft_serial.available()) 
  {
    if (iPosition < (sizeof(abBuffer)- 1))
    {
      abBuffer[iPosition++] = soft_serial.read();
      abBuffer[iPosition] = 0;
    }
  }
  
  // Does command need an "ACK" string to be sent to eRIC 
  // to complete command processing
  if (iACKrequired == ACK)
  {
    // ACK, required. Send ACK string to eRIC module
    soft_serial.print("ACK");
   
    // Start the timeout timer
    ulSendStartTime = millis();
    while(!soft_serial.available())
    {   
      if (millis() - ulSendStartTime > TIMEOUT) 
      {
        // No response from eRIC so return with diagnostic message
        // Serial.println("Cannot communicate with eRIC module.");
        return ERROR_TIMEOUT;
      }
    }
  }
  
  // Read response from eRIC module 
  iPosition = 0;
  while(soft_serial.available()) 
  {
    if (iPosition < (sizeof(abBuffer)- 1))
    {
      abBuffer[iPosition++] = soft_serial.read();
      abBuffer[iPosition] = 0;
    }
  }
  
  // Copy received response to abRxData[]
  memcpy(abRxData, abBuffer, iRxLength);
  return ERROR_OK;
}





