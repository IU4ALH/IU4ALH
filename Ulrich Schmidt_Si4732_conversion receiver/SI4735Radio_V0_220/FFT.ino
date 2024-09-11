//##########################################################################################################################//

// Functions for audio analysis. This is really unnecessary, but just looks fun.
// 2 audio spectrum Analyzers, mini waterfall and mini oscilloscope
// MUST compile with arduinoFFT version 1.60 !!!! Newer versions do not work
// Also includes a CW decoder using the Goertzel algorithm
// Idea from here: https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_03.ino


int Rpeak[24] = { 0 };
double RvReal[SAMPLES];
double RvImag[SAMPLES]; 
const int startX = 150;
const int startY = 293;
static char oldsMode = 0;
static int offset = 0;
static bool mStat = false;
int ampl;



void audioSpectrum() {
  
  if (oldsMode != sMode) {  //  sMode selects btw off, low resolution, high resolution, mini osci. audio waterfall. Spectrums display 8 KHz, waterfall 3KHz
     oldsMode = sMode;
    if (sMode == 5)
      sMode = 0;

    preferences.putChar("spectr", sMode);                  // save mode
    tft.fillRect(startX - 5, startY,122, 26, TFT_BLACK);  // overwrite last window
  }
  
  
  if (audioMuted && !pressed) {

    if (!mStat) {
      mStat = true;
      tft.fillRect(startX, startY, 128, 26, TFT_BLACK);  // overwrite when muted
      tft.setCursor(startX + 25, startY + 4);
      tft.setTextColor(TFT_YELLOW);
      tft.printf("MUTED");
      tft.setTextColor(textColor);
    }
    return;
  }


  if (mStat) {
    tft.fillRect(startX, startY, 100, 25, TFT_BLACK);
    mStat = false;
  }


 if (sMode == 0 && !pressed) {
    return;
  }
  

  if (sMode == 3) // mini oscilloscope
    tft.fillRect(startX, startY, 86, 25, TFT_NAVY);  //background
 
  for (int i = 0; i < SAMPLES; i++) {
   RvReal[i] = analogRead(AUDIO_INPUT_PIN);  // sampling
   RvImag[i] = 0;
   // no sampling period delay reults in about 16KHz sample rate
      
    ampl = (RvReal[i] / 20) - 60;
    ampl = min(max(ampl, 0), 24);    // limit amp;itude mini osci 
    
if (sMode == 3 && i < 86 ) // Mini oscilloscope     
    tft.drawPixel(startX + i , startY + ampl, TFT_WHITE);  
  }


  FFT.Windowing(RvReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  
  FFT.Compute(RvReal, RvImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(RvReal, RvImag, SAMPLES);

  

  if (sMode == 1) {  // low resolution spectrum analyzer, uses 24 channels

    for (byte band = 0; band <= 23; band++) {  // overwrite black, slow decay
      int xPos = startX + 5 * band;
      int yPos = DISP_HEIGHT - 1 - Rpeak[band];

      tft.fillRect(xPos, yPos - 1, 3, 2, TFT_BLACK);

      if (Rpeak[band] >= 2)
        Rpeak[band] -= 2;
    }

    for (int i = 2; i < (SAMPLES / 2); i++) {  // only use lower half of bins
      if (RvReal[i] > 750) {
        byte band = getBandVal(i);
        displayBand(band, (int)RvReal[i]);
      }
    }
  }

  if (sMode == 2) {  // high resolution, 85 channels

    for (int i = 2; i < SAMPLES / 3; i++) {
      // calculate the height
      int height = DISP_HEIGHT - (int)(RvReal[i] / 5);
      height = (height < startY) ? startY : height;

      for (int j = 0; j < 5; j++) {
        tft.drawPixel(startX + i, height + offset + j, TFT_BLACK);
      }
      offset = (offset + 5) % 23;

      int ampl = DISP_HEIGHT - (int)(RvReal[i] / amplitude);
      ampl = (ampl < startY) ? startY : ampl;

      tft.drawFastVLine(startX + i, ampl, DISP_HEIGHT - ampl, TFT_SKYBLUE);
    }
  }

  if (sMode == 4) {  // audio waterfall

    const int width = 84;
    const int height = 24;
    const int gradient = 100;

    // shift rows
    for (int i = height; i >= 0; i--) {
      for (int j = 0; j < width; j++) {
        wabuf[i + 1][j] = wabuf[i][j];
      }
    }

    // fill first row again
    for (int j = 0; j < width; j++) {
      wabuf[0][j] = (int)(RvReal[j / 2 + 2] / amplitude);  // reduce bandwidth to 3 KHz
    }

    // draw pixels
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        tft.drawPixel(startX + j, startY + i, valueToWaterfallColor(gradient * wabuf[i][j]));
      }
    }
  }

  if (pressed)
    tRel();
}
//##########################################################################################################################//

byte getBandVal(int i) { // values for 24 channel spectrum analyzer


  if (i <= 2) return 0;
  if (i <= 4) return 1;
  if (i <= 6) return 2;
  if (i <= 8) return 3;
  if (i <= 10) return 4;
  if (i <= 12) return 5;
  if (i <= 14) return 6;
  if (i <= 17) return 7;
  if (i <= 21) return 8;
  if (i <= 26) return 9;
  if (i <= 31) return 10;
  if (i <= 36) return 11;
  if (i <= 42) return 12;
  if (i <= 48) return 13;
  if (i <= 55) return 14;
  if (i <= 62) return 15;
  if (i <= 70) return 16;
  if (i <= 78) return 17;
  if (i <= 87) return 18;
  if (i <= 97) return 19;
  if (i <= 108) return 20;
  if (i <= 120) return 21;
  if (i <= 133) return 22;
  return 23;
}

//##########################################################################################################################//

void displayBand(int band, int dsize) {
  int dmax = 23;
  dsize /= amplitude;
  if (dsize > dmax) dsize = dmax;
  for (int s = 0; s <= dsize; s++) {
    tft.drawPixel(startX + 5 * band, DISP_HEIGHT - 1 - s, TFT_YELLOW);
    tft.drawPixel(startX + 5 * band + 1, DISP_HEIGHT - 1 - s, TFT_YELLOW);
    tft.drawPixel(startX + 5 * band + 2, DISP_HEIGHT - 1 - s, TFT_YELLOW);
  }
  if (dsize > Rpeak[band])
    Rpeak[band] = dsize;
}

//##########################################################################################################################//
// CW decoder and helper functions


void tuneCWDecoder() {  // shows a small waterfall that helps tune beat to 558 Hz.


  tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);
  tft.setCursor(0, 150);
  tft.setTextColor(TFT_YELLOW);
  tft.print("Align signal with center bar.\n\nAudio frequency should be 558Hz.\n\nPress encoder to start decoding.\n\nPress again to leave.");
  tft.setTextColor(textColor);

  const int startX = 100;
  const int startY = 70;
  const int width = 84;
  const int height = 24;
  const int gradient = 100;

  tft.fillRect(startX + width, startY - 10, 3, 44, TFT_GREEN);  // center bar

  while (true) {

   if (clw || cclw) {  // fine tune
    FREQ += clw ? 25 : -25;
    displayFREQ(FREQ);
    tuneSI5351();
    clw = 0;
    cclw = 0;
}


    for (int i = 0; i < SAMPLES; i++) {
      RvReal[i] = analogRead(AUDIO_INPUT_PIN);  // do sampling
      RvImag[i] = 0;
      delayMicroseconds(300);  // 3.33 KHz sample rate
    }

    FFT.Windowing(RvReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // do computing
    FFT.Compute(RvReal, RvImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(RvReal, RvImag, SAMPLES);


    // Waterfall  shift rows
    for (int i = height; i >= 0; i--) {
      for (int j = 0; j < width; j++) {
        wabuf[i + 1][j] = wabuf[i][j];
      }
    }

    // fill first row again
    for (int j = 0; j < width; j++) {
      wabuf[0][j] = (int)(RvReal[j + 2] / amplitude);
    }

    // draw pixels
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < 2 * width; j += 2) {
        tft.drawPixel(startX + j, startY + i, valueToWaterfallColor(gradient * wabuf[i][j / 2]));
        tft.drawPixel(startX + j, startY + i + 1, valueToWaterfallColor(gradient * wabuf[i][j / 2]));
      }
    }

    if (digitalRead(ENC_PRESSED) == LOW)
      break;
  }
  while (digitalRead(ENC_PRESSED) == LOW)
    ;
}

//##########################################################################################################################//

/* MORSE DECODER
Modified from: https://github.com/G6EJD/ESP32-Morse-Decoder/blob/master/README.md
CW Decoder by Hjalmar Skovholm Hansen OZ1JHM  VER 1.01
 Feel free to change, copy or what ever you like but respect
 that license is http://www.gnu.org/copyleft/gpl.html
 Read more here http://en.wikipedia.org/wiki/Goertzel_algorithm 
 Adapted for the ESP32/ESP8266 by G6EJD  
*/


float magnitude = 0;
int magnitudelimit = 1000;      // adjusted volume, SI4735 volume needs to be set to 50
int magnitudelimit_low = 1000;  // adjusted volume, 
int realstate = LOW;
int realstatebefore = LOW;
int filteredstate = LOW;
int filteredstatebefore = LOW;

float coeff;
float Q1 = 0;
float Q2 = 0;
float sine;
float cosine;
float sampling_freq = 25000;  // adjusted from 45000 to 25000
float target_freq = 558.0;    // adjust for your needs see above
int n = 64;                   // if you change here please change next line also
int testData[64];             // adjusted from 128  to 64
float bw;

// Noise Blanker time which shall be computed so this is initial
int nbtime = 6;  /// ms noise blanker

long starttimehigh;
long highduration;
long lasthighduration;
long hightimesavg;
long lowtimesavg;
long startttimelow;
long lowduration;
long laststarttime = 0;
#define num_chars 20
char displayBuffer[num_chars + 1] = { 0 };
char CodeBuffer[10];
int stop = LOW;
int wpm, oldwpm = -1;
bool showCodeBuffer = false;
bool bwInit = false;


void CWDecoder() {

  target_freq = 558.0;  // target frequency
  bw = sampling_freq / n;
  int k;
  float omega;
  k = (int)(0.5 + ((n * target_freq) / sampling_freq));
  omega = (2.0 * PI * k) / n;
  sine = sin(omega);
  cosine = cos(omega);
  coeff = 2.0 * cosine;

  tuneCWDecoder();

  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.drawString("BW = " + String(bw, 0) + "Hz", 190, 80);

  FREQ_OLD -= 1;
  displayFREQ(FREQ);


  while (true) {

    if (digitalRead(ENC_PRESSED) == LOW)  // leave
      return;

    if (clw || cclw) {  // fine tune

      if (clw)
        FREQ += 25;
      if (cclw)
        FREQ -= 25;
      displayFREQ(FREQ);
      tuneSI5351();
      clw = 0;
      cclw = 0;
    }

    //////////////////////////////////// The basic goertzel calculation //////////////////////////////////////


    // Read analog input and do the Goertzel
    for (byte index = 0; index < n; index++) {
      testData[index] = analogRead(AUDIO_INPUT_PIN);
      float Q0;
      Q0 = coeff * Q1 - Q2 + (float)testData[index];
      Q2 = Q1;
      Q1 = Q0;
    }

    float magnitudeSquared = (Q1 * Q1) + (Q2 * Q2) - Q1 * Q2 * coeff;  // we do only need the real part //
    magnitude = sqrt(magnitudeSquared);
    Q2 = 0;
    Q1 = 0;

    if (magnitude > magnitudelimit_low) { magnitudelimit = (magnitudelimit + ((magnitude - magnitudelimit) / 6)); }  /// moving average filter
    if (magnitudelimit < magnitudelimit_low) magnitudelimit = magnitudelimit_low;


    // Now check the magnitude //
    if (magnitude > magnitudelimit * 0.6)  // just to have some space up
      realstate = HIGH;
    else
      realstate = LOW;

    // Clean up the state with a noise blanker //

    if (realstate != realstatebefore) { laststarttime = millis(); }
    if ((millis() - laststarttime) > nbtime) {
      if (realstate != filteredstate) {
        filteredstate = realstate;
      }
    }

    if (filteredstate != filteredstatebefore) {
      if (filteredstate == HIGH) {
        starttimehigh = millis();
        lowduration = (millis() - startttimelow);
      }

      if (filteredstate == LOW) {
        startttimelow = millis();
        highduration = (millis() - starttimehigh);
        if (highduration < (2 * hightimesavg) || hightimesavg == 0) {
          hightimesavg = (highduration + hightimesavg + hightimesavg) / 3;  // now we know avg dit time ( rolling 3 avg)
        }
        if (highduration > (5 * hightimesavg)) {
          hightimesavg = highduration + hightimesavg;  // if speed decrease fast ..
        }
      }
    }

    // Now check the baud rate based on dit or dah duration either 1, 3 or 7 pauses
    if (filteredstate != filteredstatebefore) {
      stop = LOW;
      if (filteredstate == LOW) {                                                        // we did end on a HIGH
        if (highduration < (hightimesavg * 2) && highduration > (hightimesavg * 0.6)) {  /// 0.6 filter out false dits
          strcat(CodeBuffer, ".");
          showCodeBuffer = true;
        }

        if (highduration > (hightimesavg * 2) && highduration < (hightimesavg * 6)) {
          strcat(CodeBuffer, "-");
          showCodeBuffer = true;
          wpm = (wpm + (1200 / ((highduration) / 3))) / 2;  //// the most precise we can do ;o)
        }
      }

      if (filteredstate == HIGH) {  //// we did end a LOW
        float lacktime = 1;
        if (wpm > 25) lacktime = 1.0;  ///  when high speeds we have to have a little more pause before new letter or new word
        if (wpm > 30) lacktime = 1.2;
        if (wpm > 35) lacktime = 1.5;
        if (lowduration > (hightimesavg * (2 * lacktime)) && lowduration < hightimesavg * (5 * lacktime)) {  // letter space
          CodeToChar();
          CodeBuffer[0] = '\0';
          showCodeBuffer = true;
          //AddCharacter('/');
          //Serial.print("/");
        }
        if (lowduration >= hightimesavg * (5 * lacktime)) {  // word space
          CodeToChar();
          CodeBuffer[0] = '\0';
          DisplayCharacter(' ');
          Serial.print(" ");
          showCodeBuffer = true;
        }
      }
    }
    if ((millis() - startttimelow) > (highduration * 6) && stop == LOW) {
      CodeToChar();
      CodeBuffer[0] = '\0';
      stop = HIGH;
      showCodeBuffer = true;
    }
    // the end of main loop clean up//
    realstatebefore = realstate;
    lasthighduration = highduration;
    filteredstatebefore = filteredstate;


    if (oldwpm != wpm) {
      tft.fillRect(0, 60, 110, 30, TFT_BLACK);
      tft.drawString("WPM = " + String(wpm), 50, 80);
      oldwpm = wpm;
    }

    if (showCodeBuffer) {
      tft.fillRect(260, 60, 219, 20, TFT_BLACK);
      tft.setTextColor(TFT_YELLOW);
      tft.setTextSize(3);
      CodeBuffer[7] = '\0';  // no more than 6 symbols
      tft.drawString(CodeBuffer, 370, 80);
      tft.setTextSize(2);
      tft.setTextColor(textColor);
      showCodeBuffer = false;
    }
  }
}

//##########################################################################################################################//

void DisplayCharacter(char newchar) {
  for (int i = 0; i < num_chars; i++) {
    displayBuffer[i] = displayBuffer[i + 1];
  }

  static int ctr = 0;

  // dark grey
  uint16_t targetR = 8;
  uint16_t targetG = 16;
  uint16_t targetB = 8;

  if (isprint(newchar)) {
    displayBuffer[num_chars] = newchar;
    tft.setTextSize(3);
    tft.fillRect(4, 112, 470, 38, TFT_DARKDARKGREY);
    for (int i = 0; i <= num_chars; i++) {
      tft.setCursor(10 + 20 * i, 120);


      // starting color
      uint16_t r = (TFT_WHITE >> 11) & 0x1F;
      uint16_t g = (TFT_WHITE >> 5) & 0x3F;
      uint16_t b = TFT_WHITE & 0x1F;

      // calculate fading
      r = r + ((targetR - r) * (num_chars - i) / num_chars);
      g = g + ((targetG - g) * (num_chars - i) / num_chars);
      b = b + ((targetB - b) * (num_chars - i) / num_chars);

      // combine again
      uint16_t fadedColor = (r << 11) | (g << 5) | b;
      tft.setTextColor(fadedColor);
      tft.setTextSize(3);
      tft.print(displayBuffer[i]);  // older letters fade to grey
    }

    static int y = 0;
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(ctr, 180 + y);
    tft.print(newchar);  // print 5 lines, start from 0 when filled
    ctr += 14;

    if (ctr > 460) {  //new line
      y += 22;
      ctr = 0;
    }

    if (y == 110) {  // reset when start 6th line
      y = 0;
      ctr = 0;
      tft.fillRect(0, 178, 479, 110, TFT_BLACK);
    }
  }
}


//##########################################################################################################################//

// Initialize the Morse code lookup table
std::unordered_map<std::string, char> morseToCharMap = {
  { ".-", 'a' }, { "-...", 'b' }, { "-.-.", 'c' }, { "-..", 'd' }, { ".", 'e' }, { "..-.", 'f' }, { "--.", 'g' }, { "....", 'h' }, { "..", 'i' }, { ".---", 'j' }, { "-.-", 'k' }, { ".-..", 'l' }, { "--", 'm' }, { "-.", 'n' }, { "---", 'o' }, { ".--.", 'p' }, { "--.-", 'q' }, { ".-.", 'r' }, { "...", 's' }, { "-", 't' }, { "..-", 'u' }, { "...-", 'v' }, { ".--", 'w' }, { "-..-", 'x' }, { "-.--", 'y' }, { "--..", 'z' }, { ".----", '1' }, { "..---", '2' }, { "...--", '3' }, { "....-", '4' }, { ".....", '5' }, { "-....", '6' }, { "--...", '7' }, { "---..", '8' }, { "----.", '9' }, { "-----", '0' }, { "..--..", '?' }, { ".-.-.-", '.' }, { "--..--", ',' }, { "-.-.--", '!' }, { ".--.-.", '@' }, { "---...", ':' }, { "-....-", '-' }, { "-..-.", '/' }, { "-.--.", '(' }, { "-.--.-", ')' }, { ".-...", '_' }, { "...-..-", '$' }, { "...-.-", '>' }, { ".-.-.", '<' }, { "...-.", '~' }, { ".-.-", 'ä' }, { "---.", 'ö' }, { ".--.-", 'à' }
};

void CodeToChar() {
  std::string codeStr(CodeBuffer);  // Convert CodeBuffer to a string

  auto it = morseToCharMap.find(codeStr);  // lookup
  if (it != morseToCharMap.end()) {
    char decode_char = it->second;  // Get the corresponding character
    DisplayCharacter(decode_char);
    Serial.print(decode_char);
  }
}

//##########################################################################################################################//
