//#########################################################################################################################//

void setScanRange() {

  if (!getFreqLimits()) {  // keypad return pressed
    redrawMainScreen = true;
    return;
  }
  tft.fillRect(5, 3, 330, 40, TFT_BLACK);
  tft.setCursor(12, 7);
  tft.print("1 and RETURN = stop mode");
  tft.setCursor(12, 26);
  tft.print("2 and RETURN = cont. mode");
  tPress();
  readKeypadButtons();
  tft.fillRect(5, 3, 330, 40, TFT_BLACK);
  redrawMainScreen = true;
  mainScreen();
  tft.setCursor(10, 123);
  printScanRange();
  ty = 100;  // changes ty so that it assumes a SEEK UP press and starts to scan from low to high freeq
}

//##########################################################################################################################//

void printScanRange() {

  if (showScanRange == false)
    return;

  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest button row
  tft.setCursor(10, 123);
  if (keyVal == 1)
    tft.print("Stay when signal drops");
  if (keyVal == 2)
    tft.print("Move on when signal drops");
  tft.setCursor(10, 150);
  tft.printf("Range:%ldKHz - %ldKHz", lim2 / 1000, lim1 / 1000);
}
//##########################################################################################################################//

void SCANfreq_UP_DOWN(bool stop) {  // changes Freq when SCAN buttons are pressed

  uint16_t stpsize;
  if (STEP > 5000)
    stpsize = 5000;
  else
    stpsize = STEP;
  if (ty > 20 && ty < 131 && tx > 340  && FREQ < (HI_FREQ - STEP) && !stop)  // UP button
    FREQ += stpsize;
  if (ty > 210 && ty < 290 && tx > 340 && FREQ > (LOW_FREQ + STEP) && !stop)  // Down button
    FREQ -= stpsize;
  if (pressed && tx > 340 && ty > 135 && ty < 205)  // SET RANGE button
    setScanRange();
}


//##########################################################################################################################//

void freq_UP_DOWN() {  // changes freq when up or down is pressed

  uint16_t stpsize;
  if (STEP > 5000)
    stpsize = 5000;
  else
    stpsize = STEP;

  si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength to compare against squelch
  signalStrength = si4735.getCurrentRSSI();

      if (ty > 20 && ty < 131 && FREQ < (HI_FREQ - STEP))  // UP
        FREQ += stpsize;
      if (ty > 210 && ty < 290 && FREQ > (LOW_FREQ + STEP))  // DOWN
        FREQ -= stpsize;
  delay(50 + 8 * signalStrength);  // slow down when we have a signal

}


//##########################################################################################################################//


bool getFreqLimits() {
  bool result;

  drawNumPad();
  drawKeypadButtons();
  displayText(12, 10, 320, 33, "Enter low limit");
  tPress();  // wait until pressed
  result = readKeypadButtons();
  if (!result)
    return false;

  lim1 = FREQ;
  displayText(12, 10, 320, 33, "Enter high limit:");
  tPress();
  result = readKeypadButtons();
  if (!result)
    return false;
  lim2 = FREQ;

  if (lim2 > lim1) {  // invert order
    long temp = lim2;
    lim2 = lim1;
    lim1 = temp;
  }

  return true;
}

//##########################################################################################################################//
