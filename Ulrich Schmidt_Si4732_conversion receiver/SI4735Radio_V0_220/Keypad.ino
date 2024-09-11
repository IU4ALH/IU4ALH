
//draws and reads onscreen numeric keyboard

void freqScreen() {  // displays and reads frequency numeric keypad
  drawNumPad();
  drawKeypadButtons();
  tRel();
  readKeypadButtons();
}

//##########################################################################################################################//
void drawKeypadButtons() {

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(4);
  char labels[9] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

  int x_positions[] = { 25, 108, 191 };
  int y_positions[] = {70, 128, 186, };

  int index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      tft.setCursor(x_positions[j], y_positions[i]);
      tft.print(labels[index++]);
    }
  }

  tft.setCursor(25, 244);
  tft.print("0");

  struct TextPos {
    int x, y;
    const char* text;
  };

   TextPos textButtons[] = {
    { 191, 250, "." },
    { 103, 250, "RESET" },
    { 182, 250, "   " },
    { 270, 70, "KHz" },
    { 270, 128, "MHz" },
    { 270, 194, "<<" },
    { 262, 250, "RETURN" }
  };

  tft.setTextSize(2);
  tft.setTextColor(textColor);
  for (int i = 0; i < sizeof(textButtons) / sizeof(textButtons[0]); ++i) {
    tft.setCursor(textButtons[i].x, textButtons[i].y);
    tft.print(textButtons[i].text);
  }
}


//##########################################################################################################################//

bool readKeypadButtons() {

  double f = 0;
  uint16_t xPos = 10;
  uint16_t yPos = 10;
  uint16_t index = 0;
  bool decimalPoint = false;
  int decimalPosition = 0;
  double val = 0;
  long freqSave = FREQ;


  tft.fillRect(10, 3, 325, 40, TFT_BLACK);

  while (index < 8) {  //6 digits frequency input
    tPress();

    column = 1 + (tx / HorSpacing);  //  get row and column
    row = 1 + ((ty - 20) / vTouchSpacing);

    showTouch(row, column);  // draws an empty button for 200ms to confirm touch

    if (column < 4) {                        // must be a digit
      if (row == 1) val = column;            // 1-3
      if (row == 2) val = column + 3;        // 4-6
      if (row == 3) val = column + 6;        //7-9
      if (row == 4 && column == 1) val = 0;  // 0
    }
    if (row == 4 && column == 3) {// Decimal point
      if (!decimalPoint) {
        decimalPoint = true;
      }
    }

    if (!decimalPoint) {
      f *= 10;   // multiply last value
      f += val;  // add button pressed
    }

    else {
      f += val * pow(10, -decimalPosition);  // add fraction
      decimalPosition++;
    }

    if (row == 1 && column == 4) {  // Enter Khz
      tft.fillRect(10, 3, 325, 40, TFT_BLACK);
      if (!decimalPoint)
        FREQ = (long)(f * 100);
      else
        FREQ = (long)(f * 1000);
      
      if (FREQ > HI_FREQ) {// limit max input
        FREQ = freqSave;
        keyPadErr();
      }
      tRel();
      settleSynth();
      return true;
    }


    if (row == 2 && column == 4) {  //enter Mhz
      tft.fillRect(10, 3, 325, 40, TFT_BLACK);
      if (!decimalPoint)
        FREQ = (long)(f * 100000);
      else
        FREQ = (long)(f * 1000000);
      if (FREQ > HI_FREQ) {  // limit max input
        FREQ = freqSave;
        keyPadErr();
      }
      tRel();
      settleSynth();
      return true;
    }

    if (row == 3 && column == 4) {  // <<

      if (!decimalPoint) {
        f = (long)(f / 100); // divide by 100 because it will get multiplied by 10 agai
        index--;
      } else {
        String fString = String(f, 3);
        fString = fString.substring(0, fString.indexOf('.')); //eliminate digits after decimal point
        f = fString.toFloat();
        decimalPoint = false;
        decimalPosition = 0;
        index -= 4;
      }
      tft.setCursor(xPos + 30, yPos);
      tft.printf("%3.3f", f);
    }

    if (row == 4 && column == 2) {  // restart ESP if needed
       preferences.putBool("fastBoot", true);// set fastboot
      ESP.restart();
    }


    if (row == 4 && column == 4) {  // Return
      keyVal = (long)f / 10;        // keyVal is used as frequency input for other functions
      displayFREQ(0);
      tft.fillRect(10, 3, 325, 40, TFT_BLACK);  // overwrite frequency window
      FREQ = freqSave;
      FREQ_OLD = FREQ - 1;
      displayFREQ(FREQ);
      tRel();
      return false;
    }

    if (row > 4 || column > 4)
      return false;  // outside of keypad area


    tft.fillRect(10, 3, 325, 40, TFT_BLACK);
    etft.setTextColor(TFT_GREEN);
    etft.setTTFFont(Arial_32);  // display entered digits
    etft.setCursor(xPos + 30, yPos);
    if (!decimalPoint)
      etft.printf("%ld", (long)f);
    else
      etft.printf("%3.3f", f);
    index++;

    do {
      pressed = get_Touch();
      delay(20);
    } while (!pressed);

    val = 0;
  }

  FREQ = freqSave;  // too many digits
  keyPadErr();
  return false;
}


//##########################################################################################################################//


void keyPadErr() {

  tx = ty = pressed = 0;
  FREQ_OLD -= 1;  // no valid result
  tft.fillRect(10, 5, 325, 40, TFT_BLACK);
  etft.setCursor(10, 3);
  etft.print("Invalid");
  delay(500);
  tft.fillRect(10, 3, 325, 40, TFT_BLACK);
}



//##########################################################################################################################//
void drawNumPad() {
  uint16_t yb = 58;
  int h = 8;

  if (altStyle == 0)
    tft.fillRect(2, 50, 337, 240, TFT_BLACK);
  else
    drawButton(2, 50, 337, 240, TFT_YELLOW, TFT_MAROON);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      drawButton(h + j * 83, (i + 1) * yb, TILE_WIDTH, TILE_HEIGHT, TFT_BTNBDR, TFT_BTNCTR);
    }
  }
}

//##########################################################################################################################//


void showTouch(int row, int column) {

  if (row > 4 || column > 4)
    return;
  uint16_t yb = 58;
  int h = 8;
  drawButton(h + (column - 1) * 83, (row)*yb, TILE_WIDTH, TILE_HEIGHT, TFT_BTNBDR, TFT_BTNCTR);  // draw empty button
  delay(200);
  drawKeypadButtons();
  //redraw buttons
}


//##########################################################################################################################//
// Touch functions

void tRel() { // wait for touch release

  do {
    pressed = get_Touch();
    delay(50);
  } while (pressed);
}


//##########################################################################################################################//

void tDoublePress() { // wait until pressed again
  do {
    get_Touch();
    delay(50);
  } while (pressed);
  delay(50);

  do {
    pressed = get_Touch();
    delay(50);
  } while (!pressed);
}


//##########################################################################################################################//

void tPress() {  // wait for press

  do {
    pressed = get_Touch();
    delay(50);
  } while (!pressed);
}

//##########################################################################################################################//

bool longPress() {

  uint32_t t1 = millis();
  uint16_t longPressLimit = 1000;
  while (true) {
    pressed = get_Touch();
    delay(10);
    uint32_t t2 = millis();
    if (t2 - t1 > longPressLimit)
      return true;
    if (!pressed)
      return false;
  }
}

//##########################################################################################################################//

bool get_Touch() {  // implements a short beep when pressed, calls getTouch

  static uint32_t t = 0;
  uint32_t tnew;
  static bool snd = true;

  tnew = millis();
  pressed = tft.getTouch(&tx, &ty);

  if (tnew - t > 200 && ! pressed)
    snd = true;

  if (pressed && pressSound && snd) {
    sineTone (440, 20);
    snd = false;
    t = millis();
  }
  return pressed;
}
