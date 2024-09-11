void tuneSI5351() {

  LO_TX = FREQ;

  getFilterNumber();
  clockSelector();  // determine which clock to use


  if (FREQ > SWITCHPOINT) {  // use 3rd harmonics of CLK0
    LO_RX = abs((SI4735TUNE_FREQ * 1000) + (LOAboveRF ? FREQ : -FREQ)) / 3;
  } else {  // normal mode
    LO_RX = abs((SI4735TUNE_FREQ * 1000) + (LOAboveRF ? FREQ : -FREQ));
  }
  // Update the clock info
  if (displayDebugInfo == true) {
    tft.fillRect(0, 296, 150, 23, TFT_BLACK);
    tft.setCursor(5, 300);
    tft.setTextColor(textColor);

    if (rxmode) {
      if (CLK2ACTIVE)
        tft.printf("CLK2:%ld", LO_RX / 1000);
      else {
        tft.setTextColor(TFT_SKYBLUE);
        tft.printf("CLK0:%ld", (long)((float)LO_RX / 333.3333));
        tft.setTextColor(textColor);
      }
    } else {
      tft.printf("CLK1:%ld", LO_TX / 1000);
    }
  }

  if (SI4735directFreqInput) {
    setFreqSI4735();  // only used for WBFM
    return;
  }


  if (rxmode) {
    if (CLK2ACTIVE)
      si5351.set_freq(LO_RX * 100ULL, SI5351_CLK2);  // 0-200NHz
    if (CLK0ACTIVE)
      si5351.set_freq(LO_RX * 100ULL, SI5351_CLK0);  //200-500MHz clock line runs through high pass filter
  }
  if (txmode) {
    si5351.set_freq(LO_TX * 100ULL, SI5351_CLK1);
  }
}


//##########################################################################################################################//


void settleSynth() {  //This may or may not be needed. My SI5351 can't switch FREQ from one extreme to the other reliably in one step, the pll falls out of lock so a short time at 110MHz is needed
  delay(10);
  if (CLK2ACTIVE)
    si5351.set_freq(110000000 * 100ULL, SI5351_CLK2);
  if (CLK0ACTIVE)
    si5351.set_freq(110000000 * 100ULL, SI5351_CLK0);
  delay(10);
}


//##########################################################################################################################//

void fineTune() {  // reads fine tune potentiometer and adjusts FREQ

  static  uint32_t lastResult = 0;
  static long oldPot = 0;
  uint32_t os = 0;
  const float div = 10.92;
  long potResult = 0;
  static uint16_t lastPos = 0;

  for (int l = 0; l < 32; l++)
    os += analogRead(FINETUNE_PIN);
  potVal = os / div;  // oversample and  set range from 0 to 12000 (= +-1 crystal filter bandwith)
 
 if (lastResult == potVal)
   return;
 else 
    lastResult = potVal;

  // Normalize the ADC value to a range of -1 to 1
  double normalized_value = ((double)potVal / 12000) * 2 - 1;

  // quadratic transformation to stretch values around the center
  double transformed_value = normalized_value * normalized_value * (normalized_value < 0 ? -1 : 1);

  // Normalize back to 0 to 1 range
  transformed_value = (transformed_value + 1) / 2;

  // Scale the transformed value back to the ADC range
  potVal = (uint16_t)(transformed_value * 12000);

  switch (modType) {  // adjust value from -1 to +1 bandwidth
    case AM:
      potResult = (potVal - 6000) / 50 * 50;  // AM in 50 Hz Steps
      break;
    case USB:
    case LSB:
    case SYNC:
    case CW:
      potResult = (potVal - 6000) / 50 * 25;  // SSB in 25Hz
      break;
    case FM:
      potResult = (potVal - 6000) / 50 * 125;  //FM in 125 Hz
      break;
  }

  if (potResult <= oldPot - 10 || potResult >= oldPot + 10) {  // pot has changed
     disableSpectrum = true; //temporarily disable spectrum analyzer to continuously read values
    
    if (lastPos)
      tft.fillCircle(lastPos, 47, 4, TFT_BLACK);  // override previous dot
    tft.drawFastHLine(lastPos - 5, 46, 15, TFT_GRID);

    if (!potResult)
      tft.fillCircle(potVal / 37 + 10, 47, 4, TFT_GREEN);  // green center dot
    else
      tft.fillCircle(potVal / 37 + 10, 47, 4, TFT_RED);  // create moving dot
    lastPos = potVal / 37 + 10;

    FREQ -= oldPot;
    FREQ += potResult;
    oldPot = potResult;
  }
}

//##########################################################################################################################//

void saveCurrentSettings() {  // saves current FREQ, Bandwidth and Modulation after 1 minute of inactivity

  static uint32_t start;
  static bool write = false, written = false;
  const uint32_t writeDelay = 60000;  // write freq after 1 minute to EEPROM

  if (FREQ == FREQ_OLD && write == false) {
    start = millis();
    write = true;
  }

  if (write && (millis() >= start + writeDelay) && (!written) && (FREQ == FREQ_OLD)) {
    preferences.putLong("lastFreq", FREQ);
    preferences.putInt("lastBw", bandWidth);
    preferences.putChar("lastMod", modType);
    written = true;
  }

  if (FREQ != FREQ_OLD) {
    write = false;
    written = false;
  }
}
//##########################################################################################################################//

void setFreqSI4735() {  // For WBFM
  if (FREQ > FREQ_OLD)
    si4735.frequencyUp();
  if (FREQ < FREQ_OLD) {
    si4735.frequencyDown();
  }
  if (modType != WBFM)
    FREQ = 1000 * si4735.getCurrentFrequency();
  else
    FREQ = 10000 * si4735.getCurrentFrequency();
  return;
}

//##########################################################################################################################//
void clockSelector() { // Selects btw. CLK2 (0-200 MHz) and CLK0 (200-500 MHz). Both feed the AD831 via -6db attenuators. 
//CLK0 runs through a highpass filter that eliminates the main wave and filters the 3rd harmonics. 
 
  static bool CLK2set = false;
  static bool CLK0set = false;


  if (FREQ > SWITCHPOINT) {
    CLK2ACTIVE = false;
    CLK0ACTIVE = true;
  }

  else {
    CLK2ACTIVE = true;
    CLK0ACTIVE = false;
  }

  if (CLK2ACTIVE && CLK2set == false) {  // use CLK2, low band
    si5351.output_enable(SI5351_CLK0, 0);
    si5351.output_enable(SI5351_CLK2, 1);
    si5351.output_enable(SI5351_CLK1, 0);
    si5351.set_freq(0, SI5351_CLK0);
    si5351.set_freq(0, SI5351_CLK1);
    displayFREQ(FREQ);  // display new FREQ
    CLK2set = true;
    CLK0set = false;
    settleSynth();
  }

  if (CLK0ACTIVE && CLK0set == false)  //Use CLK0, high band
  {                                    
    si5351.set_freq(0, SI5351_CLK2);
    si5351.set_freq(0, SI5351_CLK1);
    si5351.output_enable(SI5351_CLK0, 1);
    si5351.output_enable(SI5351_CLK2, 0);
    si5351.output_enable(SI5351_CLK1, 0);
    displayFREQ(FREQ);  // display new FREQ
    CLK2set = false;
    CLK0set = true;
    settleSynth();
  }
}


//##########################################################################################################################//

  void getFilterNumber() {
  long FREQKHz = FREQ / 1000;
  static int oldIMode = -1;
  const int filterCount = sizeof(filter) / sizeof(filter[0]);

  for (int i = 0; i < filterCount; i++) {
    if (FREQKHz >= filter[i].startFreqKHz && FREQKHz <= filter[i].stopFreqKHz) {
      if (filter[i].injectionMode != oldIMode) {  // change injection mode when needed
        oldIMode = filter[i].injectionMode;
        LOAboveRF = filter[i].injectionMode;
        displayFREQ(FREQ); // retrigger FREQ display with the right color
      }
      filterNumber = filter[i].FilterNumber;
      switchFilter(); // activate the filter 
    }
  }
}

//##########################################################################################################################//




void switchFilter () {
/* This function uses 3 GPIOs to drive a 74LS138 which then drives 6 pnp transistors to select 1 of 6 bandpasses

                           74LS138 Logic Table
              +-------------------------------+
Enable Inputs      | Pin 4 and 5 must be low, Pin 6 high
                   +-------------------------------+
Select Inputs      | C (Pin 3) B (Pin 2) A (Pin 1) |
                   +-------------------------------+
Outputs            |  Y0 (Pin 15) Y1 (Pin 14) Y2 (Pin 13) Y3 (Pin 12) Y4 (Pin 11) Y5 (Pin 10) Y6 (Pin 9) Y7 (Pin 7)  |
                   +-------------------------------+


Logic table after inversion with pnp transistors:

+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
| G2A| G2B| G1 | C  | B  | A  | Y0 | Y1 | Y2 | Y3 | Y4 | Y5 | Y6 | Y7 |
| (4)| (5)| (6)| (3)| (2)| (1)|(15)|(14)|(13)|(12)|(11)|(10)| (9)| (7)|
+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
| 1  |  X |  X |  X |  X |  X | 0  | 0  | 0  | 0  | 0  | 0  | 0  | 0  |
|  X |  1 |  X |  X |  X |  X | 0  | 0  | 0  | 0  | 0  | 0  | 0  | 0  |
|  X |  X |  0 |  X |  X |  X | 0  | 0  | 0  | 0  | 0  | 0  | 0  | 0  

|  0 |  0 |  1 |  0 |  0 |  0 | 1  | 0  | 0  | 0  | 0  | 0  | 0  | 0  |
|  0 |  0 |  1 |  0 |  0 |  1 | 0  | 1  | 0  | 0  | 0  | 0  | 0  | 0  |
|  0 |  0 |  1 |  0 |  1 |  0 | 0  | 0  | 1  | 0  | 0  | 0  | 0  | 0  |
|  0 |  0 |  1 |  0 |  1 |  1 | 0  | 0  | 0  | 1  | 0  | 0  | 0  | 0  |
|  0 |  0 |  1 |  1 |  0 |  0 | 0  | 0  | 0  | 0  | 1  | 0  | 0  | 0  |
|  0 |  0 |  1 |  1 |  0 |  1 | 0  | 0  | 0  | 0  | 0  | 1  | 0  | 0  |
|  0 |  0 |  1 |  1 |  1 |  0 | 0  | 0  | 0  | 0  | 0  | 0  | 1  | 0  |
|  0 |  0 |  1 |  1 |  1 |  1 | 0  | 0  | 0  | 0  | 0  | 0  | 0  | 1  |
+----+----+----+----+----+----+----+----+----+----+----+----+----+----+


connect GPIO 13 to A (pin 1)
connect GPIO 14 to B (pin2)
connect GPIO 25 to C (pin3)
Outputs go to the bases of 6 pnp transistors via 680 Ohms.
Their emitters are on +5V and collectors drive the pin diodes.

*/

  static int oldFilterNumber = -1;

  if (filterNumber != oldFilterNumber) {
     oldFilterNumber = filterNumber;
    switch (filterNumber) {
      case 1:  // 74LS138 Pin 15 low
        digitalWrite(13, LOW);
        digitalWrite(14, LOW);
        digitalWrite(25, LOW);
        break;
      case 2:  // 74LS138 Pin 14 low
        digitalWrite(13, HIGH);
        digitalWrite(14, LOW);
        digitalWrite(25, LOW);
        break;
      case 3:  // 74LS138 Pin 13 low
        digitalWrite(13, LOW);
        digitalWrite(14, HIGH);
        digitalWrite(25, LOW);
        break;
      case 4:  // 74LS138 Pin 12 low
        digitalWrite(13, HIGH);
        digitalWrite(14, HIGH);
        digitalWrite(25, LOW);
        break;
      case 5:  // 74LS138 Pin 11 low
        digitalWrite(13, LOW);
        digitalWrite(14, LOW);
        digitalWrite(25, HIGH);
        break;
      case 6:  // 74LS138 Pin 10 low
        digitalWrite(13, HIGH);
        digitalWrite(14, LOW);
        digitalWrite(25, HIGH);
        break;
      case 7:  // 74LS138 Pin 9 low, not in use
        digitalWrite(13, LOW);
        digitalWrite(14, HIGH);
        digitalWrite(25, HIGH);
        break;
      case 8:  // 74LS138 Pin 7 high, got future use
        digitalWrite(13, HIGH);
        digitalWrite(14, HIGH);
        digitalWrite(25, HIGH);
        break;
    }
  }
}
