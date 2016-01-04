/*
 * Part of Elektor project 150308 eRIC Nitro.
 *
 * Put S2 in position 'X'.
 * Software serial port on pins 2 & 3.
 * 3.3 V FTDI compatible USB-to-serial converter connected to K1 (D2, D3, GND),
 * +5V connected to K4 pin 4.
 * 
 * Sends ping messages to eRIC Nitro server. Receives returned ping from server
 * and prints time for transmit-receive round trip and also the received signal
 * strength of received message.
 * Does not use any hardware flow control between MCU and eRIC module.
 *
 * MCU communicates with eRIC module through hardware UART.
 * MCU communicates with PC through software UART.
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
/* 
 * Put S2 in position 'X' (close to D1).
 * Software serial port on pins 2 & 3.
 * 
 * Sends all known queries to eRIC module, one per second,
 * and forwards response to PC.
 */

#include <SoftwareSerial.h>

#define TIMEOUT                1000 
  // Timeout value - 1000milliseconds
#define ERIC_CMD_FIRMWARE      "ER_CMD#T3"
  // eRIC OS command to read module firmware version
#define ERIC_CMD_RSSI          "ER_CMD#T8"
  // eRIC OS command to read last message RSSI
#define ACK                    1
#define NO_ACK                 0
#define MAX_RF_PAYLOAD_SIZE    32 
  // We only want to use a small RF payload.
  // eRIC module can handle much larger RF payloads than this
#define ERROR_OK               1
#define ERROR_TIMEOUT          2
#define ERROR_CMD_NOT_RETURNED 3
#define ERROR_DATA_FORMAT      4

// Declare local RF data receive and transmit buffers
char abTxData[MAX_RF_PAYLOAD_SIZE];
char abRxData[MAX_RF_PAYLOAD_SIZE];

int iMessageNumber = 0;

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
  Serial.println(F("eRIC Nitro ping client"));
  
  // Software serial port on pins 2 & 3.
  // Put S2 in position 'D'.
  soft_serial.begin(19200);
  
  // Send command to eRIC module to read module firmware
  // version
  iStatus = eRIC_SendCommand(ERIC_CMD_FIRMWARE, 
                             ACK, 
                             abRxData, 
                             sizeof(abRxData));
  
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
  unsigned long ulSendStartTime;
  unsigned long ulPingTime;  
  
  // Clear local receive buffer
  memset(abRxData, '\0', sizeof(abRxData));
  
  // Increment message number and reset to 0, if
  // it rolls over to a negative value
  iMessageNumber++;
  if(iMessageNumber < 0)
  {
    iMessageNumber = 0;  
  }
  
  // Create ping message in local buffer abTxData[]
  sprintf(abTxData, "MSG #%d", iMessageNumber);
  
  // Send ping message to eRIC module for transmission 
  // over RF interface
  soft_serial.println(abTxData);
  
  // Start the ping timout timer
  ulSendStartTime = millis();
  
  Serial.print(F("Ping data to server: "));
  Serial.println(abTxData);
  
  // Loop waiting for server to return message.
  // Loop will exit if message received or there is a timeout
  while(!eRIC_GetData(abRxData, sizeof(abRxData)))
  {
    if (millis() - ulSendStartTime > TIMEOUT) 
    {
      // No response from eRIC so return with diagnostic message
      Serial.println(F("Timeout on response from server!"));
      return;
    }
  }
  
  // Calculate round trip "transmit-receive" ping time
  ulPingTime = millis() - ulSendStartTime;

  // Print data received over RF interface and
  // ping time
  Serial.print(F("Data from server   : "));
  Serial.print(abRxData);
  Serial.print(F("Ping time: "));
  Serial.print(ulPingTime);
  Serial.println(F(" ms"));
  
  // Send RSSI command to from eRIC module OS
  // Was the command processed correctly by the eRIC module?
  if (eRIC_GetRSSI(abRxData, sizeof(abRxData)) == ERROR_OK)
  {
    // Command was  processed correctly. 
    // Print RSSI value
    Serial.print(F("RSSI     : "));
    Serial.println(abRxData);
  }
  else
  {
    // Command was  not processed correctly. 
    // Print error message
    Serial.println(F("No RSSI response from eRIC"));
  }
  
  Serial.println();
  delay(3000);
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

int eRIC_GetRSSI(char *abBuffer, int iLength)
{
  int iStatus;
  int iStringLength;
  char abLocalBuffer[MAX_RF_PAYLOAD_SIZE];
  
  // Is the length of abBuffer less than 5 characters?
  if (iLength < 7)
  {
    // Yes, return error code
    return ERROR_DATA_FORMAT;
  }
  
  // Send eRIC command to read module serial number
  iStatus = eRIC_SendCommand(ERIC_CMD_RSSI, ACK, abLocalBuffer, sizeof(abLocalBuffer)); 
 
    // Did the command get processed without error?
  if (iStatus == ERROR_OK)
  {
    // Get length of RSSI string (not including terminating '\0')
    iStringLength = strlen(abLocalBuffer);
      
    // iStringLength does not include length with terminating '\0'
    // Adjust iStringLength so that it does not overflow array abBuffer[]
    if ((iStringLength + 1) > iLength)
    {
      iStringLength = iLength - 1;
    }
    
    // Copy local string to array abBuffer[]
    memcpy(abBuffer, abLocalBuffer, iStringLength);
    
    // Add terminating null character
    abBuffer[iStringLength] = '\0';
  }
    
  return iStatus;
}

