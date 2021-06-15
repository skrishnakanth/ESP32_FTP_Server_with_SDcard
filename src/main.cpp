#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32FtpServer.h"
#include "files.h"

FtpServer ftpSrv;

void setup()
{
  Serial.begin(115200);
  SD_card_status(); /* Status of SD card*/
  Auth_decode();    /* Decoding auth file from SD card for WIFI and FTP config*/
  WiFi.begin(wifi_name, wifi_pass);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifi_name);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (SD.begin())
  {
    Serial.println("SD opened!");
    /* username, password for ftp.  
       set ports in ESP32FtpServer.h  (default 21 for FTP, 50009 for passive data port)*/
    ftpSrv.begin(ftp_name, ftp_pass);
  }
}

void loop()
{
  ftpSrv.handleFTP();
}
