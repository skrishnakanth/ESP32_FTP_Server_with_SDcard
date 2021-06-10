#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32FtpServer.h"

const char *ssid = "Krishna";        /* WIFI user name */
const char *password = "00000000";   /* WIFI password */
const char *ftp_user_name = "admin"; /* FTP Authorisation user name */
const char *ftp_password = "pass";   /* FTP password */

FtpServer ftpSrv;

void setup()
{

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (SD.begin())
  {
    Serial.println("SD opened!");
    /* username, password for ftp.  
       set ports in ESP32FtpServer.h  (default 21 for FTP, 50009 for passive data port)*/
    ftpSrv.begin(ftp_user_name, ftp_password); 
  }
}

void loop()
{
  ftpSrv.handleFTP();
}
