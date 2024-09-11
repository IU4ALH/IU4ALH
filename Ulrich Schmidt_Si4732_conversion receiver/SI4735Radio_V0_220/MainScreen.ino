void mainScreen() {  // main screen does not block loop(), all other screens do

  if (redrawMainScreen) {  // redraw main screen when coming from a function that overwrites it
    drawMainButtons();
    DrawSmeter();
    printModulation();
    printBandWidth();
    printTinySAMode();
    printAGC();
    readSquelchPot(true);  // read and draw position
    redrawMainScreen = false;
  }
  readMainBtns();
}

//##########################################################################################################################//

void drawMainButtons() {


  if (!altStyle)  // restore background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  // plain buttons background

  draw8Buttons(TFT_BTNCTR, TFT_BTNBDR);

  for (int j = 0; j < 4; j++) {
    drawButton(8 + j * 83, 121, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);  // only for main screen draw a 3rd row of empty buttons on top
  }

  struct Button {
    int x;
    int y;
    const char *label;
  };


  Button buttons[] = {
    { 275, 245, "Set" }, { 185, 245, "Select" }, { 110, 255, "Scan" }, { 100, 198, "Bandw" }, { 25, 198, "Step" }, { 185, 188, "Save" }, { 270, 188, "Load" }, { 102, 130, "Osc" }, { 185, 132, "Range" }, { 270, 132, "Band" }, { 21, 151, " " }, { 102, 151, "Hi / Lo" }, { 185, 265, "Band" }, { 275, 265, "Freq" }, { 185, 208, "Memo" }, { 267, 208, "Memo" }, { 185, 151, "View" }, { 270, 151, "View" }, { 20, 132, "Web" }, { 20, 153, "Tools" }
  };

  etft.setTTFFont(Arial_14);
  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.setTextColor(textColor);
    if (i == 2 && scanMode)
      etft.setTextColor(TFT_SKYBLUE);
    etft.print(buttons[i].label);
  }

  drawButton(8, 234, TILE_WIDTH, TILE_HEIGHT, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print("More");
  etft.setTextColor(textColor);
}

//##########################################################################################################################//

void readMainBtns() {

  if (!pressed)
    return;

  int buttonID = getButtonID();

  if (row > 4 || column > 4)
    return;  // outside of keypad area
  redrawMainScreen = true;
  long freqOld = FREQ;                         // save freq for returning from waterfall
  tft.fillRect(140, 295, 250, 24, TFT_BLACK);  // overwrite frozen spectrum
  switch (buttonID) {

    case 21:
      drawIBtns();
      readIBtns();
      tRel();
      break;
    case 22:
      low_High_Injection();
      tRel();
      break;
    case 23:
      waterFall(true);  // use keypad for waterfall
      tRel();
      FREQ = freqOld;
      displayFREQ(FREQ);
      tuneSI5351();  // return to previous frequency
      while (digitalRead(ENC_PRESSED) == LOW)
        ;
      break;
    case 24:
      setBand(true);  // select band for waterfall
      tRel();
      FREQ = freqOld;
      displayFREQ(FREQ);
      tuneSI5351();
      while (digitalRead(ENC_PRESSED) == LOW)
        ;
      break;
    case 31:
      setSTEP(0);  // use touchbuttons
      break;
    case 32:
      setBandwidth(0); // use touchbuttons
      tRel();
      break;
    case 33:
      showMemo(false);
      writeMemo();
      tRel();
      break;
    case 34:
      showMemo(true);
      readMemo();
      tft.fillRect(5, 295, 374, 24, TFT_BLACK);  // overwrite remanents of station names
      displayDebugInfo = true;                   // restore debug info
      tRel();
      break;
    case 41:
      SecScreen();
      tRel();
      break;
    case 42:
      ScanMode();
      tRel();
      tx = ty = pressed = 0;
      break;
    case 43:
      setBand(false);  // Select Band
      tRel();
      break;
    case 44:
      freqScreen();
      break;
    default:
      redrawMainScreen = true;
      tx = ty = pressed = 0;
      return;
  }
}

//##########################################################################################################################//

void setBandwidth(int mode) {  // mode 0 = select from menu. mode -1 and 1 are touching the indicator area
  const int TEXT_Y = 195;
  const int positions[] = { 24, 106, 191, 272 };
  
  const uint8_t usbLsbBandWidths[] = { 0, 1, 2, 3 };
  const uint8_t amBandWidths[] = { 3, 2, 1, 0 };
  const uint8_t cwBandWidths[] = { 4, 5, 3, 3 };

  const char *amBandwidth[] = { "2KHz", "3KHz", "4KHz", "6KHz" };
  const char *ssbBandwidth[] = { "1.2KHz", "2.2KHz", "3KHz", "4KHz" };
  const char *cwBandwidth[] = { "0.5KHz", "1.0KHz", "", "" };
  const char **bandwidth = nullptr;



  if (modType == WBFM)
    return;

  if (mode == 0) {
    for (int j = 0; j < 4; j++) {
      drawButton(8 + j * 83, 235, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
      drawButton(8 + j * 83, 178, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    }

    tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest row

    etft.setTTFFont(Arial_14);
    etft.setTextColor(TFT_GREEN);

    if (modType == AM)
      bandwidth = amBandwidth;
    else if (modType == USB || modType == LSB || modType == SYNC)
      bandwidth = ssbBandwidth;
    else if (modType == CW)
      bandwidth = cwBandwidth;


    if (bandwidth != nullptr) {
      for (int i = 0; i < 4; ++i) {

        if (modType == AM)
          etft.setCursor(positions[i], TEXT_Y);
        if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
          etft.setCursor(positions[i] - 10, TEXT_Y);  // text is wider

        etft.print(bandwidth[i]);
      }
      etft.setTextColor(textColor);  // Reset to default text color
    }


    tDoublePress();

    delay(10);  // Wait until pressed

    row = 1 + ((ty - 20) / vTouchSpacing);
    column = 1 + (tx / HorSpacing);


    if (row == 3) {

      if (modType == USB || modType == LSB || modType == SYNC) {
        bandWidth = usbLsbBandWidths[column - 1];
        lastSSBBandwidth = bandWidth;
      } else if (modType == AM) {
        bandWidth = amBandWidths[column - 1];
        lastAMBandwidth = bandWidth;
      } else if (modType == CW) {
        bandWidth = cwBandWidths[column - 1];
        lastSSBBandwidth = bandWidth;
      }
    }

  }  // endif mode == 0


else { // mode -1 and mode 1 are used when touching the indicator area on the left or right

const uint8_t* bandWidths;
int maxIndex = 3;
int i = 0;
bool isIncrement = (mode == 1);

if (modType == AM) {
    bandWidths = amBandWidths;
} else if (modType == USB || modType == LSB || modType == SYNC) {
    bandWidths = usbLsbBandWidths;
} else if (modType == CW) {
    bandWidths = cwBandWidths;
    maxIndex = 1;
} else {
    return;  
}

// find index
while (i < maxIndex && bandWidths[i] != bandWidth) {
    i++;
}

// modify index
if (isIncrement && i < maxIndex) {
    i++;
} else if (!isIncrement && i > 0) {
    i--;
}

// update bandwidth
bandWidth = bandWidths[i];
  
}
   // set bandwidth
  if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {
    si4735.setSSBAudioBandwidth(bandWidth);
    if (bandWidth <= 2) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
  }

  if (modType == AM) {
    si4735.setBandwidth(bandWidth, 1);
  }
  printBandWidth();
}

//##########################################################################################################################//

void printBandWidth() {

  if (altStyle)
    tft.fillRoundRect(75, 95, 92, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(92, 26);
    spr.pushImage(0, 0, 92, 26, (uint16_t *)Oval92);
    spr.pushSprite(75, 92);
    spr.deleteSprite();
  }

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(77, 98);

  switch (modType) {
    case WBFM:
      tft.print(" 300KHz");
      break;
    case FM:
      tft.print("12.5KHz");
      break;
    case CW:
      if (bandWidth == 4)
        tft.print(" 0.5KHz");
      if (bandWidth == 5)
        tft.print(" 1.0KHz");
      break;
    case USB:
    case LSB:
    case SYNC:
      tft.print(bandWidth == 0 ? " 1.2KHz" : bandWidth == 1 ? " 2.2KHz"
                                           : bandWidth == 3 ? " 4.0KHz"
                                                            : " 3.0KHz");
      break;
    case AM:
      tft.print(bandWidth == 0 ? " 6.0KHz" : bandWidth == 1 ? " 4.0KHz"
                                           : bandWidth == 2 ? " 3.0KHz"
                                                            : " 2.0KHz");
      break;
  }
}
//##########################################################################################################################//


void setTinySAMmode() {

  int h = 8;

  for (int j = 0; j < 2; j++) {
    drawButton(h + j * 83, 145, 75, 30, TFT_MAROON, TFT_MAROON);
  }

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(22, 155);
  tft.print("IF");
  tft.setCursor(105, 155);
  tft.print("RF");

  int16_t row = 0, column = 0;

  tDoublePress();  // wait until pressed

  column = 1 + (tx / 90);  //  get row and column
  row = 1 + ((ty - 20) / VerSpacing);

  if (row == 2 && column == 2) {
    tinySA_RF_Mode = true;
    digitalWrite(TINYSA_PIN, HIGH);  // switches the tinySA input between RF and IF
  } else if (row == 2 && column == 1) {
    tinySA_RF_Mode = false;
    digitalWrite(TINYSA_PIN, LOW);
  }


  printTinySAMode();
  redrawMainScreen = true;
  tx = ty = pressed = 0;
}

//##########################################################################################################################//

void printTinySAMode() {

  if (altStyle)
    tft.fillRoundRect(174, 95, 45, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(55, 26);
    spr.pushImage(0, 0, 55, 26, (uint16_t *)Oval55);
    spr.pushSprite(170, 92);
    spr.deleteSprite();
  }

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(185, 98);

  if (tinySA_RF_Mode)
    tft.print("RF");
  else
    tft.print("IF");
}


//##########################################################################################################################//

void ScanMode() {
  scanMode = !scanMode;

  if (!scanMode)
    showScanRange = true;  // reset to show the range next time it will be set
  drawBigBtns();
}

//##########################################################################################################################//

int getButtonID(void) {


  if (ty > 293 || ty < 100) // no buttons there
   return 0;

  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 25) / vTouchSpacing); 
  int buttonID = row * 10 + column;
  return buttonID;
}

//##########################################################################################################################//
void low_High_Injection() {  //changes injection mode

  LOAboveRF = !LOAboveRF;

  restartSynth();
  tuneSI5351();

  if (LOAboveRF)
    displayText(5, 300, 300, 19, "LO frequency above RF");
  else
    displayText(5, 300, 300, 19, "LO frequency below RF");

  delay(500);
  displayText(5, 300, 300, 19, "                      ");

  FREQ_OLD -= 1;  //retrigger freq display
}
