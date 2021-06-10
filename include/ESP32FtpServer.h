
/*
*  FTP SERVER FOR ESP8266
 * based on FTP Serveur for Arduino Due and Ethernet shield (W5100) or WIZ820io (W5200)
 * based on Jean-Michel Gallego's work
 * modified to work with esp8266 SPIFFS by David Paiva (david@nailbuster.com)
 * modified to work on ESP32 with SD card by Krishnakanth.s 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */
//  2017: modified by @robo8080
//  2019: modified by @fa1ke5
//  2021: modified by krishnakanth.s


#define FTP_DEBUG
#ifndef FTP_SERVERESP_H
#define FTP_SERVERESP_H

#include <SD.h>
#include <FS.h>
#include <WiFiClient.h>

#define FTP_SERVER_VERSION "FTP-2021-06-07"

#define FTP_CTRL_PORT 21         /* Command port on wich server is listening */
#define FTP_DATA_PORT_PASV 50009 /* Data port in passive mode */

#define FTP_TIME_OUT 5       
#define FTP_CMD_SIZE 255 + 8
#define FTP_CWD_SIZE 255 + 8 
#define FTP_FIL_SIZE 255     
#define FTP_BUF_SIZE 4096 

class FtpServer
{
public:
  void begin(String uname, String pword);
  void handleFTP();

private:
  bool haveParameter();
  bool makeExistsPath(char *path, char *param = NULL);
  void iniVariables();
  void clientConnected();
  void disconnectClient();
  boolean userIdentity();
  boolean userPassword();
  boolean processCommand();
  boolean dataConnect();
  boolean doRetrieve();
  boolean doStore();
  void closeTransfer();
  void abortTransfer();
  boolean makePath(char *fullname);
  boolean makePath(char *fullName, char *param);
  uint8_t getDateTime(uint16_t *pyear, uint8_t *pmonth, uint8_t *pday,
                      uint8_t *phour, uint8_t *pminute, uint8_t *second);
  char *makeDateTimeStr(char *tstr, uint16_t date, uint16_t time);
  int8_t readChar();

  IPAddress dataIp; 
  WiFiClient client;
  WiFiClient data;

  File file;

  boolean dataPassiveConn;
  uint16_t dataPort;
  char buf[FTP_BUF_SIZE];     
  char cmdLine[FTP_CMD_SIZE]; 
  char cwdName[FTP_CWD_SIZE]; 
  char command[5];           
  boolean rnfrCmd;            
  char *parameters;         
  uint16_t iCL;              
  int8_t cmdStatus,           
      transferStatus;         
  uint32_t millisTimeOut,     
      millisDelay,
      millisEndConnection, 
      millisBeginTrans,    
      bytesTransfered;     
  String _FTP_USER;
  String _FTP_PASS;
};

#endif // FTP_SERVERESP_H
