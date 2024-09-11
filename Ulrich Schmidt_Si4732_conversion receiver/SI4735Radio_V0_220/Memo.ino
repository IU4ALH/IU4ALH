
//Memory functions for 8 memo pages with 13 memory slots each
// Use encoder to select a slot to listen without leaving memo. Press or touch to set and leave.
//Press SCAN once, then press the memory slots that should be scanned.
// Press SCAN again and scanning begins.
// Use struct MemoInfo to display station names.

//##########################################################################################################################//

int currentPage;
const int totalPages = 8;  // Total number of pages
long SAVE_FREQ = FREQ;

//##########################################################################################################################/

void printMemoName() {
  static long OLDFREQ = -1;
  static const char* lastMemoName = "";
  bool found = false;
  int i = 0;

  displayDebugInfo = false;  // make room for memo name

  if (FREQ != OLDFREQ) {
    OLDFREQ = FREQ;
    tft.setTextColor(TFT_GREEN);
    if (strcmp(lastMemoName, "") != 0) {
      displayText(5, 300, 250, 20, "");
      lastMemoName = "";
    }

    for (i = 0; i < 127; i++) {
      if (memo[i].FreqKHz == FREQ / 1000) {
        found = true;
        break;
      }
    }

    // Display memo name if  match is found
    if (found) {
      displayText(5, 300, 250, 20, memo[i].memoName);
      lastMemoName = memo[i].memoName;
    }
  }

  tft.setTextColor(textColor);
}

//##########################################################################################################################//


void showMemo(bool isRead) {  // displays memo buttons

  long FR;
  int index;
  currentPage = 0;
  row = 0;
  column = 0;


  tft.setTextSize(2);
  while (true) {
    if (!altStyle)
      tft.fillRect(2, 50, 337, 240, TFT_BLACK);
    else
      drawButton(2, 50, 337, 240, TFT_YELLOW, TFT_MAROON);  // background
    draw16Buttons();                                        // buttons

    tft.fillRect(270, 300, 110, 18, TFT_BLACK);  // clear status line

    index = currentPage * 16;
    int x_positions[] = { 10, 93, 176, 262 };
    int y_positions[] = { 70, 128, 186, 244 };
    char fkey[12];  // store the key as a string

    for (int i = 0; i < 4; i++) {  // display tiles and freq values
      for (int j = 0; j < 4; j++) {
        etft.setCursor(x_positions[j], y_positions[i]);
        snprintf(fkey, sizeof(fkey), "%d", index + 1);  // Convert frequency to string
        FR = preferences.getLong(fkey, 0);
        index++;

        etft.setTextColor(TFT_WHITE);
        etft.setTTFFont(Arial_14);

        if (index % 16 == 0 && index <= 112 && index != 0)
          etft.print(" NEXT");
        else if ((index % 16 == 15 && index <= 112) || index == 128)
          etft.print(" EXIT");
        else if (((index % 16 == 14 && index <= 111) || index == 127) && isRead)
          etft.print(" SCAN");
        else if (((index % 16 == 14 && index <= 111) || index == 127) && !isRead)
          etft.print(" N//A");

        else {  // display memo frequencies
          etft.print(FR / 1000000);
          etft.print(".");
          etft.setTextColor(TFT_YELLOW);
          etft.setTTFFont(Arial_9);
          if (((FR % 1000000) / 1000) < 100)  // insert 0 if Freq < 100
            etft.print("0");
          etft.printf("%ld", (FR % 1000000) / 1000);
          etft.setCursor(x_positions[j] + 40, y_positions[i] + 25);
          etft.setTTFFont(Arial_8);
          etft.setTextColor(TFT_SKYBLUE);
          etft.printf("%d", index);
          etft.setTextColor(textColor);
        }
      }
    }

    int result = 99;
    if (isRead)
      result = tuneMemo();

    if (isRead && result == 1) {  // tuneMemo returns 1 for encoder pressed
      tx = ty = 999;              // readMemo() will interpret this as outside of touch area and return
      return;
    }

    if (isRead && result == 2)  // tuneMemo returns 2 for button pressed
      tRel();

    if (!isRead)  // Save Memo
      tDoublePress();

    column = 1 + (tx / HorSpacing);      // get column
    row = 1 + ((ty - 20) / VerSpacing);  // get row

    if (row == 4 && column == 4 && currentPage == totalPages - 1) {  // Last page reached, or Exit pressed
      redrawMainScreen = true;
      tx = ty = pressed = 0;
      tRel();
      mainScreen();
      return;
    }
    if (result == -1) {  /// Encoder counterclockwise,  tuneMemo returned -1, load prev. page.
      currentPage -= 1;

    } else if ((row == 4 && column == 4) || result == 0) {  // "Next" button , or tuneMemo returned 0, load next page.
      currentPage = (currentPage + 1) % totalPages;

    }

    else
      break;
  }
}
//##########################################################################################################################//
void memoAction(bool isWrite) {

  int row, column;

  column = 1 + (tx / 90);  // get row and column
  row = 1 + ((ty - 20) / VerSpacing);

  if (row > 4 || column > 4)
    return;  // outside of key area

  int buttonID = (currentPage * 16) + (row - 1) * 4 + (column - 1) + 1;

  if (buttonID >= 1 && buttonID <= totalPages * 16) {
    char fkey[5], bkey[5], mkey[5];
    snprintf(fkey, sizeof(fkey), "%d", buttonID);
    snprintf(bkey, sizeof(bkey), "b%d", buttonID);  // add b to string (bandwith)
    snprintf(mkey, sizeof(mkey), "m%d", buttonID);  // add m to string (modulation)

    if ((buttonID % 16 == 15 && buttonID < 127) || buttonID == 128) {  // Exit pressed
      return;
    }

    if ((buttonID == 14 || buttonID == 30 || buttonID == 46 || buttonID == 62 || buttonID == 78 || buttonID == 94 || buttonID == 110 || buttonID == 127) && !isWrite) {  // Scan pressed
      memoScanner();
      return;
    }
    if (isWrite) {
      preferences.putLong(fkey, FREQ);
      preferences.putInt(bkey, bandWidth);
      preferences.putChar(mkey, modType);
    } else {
      FREQ = preferences.getLong(fkey, 0);  // read frequency
      bandWidth = preferences.getInt(bkey, 0);

      if (preferences.getChar(mkey, 0) != modType) {  // only reload when modType is different
        modType = preferences.getChar(mkey, 0);
        loadModulation4735();
      }
    }
    tRel();
  }
}

//##########################################################################################################################//

void memoScanner() {  // SCAN was pressed, press memo buttons to enter frequencies and press SCAN again to start

  long fBuffer[16] = { 0 };  // buffer for memoscan
  char idBuffer[16] = { 0 };
  int index = 0;
  int OldsignalStrength = 0;
  int row, column;
  char fkey[12];

  tPress();

  while (true) {

    tPress();
    tRel();

    column = 1 + (tx / 90);  // get row and column
    row = 1 + ((ty - 20) / VerSpacing);
    if (row > 4 || column > 4)
      return;  // outside of key area
    int buttonID = (currentPage * 16) + (row - 1) * 4 + (column - 1) + 1;

    tft.fillRect(410, 4, 30, 17, TFT_BLACK);
    tft.setCursor(410, 5);
    tft.printf("%d ", buttonID);


    if (buttonID == 14 && index == 0)  // scan pressed 2x by mistake 
      return;

    if (buttonID == 14 || buttonID == 30 || buttonID == 46 || buttonID == 62 || buttonID == 78 || buttonID == 94 || buttonID == 110 || buttonID == 127)  //SCAN pressed again
      break;
    if (buttonID == 15 || buttonID == 31 || buttonID == 47 || buttonID == 63 || buttonID == 79 || buttonID == 95 || buttonID == 111 || buttonID == 128)  // Exit pressed
      return;


    snprintf(fkey, sizeof(fkey), "%d", buttonID);   // make string
    fBuffer[index] = preferences.getLong(fkey, 0);  // load frequency into buffer
    idBuffer[index] = buttonID;
    index++;
  }  //endwhile select frequencies

  while (true) { // run scan loop
    int idx = 0;
    while (idx < index) {
      FREQ = fBuffer[idx];
      //loadModulation4735(); // scan is limited to AM, loading the SSB patch would take too long
      printMemoName();
      displayFREQ(FREQ);
      tuneSI5351();

      drawMarker(idBuffer[idx], TFT_RED);  // draws a red or green dot on the tile being scanned

      delay(100); // this is the main delay for scanning

      drawMarker(idBuffer[idx], TFT_NAVY);

      int ctr = 0;
      while (true) {
        si4735.getCurrentReceivedSignalQuality(0);
        signalStrength = si4735.getCurrentRSSI();
        SNR = si4735.getCurrentSNR();
        readSquelchPot(false);  // do not draw position
        setSquelch();
        delay(15);

        pressed = get_Touch();
        if (pressed) {
          tft.fillRect(345, 2, 130, 20, TFT_BLACK);
          ctr = 0;
          tx = ty = pressed = 0;
          return;
        }

        if (((signalStrength > currentSquelch) && !SNRSquelch) || (SNRSquelch && SNR)) // signal found

        {
          drawMarker(idBuffer[idx], TFT_GREEN);
          if (OldsignalStrength != signalStrength) {
            tft.setTextColor((TFT_GREEN));
            tft.fillRect(345, 2, 130, 20, TFT_BLACK);
            tft.setCursor(345, 6);
            tft.printf("%d dbyV", signalStrength - GAINREDUCTION);
            OldsignalStrength = signalStrength;
          }
          ctr = 100;
          delay(100);
        }

        if ((signalStrength < currentSquelch) || (SNRSquelch && !SNR)) { // countdown timer until scan continues
          ctr--;

          tft.setTextColor((TFT_RED));
          tft.fillRect(345, 3, 130, 20, TFT_BLACK);
          tft.setCursor(345, 6);
          tft.printf("  %d", ctr / 10);
          drawMarker(idBuffer[idx], TFT_NAVY);
        }

        if (ctr <= 0) {
          tft.fillRect(345, 5, 130, 16, TFT_BLACK);
          tft.setTextColor((textColor));
          break;
        }
      }
      idx++;
    }
  }



}

//##########################################################################################################################//
void writeMemo() {
  memoAction(true);
}
//##########################################################################################################################//
void readMemo() {
  memoAction(false);
}
//##########################################################################################################################//

int tuneMemo() {  // tunes through memory bank with encoder

  char memoString[4] = { 0 };  // store the selected memo as string

  encLockedtoSynth = false;
  clw = false;
  cclw = false;
  static int dex = 0;  // index
  int oldDex = 0;
  long SAVE_FREQ = FREQ;

  printMemoName();  // initial print
  dex = (currentPage * 16) + 1;

  do {
    pressed = get_Touch();
    delay(50);  // otherwise encoder will skip steps

    if (clw)
      dex++;
    if (cclw)
      dex--;
    if (dex < 1)
      dex = 1;
    if (dex > 128)
      dex = 1;

    if ((dex == 14 || dex == 30 || dex == 46 || dex == 62 || dex == 78 || dex == 94 || dex == 110 || dex == 126) && clw) {
      dex += 3;  // skip SCAN, EXIT and NEXT
      return 0;
    }

    if ((dex == 16 || dex == 32 || dex == 48 || dex == 64 || dex == 80 || dex == 96 || dex == 112 || dex == 128) && cclw) {

      return -1;  // load previous page
    }


    if (oldDex != dex) {
      drawMarker(oldDex % 16, TFT_NAVY);
      drawMarker(dex % 16, TFT_GREEN);

      snprintf(memoString, sizeof(memoString), "%d", dex);  // Convert
      FREQ = preferences.getLong(memoString, 0);            // read frequency
      printMemoName();
      displayFREQ(FREQ);
      settleSynth();
      tuneSI5351();
    }

    if (digitalRead(ENC_PRESSED) == LOW) {  // use encoder to load memo
      char bkey[5], mkey[5];
      snprintf(bkey, sizeof(bkey), "b%d", dex);
      snprintf(mkey, sizeof(mkey), "m%d", dex);
      bandWidth = preferences.getInt(bkey, 0);
      modType = preferences.getChar(mkey, 0);
      loadModulation4735();
      tft.fillRect(345, 2, 130, 20, TFT_BLACK);
      FREQ_OLD = FREQ - 1;  // trigger  displayFREQ
      encLockedtoSynth = true;
      while (digitalRead(ENC_PRESSED) == LOW)
        ;
      return 1;
    }
    oldDex = dex;


    si4735.getCurrentReceivedSignalQuality(0);
    signalStrength = si4735.getCurrentRSSI();
    SNR = si4735.getCurrentSNR();
    tft.fillRect(345, 2, 130, 20, TFT_BLACK); // overwrite last value
    tft.setCursor(345, 5);

    tft.setTextColor(((signalStrength > currentSquelch) && !SNRSquelch) || (SNRSquelch && SNR) ? TFT_GREEN : TFT_MAROON);
    tft.printf("%d dbyV", signalStrength - GAINREDUCTION);
    tft.setTextColor((textColor));


    bool draw = false;
    readSquelchPot(draw);  // do not draw position
    setSquelch();
    clw = 0;
    cclw = 0;
  } while (!pressed);
  encLockedtoSynth = true;
  tft.fillRect(345, 2, 130, 20, TFT_BLACK);
  drawMarker(dex % 16, TFT_NAVY);
  FREQ = SAVE_FREQ;
  FREQ_OLD = FREQ - 1;  // trigger  displayFREQ
  return 2;
}

//##########################################################################################################################//


void drawMarker(int buttonID, uint16_t COLOR) {  // draws a circle on the memo tiles
  int idx = (buttonID - 1) % 16;
  int row = idx / 4;
  int column = idx % 4;
  int x_positions[] = { 10, 93, 176, 262 };
  int y_positions[] = { 75, 132, 191, 249 };
  tft.fillCircle(x_positions[column] + 20, y_positions[row] + 25, 6, COLOR);
}
