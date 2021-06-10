/*
 * FTP Server for ESP8266
 * based on FTP Serveur for Arduino Due and Ethernet shield (W5100) or WIZ820io (W5200)
 * based on Jean-Michel Gallego's work
 * modified to work with esp8266 SPIFFS by David Paiva david@nailbuster.com
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
// 2017: modified by @robo8080
// 2019: modified by @fa1ke5
// 2021: modified by krishnakanth.S

#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include "ESP32FtpServer.h"

WiFiServer ftpServer(FTP_CTRL_PORT);
WiFiServer dataServer(FTP_DATA_PORT_PASV);

void FtpServer::begin(String uname, String pword)
{
  _FTP_USER = uname;
  _FTP_PASS = pword;

  ftpServer.begin();
  delay(10);
  dataServer.begin();
  delay(10);
  millisTimeOut = (uint32_t)FTP_TIME_OUT * 60 * 1000;
  millisDelay = 0;
  cmdStatus = 0;
  iniVariables();
}

void FtpServer::iniVariables()
{
  dataPort = FTP_DATA_PORT_PASV;
  dataPassiveConn = true;
  strcpy(cwdName, "/");
  rnfrCmd = false;
  transferStatus = 0;
}

void FtpServer::handleFTP()
{
  if ((int32_t)(millisDelay - millis()) > 0)
    return;

  if (ftpServer.hasClient())
  {
    client.stop();
    client = ftpServer.available();
  }

  if (cmdStatus == 0)
  {
    if (client.connected())
      disconnectClient();
    cmdStatus = 1;
  }
  else if (cmdStatus == 1)
  {
    abortTransfer();
    iniVariables();

#ifdef FTP_DEBUG
    Serial.println("Ftp server waiting for connection on port " + String(FTP_CTRL_PORT));
#endif

    cmdStatus = 2;
  }
  else if (cmdStatus == 2)
  {

    if (client.connected())
    {
      clientConnected();
      millisEndConnection = millis() + 10 * 1000;
      cmdStatus = 3;
    }
  }
  else if (readChar() > 0)
  {
    if (cmdStatus == 3)
      if (userIdentity())
        cmdStatus = 4;
      else
        cmdStatus = 0;
    else if (cmdStatus == 4)
      if (userPassword())
      {
        cmdStatus = 5;
        millisEndConnection = millis() + millisTimeOut;
      }
      else
        cmdStatus = 0;
    else if (cmdStatus == 5)
    {
      if (!processCommand())
        cmdStatus = 0;
      else
        millisEndConnection = millis() + millisTimeOut;
    }
  }
  else if (!client.connected() || !client)
  {
    cmdStatus = 1;
#ifdef FTP_DEBUG
    Serial.println("client disconnected");
#endif
  }

  if (transferStatus == 1)
  {
    if (!doRetrieve())
      transferStatus = 0;
  }
  else if (transferStatus == 2)
  {
    if (!doStore())
      transferStatus = 0;
  }
  else if (cmdStatus > 2 && !((int32_t)(millisEndConnection - millis()) > 0))
  {
    client.println("530 Timeout");
    millisDelay = millis() + 200;
    cmdStatus = 0;
  }
}

void FtpServer::clientConnected()
{
#ifdef FTP_DEBUG
  Serial.println("Client connected!");
#endif
  client.println("220--- Welcome to FTP for ESP32 ---");
  client.println("220---   By krishnakanth.S   ---");
  client.println("220 --   Version " + String(FTP_SERVER_VERSION) + "   --");
  iCL = 0;
}

void FtpServer::disconnectClient()
{
#ifdef FTP_DEBUG
  Serial.println(" Disconnecting client");
#endif
  abortTransfer();
  client.println("221 Goodbye");
  client.stop();
}

boolean FtpServer::userIdentity()
{
  if (strcmp(command, "USER"))
    client.println("500 Syntax error");
  if (strcmp(parameters, _FTP_USER.c_str()))
    client.println("530 user not found");
  else
  {
    client.println("331 OK. Password required");
    strcpy(cwdName, "/");
    return true;
  }
  millisDelay = millis() + 100;
  return false;
}

boolean FtpServer::userPassword()
{
  if (strcmp(command, "PASS"))
    client.println("500 Syntax error");
  else if (strcmp(parameters, _FTP_PASS.c_str()))
    client.println("530 ");
  else
  {
#ifdef FTP_DEBUG
    Serial.println("OK. Waiting for commands.");
#endif
    client.println("230 OK.");
    return true;
  }
  millisDelay = millis() + 100;
  return false;
}

boolean FtpServer::processCommand()
{

  /*  CDUP - Change to Parent Directory */

  if (!strcmp(command, "CDUP") || (!strcmp(command, "CWD") && !strcmp(parameters, "..")))
  {
    bool ok = false;
    if (strlen(cwdName) > 1)
    {
      if (cwdName[strlen(cwdName) - 1] == '/')
        cwdName[strlen(cwdName) - 1] = 0;
      char *pSep = strrchr(cwdName, '/');
      ok = pSep > cwdName;
      if (ok)
      {
        *pSep = 0;
        ok = SD.exists(cwdName);
      }
    }
    if (!ok)
      strcpy(cwdName, "/");
    client.println("250 Ok. Current directory is " + String(cwdName));
  }

  /*  CWD - Change Working Directory */

  else if (!strcmp(command, "CWD"))
  {

    char path[FTP_CWD_SIZE];
    if (haveParameter() && makeExistsPath(path))
    {
      strcpy(cwdName, path);
      client.println("250 Ok. Current directory is " + String(cwdName));
    }
  }

  /*  PWD - Print Directory */

  else if (!strcmp(command, "PWD"))
    client.println("257 \"" + String(cwdName) + "\" is your current directory");

  /*  QUIT */

  else if (!strcmp(command, "QUIT"))
  {
    disconnectClient();
    return false;
  }

  /* MODE - Transfer Mode*/

  else if (!strcmp(command, "MODE"))
  {
    if (!strcmp(parameters, "S"))
      client.println("200 S Ok");
    else
      client.println("504 Only S(tream) is suported");
  }

  /* PASV - Passive Connection management */

  else if (!strcmp(command, "PASV"))
  {
    if (data.connected())
      data.stop();
    dataIp = WiFi.localIP();
    dataPort = FTP_DATA_PORT_PASV;
#ifdef FTP_DEBUG
    Serial.println("Connection management set to passive");
    Serial.println("Data port set to " + String(dataPort));
#endif
    client.println("227 Entering Passive Mode (" + String(dataIp[0]) + "," + String(dataIp[1]) + "," +
                   String(dataIp[2]) + "," + String(dataIp[3]) + "," + String(dataPort >> 8) + "," + String(dataPort & 255) + ").");
    dataPassiveConn = true;
  }

  /* PORT - Data Port */

  else if (!strcmp(command, "PORT"))
  {
    if (data)
      data.stop();
    dataIp[0] = atoi(parameters);
    char *p = strchr(parameters, ',');
    for (uint8_t i = 1; i < 4; i++)
    {
      dataIp[i] = atoi(++p);
      p = strchr(p, ',');
    }
    dataPort = 256 * atoi(++p);
    p = strchr(p, ',');
    dataPort += atoi(++p);
    if (p == NULL)
      client.println("501 Can't interpret parameters");
    else
    {

      client.println("200 PORT command successful");
      dataPassiveConn = false;
    }
  }

  /*  STRU - File Structure */

  else if (!strcmp(command, "STRU"))
  {
    if (!strcmp(parameters, "F"))
      client.println("200 F Ok");
    else
      client.println("504 Only F(ile) is suported");
  }

  /* TYPE - Data Type */

  else if (!strcmp(command, "TYPE"))
  {
    if (!strcmp(parameters, "A"))
      client.println("200 TYPE is now ASII");
    else if (!strcmp(parameters, "I"))
      client.println("200 TYPE is now 8-bit binary");
    else
      client.println("504 Unknow TYPE");
  }

  /*  ABOR - Abort */

  else if (!strcmp(command, "ABOR"))
  {
    abortTransfer();
    client.println("226 Data connection closed");
  }

  /*  DELE - Delete a File */

  else if (!strcmp(command, "DELE"))
  {
    char path[FTP_CWD_SIZE];
    if (strlen(parameters) == 0)
      client.println("501 No file name");
    else if (makePath(path))
    {
      if (!SD.exists(path))
        client.println("550 File " + String(parameters) + " not found");
      else
      {
        if (SD.remove(path))
          client.println("250 Deleted " + String(parameters));
        else
          client.println("450 Can't delete " + String(parameters));
      }
    }
  }

  /*  LIST - List */

  else if (!strcmp(command, "LIST"))
  {
    if (dataConnect())
    {
      client.println("150 Accepted data connection");
      uint16_t nm = 0;
      File dir = SD.open(cwdName);
      if ((!dir) || (!dir.isDirectory()))
        client.println("550 Can't open directory " + String(cwdName));
      else
      {
        File file = dir.openNextFile();
        while (file)
        {
          String fn, fs;
          fn = file.name();
          int i = fn.lastIndexOf("/") + 1;
          fn.remove(0, i);
#ifdef FTP_DEBUG
          Serial.println("File Name = " + fn);
#endif
          fs = String(file.size());
          if (file.isDirectory())
          {
            data.println("01-01-2021  10:09AM <DIR> " + fn);
          }
          else
          {
            data.println("01-01-2021  10:09AM " + fs + " " + fn);
          }
          nm++;
          file = dir.openNextFile();
        }
        client.println("226 " + String(nm) + " matches total");
        data.stop();
      }
    }
    else
    {
      client.println("425 No data connection");
      data.stop();
    }
  }

  /*  MLSD - Listing for Machine Processing (see RFC 3659) */

  else if (!strcmp(command, "MLSD"))
  {
    if (!dataConnect())
      client.println("425 No data connection MLSD");
    else
    {
      client.println("150 Accepted data connection");
      uint16_t nm = 0;
      File dir = SD.open(cwdName);
      if ((!dir) || (!dir.isDirectory()))
        client.println("550 Can't open directory " + String(cwdName));
      else
      {
        File file = dir.openNextFile();
        while (file)
        {

          String fn, fs;
          fn = file.name();
          int pos = fn.lastIndexOf("/");
          fn.remove(0, pos + 1);
          fs = String(file.size());
          if (file.isDirectory())
          {
            data.println(fn);
          }
          else
          {
            data.println(fs + " " + fn);
          }
          nm++;
          file = dir.openNextFile();
        }
        client.println("226-options: -a -l");
        client.println("226 " + String(nm) + " matches total");
      }
      data.stop();
    }
  }

  /*  NLST - Name List */

  else if (!strcmp(command, "NLST"))
  {
    if (!dataConnect())
      client.println("425 No data connection");
    else
    {
      client.println("150 Accepted data connection");
      uint16_t nm = 0;
      File dir = SD.open(cwdName);
      if (!SD.exists(cwdName))
        client.println("550 Can't open directory " + String(parameters));
      else
      {
        File file = dir.openNextFile();
        while (file)
        {
          data.println(file.name());
          nm++;
          file = dir.openNextFile();
        }
        client.println("226 " + String(nm) + " matches total");
      }
      data.stop();
    }
  }

  /*  NOOP */

  else if (!strcmp(command, "NOOP"))
  {

    client.println("200 Zzz...");
  }

  /*  RETR - Retrieve */

  else if (!strcmp(command, "RETR"))
  {
    char path[FTP_CWD_SIZE];
    if (strlen(parameters) == 0)
      client.println("501 No file name");
    else if (makePath(path))
    {
      file = SD.open(path, "r");
      if (!file)
        client.println("550 File " + String(parameters) + " not found");
      else if (!file)
        client.println("450 Can't open " + String(parameters));
      else if (!dataConnect())
        client.println("425 No data connection");
      else
      {
#ifdef FTP_DEBUG
        Serial.println("Sending " + String(parameters));
#endif
        client.println("150-Connected to port " + String(dataPort));
        client.println("150 " + String(file.size()) + " bytes to download");
        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStatus = 1;
      }
    }
  }

  /* STOR - Store */

  else if (!strcmp(command, "STOR"))
  {
    char path[FTP_CWD_SIZE];
    if (strlen(parameters) == 0)
      client.println("501 No file name");
    else if (makePath(path))
    {
      file = SD.open(path, "w");
      if (!file)
        client.println("451 Can't open/create " + String(parameters));
      else if (!dataConnect())
      {
        client.println("425 No data connection");
        file.close();
      }
      else
      {
#ifdef FTP_DEBUG
        Serial.println("Receiving " + String(parameters));
#endif
        client.println("150 Connected to port " + String(dataPort));
        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStatus = 2;
      }
    }
  }

  /*  MKD - Make Directory */

  else if (!strcmp(command, "MKD"))
  {
    char path[FTP_CWD_SIZE];
    if (haveParameter() && makePath(path))
    {
      if (SD.exists(path))
      {
        client.println("521 Can't create \"" + String(parameters) + ", Directory exists");
      }
      else
      {
        if (SD.mkdir(path))
        {
          client.println("257 \"" + String(parameters) + "\" created");
        }
        else
        {
          client.println("550 Can't create \"" + String(parameters));
        }
      }
    }
  }

  /* RMD - Remove a Directory */

  else if (!strcmp(command, "RMD"))
  {
    char path[FTP_CWD_SIZE];
    if (haveParameter() && makePath(path))
    {
      if (SD.rmdir(path))
      {
#ifdef FTP_DEBUG
        Serial.println(" Deleting " + String(parameters));

#endif
        client.println("250 \"" + String(parameters) + "\" deleted");
      }
      else
      {
        client.println("550 Can't remove \"" + String(parameters) + "\". Directory not empty?");
      }
    }
  }

  /*  RNFR - Rename From */

  else if (!strcmp(command, "RNFR"))
  {
    buf[0] = 0;
    if (strlen(parameters) == 0)
      client.println("501 No file name");
    else if (makePath(buf))
    {
      if (!SD.exists(buf))
        client.println("550 File " + String(parameters) + " not found");
      else
      {
#ifdef FTP_DEBUG
        Serial.println("Renaming " + String(buf));
#endif
        client.println("350 RNFR accepted - file exists, ready for destination");
        rnfrCmd = true;
      }
    }
  }

  /* RNTO - Rename To */

  else if (!strcmp(command, "RNTO"))
  {
    char path[FTP_CWD_SIZE];
    if (strlen(buf) == 0 || !rnfrCmd)
      client.println("503 Need RNFR before RNTO");
    else if (strlen(parameters) == 0)
      client.println("501 No file name");
    else if (makePath(path))
    {
      if (SD.exists(path))
        client.println("553 " + String(parameters) + " already exists");
      else
      {
#ifdef FTP_DEBUG
        Serial.println("Renaming " + String(buf) + " to " + String(path));
#endif
        if (SD.rename(buf, path))
          client.println("250 File successfully renamed or moved");
        else
          client.println("451 Rename/move failure");
      }
    }
    rnfrCmd = false;
  }

  /* FEAT - New Features*/

  else if (!strcmp(command, "FEAT"))
  {
    client.println("211-Extensions suported:");
    client.println(" MLSD");
    client.println("211 End.");
  }

  /*  SIZE - Size of the file */

  else if (!strcmp(command, "SIZE"))
  {
    char path[FTP_CWD_SIZE];
    if (strlen(parameters) == 0)
      client.println("501 No file name");
    else if (makePath(path))
    {
      file = SD.open(path, "r");
      if (!file)
        client.println("450 Can't open " + String(parameters));
      else
      {
        client.println("213 " + String(file.size()));
        file.close();
      }
    }
  }

  /*  SITE - System command */

  else if (!strcmp(command, "SITE"))
  {
    client.println("500 Unknow SITE command " + String(parameters));
  }

  /*  Unrecognized commands ...*/
  else
    client.println("500 Unknow command");

  return true;
}

boolean FtpServer::dataConnect()
{
  unsigned long startTime = millis();
  if (!data.connected())
  {
    while (!dataServer.hasClient() && millis() - startTime < 10000)
    {
      yield();
    }
    if (dataServer.hasClient())
    {
      data.stop();
      data = dataServer.available();
#ifdef FTP_DEBUG
      Serial.println("ftpdataserver client....");
#endif
    }
  }

  return data.connected();
}

boolean FtpServer::doRetrieve()
{
  if (data.connected())
  {
    int16_t nb = file.readBytes(buf, FTP_BUF_SIZE);
    if (nb > 0)
    {
      data.write((uint8_t *)buf, nb);
      bytesTransfered += nb;
      return true;
    }
  }
  closeTransfer();
  return false;
}

boolean FtpServer::doStore()
{
  if (data.connected())
  {
    int16_t nb = data.readBytes((uint8_t *)buf, FTP_BUF_SIZE);
    if (nb > 0)
    {
      file.write((uint8_t *)buf, nb);
      bytesTransfered += nb;
    }
    return true;
  }
  closeTransfer();
  return false;
}

void FtpServer::closeTransfer()
{
  uint32_t deltaT = (int32_t)(millis() - millisBeginTrans);
  if (deltaT > 0 && bytesTransfered > 0)
  {
    client.println("226-File successfully transferred");
    client.println("226 " + String(deltaT) + " ms, " + String(bytesTransfered / deltaT) + " kbytes/s");
  }
  else
    client.println("226 File successfully transferred");

  file.close();
  data.stop();
}

void FtpServer::abortTransfer()
{
  if (transferStatus > 0)
  {
    file.close();
    data.stop();
    client.println("426 Transfer aborted");
#ifdef FTP_DEBUG
    Serial.println("Transfer aborted!");
#endif
  }
  transferStatus = 0;
}

int8_t FtpServer::readChar()
{
  int8_t rc = -1;

  if (client.available())
  {
    char c = client.read();
#ifdef FTP_DEBUG
    Serial.print(c);
#endif
    if (c == '\\')
      c = '/';
    if (c != '\r')
    {
      if (c != '\n')
      {
        if (iCL < FTP_CMD_SIZE)
          cmdLine[iCL++] = c;
        else
          rc = -2;
      }
      else
      {
        cmdLine[iCL] = 0;
        command[0] = 0;
        parameters = NULL;
        if (iCL == 0)
          rc = 0;
        else
        {
          rc = iCL;

          parameters = strchr(cmdLine, ' ');
          if (parameters != NULL)
          {
            if (parameters - cmdLine > 4)
              rc = -2;
            else
            {
              strncpy(command, cmdLine, parameters - cmdLine);
              command[parameters - cmdLine] = 0;

              while (*(++parameters) == ' ')
                ;
            }
          }
          else if (strlen(cmdLine) > 4)
            rc = -2;
          else
            strcpy(command, cmdLine);
          iCL = 0;
        }
      }
    }
    if (rc > 0)
      for (uint8_t i = 0; i < strlen(command); i++)
        command[i] = toupper(command[i]);
    if (rc == -2)
    {
      iCL = 0;
      client.println("500 Syntax error");
    }
  }
  return rc;
}

boolean FtpServer::makePath(char *fullName)
{
  return makePath(fullName, parameters);
}

boolean FtpServer::makePath(char *fullName, char *param)
{
  if (param == NULL)
    param = parameters;

  if (strcmp(param, "/") == 0 || strlen(param) == 0)
  {
    strcpy(fullName, "/");
    return true;
  }

  if (param[0] != '/')
  {
    strcpy(fullName, cwdName);
    if (fullName[strlen(fullName) - 1] != '/')
      strncat(fullName, "/", FTP_CWD_SIZE);
    strncat(fullName, param, FTP_CWD_SIZE);
  }
  else
    strcpy(fullName, param);
  uint16_t strl = strlen(fullName) - 1;
  if (fullName[strl] == '/' && strl > 1)
    fullName[strl] = 0;
  if (strlen(fullName) < FTP_CWD_SIZE)
    return true;

  client.println("500 Command line too long");
  return false;
}

uint8_t FtpServer::getDateTime(uint16_t *pyear, uint8_t *pmonth, uint8_t *pday,
                               uint8_t *phour, uint8_t *pminute, uint8_t *psecond)
{
  char dt[15];
  if (strlen(parameters) < 15 || parameters[14] != ' ')
    return 0;
  for (uint8_t i = 0; i < 14; i++)
    if (!isdigit(parameters[i]))
      return 0;

  strncpy(dt, parameters, 14);
  dt[14] = 0;
  *psecond = atoi(dt + 12);
  dt[12] = 0;
  *pminute = atoi(dt + 10);
  dt[10] = 0;
  *phour = atoi(dt + 8);
  dt[8] = 0;
  *pday = atoi(dt + 6);
  dt[6] = 0;
  *pmonth = atoi(dt + 4);
  dt[4] = 0;
  *pyear = atoi(dt);
  return 15;
}

bool FtpServer::haveParameter()
{
  if (parameters != NULL && strlen(parameters) > 0)
    return true;
  client.println("501 No file name");
  return false;
}

bool FtpServer::makeExistsPath(char *path, char *param)
{
  if (!makePath(path, param))
    return false;
  if (SD.exists(path))
    return true;
  client.println("550 " + String(path) + " not found.");

  return false;
}
