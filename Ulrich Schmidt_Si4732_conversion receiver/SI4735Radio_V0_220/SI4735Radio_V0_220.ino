//##########################################################################################################################//
/*

Wideband receiver:

Here is the description of a wideband receiver project which covers 0.1-500 MHz. It is intended as an alternative to a SDR receiver. It fits in a small box and does not require 
a PC or RPi for signal processing. 
It uses ESP32, ILI9488 touch display, SI4732, SI5351 and AD831 as main components. A TinySA is used as panorama adapter. It uses potentiometers for volume, squelch and fine tune.
The software is a bit rough since I am not a programmer. It does include some nice features though, such as morse decoder, slow scan waterfall, memory bank scanning, web tools, etc.
To build the hardware you must have experience with RF circuits and the appropiate toolset. For building the filters you need a network analyzer or nanoVNA.
There is no schematics available, a lot of the hardware is based on try and error, but if you have experience with RF circuits you will be able to build it.


Brief hardware description:

1.
RF enters a +25dB wideband LNA. I used an MSA-0886 gain block, but there are better ones with lower noise available.
Important is to select a LNA that goes down to 0 MHz. 

2. WBFM uses a seperate antenna connector and no LNA or filters. There is basic support in the software, but no features.

3. RF then enters the filter bank. The filters are crucial and the most difficult part of the project. The RF gets filtered by one of 6 filters: 
0-30MHz lowpass = Filter 1, 25 - 65 bandpass = 2, 65 - 135 bandpass = 3, 116 - 148 steep bandpass = 4 , 148 - 220 = bandpass 5, above 220 highpass = 6
I used hand wound inductors and tuned them with a NanoVNA.This is not a good solution, there is not enough dampening for far off frequencies and losses are high. 
Better would be to use smd inductors and a properly designed PCB. If you can't do that, use a veroboard, place input and output SMA connectors 
close together, connect them with a thick ground wire and build the filter for the highest frequency closest to the connectors and the lowest frequencies furthest from the connectors. 

Frequency ranges are described in struct FilterInfo and can be adjusted as needed. I had to adjust several, since the real life filter parameters 
did not exactly meet the designed frequencies. The software activates the filter in use.
Filters are Chebyshev 5th order filters. 
Filters were designed with an online filter tool and a ripple of 1db.  The selected filter gets connected to the output of the LNA and the input of the mixer
through two pin diodes (BAR64). 
The pin diodes are placed as close to the SMA connectors as possible and connect to the repective filters via RG174 cables.    
Current for each pin diode is about 5ma. For low frequencies you could also use 1N4148.
The 0-30 MHz low pass has two additional inductors from input and output (100uH) to ground to provide a DC path for the pin diodes. All other filters are ground shunted.
A 74LS138 3:8 decoder is connected to 3 GPIO's of the ESP32 and its outputs drive the bases of 6 PNP transistor through 680 Ohms resistors.
Their emitters are connected to +5V and the collectors drive the respective pair of pin diodes through 1K resistors.


4. The filtered RF then gets mixed down in an AD831 to the IF of 21.4MHz. I used the AD831 schematics from the application note with a 9V supply. 
This is the only component that requires +9V supply. I also tried an ADE-1 diode ring mixer, but it required a much higher oscillator level, producing more spurs and interferences.
The AD 831 LO input is connected to two -6dB attenuators. One of them is connected directly to CLK2 of the SI5351 and is used for 0-200 MHz reception. The other one is connected 
to CLK0 through a Chebyshev 7th order high pass. It lets pass the 3rd harmonics which is used for 200-500MHz reception. This should better be a band pass,since it lets also pass the 5th harmonics 
which still produces mirrors of digital television. If CLK0 is selected in software, the VFO gets programmed with 1/3 of its frequency.

CLK2 uses mainly high injection (LO above RF), but this can be configured in struct FilterInfo. If high injection produces too many mirrors, try low injection and viceversa.
This can be done in segments, so you can for example have too many mirrors in the 2m amateur band, define start and end frequencies in the structure and change injection mode. 
CLK0 MHz uses mainly low injection (LO below RF). 500MHz RF is not the limit, I have tried up to 650MHz, but sensitivity drops rapidly. 

5. The 3rd clock from the SI5351 (CLK1) can be selected in software and used as clock source for a transmitter.

6. The IF side of the AD831 is terminated with 50 Ohms and enters a crystal filter which is made of 5 cheap 21.4 MHz 2 pole crystal filters (Aliexpress) in series and ground capacitors 
in between. The bandwith is +- 5 KHz, with around 70dB dampening at +- 10 KHz. There is a depression of around 2dB at the center frequency.
This filter unexpectedly allows excellent FM narrowband flank demodulation, so I did not build a seperate NBFM demodulator like originally planned. The ground capacitors were hand picked with a 
nanoVNA. The filters flanks should not be too steep, otherwise FM demodulation will get distorted. It would even be better to use 2 separate filters, a narrow one for SSB and a wider one for AM/NBFM.
My filter's center frequency is off around 3KHz (21.397MHz) for some reason, but this can be compensated in software. The filter is critical for not overloading the SI4732. 
The IF can be configured in the software, anything up to 30MHz is possible, so you can also use filters for different frequencies. The higher the IF, the better.

7. A TinySA is used as optional panorama adapter. Software connects it through pin diodes to either the IF output of AD831 (through a 20dB attenuator), or to the output of the MSA-0886 preamplifier. 
   Please be aware that the TinySA has a quite limited resolution.

8. The crystal filter output connects to a SI4732, terminated with 50 Ohms. There is quite a loss (10 - 15 db estimated) through the crystal filter, since it does not use impendance transformers,
but the active mixer provides enough amplification to overcome it.
The SI4732 is controlled by an ESP32 development board. SI4732 is in the standard configuration with a 32.768 KHz crystal, 
but it's frequency stability is not great. WBFM reception is possible and a seperate antenna connector is used for the FM 64-108MHz input. The software for FM radio reception is quite basic.
The audio outputs are connected together and go to a squelch transistor which is driven by the one of the ESP32 GPIO's. It grounds the audio signal when triggered and  eliminates noise and the hard cracks when switching mode.
From the squelch transistor the signal goes to the volume potentiomenter and then to a LM386 audioamplifier which drives headphones and speaker. DAC_CHANNEL_2  is connected via 330KOhms
to the input of the audio amplifier and provides a short touch sound.

9. The ESP32 drives the SI4732 and SI5351 on the same I2C bus. A squelch potentiometer and a fine tune potentiometer are connected to the ESP32.
The fine tune potentiometer is also used to adjust color spectrum in waterfall mode. The fine tune pot needs to be of good quality, since any noise causes frequency jumps.
It is important to use a 38pin version of the ESP32 board since almost all of the GPIO's will be used. 
The ESP32 also drives the 3.5" ILI9488 touch display in a standard configutation for the tft eSPI library.
This code will only work with ILI9488 480*320 pixel displays and is not adaptable for smaller displays.  

10. 12V DC input input gets regulated through two linear regulators down to +5 and +9V. Power consumption is about 400ma. 

11. For the FFT analysis and the morse decoder, audio output goes via 4.7K and 1 microfarad to the base of a npn transistor amplifier. Emitter via 100 Ohms to ground, base via 47K to collector and collector via 1K to +3.3V.
Collector then goes to pin 36 of the ESP which does the sampling. Gain is about 20 dB. Both applications require the volume of the SI4732 to be set to a fixed level, in this case 50, 
setVolume(50).

12. I have not designed a PCB, nor drawn full schematics. The ESP board is directly attached to the backside of the display. The HF part is build upon the copper sides of two seperate 
(unetched) PCB's with the SI4732, AD831, crystal filter and audio amp on one and the LNA and the filters on the other one. 
SI5351 is placed in a metal box and connected through coax cables with the mixer.There is a shielding plate between the display and the RF boards.

*/


//libraries
#include "FS.h"
#include <si5351.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SI4735.h>
#include "Rotary.h"
#include <Preferences.h>  //writes, reads and stores data on the EEPROM
#include "patch_full.h"
#include <font_Arial.h>  // Fonts are available in the following sizes:8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 28, 32, 40, 48, 60, 72 and 96
#include <DSEG7_Classic_Mini_Regular_34.h>
#include "Sprites.h"
#include <SPIFFS.h>
#include <FS.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "WiFi.h"
#include <PNGdec.h>
#include <JPEGDecoder.h>
#include "arduinoFFT.h"   //  !!! Must use version 1.6!! Later versions give compilation error
#include <unordered_map>  // morse mapping
#include <driver/dac.h>
#include <math.h>



//##########################################################################################################################//
//colors
#define TFT_BLACK 0x0000 /*   0,   0,   0 */
#define TFT_NAVY 0x000F  /*   0,   0, 128 */
#define TFT_DARKDARKGREEN 0x1b41
#define TFT_DARKGREEN 0x03E0
#define TFT_MIDGREEN 0x0584
#define TFT_DARKCYAN 0x03EF  /*   0, 128, 128 */
#define TFT_MAROON 0x7800    /* 128,   0,   0 */
#define TFT_PURPLE 0x780F    /* 128,   0, 128 */
#define TFT_OLIVE 0x7BE0     /* 128, 128,   0 */
#define TFT_LIGHTGREY 0xD69A /* 211, 211, 211 */
#define TFT_DARKGREY 0x7BEF  /* 128, 128, 128 */
#define TFT_DARKDARKGREY 0x2945
#define TFT_SILVERBLUE 0x8D5F
#define TFT_BLUE 0x001F  /*   0,   0, 255 */
#define TFT_GREEN 0x07E0 /*   0, 255,   0 */
#define TFT_CYAN 0x07FF  /*   0, 255, 255 */
#define TFT_RED 0xF800
#define TFT_DARKRED 0x90C1 /* 255,   0,   0 */
#define TFT_MAGENTA 0xF81F /* 255,   0, 255 */
#define TFT_YELLOW 0xFFE0  /* 255, 255,   0 */
#define TFT_WHITE 0xFFFF   /* 255, 255, 255 */
#define TFT_ORANGE 0xFDA0  /* 255, 180,   0 */
#define TFT_DEEPORANGE 0xf401
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK 0xFE19 /* 255, 192, 203 */
#define TFT_DARKBROWN 0x6a44
#define TFT_BROWN 0x9A60 /* 150,  75,   0 */
#define TFT_LIGHTBROWN 0x9347
#define TFT_GOLD 0xFEA0    /* 255, 215,   0 */
#define TFT_SILVER 0xC618  /* 192, 192, 192 */
#define TFT_SKYBLUE 0x867D /* 135, 206, 235 */
#define TFT_VIOLET 0x915C  /* 180,  46, 226 */
#define TFT_GREY 0x5AEB
#define TFT_FOREGROUND TFT_GOLD
#define TFT_GRID TFT_YELLOW

//plain button colors
#define TFT_BTNBDR TFT_NAVY
#define TFT_BTNCTR TFT_BLUE
#define TFT_MAINBTN_BDR TFT_NAVY
#define TFT_MAINBTN_CTR TFT_GREY

// button and sprite button sizes
#define TILE_WIDTH 75
#define TILE_HEIGHT 50
#define SPRITEBTN_WIDTH 80
#define SPRITEBTN_HEIGHT 50

//display size
#define DISP_WIDTH 480
#define DISP_HEIGHT 320

// modulation
#define AM 1
#define LSB 2
#define USB 3
#define FM 4  // not currently in use since AM flank demodulation is working very well for NFM
#define WBFM 5
#define SYNC 6  //AM Sync, doesn't work well
#define CW 7

#define GAINREDUCTION 20        //  using a LNA needs correction in dB
#define FMSTARTFREQ 10270       // WBFM starts here, use frequency of a nice radio station
#define PATCHLOADSPEED 2000000  // I2C bus speed when loading SSB patch. 2MHz works fine

//ESP32 GPIO's
#define ENCODER_PIN_A 17       // Encoder left and right pins
#define ENCODER_PIN_B 16       // Encoder left and right pins
#define ENC_PRESSED 33         // encoder pressbutton pin
#define ESP32_I2C_SDA 21       // I2C bus pin on ESP32
#define ESP32_I2C_SCL 22       // I2C bus pin on ESP32
#define RESET_PIN 12           //SI4735 reset
#define SQUELCH_POT 35         // Squelch pot center pin, the other pins go to ground and +3.3V. Squelch pot is 5KOhm lin, good quality
#define MUTEPIN 32             // goes via 3.3K to base of external mute transistor
#define FINETUNE_PIN 34        // finetune pot center pin, the other pins go to ground and +3.3V Finetune  pot is also 5KOhm lin, good quality
#define DAC_PIN DAC_CHAN_1     // output for touchsound, goes via 330K to  AF amplifier input
#define TINYSA_PIN 27          //  GPIO for pin diodes that selects input for the TinySA
#define AUDIO_INPUT_PIN 36     // connected via a transistor amplifier to audio output of SI4732

// Waterfall
#define WATERFALL_SCREEN_WIDTH 480
#define WATERFALL_SCREEN_HEIGHT 231  // more than that will lead to a memory allocation error
#define FRAMEBUFFER_HALF_WIDTH 120   // need to use two half size frame buffers, couldn't allocate a full size 240*290 buffer
#define FRAMEBUFFER_FULL_WIDTH 240
#define FRAMEBUFFER_HEIGHT 231

//TFT
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_RESET 4
//SPIFFS
#define FORMAT_SPIFFS_IF_FAILED true  //format the SPIFFS if not already formatted
//FFT
#define SAMPLES 256



//Connections with ILI 9488 3.5" display:
/// Touschscreen TDO -> 19
//TFT_SDI -> 23
//TFT_CLK ->18
//TFT_CS   15  // Chip select control pin
//TFT_DC    2  // Data Command control pin
//TFT_RST   4  // Reset pin (could connect to RST pin)
//TOUCH_CS 5     // Chip select pin (T_CS) of touch screen


// TFT display defines in user setup.h
/* 
#define TFT_MISO 19 //!!!!Connect ESP32 19 with touchscreen TDO. NOT with TFT_MISO. Connect other pins as below
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)
//#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
#define TOUCH_CS 5     // Chip select pin (T_CS) of touch screen


74LS138 3-Line To 8-Line Decoder to drive 6 filters with 3 GPIO's: 
connect GPIO 13 to A (pin 1)
connect GPIO 14 to B (pin2)
connect GPIO 25 to C (pin3)

GPIO's used: 2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33,34,35,36 
// Still available GPIO's (input only):  39
*/
//##########################################################################################################################//
//Classes/methods

Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_ext etft = TFT_eSPI_ext(&tft);
TFT_eSprite spr = TFT_eSprite(&tft);
Si5351 si5351(0x60);  //Si5351 I2C Address 0x60
SI4735 si4735;
Preferences preferences;  // EEPROM save data
PNG png;
File myfile;
arduinoFFT FFT = arduinoFFT();
//##########################################################################################################################//

// global variables

const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h
volatile bool clw = false;                               // encoder direction clockwise
volatile bool cclw = false;                              // counter clock wise
bool encRead = false;


long STEP;  //STEP size
long LASTSTEP = -1;
long LOW_FREQ = 100000;        // lower frequency
long HI_FREQ = 650000000;      // Max frequency
long SWITCHPOINT = 200000000;  // above 200 MHz use 3rd harmonics of CLK0 filtered through a highpass
long FREQ = 1000000;           //VFO frequency initial value
long FREQ_OLD = FREQ - 1;      // start with != FREQ value
long LO_RX;                    // local oscillator frequency
long LO_TX = FREQ;
int SI4735TUNE_FREQ = 21397;  // mid frequency of crystal filter, SI4732 needs to be tuned to it. My filter is 3KHz off, so the IF is 21397KHz
long freq, freqold, fstep;
long lastAMFREQ = -1;
long potVal;      // fine tune potentiometer read value
long lim2 = 0;    // frequency limits from touchpad entries
long lim1 = 0;    // frequency limits from touchpad entries
long keyVal = 0;  // scan mode frequencies delivered from touchpad

bool rxmode = true;
bool txmode = false;
bool tinySA_RF_Mode = false;  // start tinySA in IF mode
bool scanMode = false;
bool pressed = false;
bool pressSound = true;
bool redrawMainScreen = true;  // redraw  main window elements
bool encLockedtoSynth = true;  // locks encoder to SI5351
bool ssbLoaded = false;
bool CLK2ACTIVE = true;              //0-200NHZ
bool CLK0ACTIVE = false;             //200-500MHZ
bool SI4735directFreqInput = false;  // for debugging , will set SI4732 Frequency
bool altStyle = false;               // false = use sprites, true = use tiles
bool displayDebugInfo = true;        // display clock info and looptimer
bool showScanRange = true;           // show scan range after entered
bool LOAboveRF = true;               // true = LO above RF, false = LO below RF
bool WBFMactive = false;             // to switch back to AM/SSB after WBFM
bool showFREQ = true;                // false supresses FREQ display
bool sevenSeg = false;               // use 7 segemnt font
const int Xsmtr = 10;                // Smeter position
const int Ysmtr = 58;                // Smeter position
bool showSNR = false;                // shows SNR in Smeter
const int HorSpacing = 86;           // buttons horizontal distance
const int VerSpacing = 65;           // buttons vertical distance
const int vTouchSpacing = 72;        // adjust vertical touch spacing

uint8_t bandWidth = 0;
uint8_t lastSSBBandwidth = 3;
uint8_t lastAMBandwidth = 0;
uint8_t AGCDIS = 0;  //This param selects whether the AGC is enabled or disabled (0 = AGC enabled; 1 = AGC disabled);
uint8_t AGCIDX = 0;  //AGC Index (0 = Minimum attenuation (max gain); 1 – 36 = Intermediate attenuation); if >greater than 36 - Maximum attenuation (min gain) ).
uint8_t SNR = 0;
uint8_t currentSquelch = 0;  // squelch trigger
uint8_t signalStrength = 0;
uint8_t modType = 1;           //modulation type
uint16_t tx = 0, ty = 0;       // touchscreen coordinates
uint16_t row = 0, column = 0;  // buttons are grouped in rows and columns
uint16_t filterNumber;         // active filter number


// misc
char txtbuf[1500] = { 0 };        // buffer for webpage text extractor
uint16_t wabuf[26][95] = { 0 };   // audio waterfall buffer
uint16_t textColor = TFT_ORANGE;  // was TFT_GOLD
const char ver[] = "V0.220";      // version
bool audioMuted = false;          // mute state
char sMode = 0;                   // audio spectrum analyzer mode 0= off, 1 = 24 channel, 2 = 85 channel, 3= minioscilloscope, 4 = waterfall, 5 = waterfall + miniosci
int amplitude = 150;              // amplitute divider for spectrum analyzer
int fTrigger = 0;                 // triggers functions in main loop
bool disableSpectrum = false;     // stops the spectrum analyzer during time critical operations
bool SNRSquelch = false;

// Waterfall
uint16_t* framebuffer1;  // First framebuffer for pixel data, each is 240 pixels wide
uint16_t* framebuffer2;  // Second framebuffer for pixel data
int currentLine = 0;     // Current line being written to
uint16_t newLine[FRAMEBUFFER_FULL_WIDTH];

// Sprites
extern const uint16_t But4[], But5[], But6[], But7[], But8[], But9[], But10[], But11[];  // sprite buttons in sprite.h
const uint16_t* buttonImages[] = { But4, But5, But6, But7, But8, But9, But10, But11 };
uint16_t buttonSelected = 1;  // 0-7 are available


// Web
uint8_t imageSelector = 0;
bool swappedJPEG = false;
const char* ssid = "YourSSID";
const char* password = "Yourpassword";
int yShift = 0;
int xShift = 0;
int reportSelector = 0;

// URL's for png and jpeg images. Change as you like
const char* host = "https://www.sws.bom.gov.au/Images/HF%20Systems/Global%20HF/Ionospheric%20Map/West/fof2_maps.png";
const char* host1 = "https://www.sws.bom.gov.au/Images/HF%20Systems/Global%20HF/Fadeout%20Charts/rtalf.jpeg";
const char* host2 = "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/FD/GEOCOLOR/339x339.jpg";
const char* host3 = "https://www.sws.bom.gov.au/Images/SOLROT/noscript/SOL_IMG_2.jpg";
const char* host4 = "https://cdn.star.nesdis.noaa.gov/GOES16/ABI/SECTOR/CAM/GEOCOLOR/1000x1000.jpg";
const char* host5 = "https://services.swpc.noaa.gov/images/animations/d-rap/global/d-rap/latest.png";

const char* al = "https://services.swpc.noaa.gov/products/alerts.json";     // Spaceweather alerts from NOAA
const char* fr = "https://services.swpc.noaa.gov/text/3-day-forecast.txt";  // Forecast from NOAA

//##########################################################################################################################//
// structures

PROGMEM struct BandInfo {
  const char* bandName;
  long startFreqKHz;
  long stopFreqKHz;
  bool isAmateurRadioBand;
  int bandNumber;
};

BandInfo bands[] = {
  { "LW", 150, 280, false, 1 },
  { "MW", 520, 1710, false, 2 },
  { "160M", 1800, 2000, true, 3 },       // Amateur radio band,
  { "120M", 2300, 2495, false, 4 },      // Broadcast SW band,
  { "90M", 3200, 3400, false, 5 },       // Broadcast SW band,
  { "80M", 3500, 4000, true, 6 },        // Amateur radio band,
  { "75M", 3900, 4000, false, 7 },       // Broadcast SW band,
  { "60M", 5250, 5450, true, 8 },        // Amateur radio band,
  { "49M", 5900, 6200, false, 9 },       // Broadcast SW band,
  { "41M", 7200, 7600, false, 10 },      // Broadcast SW band,
  { "40M", 7000, 7300, true, 11 },       // Amateur radio band,
  { "31M", 9500, 9900, false, 12 },      // Broadcast SW band,
  { "30M", 10100, 10150, true, 13 },     // Amateur radio band,
  { "25M", 11600, 12100, false, 14 },    // Broadcast SW band,
  { "EXIT", 0, 0, false, 15 },           // Placeholder for exit
  { "NEXT", 0, 1, false, 16 },           // Placeholder for next
  { "22M", 13570, 13870, false, 17 },    // Broadcast SW band,
  { "20M", 14000, 14350, true, 18 },     // Amateur radio band,
  { "19M", 15100, 15800, false, 19 },    // Broadcast SW band,
  { "17M", 18068, 18168, true, 20 },     // Amateur radio band,
  { "16M", 17550, 17900, false, 21 },    // Broadcast SW band,
  { "15M", 21000, 21450, true, 22 },     // Amateur radio band,
  { "13M", 21450, 21850, false, 22 },    // Broadcast SW band,
  { "12M", 24890, 24990, true, 24 },     // Amateur radio band,
  { "11M", 25670, 26100, false, 24 },    // Broadcast SW band,
  { "10M", 28000, 29700, true, 26 },     // Amateur radio band,
  { "CB", 26965, 27600, false, 27 },     // CB band,
  { "4M", 50000, 54000, true, 28 },      // Amateur radio band,
  { "AIR", 118000, 128000, false, 30 },  // Air band,
  { "2M", 144000, 148000, true, 29 },    // Amateur radio band,
  { "EXIT", 0, 0, false, 15 },           // Placeholder for exit
  { "NEXT", 0, 1, false, 16 },           // Placeholder for next
  { "70CM", 430000, 440000, true, 31 },  // Amateur radio band,
  { "AIR2", 118000, 123000, false, 32 },
  { "Test", 127650, 127700, false, 33 },
  { "USR", 10000, 10100, false, 34 },  // User defined bands
  { "USR", 15000, 15100, false, 35 },
  { "USR", 20000, 20100, false, 36 },
  { "USR", 25000, 25100, false, 37 },
  { "USR", 30000, 30100, false, 38 },
  { "USR", 35000, 35100, false, 39 },
  { "USR", 40000, 40100, false, 40 },
  { "USR", 45000, 45100, false, 41 },
  { "USR", 50000, 50100, false, 42 },
  { "USR", 55000, 55100, false, 43 },
  { "USR", 60000, 60100, false, 44 },
  { "USR", 65000, 65100, false, 45 },
  { "EXIT", 0, 0, false, 15 },
};


PROGMEM struct MemoInfo {
  const char* memoName;
  long FreqKHz;
  int memoNumber;
};

MemoInfo memo[] = {               // holds station names that will be displayed at the statusline
  { "Coco Weather", 127675, 1 },  // here the local airport frequencies. Change as you please
  { "Coco Control", 119600, 2 },
  { "Coco Approach", 120500, 3 },
  { "Coco Clearance ", 121300, 4 },
  { "Coco Ground ", 121900, 5 },
  { "Coco Radio", 126800, 6 },
  { "Coco Tower ", 118600, 7 },
  { "Pavas Tower", 118300, 8 },
  { "Pavas Ground", 121700, 9 },
  { "-- ", 0, 10 },
  { "-- ", 0, 11 },
  { "-- ", 0, 12 },
  { "-- ", 0, 13 },
  { "-- ", 0, 14 },
  { "-- ", 0, 15 },
  { "-- ", 0, 16 },
  { "-- ", 0, 17 },
  { "-- ", 0, 18 },
  { "-- ", 0, 19 },
  { "-- ", 0, 20 },
  { "-- ", 0, 21 },
  { "-- ", 0, 22 },
  { "-- ", 0, 23 },
  { "-- ", 0, 24 },
  { "-- ", 0, 25 },
  { "-- ", 0, 26 },
  { "-- ", 0, 27 },
  { "-- ", 0, 28 },
  { "-- ", 0, 29 },
  { "-- ", 0, 30 },
  { "-- ", 0, 31 },
  { "-- ", 0, 32 },
  { "-- ", 0, 33 },
  { "-- ", 0, 34 },
  { "-- ", 0, 35 },
  { "-- ", 0, 36 },
  { "-- ", 0, 37 },
  { "-- ", 0, 38 },
  { "-- ", 0, 39 },
  { "-- ", 0, 40 },
  { "-- ", 0, 41 },
  { "-- ", 0, 42 },
  { "-- ", 0, 43 },
  { "-- ", 0, 44 },
  { "-- ", 0, 45 },
  { "-- ", 0, 46 },
  { "-- ", 0, 47 },
  { "-- ", 0, 48 },
  { "-- ", 0, 49 },
  { "-- ", 0, 50 },
  { "-- ", 0, 51 },
  { "-- ", 0, 52 },
  { "-- ", 0, 53 },
  { "-- ", 0, 54 },
  { "-- ", 0, 55 },
  { "-- ", 0, 56 },
  { "-- ", 0, 57 },
  { "-- ", 0, 58 },
  { "-- ", 0, 59 },
  { "-- ", 0, 60 },
  { "-- ", 0, 61 },
  { "-- ", 0, 62 },
  { "-- ", 0, 63 },
  { "-- ", 0, 64 },
  { "-- ", 0, 65 },
  { "-- ", 0, 66 },
  { "-- ", 0, 67 },
  { "-- ", 0, 68 },
  { "-- ", 0, 69 },
  { "-- ", 0, 70 },
  { "-- ", 0, 71 },
  { "-- ", 0, 72 },
  { "-- ", 0, 73 },
  { "-- ", 0, 74 },
  { "-- ", 0, 75 },
  { "-- ", 0, 76 },
  { "-- ", 0, 77 },
  { "-- ", 0, 78 },
  { "-- ", 0, 79 },
  { "-- ", 0, 80 },
  { "-- ", 0, 81 },
  { "-- ", 0, 82 },
  { "-- ", 0, 83 },
  { "-- ", 0, 84 },
  { "-- ", 0, 85 },
  { "-- ", 0, 86 },
  { "-- ", 0, 87 },
  { "-- ", 0, 88 },
  { "-- ", 0, 89 },
  { "-- ", 0, 90 },
  { "-- ", 0, 91 },
  { "-- ", 0, 92 },
  { "-- ", 0, 93 },
  { "-- ", 0, 94 },
  { "-- ", 0, 95 },
  { "-- ", 0, 96 },
  { "-- ", 0, 97 },
  { "-- ", 0, 98 },
  { "-- ", 0, 99 },
  { "-- ", 0, 100 },
  { "-- ", 0, 101 },
  { "-- ", 0, 102 },
  { "-- ", 0, 103 },
  { "-- ", 0, 104 },
  { "-- ", 0, 105 },
  { "-- ", 0, 106 },
  { "-- ", 0, 107 },
  { "-- ", 0, 108 },
  { "-- ", 0, 109 },
  { "-- ", 0, 110 },
  { "-- ", 0, 111 },
  { "-- ", 0, 112 },
  { "-- ", 0, 113 },
  { "-- ", 0, 114 },
  { "-- ", 0, 115 },
  { "-- ", 0, 116 },
  { "-- ", 0, 117 },
  { "-- ", 0, 118 },
  { "-- ", 0, 119 },
  { "-- ", 0, 120 },
  { "-- ", 0, 121 },
  { "-- ", 0, 122 },
  { "-- ", 0, 123 },
  { "-- ", 0, 124 },
  { "-- ", 0, 125 },
  { "-- ", 0, 126 },
  { "-- ", 0, 127 },
  { "-- ", 0, 128 }
};





struct FilterInfo {  //0-30MHz lowpass = Filter 1, 25 - 65 bandpass = 2, 65 - 135 bandpass = 3, 116 - 148 bandpass = 4 , 148 - 220 = bandpass 5, above 220 highpass = 6
  long startFreqKHz;  // Adjust frequencies as needed
  long stopFreqKHz;
  bool injectionMode;  // false = oscillator freq below RF, true = oscillator freq above RF, if there are too many mirrors or interference, change injection mode
  int FilterNumber;
};

// Initialize the filters
FilterInfo filter[] = {
  { 100, 519, true, 1 },
  { 520, 1620, false, 1 },  // example: use oscillator below RF
  { 1621, 30000, true, 1 },
  { 30001, 65000, true, 2 },
  { 65001, 116000, true, 3 },
  { 116001, 148000, true, 4 },
  { 148001, 200000, true, 5 },
  { 200001, 4300000, true, 6 },
  { 430001, 4400000, false, 6 }, // example for changing the injection mode only in the 70cm band
  { 440001, 600000, true, 6 }
};


//##########################################################################################################################//

//CODE START

//##########################################################################################################################//

void IRAM_ATTR RotaryEncFreq() {  // rotary encoder interrupt handler
  int encoderStatus = encoder.process();
  if (encoderStatus) {
    if (encoderStatus == DIR_CW)
      clw = true;
    else
      cclw = true;
  }
}
//##########################################################################################################################//
void setup() {
  Serial.begin(115200);
  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL, 400000);
  preferences.begin("data", false);  // preferences namespace is data
  loadLastSettings();                // load settings from EEPROM
  bootScreen(true);                  // true = show bootscreen and init HW
  etft.TTFdestination(&tft);
  etft.setTTFFont(Arial_13);
  uint16_t calData[5] = { 312, 3331, 453, 2794, 5 };  // works better on top of the screen
  //uint16_t calData[5] = { 297, 3313, 406, 2615, 5 };  // works better on bottom
  // uint16_t calData[5] = { 305, 3313, 425, 2700, 5 };  // original values
  tft.setTouch(calData);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENC_PRESSED, INPUT_PULLUP);
  pinMode(MUTEPIN, OUTPUT);
  pinMode(36, INPUT);
  pinMode(39, INPUT);
  pinMode(13, OUTPUT);  // pin to select filters
  pinMode(14, OUTPUT);  // pin to select filters
  pinMode(25, OUTPUT);  // pin to select filters
  pinMode(27, OUTPUT);  // TinySA pin diode
  pinMode(DAC_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), RotaryEncFreq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), RotaryEncFreq, CHANGE);
  SI5351_Init();
  drawBigBtns();  // UP, MODE, DOWN
  radioInit();
}

//##########################################################################################################################//


void loop() {


  encoderChangeFREQ();  // check whether encoder has moved
  displaySTEP();        // check STEP   
  fineTune();             // read frequency potentiometer
  saveCurrentSettings();  // save settings after 1 minute if freq has not changed

  if (FREQ != FREQ_OLD) {  // If FREQ changes, update Si5351A
    FREQCheck();           //check whether within FREQ range
    displayFREQ(FREQ);     // display new FREQ
    tuneSI5351();          // and tune it in
    FREQ_OLD = FREQ;
  }

  if (fTrigger % 3 == 0) {   // functions that don't have to run in every loop cycle
    readTouchCoordinates();  // this function takes long. Calling it 1 out of 3 loop cycles is suficient. Less than 3 and it will not work reliably
    readSquelchPot(true);    // true = read and draw position circle
    setSquelch();
    setMode();  //  set mode (tune or scan mode) must run after readTouchCoordinates(). Needs a recent value of pressed
  }


  if (fTrigger % 15 == 0 && disableSpectrum == false) {  // spectrum eats a lot of processing time
    audioSpectrum();
  }


  if (fTrigger % 50 == 0) {  // Smeter about 4x second to reduce noise on I2C
    Smeter();
    loopTimer();
    fTrigger = 0;
    disableSpectrum = false;
  }

  mainScreen();  // build main window
  fTrigger++;    // function trigger counter, triggers functions that should not run every cycle
}
//##########################################################################################################################//

void restartSynth() {  // restart SI5153 when CLK source gets changed

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(preferences.getLong("calib", 0), SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA); // 2MA provides more than suficcient level for teh mixer
}

//##########################################################################################################################//

void encoderChangeFREQ() {

  if (encLockedtoSynth) {  // if encoder is used to change FREQ
    if (clw) {
      disableSpectrum = true;  // block spectrum analyzer when moving encoder to make tuning more responsive
      FREQ += STEP;
      clw = false;
    } else if (cclw) {
      disableSpectrum = true;
      FREQ -= STEP;
      cclw = false;
    }
  }
}
//##########################################################################################################################//

void FREQCheck() {  // sanity check and eliminate rounding error when keypad is used for frequency input

  if (FREQ < LOW_FREQ)
    FREQ = LOW_FREQ;

  if (FREQ > HI_FREQ)
    FREQ = HI_FREQ;


  if (FREQ % 10 >= 8 || FREQ % 10 <= 2) {  // eliminate frequency rounding errors. Keypad entry uses floats, so rounding errors may come up
    long lowerEnd = (FREQ / 10) * 10;
    long upperEnd = lowerEnd + 10;

    if (FREQ % 10 >= 9) {
      FREQ = upperEnd;  // Round up
    } else {
      FREQ = lowerEnd;  // Round down
    }
  }
}

//##########################################################################################################################//

//Frequency display, much of the stuff is to reduce flicker when tuning. Updates only the section that changes, not the entire frequency display
void displayFREQ(long freq) {
  static uint32_t oldFreq = 0;
  uint16_t xPos = 8;
  uint16_t yPos = 10;
  static bool lastMode = true;  // to switch btw tx and rx mode


  if (showFREQ == false)
    return;


  long old_Mhz = oldFreq / 1000000;
  long old_Khz = (oldFreq - (old_Mhz * 1000000)) / 1000;
  long old_Hz = (oldFreq - (old_Mhz * 1000000)) - old_Khz * 1000;

  long new_Mhz = freq / 1000000;
  long new_Khz = (freq - (new_Mhz * 1000000)) / 1000;
  long new_Hz = (freq - (new_Mhz * 1000000)) - new_Khz * 1000;


  if (!sevenSeg) {  // Arial font

    if (oldFreq != freq) {  // overwrite digits that have changed
      if (old_Mhz != new_Mhz || lastMode != rxmode)
        tft.fillRect(xPos, yPos, 155, 34, TFT_BLACK);

      if (old_Khz != new_Khz)
        tft.fillRect(xPos + 75, yPos, 87, 34, TFT_BLACK);

      if (old_Hz != new_Hz)
        tft.fillRect(xPos + 155, yPos, 67, 34, TFT_BLACK);

      lastMode = rxmode;
    }
    etft.setTTFFont(Arial_32);

    if (LOAboveRF)  // set colors
      etft.setTextColor(TFT_GREEN); // normal color

    if (txmode) {
      etft.setTextColor(TFT_ORANGE);  //tx mode
      tft.fillRect(xPos, yPos, 310, 34, TFT_BLACK);
    }

    if (SI4735directFreqInput) { //WBFM
      etft.setTextColor(TFT_MAGENTA);
    }

    if (FREQ > SWITCHPOINT)  // > 200 MHz
      etft.setTextColor(TFT_SKYBLUE);

    if (!LOAboveRF)                   // alternate injection mode
      etft.setTextColor(TFT_YELLOW);

    if (new_Mhz >= 10) {  // position cursor
      etft.setCursor(xPos, yPos);
    } else if (new_Mhz >= 1) {
      etft.setCursor(xPos + 20, yPos);
    } else {
      etft.setCursor(xPos + 50, yPos);
    }
    etft.printf("%3.0ld", new_Mhz);

    etft.setCursor(xPos + 75, yPos);
    etft.printf(".%03ld.", new_Khz);
    etft.setTTFFont(Arial_20);
    etft.printf("%03ld", new_Hz);
    etft.setTTFFont(Arial_32);
    etft.setCursor(xPos + 208, yPos);
    etft.print(" MHz");

  }

  else {

    // 7 segment font
    if (oldFreq != freq) {  // fill only the digits that have changed
      if (old_Mhz != new_Mhz || lastMode != rxmode)
        tft.fillRect(xPos, yPos - 3, 90, 37, TFT_BLACK);

      if (old_Khz != new_Khz)
        tft.fillRect(xPos + 95, yPos - 3, 93, 37, TFT_BLACK);

      if (old_Hz != new_Hz)
        tft.fillRect(xPos + 205, yPos - 3, 80, 37, TFT_BLACK);

      lastMode = rxmode;
    }
    if (txmode) {
      tft.setTextColor(TFT_ORANGE);
      tft.fillRect(xPos, yPos, 310, 34, TFT_BLACK);
    } else if (SI4735directFreqInput) {
      tft.setTextColor(TFT_MAGENTA);
    } else
      tft.setTextColor(TFT_GREEN);

    if (FREQ > SWITCHPOINT)  // > 200 MHz
      tft.setTextColor(TFT_SKYBLUE);

    if (!LOAboveRF)
      tft.setTextColor(TFT_YELLOW);

    tft.setTextSize(0);
    tft.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
    if (new_Mhz >= 100) {
      tft.setCursor(xPos, yPos + 31);
    }

    if (new_Mhz >= 10 && new_Mhz < 100) {
      tft.setCursor(xPos + 20, yPos + 31);
    }
    if (new_Mhz >= 1 && new_Mhz < 10) {
      tft.setCursor(xPos + 45, yPos + 31);
    }
    tft.printf("%3.0ld.", new_Mhz);

    tft.setCursor(xPos + 101, yPos + 31);
    tft.printf("%03ld.", new_Khz);
    tft.printf("  %03ld", new_Hz);
    tft.setFreeFont(NULL);
    tft.setTextSize(2);
  }

  oldFreq = freq;
}

//##########################################################################################################################//
//STEP display in upper right corner
void displaySTEP() {
  static uint32_t OLDSTEP;

  if (OLDSTEP == STEP)
    return;
  else
    OLDSTEP = STEP;

  tft.setTextColor(textColor);
  uint16_t xPos = 345;
  uint16_t yPos = 24;

  // Clear previous text
  tft.fillRect(xPos, yPos, 130, 20, TFT_BLACK);
  tft.setCursor(xPos, yPos);
  tft.setTextSize(2);

  if (modType == WBFM) {
    tft.print("Step:100KHz");
    return;
  }

  if (STEP < 1000)
    tft.printf("Step:%ldHz", STEP);
  if (STEP >= 1000 && STEP < 1000000)
    tft.printf("Step:%ldKHz", STEP / 1000);
  if (STEP >= 1000000)
    tft.printf("Step:%ldMHz", STEP / 1000000);
}

//##########################################################################################################################//

//STEP switching


void setSTEP(int mode) {
  // mode 0 = change STEP with touchbuttons, -1 = touch stepdisplay left = next stepsize down, +1 = touch stepdisplay right = next stepsize up


  int pos = 0;
  uint32_t AMStepSize[9] = { 1000, 5000, 6000, 9000, 10000, 100000, 1000000, 10000000, 0 };
  uint32_t SSBStepSize[9] = { 10, 100, 1000, 5000, 10000, 100000, 1000000, 10000000, 0 };
  uint32_t FMStepSize[9] = { 1000, 5000, 6250, 12500, 25000, 100000, 1000000, 10000000, 0 };
  const char* AMLabels[] = { "1K", "5K", "6K", "9K", "10K", "100K", "1M", "10M" };
  const char* SSBLabels[] = { "10Hz", "100Hz", "1K", "5K", "10K", "100K", "1M", "10M" };
  const char* FMLabels[] = { "1K", "5K", "6.25K", "12.5K", "25K", "100K", "1M", "10M" };

  if (pressed && mode == 0) {

    draw8Buttons(TFT_BTNCTR, TFT_BTNBDR);

    tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest row of main screen

    int positions[][2] = { { 21, 198 }, { 105, 198 }, { 185, 198 }, { 275, 198 }, { 21, 255 }, { 105, 255 }, { 185, 255 }, { 275, 255 } };

    tft.setTextColor(TFT_GREEN);

    for (int i = 0; i < 8; i++) {
      tft.setCursor(positions[i][0], positions[i][1]);
      if (modType == AM)
        tft.print(AMLabels[i]);
      if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
        tft.print(SSBLabels[i]);
      if (modType == FM)  // for future use
        tft.print(FMLabels[i]);
    }

    tft.setTextColor(textColor);

    tDoublePress();  // wait for touch

    pressed = get_Touch();
    column = tx / HorSpacing;
    row = 1 + ((ty - 20) / vTouchSpacing);


    if (row < 3) {  // outside of area
      pressed = 0;
      tx = 0;
      ty = 0;
      return;
    }


    if (row == 3)
      pos = column;
    if (row == 4)
      pos = column + 4;


    if (modType == AM)
      STEP = AMStepSize[pos];
    if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
      STEP = SSBStepSize[pos];
    if (modType == FM)
      STEP = FMStepSize[pos];
  }


  // mode 1 and mode -1 are used when touching the STEP display either on left or right side
  if (mode) {

    int i = 1;
    bool isIncrement = (mode == 1);

    if (modType == AM) {
      while (AMStepSize[i] != STEP)
        i++;
      STEP = isIncrement ? AMStepSize[i + 1] : AMStepSize[i - 1];
    }

    if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {
      while (SSBStepSize[i] != STEP)
        i++;
      STEP = isIncrement ? SSBStepSize[i + 1] : SSBStepSize[i - 1];
    }

  }  //endif mode

  tRel();
  tx = ty = pressed = 0;
  displaySTEP();

  FREQ /= STEP;  // snap FREQ to STEP
  FREQ *= STEP;

  redrawMainScreen = true;
  mainScreen();
}
//##########################################################################################################################//

void readTouchCoordinates() {

  pressed = get_Touch();  // Pressed will be set true if there is a valid touch on the screen this method is very slow

#ifdef DEBUG
  tft.fillRect(340, 10, 140, 15, TFT_BLACK);
  tft.setTextColor(textColor);
  tft.setCursor(340, 10);
  tft.printf("x:%d y:%d pr%d", tx, ty, pressed);
#endif

  if (pressed && ty > 295 && tx > 150 && tx < 300)  // sMode selects btw off, low resolution, high resolution, mini osci. audio waterfall.
    sMode++;

  indicatorTouch();  //when indicators get touched directly
}
//##########################################################################################################################//


void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color1, uint32_t color2) {  // draws buttons as plain rectangles or sprites

  if (altStyle) {  //draw plain buttons

    tft.fillRectVGradient(x, y + 4, w, h / 2, color1, color2);
    tft.fillRectVGradient(x, y + (h / 2) + 4, w, h / 2, color2, color1);
    //if (w < 130)  // silver frame around small buttons
    tft.drawRect(x, y, w, h, TFT_LIGHTGREY);
  }


  else {  //draw sprites, use height as selection criteria

    if (h == 50) {
      spr.createSprite(SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT);
      spr.pushImage(0, 0, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[buttonSelected]);  // draw selected button sprite
      spr.pushSprite(x, y + 4);
      spr.deleteSprite();
    }

    if (h == 78) {
      spr.createSprite(130, 80);
      spr.pushImage(0, 0, 130, 80, (uint16_t*)But3);  // draw gig green neon sprite for big buttons
      spr.pushSprite(x, y);
      spr.deleteSprite();
    }

    if (h == 30) {
      spr.createSprite(80, 25);
      spr.pushImage(0, 0, 80, 25, (uint16_t*)buttonImages[buttonSelected]);  // draw half button
      spr.pushSprite(x, y);
      spr.deleteSprite();
    }
  }
}
//##########################################################################################################################//

void draw8Buttons(uint16_t color1, uint16_t color2) {  // draws 2 rows with 4 buttons each

  int h = 8;
  tft.fillRect(2, 48, 337, 8, TFT_BLACK);
  tft.fillRect(2, 288, 337, 4, TFT_BLACK);  // overwrite remains of previous sprites
  for (int j = 0; j < 4; j++) {
    drawButton(h + j * 83, 235, TILE_WIDTH, TILE_HEIGHT, color1, color2);
    drawButton(h + j * 83, 178, TILE_WIDTH, TILE_HEIGHT, color1, color2);
  }
}
//##########################################################################################################################//

void draw16Buttons() {  // draws 4 rows with 4 buttons each

  uint16_t yb = 58;
  int h = 6;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      drawButton(h + j * 83, (i + 1) * yb, 78, 50, TFT_DARKGREY, TFT_BTNCTR);
    }
  }
}

//##########################################################################################################################//


void Smeter() {
  // modified from ATS 25 code
  int spoint;
  static int s = 0;
  static int oldS = 0;

  si4735.getCurrentReceivedSignalQuality(0);
  signalStrength = si4735.getCurrentRSSI();
  si4735.getCurrentReceivedSignalQuality(0);
  SNR = si4735.getCurrentSNR();


  if (modType == AM)
    s = signalStrength - GAINREDUCTION;
  else
    s = signalStrength - GAINREDUCTION + 8;  // to compensate, for some reason  signalStrength in SSB shows about 8 db less

  if (showSNR) {

    if (altStyle)
      tft.fillRect(290, 62, 45, 30, TFT_NAVY);
    else
      tft.fillRect(290, 62, 45, 30, TFT_BLACK);

    tft.setCursor(290, 64);
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW);
    tft.printf("%ddBuV", s);
    tft.setCursor(290, 82);
    tft.printf("SNR:%d", SNR);
    tft.setTextSize(2);
  }

  else {
    etft.fillRect(290, 62, 45, 12, TFT_BLACK);
    etft.setCursor(290, 64);
    etft.setTTFFont(Arial_9);
    etft.setTextColor(TFT_YELLOW);
    etft.printf("%ddB", s);
  }


  /*
  if ((s >= 0) and (s <= 5)) spoint = 12;                     // S0
  if ((s > 5) and (s <= 8)) spoint = 24;                      // S1
  if ((s > 8) and (s <= 12)) spoint = 36;                     // S2
  if ((s > 12) and (s <= 16)) spoint = 48;                    // S3
  if ((s > 16) and (s <= 20)) spoint = 48 + (s - 4) * 2;      // S4
  if ((s > 20) and (s <= 30)) spoint = 60 + (s - 10) * 2;     // S5
  if ((s > 30) and (s <= 40)) spoint = 72 + (s - 16) * 2;     // S6
  if ((s > 40) and (s <= 50)) spoint = 84 + (s - 22) * 2;     // S7
  if ((s > 50) and (s <= 62)) spoint = 96 + (s - 28) * 2;     // S8
  if ((s > 62) and (s <= 74)) spoint = 108 + (s - 34) * 2;    // S9
  if ((s > 74) and (s <= 86)) spoint = 124 + (s - 44) * 2;    // S9 +10
  if ((s > 86) and (s <= 98)) spoint = 140 + (s - 54) * 2;    // S9 +20
  if ((s > 98) and (s <= 110)) spoint = 156 + (s - 64) * 2;   // S9 +30
  if ((s > 110) and (s <= 122)) spoint = 172 + (s - 74) * 2;  // S9 +40
  if ((s > 122) and (s <= 125)) spoint = 188 + (s - 84) * 2;  // S9 +50
  if (s > 125) spoint = 204;                                  // S9 +60

*/


  if ((s >= 0) and (s <= 5)) spoint = 12;                      // S0
  if ((s > 5) and (s <= 8)) spoint = 24;                       // S1
  if ((s > 8) and (s <= 12)) spoint = 36;                      // S2
  if ((s > 12) and (s <= 16)) spoint = 48;                     // S3
  if ((s > 16) and (s <= 20)) spoint = 48 + (s - 16) * 4;      // S4 (stretched)
  if ((s > 20) and (s <= 30)) spoint = 64 + (s - 20) * 4;      // S5 (stretched)
  if ((s > 30) and (s <= 40)) spoint = 104 + (s - 30) * 4;     // S6 (stretched)
  if ((s > 40) and (s <= 50)) spoint = 144 + (s - 40) * 4;     // S7 (stretched)
  if ((s > 50) and (s <= 62)) spoint = 184 + (s - 50) * 4;     // S8 (stretched)
  if ((s > 62) and (s <= 74)) spoint = 232 + (s - 62) * 4;     // S9 (stretched)
  if ((s > 74) and (s <= 86)) spoint = 280 + (s - 74) * 2;     // S9 +10 (compressed)
  if ((s > 86) and (s <= 98)) spoint = 304 + (s - 86) * 2;     // S9 +20 (compressed)
  if ((s > 98) and (s <= 110)) spoint = 328 + (s - 98) * 2;    // S9 +30 (compressed)
  if ((s > 110) and (s <= 122)) spoint = 352 + (s - 110) * 2;  // S9 +40 (compressed)
  if ((s > 122) and (s <= 125)) spoint = 376 + (s - 122) * 2;  // S9 +50 (compressed)
  if (s > 125) spoint = 382;                                   // S9 +60 (compressed)


  if (modType == WBFM)
    spoint *= 2;  // rough wideband FM correction


  int tik = 0;
  int met = spoint + 2;

  // overwrite previous bars
  if (showSNR) {

    if (s < oldS) {  //only overwrite when signal decreasing
      if (altStyle)
        tft.fillRect(Xsmtr, Ysmtr + 19, 280, 6, TFT_NAVY);
      else
        tft.fillRect(3, Ysmtr + 19, 280, 6, TFT_BLACK);
    }
  }

  else {
    if (s < oldS) {
      if (altStyle)
        tft.fillRect(Xsmtr, Ysmtr + 20, 315, 15, TFT_NAVY);
      else
        tft.fillRect(3, Ysmtr + 20, 335, 15, TFT_BLACK);
    }
  }


  while (met > 11 and tik < 19) {

    uint16_t input = 16 * tik;
    uint8_t red, green, blue;

    if (input > 180) {
      // White color over S9
      red = 31;
      green = 63;
      blue = 31;
    } else if (input <= 90) {
      // Red to Orange to yellow change
      red = 31;
      green = (63 * input) / 90;
      blue = 0;
    } else if (input <= 180) {
      // Yellow to green change
      red = 31 - (31 * (input - 90)) / 90;
      green = 63;
      blue = 0;
    }

    // Combine to rgb565
    uint16_t rgb565 = (red << 11) | (green << 5) | blue;

    if (showSNR) {

      if (tik) tft.fillRect(Xsmtr + 16 + (tik * 15.6), Ysmtr + 19, 13, 6, rgb565);
      else if (s) tft.fillRect(Xsmtr + 9, Ysmtr + 19, 19, 6, TFT_RED);
      met -= 15;
      tik++;

      int snrColor = (2000 - 50 * SNR);
      if (snrColor < 0)
        snrColor = 0;
      int snrX = 9 * SNR;
      if (snrX > 300)
        snrX = 300;

      tft.fillRect(Xsmtr - 5, Ysmtr + 27, 280, 3, TFT_GREY);
      tft.fillRect(Xsmtr + 10 + snrX, Ysmtr + 27, 10, 2, valueToWaterfallColor(snrColor));

    }

    else {

      if (tik) tft.fillRect(Xsmtr + 16 + (tik * 15.6), Ysmtr + 21, 13, 9, rgb565);
      else if (s) tft.fillRect(Xsmtr + 9, Ysmtr + 21, 19, 9, TFT_RED);
      met -= 15;
      tik++;
    }
  }

  oldS = s;
}

//##########################################################################################################################//

void DrawSmeter() {  // modified ATS 25 code

  String IStr;
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextDatum(BC_DATUM);

  for (int i = 0; i < 10; i++) {
    IStr = String(i);
    tft.setCursor((Xsmtr + 5 + (i * 16) * 1.3), Ysmtr + 8);
    tft.print(i);
  }

  for (int i = 1; i < 5; i++) {
    IStr = String(i * 10);
    tft.setCursor((Xsmtr + 165 + (i * 15) * 1.3), Ysmtr + 8);
    if ((i == 2) or (i == 4) or (i == 6)) {
      tft.print(" +");
      tft.print(i * 10);
    }
  }

  tft.setTextSize(2);
}
//##########################################################################################################################//


void loopTimer() {  // displays loop cycle time


  static bool blink = false;
  blink = !blink;
  if (blink)
    tft.fillCircle(470, 10, 3, TFT_BLACK);
  else
    tft.fillCircle(470, 10, 3, TFT_BLUE);  // main loop blinker

  if (displayDebugInfo == false)
    return;

  long now;
  static long before;
  long cycleTime;
  now = micros();  // measure loop time
  cycleTime = now - before;
  before = now;
  tft.fillRect(280, 296, 120, 23, TFT_BLACK);
  tft.setCursor(280, 300);
  tft.setTextColor(textColor);
  cycleTime /= 50000;  // convert microseconds to seconds and divide by 50 since it gets called only 1 out of 50 loops
  if (cycleTime < 1000)
    tft.printf("Loop:%ldms", cycleTime);
}


//##########################################################################################################################//

void readSquelchPot(bool draw) {  // reads value of squelch potentiometer and if draw is true displays a circle

  static long oldPot = 0;
  const int div = 13;
  uint16_t current;
  uint16_t pot;


  for (int l = 0; l < 35; l++)
    current = analogRead(SQUELCH_POT);
  pot = current / div;  // oversample and  set range

  if (pot <= oldPot - 2 || pot >= oldPot + 2 || redrawMainScreen) {  // eliminate A/D convrter noise, update only when  pot turned

    disableSpectrum = true;  //temporarily disable spectrum analyzer to  read ADC without interruption

    currentSquelch = pot / 2;  //  set range to 0 - 127


    if (!draw)  // do not draw squelch position circle (when squelch used in memo functions)
      return;

    tft.fillRect(oldPot + 8, 53, 14, 10, TFT_BLACK);
    tft.drawFastHLine(0, 57, 340, TFT_GRID);

    if (SNRSquelch)
      return;

    if (signalStrength > currentSquelch)
      tft.fillCircle(pot + 12, 57, 4, TFT_GREEN);
    else
      tft.fillCircle(pot + 12, 57, 4, TFT_MAGENTA);
    oldPot = pot;
  }
}

//##########################################################################################################################//

void setSquelch() {
  bool mute = !SNRSquelch ? signalStrength < currentSquelch : !SNR;

  if (audioMuted != mute) {
    audioMuted = mute;
    si4735.setAudioMute(mute);  // set both software and hardware mute for better noise supression
    si4735.setHardwareAudioMute(mute);
  }
}
//##########################################################################################################################//

void displayText(int x, int y, int length, int height, const char* text) {  // helper to display text
  tft.fillRect(x, y, length, height, TFT_BLACK);
  tft.setCursor(x, y);
  tft.print(text);
}

//##########################################################################################################################//

void indicatorTouch() {
  // Change step, modulation, bandwidth, AGC when touching the indicator areas

  int upper_y = 65;
  int lower_y = 103;

  // Toggle squelch mode
  if (pressed && tx < 340 && ty > 20 && ty < 50) {
    SNRSquelch = !SNRSquelch;
    tRel();

    if (SNRSquelch) {
      tft.fillRect(3, 51, 337, 13, TFT_BLACK);
    }

    tft.drawFastHLine(0, 57, 340, TFT_GRID);
    pressed = tx = ty = 0;
  }

  // Toggle AM, LSB, USB
  if (pressed && tx > 3 && tx < 40 && ty > upper_y && ty < lower_y) {
    modType++;

    if (modType > 3) {
      modType = 1;
    }

    loadModulation4735();
    printModulation();
    printBandWidth();
    tRel();
    pressed = tx = ty = 0;
  }

  // Step down (decrease frequency step)
  if (pressed && tx > 330 && tx < 395 && ty < 20) {
    setSTEP(-1);
    tRel();
  }

  // Step up (increase frequency step)
  if (pressed && tx > 415 && tx < 479 && ty < 20) {
    setSTEP(1);
    tRel();
  }

  // Bandwidth down
  if (pressed && tx > upper_y && tx < 108 && ty > upper_y && ty < lower_y) {
    setBandwidth(-1);
    tRel();
  }

  // Bandwidth up
  if (pressed && tx > 108 && tx < 155 && ty > upper_y && ty < lower_y) {
    setBandwidth(1);
    tRel();
  }

  // Toggle TinySA mode
  if (pressed && tx > 173 && tx < 215 && ty > upper_y && ty < lower_y) {
    tinySA_RF_Mode = !tinySA_RF_Mode;

    if (tinySA_RF_Mode) {
      digitalWrite(TINYSA_PIN, HIGH);
    } else {
      digitalWrite(TINYSA_PIN, LOW);
    }

    printTinySAMode();
  }

  // AGC/ATT down
  if (pressed && tx > 235 && tx < 260 && ty > upper_y && ty < lower_y)
    setAGCMode(-1);

  // AGC/ATT up
  if (pressed && tx > 310 && tx < 325 && ty > upper_y && ty < lower_y)
    setAGCMode(1);
}



//##########################################################################################################################//

void sineTone(float frequency, int duration) { // sine wave generator for touch sound
  const int sampleRate = 10000;
  const int numSamples = sampleRate * duration / 1000;
  const float amplitude = 128;  
  const float offset = 128;

  dac_output_enable(DAC_PIN); 

  for (int i = 0; i < numSamples; i++) {
    float sample = sin(2 * M_PI * frequency * i / sampleRate) * amplitude + offset;

    //  fade out
    if (i > numSamples * 0.8) {
      float fadeFactor = 1.0 - (float)(i - numSamples * 0.8) / (numSamples * 0.2);
      sample = (sample - offset) * fadeFactor + offset;
    }

    dac_output_voltage(DAC_PIN, (uint8_t)sample);
    delayMicroseconds(1000000 / sampleRate);
  }
  dac_output_disable(DAC_PIN);
}
