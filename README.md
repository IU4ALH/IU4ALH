- Hi..
- this is a sketchs in .ino files for  dsp Radio receiver with chip Si4732 or Si4735 .
- Use ESP32 arduino Boards and 2.8 TFT display touchscreen  TFT_eSPI Ili9341, and also version for 3.5 - 4.0 inch displays touchscreen TFT_eSPI Ili9488
- ( with the help of YO2LDK Alex for adaptation to 3.5 - 4.0 inch displays).
- Initial code and job this files is a great PE0MGB Gert ( V. 3.5.ino) ...and PU2CLR Ricardo for Si4735 library and SSB patch,
- Mod. V. 4.0 ( original) made by Bernard Binns,
- Mod. V. 5.2a made by me  IU4ALH Antonino...( all two versions for 2.8 and for 3.5 - 4.0 inch displays);
- i have only added a few changes in screen look and i have added functions fully RDS services on FM broadcasting
-   ( station name, news and info, Time station).
- Also new updated  booth firmwares  (2.8  and 4.0 inch displays , versions 5.3a WiFI- 5.3b WIFI) ,Time/Date via Wi-Fi  ( only for user with wi-fi ) made by friend Emphi Erte from Indonesia.
Also the new firmware updated to 17.09.2022 (for 2.4 and 2.8 inch displays), version V.5.2b and version V.5.2b "Dark-Mode", with implementation of the Squelch function (activated by the "SQLCH" button ( put in place of the "CHIP" key which does not seem to be a very useful function), or by pressing the encoder button ... a first press for Volume, a second press for Squelch ... and adjustable by rotating the encoder ( values from 0 -no squelch- to 50 -max squelch-) masterfully made by our friend Lyle Hancock Sr. from the USA. - And now the latest version of this firmware for 3.5 - 4.0 inches (as of 03.28.2023) SI4735_V52bK_ILI9488_DARK , updated with the implementations made by our friend Alfredo De Simone , which has improved some aspects about the geographical areas in relation to the meteorological data and time, as well as the display with multiple settings in relation to the rotation of the display in use.
-   In the repository you will also find all the necessary and correct libraries to work with the Si4735 / 32 chip and the TFT_eSPI displays.
- For all library in use, schematic, etc. please refer on PE0MGB Github and PU2CLR Github. 

I added Three files.rar : si473x_ESP32_V2_PCB _Receiver.rar and ats25miniv2.rar .
They were passed on to me by my dear friend Silvio Assunçao who created them with true mastery.

-1- The first file (si473x_ESP32_V2_PCB _Receiver.rar) refers to the project (Gerbers) of a PCB (smd components) for the construction of a receiver with ESP32 - Si4732/35 - and the possibility of inserting multiple displays of different sizes, in fact it is possible to insert a 2.4 - 2.8 - 3.2 inch TFT display - touch- driver Ili9341, as well as a 3.5 inch TFT display - touch- driver Ili9488 (it is however also possible to insert a 4.0 inch display);

-2- the second file (ats25miniv2.rar) is a complete receiver project similar to the ATS25, but much miniaturised. In addition to the PCB project, demonstration photos and an electrical diagram are also included in the file... needless to remember that SMD components are used due to its miniaturization.
* - Both projects have already provided all the hardware options related to the latest firmware at the moment most developed ( V 4.15 Air by Bernard Binns - download on harduino.ru ), including the CW-RTTY-FeldHell digital signal decoder - visualization of audio spectrum and waterfall graphs; automatic switching of FM/AM-SW antennas; full battery check ; display brightness control; and other options...

-3-the third file (si473X_PCB_ V6.2.zip) is the same project, but where mostly standard components are used, minimizing the use of SMDs.

I have upload also this file from Silvio Assuncao..."Si473x PCB board v.7.zip"  ... same pcb , but in one piece of board, which after printing will have to be cut manually along the line traced on it PCBs.This work was necessary because in the previous version V. 6.2, when inserting the gerber files on the JLCPCB site to order their printing, the system indicated that there were two cards to be printed, and not one, therefore it was requested an added expense that increased the overall cost of the PCBs; in this way the PCB now appears to be in a single piece to be printed and the system recognizes only one piece, without adding further costs.

* I have also added another project " RX_SI4732_35_-IK8SUT_PROJECT.zip " complete with diagrams and Si4732/35 receiver PCB - esp32 - with digital mode decoder, created by the very good and dear friend IK8SUT Antonio Lalopa, but this time standard components are used (to the delight of those who, like me, love this type of montage...).
The possibility of mounting the 38-pin ESP32 card is inserted in the PCB, in the two versions, both the wider and the narrower one; as well as the mounting of the Si4735 chip or the Si4732 chip is supported.
