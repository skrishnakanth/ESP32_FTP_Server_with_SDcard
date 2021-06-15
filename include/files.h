#ifndef __FILES_H__
#define __FILES_H__

#include "FS.h"
#include "SD.h"
#include "SPI.h"

extern char wifi_name[20];
extern char wifi_pass[20];
extern char ftp_name[20];
extern char ftp_pass[20];

extern char auth_data[200];
void SD_card_status();
void Auth_decode();
bool readFile(fs::FS &fs, const char * path);


#endif //__FILES_H__