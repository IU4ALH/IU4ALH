//##########################################################################################################################//
// Next functions are to draw a slow scan waterfall, resolution is limited to 240 * 231 due to amount of framebuffer needed
// Typical draw time is btw. 10 minutes and several hours for one screen
// Uses fine tune potentiometer to adjust colors

void waterFall(bool useKeypad) {

  if (useKeypad) {
    if (!getFreqLimits())  // exit, keypad return was pressed
      return;
  }

  pressed = false;

  long endPoint = lim1;
  long startPoint = lim2;
  float span = (endPoint - startPoint) / 1000000.0;


  // need 2 framebuffers, 120 * 300, can't allocate 1 framebuffer big enough
  framebuffer1 = (uint16_t*)malloc(FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));
  framebuffer2 = (uint16_t*)malloc(FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));

  if (framebuffer1 == NULL || framebuffer2 == NULL)
    buffErr();

  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  wIntro(startPoint, endPoint);


  // Initialize the framebuffers with black
  memset(framebuffer1, 0, FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));
  memset(framebuffer2, 0, FRAMEBUFFER_HALF_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));

  tft.setCursor(0, 170);
  tft.setTextSize(2);
  tft.printf("Framebuffers initialized.\n\nFree heap:%ld", ESP.getFreeHeap());
  tft.setTextColor(textColor);
  delay(500);

  int xPos = 0;
  float accum = 0;
  long offset = 0;
  long total = 0;
  long strength = 0;
  uint32_t startTime;
  uint16_t resolution = 1000;  // 1KHz
  float mult = 1.0;            // signal strenght multiplier
  bool running = true;
  long cellSize = (endPoint - startPoint) / FRAMEBUFFER_FULL_WIDTH;  // cellSize is the frequency range that gets scanned for 1 pixel
  uint16_t stp = cellSize / 25;                                      // can't change filter bandwith, need to scan each cell. 25 steps in every cellSize sufficient up to 30 MHz range

  if (stp < resolution)  // stepsize below resolution makes no sense, so we change it to resolution
    stp = resolution;
  if (stp > 5000)
    stp = 5000;  // for wide scans > 30MHz

  displayDebugInfo = false;
  showFREQ = false;  // do not show FREQ display

  // draws a scale and 10 frequency marks on top of the screen
  float scale = 0;
  tft.setTextSize(1);
  tft.drawFastHLine(0, 0, WATERFALL_SCREEN_WIDTH, TFT_WHITE);
  tft.drawFastVLine(WATERFALL_SCREEN_WIDTH - 1, 0, 5, TFT_WHITE);
  for (int i = 0; i < WATERFALL_SCREEN_WIDTH; i += WATERFALL_SCREEN_WIDTH / 10) {
    tft.drawFastVLine(i, 0, 5, TFT_WHITE);
    tft.setCursor(i, 8);
    if (i < WATERFALL_SCREEN_WIDTH)
      tft.printf("%.2f", scale + startPoint / 1000000.0);
    scale += span / 10;
  }

  tft.setTextSize(2);

  /// loop that reads RSSI and feeds the framebuffers
  while (running) {

    if (!xPos)
      startTime = millis();

    while (offset < cellSize) {
      FREQ = startPoint + total + offset;
      tuneSI5351();
      mult = (analogRead(FINETUNE_PIN) / 100) - 1;  //  mult =  0 - 39, adjust color gradient with finetune pot
      si4735.getCurrentReceivedSignalQuality(0);
      strength = si4735.getCurrentRSSI();

      long corrFactor = (cellSize / 2000);  // emprirical correction so that color gradient stays within pot range
      if (corrFactor > 50)
        corrFactor = 50;
      accum += abs((strength * mult) / (corrFactor + 1));
      offset += stp;
    }


    uint16_t clr = valueToWaterfallColor(((int)accum - 800) * 3);  // transform to rgb565
    tft.fillCircle(470, 315, 4, clr);
    tft.drawPixel(2 * xPos + 1, 0, TFT_DARKGREY);
    xPos++;
    newLine[xPos] = clr;  // fill array, framebuffer is 240 * 230

    accum = 0;
    offset = 0;
    total += cellSize;


    if (xPos == 239) {

      tft.drawFastHLine(0, 0, WATERFALL_SCREEN_WIDTH, TFT_WHITE);
      addLineToFramebuffer(newLine);  // Add the new line to frambuffer and update display

      uint32_t lineTime = (millis() - startTime);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(250, 300);
      tft.printf("%ld min per screen", lineTime * WATERFALL_SCREEN_HEIGHT / 1000 / 60);


      total = 0;
      xPos = 0;
    }

    if (digitalRead(ENC_PRESSED) == LOW)
      break;

  }  // end while running


  // rebuild the main screen
  rebuildMainScreen();

  return;
}


//##########################################################################################################################//

void rebuildMainScreen() {

  free(framebuffer1);
  free(framebuffer2);
  tft.setTextSize(2);
  tRel();
  bootScreen(false);  // restore screen
  drawBigBtns();      // redraw buttons
  STEP = 0;           // change STEP so that the display updates
  displaySTEP();
  STEP = 5000;  // change step back
  showFREQ = true;
  displayFREQ(FREQ);
  redrawMainScreen = true;
  displayDebugInfo = true;
  si4735.setAudioMute(false);
  si4735.setHardwareAudioMute(false);
  tx = ty = pressed = 0;
}

//##########################################################################################################################//

void wIntro(long startPoint, long endPoint) {

  modType = AM;
  loadModulation4735();
  si4735.setAudioMute(true);
  si4735.setHardwareAudioMute(true);
  displayDebugInfo = false;
  tft.setCursor(0, 50);
  tft.setTextSize(2);
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.println("Slow waterfall for band monitoring\n");
  tft.println("Use fine tune pot for sensitivity and");
  tft.println("use dot in lower right corner to adjust color gradient.\n");
  tft.println("Press encoder to leave.");
  tft.setCursor(12, 300);
  tft.setTextColor(TFT_GREEN);
  tft.printf("%.2fMHz-%.2fMHz", startPoint / 1000000.0, endPoint / 1000000.0);
}

//##########################################################################################################################//
void buffErr() {

  tft.setCursor(0, 50);
  tft.println("No memory for framebuffers");
  tft.printf("\nFree heap:%ld", ESP.getFreeHeap());
  tft.setCursor(0, 70);
  delay(3000);
  ESP.restart();
}

//##########################################################################################################################//

// function to interpolate between two colors
uint16_t interpolate(uint16_t color1, uint16_t color2, float factor) {
  // Extract RGB elements from RGB565
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;

  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;

  // Interpolate each component
  uint8_t r = r1 + factor * (r2 - r1);
  uint8_t g = g1 + factor * (g2 - g1);
  uint8_t b = b1 + factor * (b2 - b1);

  // Combine back
  return (r << 11) | (g << 5) | b;
}


//##########################################################################################################################//


uint16_t valueToWaterfallColor(int value) {
  // Define RGB565 color values for transitions
  uint16_t colors[] = {
    TFT_NAVY,     //  (0-400)
    TFT_BLUE,     //  (401-800)
    TFT_SKYBLUE,  //  (801-1200)
    TFT_WHITE,    //  (1201-1400)
    TFT_ORANGE,   // (1401-1600)
    TFT_RED       // (1601-max)
  };

  // Define transition points
  int transitionPoints[] = { 0, 400, 800, 1200, 1400, 1800, 20000 };

  // Find the right transition
  for (int i = 0; i < 5; i++) {
    if (value >= transitionPoints[i] && value <= transitionPoints[i + 1]) {
      float factor = float(value - transitionPoints[i]) / (transitionPoints[i + 1] - transitionPoints[i]);
      return interpolate(colors[i], colors[i + 1], factor);
    }
  }

  // value out of range, return closest color
  return value < 0 ? colors[0] : colors[5];
}


//##########################################################################################################################//


void addLineToFramebuffer(uint16_t* newLine) {
  // Copy the new line into framebuffers
  memcpy(&framebuffer1[currentLine * FRAMEBUFFER_HALF_WIDTH], newLine, FRAMEBUFFER_HALF_WIDTH * sizeof(uint16_t));
  memcpy(&framebuffer2[currentLine * FRAMEBUFFER_HALF_WIDTH], newLine + FRAMEBUFFER_HALF_WIDTH, FRAMEBUFFER_HALF_WIDTH * sizeof(uint16_t));

  // Displayframebuffer
  updateDisplay();

  // Move to the next liine
  currentLine = (currentLine + 1) % FRAMEBUFFER_HEIGHT;
}


//##########################################################################################################################//

void updateDisplay() {
  // Scroll screen content down
  for (int y = WATERFALL_SCREEN_HEIGHT - 1; y > 0; y--) {
    for (int x = 0; x < WATERFALL_SCREEN_WIDTH; x++) {
      // Interpolate color from the framebuffers
      uint16_t color = getInterpolatedColor(x, (currentLine + y) % FRAMEBUFFER_HEIGHT);

      if ((x % (WATERFALL_SCREEN_WIDTH / 10) == 0) && (x != 0)) {
        if ((y % (WATERFALL_SCREEN_HEIGHT / 15) == 0) && (y != 0)) {
          tft.drawPixel(x, WATERFALL_SCREEN_HEIGHT - y + 30, TFT_WHITE);  // Draw white dots as orientation help
        } else {
          tft.drawPixel(x, WATERFALL_SCREEN_HEIGHT - y + 30, TFT_BLACK);  // over black lines
        }
      } else {
        tft.drawPixel(x, WATERFALL_SCREEN_HEIGHT - y + 30, color);
      }
    }
  }

  // Draw new line on the top
  for (int x = 0; x < WATERFALL_SCREEN_WIDTH; x++) {
    uint16_t color = getInterpolatedColor(x, currentLine);
    tft.drawPixel(x, 30, color);
  }
}

//##########################################################################################################################//

uint16_t getInterpolatedColor(int x, int y) {
  // Calculate the scale factor
  float scaleFactorX = (float)FRAMEBUFFER_HALF_WIDTH / (WATERFALL_SCREEN_WIDTH / 2);

  // Find the nearest indices in the original framebuffer
  int originalX1 = (int)((x % (WATERFALL_SCREEN_WIDTH / 2)) * scaleFactorX);
  int originalX2 = min(originalX1 + 1, FRAMEBUFFER_HALF_WIDTH - 1);
  float fractionX = ((x % (WATERFALL_SCREEN_WIDTH / 2)) * scaleFactorX) - originalX1;

  uint16_t color1, color2;
  if (x < WATERFALL_SCREEN_WIDTH / 2) {
    color1 = framebuffer1[y * FRAMEBUFFER_HALF_WIDTH + originalX1];
    color2 = framebuffer1[y * FRAMEBUFFER_HALF_WIDTH + originalX2];
  } else {
    color1 = framebuffer2[y * FRAMEBUFFER_HALF_WIDTH + originalX1];
    color2 = framebuffer2[y * FRAMEBUFFER_HALF_WIDTH + originalX2];
  }

  // Interpolate the colors
  return interpolateColor(color1, color2, fractionX);
}

//##########################################################################################################################//


uint16_t interpolateColor(uint16_t color1, uint16_t color2, float fraction) {
  // Extract RGB
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;

  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;

  // Interpolate
  uint8_t r = r1 + fraction * (r2 - r1);
  uint8_t g = g1 + fraction * (g2 - g1);
  uint8_t b = b1 + fraction * (b2 - b1);

  // Combine into a 16-bit color
  return (r << 11) | (g << 5) | b;
}
