#include "files.h"
#define FTP_DEBUG
char auth_data[200] = {0};

char wifi_name[20] = {0};
char wifi_pass[20] = {0};
char ftp_name[20] = {0};
char ftp_pass[20] = {0};

void SD_card_status()
{

    if (!SD.begin())
    {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB approx:%lluGB\n", cardSize, cardSize / 1024);
    delay(2000);
}

void Auth_decode()
{
    readFile(SD, "/auth.txt");
    int size_data = strlen(auth_data);
    int count = 0;
    int val = 0;
    for (int i = 0; i < size_data; i++)
    {
        if (auth_data[i] == '"')
        {
            count++;
            i++;
            val = 0;
        }
        switch (count)
        {
        case 1:
            wifi_name[val] = auth_data[i];
            val++;
            break;
        case 3:
            wifi_pass[val] = auth_data[i];
            val++;
            break;
        case 5:
            ftp_name[val] = auth_data[i];
            val++;
            break;
        case 7:
            ftp_pass[val] = auth_data[i];
            val++;
            break;
        default:
            break;
        }
    }
#ifdef FTP_DEBUG
    Serial.printf("the count values =%d\n", count);
    Serial.printf("WIFI name:%s\n", wifi_name);
    Serial.printf("WIFI pass:%s\n", wifi_pass);
    Serial.printf("ftp_name:%s\n", ftp_name);
    Serial.printf("ftp_pass:%s\n", ftp_pass);
#endif
}

bool readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        Serial.println("There is no file named \"auth.txt\" exists !!!");
        return false;
    }
    Serial.println("Read from file is Done !!!");
    int i = 0;
    while (file.available())
    {
        auth_data[i] = file.read();
        i++;
    }
    file.close();
    return true;
}
