
void SecScreen() {

  tft.fillRect(3, 119, 334, 58, TFT_BLACK); // overwrite highest row from mainscreen
  redrawMainScreen = true;
  drawSecBtns();
  readSecBtns();
}

//##########################################################################################################################//

void drawSecBtns() {

  draw8Buttons(TFT_DARKGREEN, TFT_DARKDARKGREEN);

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_GREEN);

  etft.setTTFFont(Arial_14);
  drawButton(8, 236, 74, 49, TFT_MIDGREEN, TFT_DARKGREEN);

  struct Button {
  int x;
  int y;
  const char* label;
};


Button buttons[] = {
  { 18, 190, "Quick" }, { 18, 210, "Save" },
  { 100, 190, "Quick" }, { 100, 210, "Load" },
  { 190, 195, "Bfo" }, { 182, 208, " " },
  { 273, 195, "Attn" }, { 265, 208, " " },
  { 270, 255, "Style" }, { 185, 245, "Sprite" }, { 185, 265, "Style" },
  { 100, 245, "Clock" }, { 100, 265, "Out" },{ 20, 254, "More" }
};

  for (int i = 0; i < 14; i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }

if (modType != AM){ // AGC only working in AM
 etft.setCursor(273, 195);
 etft.setTextColor(TFT_DARKGREY);
 etft.print("Attn");
 etft.setTextColor(textColor);
}

  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readSecBtns() {

  if (!pressed) return;

  int buttonID = getButtonID();

  if (row > 4 || column > 4) return;  // outside of key area

  switch (buttonID) {
    case 31:
      preferences.putLong("savFreq", FREQ);  // quicksave
      preferences.putInt("bandWidth", bandWidth);
      preferences.putChar("modType", modType);
      break;
    case 32:
      FREQ = preferences.getLong("savFreq", 0);  // quickload
      bandWidth = preferences.getInt("bandWidth", 0);
      modType = preferences.getChar("modType", 0);
      loadModulation4735();
      displayFREQ(FREQ);
      break;
    case 33:
      setBFO(); 
      tRel();
      break;
    case 34:
      setAGCMode(0);
      tRel();
      break;
    case 41:
      ThirdScreen();
      break;
    case 42:
      setClockMode();
      break;
    case 43:
      selectButtonStyle();
      break;
    case 44:
      altStyle = !altStyle; // change between plain and sprite style
      preferences.putBool("lastStyle", altStyle);
      drawBigBtns();  // redraw with new style
      break;
    default:
      redrawMainScreen = true;
      tx = ty = pressed = 0;
      return;
  }
  redrawMainScreen = true;
  tRel();
}

//##########################################################################################################################//
void setClockMode() {  // switch clocks btw tx and rxmode
  restartSynth();      // need to restart so that clock switches reliably
  if (txmode) {
    rxmode = true;
    txmode = false;
    si5351.output_enable(SI5351_CLK1, 0);
    si5351.output_enable(SI5351_CLK2, 1);
    tft.fillRect(320, 5, 155, 18, TFT_BLACK);

 } else if (rxmode) {
    txmode = true;
    rxmode = false;
    tft.fillRect(320, 5, 155, 18, TFT_BLACK);
    tft.setCursor(322, 5);
    tft.setTextColor(TFT_ORANGE);
    tft.print("Output CLK1");
    si5351.output_enable(SI5351_CLK1, 1);
    si5351.output_enable(SI5351_CLK2, 0);
  }
  tft.setTextColor(textColor);
  tuneSI5351();
  FREQ_OLD -= 1; // trigger  FREQ display update
}


//##########################################################################################################################//

void setAGCMode(int mode) {
    // AGCDIS: AGC enabled (0) or disabled (1)
    // AGCIDX: AGC Index (0 = max gain, 1-36 = intermediate, >36 = min gain)
    // mode 0 = button control, mode -1 or 1 = icon control

    if (modType != AM) {
        return;  
    }

    if (mode) {
        // Handle icon control for AGC adjustment
        while (pressed) {
            pressed = tft.getTouch(&tx, &ty);
            if (mode == 1) {
                AGCIDX++;
            } else if (mode == -1 && AGCIDX) {
                AGCIDX--;
            }

            if (AGCIDX > 36) {
                AGCIDX = 36;
            }

            AGCDIS = (AGCIDX != 0);  // Disable AGC if AGCIDX is 0
            writeAGCMode();
            printAGC();
            delay(50);
        }
        return;
    }

    encLockedtoSynth = false;
    tft.setCursor(8, 160);
    tft.print("Use encoder to change Attn.");

    while (digitalRead(ENC_PRESSED) == HIGH) {
        delay(50);

        if (clw) {
            AGCIDX++;
        } else if (cclw) {
            AGCIDX--;
        }

        if (AGCIDX > 36) {
            AGCIDX = 0;
        }

        AGCDIS = (AGCIDX != 0);  // Disable AGC if AGCIDX is 0, enable otherwise
        writeAGCMode();

        if (clw != cclw) {
            printAGC();
        }

        clw = false;
        cclw = false;
    }

    // Prevent jumping into setStep function
    while (digitalRead(ENC_PRESSED) == LOW)
       ;

    encLockedtoSynth = true;
    redrawMainScreen = true;
    tx = ty = pressed = 0;
}


//##########################################################################################################################//



void writeAGCMode() {

  if (modType == AM) 
  si4735.setAutomaticGainControl(AGCDIS, AGCIDX);  
  if (modType == LSB || modType == USB || modType == SYNC) { 
    si4735.setSsbAgcOverrite(1, AGCIDX); // disable AGC to eliminate SSB humming noise
    printAGC(); 
  }
}
//##########################################################################################################################//


void printAGC() {

  tft.setTextColor(TFT_GREEN);
  if (altStyle)
  tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);
 
 else {  
  
  spr.createSprite(102, 26); 
      spr.pushImage(0, 0,102, 26, (uint16_t *)Oval102);
      spr.pushSprite(225,92);
      spr.deleteSprite();
    }
   
    tft.setCursor(235, 98);
    if (modType == AM)
       tft.printf(AGCDIS == false ? "AGC:ON" : "Attn:%d ", AGCIDX);
    else
       tft.print("AGC:OFF");
    tft.setTextColor(textColor);
  
}
//##########################################################################################################################//

void setBFO() {

  int offset = 0, oldOffset = 0;

  encLockedtoSynth = false;
  tft.setCursor(5, 140);
  tft.print("Use encoder to set BFO");

  if (modType == USB)
    offset = preferences.getInt("USBBfoOffset", 0);
  if (modType == LSB)
    offset = preferences.getInt("LSBBfoOffset", 0);
  if (modType == SYNC)
    offset = preferences.getInt("SYNCBfoOffset", 0);
  if (modType == CW) 
    offset = preferences.getInt("CWBfoOffset", 0);

  while (digitalRead(ENC_PRESSED) == HIGH) {

    if (oldOffset != offset) {
      tft.fillRect(2, 155, 220, 20, TFT_BLACK);
      tft.setCursor(5, 158);
      tft.printf("BFO Offset: %d", offset);
    }
    oldOffset = offset;

    delay(50);
    if (clw)
      offset += 25;
    if (cclw)
      offset -= 25;

    si4735.setSSBBfo(offset);

    clw = false;
    cclw = false;
  }

  while (digitalRead(ENC_PRESSED) == LOW)  // wait so that it does not jump into the setStep function
    ;


  if (modType == USB)
    preferences.putInt("USBBfoOffset", offset);

  if (modType == LSB)
    preferences.putInt("LSBBfoOffset", offset);

  if (modType == USB)
    preferences.putInt("USBBfoOffset", offset);

  if (modType == SYNC)
    preferences.putInt("SYNCBfoOffset", offset);

  if (modType == CW)
    preferences.putInt("CWBfoOffset", offset);  

  encLockedtoSynth = true;
  redrawMainScreen = true;

  tft.fillRect(2, 155, 180, 20, TFT_BLACK);
  tft.setCursor(5, 158);
  tft.printf("BFO Offset: %d", offset);
}

//##########################################################################################################################//

void selectButtonStyle() {  // selects btw. different sprites for the buttons

  tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  for (int i = 0; i < 8; i++) {
    spr.createSprite(SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT);
    spr.pushImage(0, 0, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[i]);  // Use the corresponding button image

    if (i < 4)
      spr.pushSprite(8 + i * 83, 178);
    else
      spr.pushSprite(8 + (i - 4) * 83, 235);
    spr.deleteSprite();
  }
  tPress();
  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 20) / vTouchSpacing);
  if (row > 4 || column > 4)
    return;
  buttonSelected = (column - 1) + 4 * (row - 3);
  preferences.putInt("sprite", buttonSelected);  // write selection to EEPROM
}

//##########################################################################################################################//
