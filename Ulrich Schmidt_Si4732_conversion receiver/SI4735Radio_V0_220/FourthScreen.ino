void FourthScreen() {

  // fourth screen is for development

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);
  draw8Buttons(TFT_BTNCTR, TFT_BTNBDR);
  redrawMainScreen = true;
  drawFoBtns();
  readFoBtns();
}

//##########################################################################################################################//

void drawFoBtns() {

  draw8Buttons(TFT_DEEPORANGE, TFT_BROWN);
  tft.fillRect(10, 120, 250, 15, TFT_BLACK);
  
  /*
  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_RED);
  etft.setCursor(10, 120);
  etft.print("DEVELOPMENT PLAYGROUND");

 */

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_ORANGE);

  struct Button {
    int x;
    int y;
    const char* text;
  };

  Button buttons[] = {
    { 15, 190, "1" }, { 15, 213, "2" }, { 100, 190, "3" }, { 100, 213, "4" }, { 188, 190, "7Seg" }, { 188, 210, "Font" }, { 265, 190, "Touch" }, { 263, 213, "Sound" }, { 272, 245, "   " }, { 272, 268, "Sqlch" }, { 183, 245, "Show" }, { 183, 268, "SNR" }, { 100, 245, "GPIO" }, { 100, 268, "Test" }
  };


  for (int i = 0; i < 14; i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].text);
  }


  etft.setCursor(272, 245);
  if (SNRSquelch)
    etft.print("Poti");
  else  
    etft.print("SNR");


  drawButton(8, 236, 74, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setCursor(20, 254);
  etft.print("BACK");
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readFoBtns() {

  if (!pressed) return;
  int buttonID = getButtonID();
  
  
  if (row < 3 || row > 4 || column > 4) 
     return;  // outside of area

  switch (buttonID) {
    case 31:
      break;
    case 32:
      break;
    case 33:
      tft.fillRect(5, 5, 325, 38, TFT_BLACK);
      sevenSeg = !sevenSeg;
      displayFREQ(FREQ);
      preferences.putBool("sevenSeg", sevenSeg);
      break;
    case 34:
      pressSound = !pressSound;
      preferences.putBool("pressSound", pressSound);
      break;
    case 41:
      return;
    case 42:
      gpioTest();
      tPress();
      break;
    case 43:
      showSNR = !showSNR;
      preferences.putBool("showSNR", showSNR);
      break;
    case 44:
      SNRSquelch = !SNRSquelch;  //  SNRSquelch use SNR to trigger audio mute and the squelch pot will not be used. This only works in AM mode
      preferences.putBool("SNRSquelch", SNRSquelch);
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

void gpioTest() {  // set GPIO's manually for filter testing

  draw8Buttons(TFT_BTNCTR, TFT_BTNBDR);

  int cursorPositions[][2] = {
    { 15, 190 }, { 15, 213 }, { 100, 190 }, { 100, 213 }, { 182, 190 }, { 182, 213 }, { 265, 190 }, { 263, 213 }, { 262, 245 }, { 262, 268 }, { 178, 245 }, { 178, 268 }, { 100, 245 }, { 100, 268 }
  };

  const char* texts[] = {
    "GPIO", " 13", "GPIO", " 14", "GPIO", " 25",
    "GPIO", " 27", "", "", "", "",
    "", ""
  };

  for (int i = 0; i < 14; i++) {
    etft.setCursor(cursorPositions[i][0], cursorPositions[i][1]);
    etft.print(texts[i]);
  }

  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(27, OUTPUT);


  int gpioPins[] = { 13, 14, 25, 27 };
  bool gpioStates[] = { LOW, LOW, LOW, LOW };
  int circles[][2] = {
    { 60, 213 },
    { 140, 213 },
    { 222, 213 },
    { 305, 213 }
  };


  while (true) {
    for (int i = 0; i < 4; i++) {
      gpioStates[i] = digitalRead(gpioPins[i]);
      tft.fillCircle(circles[i][0], circles[i][1], 4, gpioStates[i] ? TFT_GREEN : TFT_RED);
    }

    tRel();

    int buttonID = getButtonID();
    switch (buttonID) {
      case 31:
        gpioStates[0] = !gpioStates[0];
        digitalWrite(gpioPins[0], gpioStates[0]);
        break;
      case 32:
        gpioStates[1] = !gpioStates[1];
        digitalWrite(gpioPins[1], gpioStates[1]);
        break;
      case 33:
        gpioStates[2] = !gpioStates[2];
        digitalWrite(gpioPins[2], gpioStates[2]);
        break;
      case 34:
        gpioStates[3] = !gpioStates[3];
        digitalWrite(gpioPins[3], gpioStates[3]);
        break;
      case 41:
      case 44:
        redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
      case 42:
      case 43:
        break;
    }
    tx = ty = pressed = 0;
    delay(100);
  }
}
