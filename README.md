# ESP32_FTP_Server_with_SDcard
## Description :rainbow:
The FTP (File Transfer Protocol):vhs: over ESP32 SD_MMC  type card implementation exists already in code hosting platform. This project the SD_MMC card is replaced with Micro_SD card. The general type of micro SD card module can be used. Our work is just simple by interfacing SD_card module to ESP32 :pager: over SPI Interface and dump the built bin file to make ESP32 as your FTP Server.

## Requirements :wrench:
* [VS code](https://code.visualstudio.com/download) :link:
* PlatformIO Extension
* Platform - Espressif32
* [Lolin32 Esp32 module](https://robu.in/product/wemos-lolin32-v1-0-0-based-on-esp32-rev1-wifi-bluetooth-board/) :link:

* [Micro SD card](https://www.flipkart.com/sandisk-sdhc-16-gb-microsd-card-class-4-10-mb-s-memory/p/itm5290077029cd3?pid=ACCFMDZQJ2JJM5U8&lid=LSTACCFMDZQJ2JJM5U8IVLVMC&marketplace=FLIPKART&cmpid=content_memory-card_12825718239_u_8965229628_gmc_pla&tgi=sem,1,G,11214002,u,,,516793455547,,,,c,,,,,,,&ef_id=CjwKCAjwn6GGBhADEiwAruUcKg3eZ35fHfAmCZ3Sbwa5ub6zOSBw4JQ_UpVB6q2lpvCSSG_5HLwqhhoCbxUQAvD_BwE:G:s&s_kwcid=AL!739!3!516793455547!!!u!293946777986!&gclid=CjwKCAjwn6GGBhADEiwAruUcKg3eZ35fHfAmCZ3Sbwa5ub6zOSBw4JQ_UpVB6q2lpvCSSG_5HLwqhhoCbxUQAvD_BwE) :link:
* [Micro SD Storage Board](https://www.amazon.in/Storage-Memory-Shield-Adapter-Arduino/dp/B00HFQEKI0) :link:
```
Note !!!

* I recommend you to go with VS code IDE with mentioned Extensions and Platform for proper Execution..  
```
## Process :bookmark_tabs:
1. Install Visual Stdio Code and add Platform IO Extension.
2. Download the source code here.
3. Import the project to VS code.
4. To add the WIFI and FTP Authorisation,just insert a file name "auth.txt" in SD card. Inside the auth.txt follow the format of text . I have also attached a reference auth.txt in this repository. Please do check it.
```txt
wifi_user_name="*****"// size of the wifi_user_name 20 bytes.
wifi_pass="******"// size of the wifi_pass 20 bytes.
FTP_user_name="******"// size of the FTP_user_name 20 bytes.
FTP_pass="******"// size of the FTP_pass 20 bytes.
```
5. Now Attach the SD Card Module with ESP32 through SPI.

    | Pins_ESP32 | Pins_SD-card Module |
    |------------|-------------------- |
    | 19/MISO    |  MISO               |
    | 23/MOSI    |  MOSI               |
    | 18/SCL     |  SCL                |
    | 5/SS       |  SS                 |
    | 5v         |  Vcc                |
    | GND        |  GND                |

6. Flash the code into ESP32 kid :baby:.
7. Finally you have your hands on FTP Server of ESP32 :clap:.

```
Note!!!

* The Authorisation name and password of both WIFI and FTP must be less then 20 character of each.
* For debug purposes connect the ESP32 with terminal program for serial communication.
* The IP address of the ESP32 will be available on that terminal.

```

## Connection Establishment methods :electric_plug:
There are many ways to establish FTP clients for the server. Some of them are

    -> External app -[Filezilla]
    -> Windows File Explorer
    -> Internet Explorer
    -> Chrome Browser
    -> Linux Terminal

### Working Demos :movie_camera:
---
  ## @Windows File Explorer :file_folder:
---
Steps to Follow :walking:....

    * Open my computer --> right click --> Add a network location --> click on open custom network location  --> type in FTP address of ESP32 as per the format below 
        " ftp://ftp_user_name:ftp_password@ESP32_ip_addr "
    
        ftp_user_name --> FTP Authorisation user name set by you in config.
        ftp_password  --> FTP Authorisation password set by you in config. 
        ESP32_ip_addr --> ESP32 IP address shown on the terminal.

        then click ok --> click Finish.
    Finally the FTP Server opens on File Explorer as shown in below image...
    
<img src=".image/file_explorer.png" width="800" height="480">


---
  ## @Internet Explorer :earth_asia:
---
  Steps to Follow :walking:....

    * Open Internet Explorer in Windows machine --> enter the FTP url as

        " ftp://ftp_user_name:ftp_password@ESP32_ip_addr "
    
        ftp_user_name --> FTP Authorisation user name set by you in config.
        ftp_password  --> FTP Authorisation password set by you in config. 
        ESP32_ip_addr --> ESP32 IP address shown on the terminal.

        then click enter.
    Finally the FTP Server opens on Internet Explorer as shown in below image...

<img src=".image/internet_explorer.png" width="800" height="480">

~~~
Note!!!
* If there is any issue in connection,please do check the setting of Internet Explorer.

steps for checking settings in Internet explorer
    1. open internet explorer.
    2. click on the tools button on the top right corner.
    3. select the internet options.
    4. click on to advance section.
    5. under browsing 
        *make sure that "Enable FTP Folder view" and "use PASSIVE FTP" is enabled.
    6. After clicking apply,restart the browser.
    7. open internet explorer and enter the FTP url as instructed above.

Now you will have the access to your FTP Server. 

~~~

---
## @Linux Terminal :pager:
---
  Steps to Follow :walking:....

* Open Linux Terminal and enter "ftp"
```shell 
    krishna@krishna:~$ ftp
```
* Enter "open ESP32_ip_addr"
``` 
    ftp> open 172.20.10.3
```
* Enter FTP Authorisation name and password
``` 
    Name (172.20.10.3:krishna): admin
    331 OK. Password required
    Password:
```
* Enter into passive mode as using "passive" cmd
``` 
    ftp> passive
```
Finally Enter the required cmd to access the FTP server. [Check out the image below :point_down: for example]

<img src=".image/linux_terminal.png" width="800" height="480">

---

# Images :black_square_button:

## @ Front
<img src=".image/front _of_KID.jpeg" width="400" height="600">

---
## @ Back
<img src=".image/back_of_KID.jpeg" width="400" height="600">

---

# Bugs :bug:
* There will be some issue in viewing the root directory of Server in External app like Filezilla. It will be fixed soon.
* Multiple time disconnections while transfer file through External App.
* Time and Date issues will be fixed soon.

# Next Updates :triangular_flag_on_post:

* Fix the issues mentioned above.