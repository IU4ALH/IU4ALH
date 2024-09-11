//##########################################################################################################################//
// Internet tool collection


void connectWIFI() {

  int ctr = 0;
  tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);
  WiFi.begin(ssid, password);
  tft.setCursor(5, 20);
  tft.print("Connecting to WIFI\n");
  while (WiFi.status() != WL_CONNECTED) {
    ctr++;
    if (ctr == 20)
      ESP.restart();
    tft.print(".");
    delay(500);
  }
  tft.println("connected.");
}

//##########################################################################################################################//


void getimage(bool fill) {  // downloads a jpg or png file

  if (fill)
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

  if (!SPIFFS.begin()) {
    tft.printf("\n\nSPIFFS Mount Failed");
    SPIFFS.format();
    ESP.restart();
  } else
    tft.printf("\n\nSPIFFS mounted\nDownloading...");

  File file = SPIFFS.open("/image.img", FILE_WRITE);

  if (!file) {
    tft.print("Failed to open file for writing\n");
    return;
  }

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();  // Do not use HTTPS certificate

  switch (imageSelector) {  // use the predefined URL's
    case 0:
      http.begin(client, host);
      break;
    case 1:
      http.begin(client, host1);
      break;
    case 2:
      http.begin(client, host2);
      break;
    case 3:
      http.begin(client, host3);
      break;
    case 4:
      http.begin(client, host4);
      break;
    case 5:
      http.begin(client, host5);
      break;
  }

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    http.writeToStream(&file);
    tft.println("\nDownload successful");
  } else {
    tft.println("Download failed");
  }

  file.close();
  http.end();  // Close connection
}


//##########################################################################################################################//

void displayPNG() {

  int rc = png.open("/image.img", myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rc == PNG_SUCCESS) {
    rc = png.decode(NULL, 0);
    png.close();
  } else {
    tft.printf("Error opening PNG file");
  }
}


//##########################################################################################################################//

void *myOpen(const char *filename, int32_t *size) {
  myfile = SPIFFS.open(filename);
  if (!myfile) return NULL;
  *size = myfile.size();
  return &myfile;
}

//##########################################################################################################################//
void myClose(void *handle) {
  if (myfile) myfile.close();
}

//##########################################################################################################################//
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}

//##########################################################################################################################//
int32_t mySeek(PNGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

//##########################################################################################################################//
void PNGDraw(PNGDRAW *pDraw) {
  uint16_t usPixels[2500];
  const uint32_t leftShift = -110, downShift = 70;
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushRect(leftShift + xShift, pDraw->y - downShift + yShift, pDraw->iWidth, 1, usPixels);
}

//##########################################################################################################################//

void drawIBtns() {

  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite row 3 from mainscreen
  draw8Buttons(TFT_BLUE, TFT_NAVY);          // draw new buttons
  struct Button {
    int x;
    int y;
    const char *label;
  };

  Button buttons[] = {
    { 20, 190, "FOF2" }, { 20, 210, "Map" }, { 100, 190, "SW" }, { 100, 210, "Fade" }, { 190, 200, "Earth" }, { 265, 248, "NOAA" }, { 273, 200, "Sun" }, { 270, 268, "Alerts" }, { 190, 248, "SW" }, { 190, 268, "Cond." }, { 100, 248, "Sat" }, { 100, 268, "Image" }, { 20, 248, "Fore" }, { 20, 268, "cast" }
  };

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_GREEN);

  etft.setTTFFont(Arial_14);

  for (int i = 0; i < 14; i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }
}

//##########################################################################################################################//

void readIBtns() {

  tDoublePress();

  preferences.putBool("fastBoot", true);    // Does not go back to main menu, uses fast restart instead. Image displays eats up too much memory and does not reliably free it
                                            //, so better reboot after leaving
  int buttonID = getButtonID();
 
  if (buttonID >= 31) 
      pressSound = false; // avoid squeaking sound while rebooting

  switch (buttonID) {
    case 31:  // propagation forecast
      imageSelector = 0;
      PNGWrapper(15);                         // wrapper reloads image every 15 minutes
      break;
    case 32:  // SW fade map
      imageSelector = 1;
      swappedJPEG = false;
      xShift = -40;
      JPEGWrapper(15);
      break;
    case 33:  // globe view
      imageSelector = 2;
      swappedJPEG = true;
      xShift = +60;
      yShift = -10;
      JPEGWrapper(15);
      break;
    case 34:  // sun image
      imageSelector = 3;
      swappedJPEG = true;
      yShift = -70;
      JPEGWrapper(15);
      break;
    case 41: // propagation forecast
      reportSelector = 0;
      reportWrapper(30);
      break;
    case 42:
      imageSelector = 4;  // Central America sat image
      swappedJPEG = true;
      xShift = -420;
      yShift = -620;
      JPEGWrapper(15);
      break;
    case 43:
      imageSelector = 5;  // condition map
      xShift = 0;
      yShift = -15;
      PNGWrapper(30);
      break;
    case 44:  // NOAA spaceweathr alerts
      reportSelector = 1;
      reportWrapper(30);
      break;
  }
  tx = ty = pressed = 0;
  rebuildMainScreen();
  return;
  tRel();
}

//##########################################################################################################################//

void displayJpeg() {
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);

  drawJpeg("/image.img", 0, 0);  // All image files are downloaded under filename image.img
}

//##########################################################################################################################//

void drawJpeg(const char *filename, int xpos, int ypos) {
  File jpegFile = SPIFFS.open(filename, "r");  // Declare and openfile
  if (!jpegFile) {
    tft.println("Failed to open file");
    return;
  }
  JpegDec.decodeFsFile(filename);
  renderJPEG(xpos, ypos);
  jpegFile.close();
}
//##########################################################################################################################//


void renderJPEG(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;
  uint32_t min_w = min((uint32_t)mcu_w, max_x % mcu_w);
  uint32_t min_h = min((uint32_t)mcu_h, max_y % mcu_h);
  bool isSwapped = swappedJPEG;

  while (isSwapped ? JpegDec.readSwappedBytes() : JpegDec.read()) {
    pImg = JpegDec.pImage;
    uint16_t mcu_x = JpegDec.MCUx;
    uint16_t mcu_y = JpegDec.MCUy;
    uint16_t mcu_pix_x = mcu_x * mcu_w;
    uint16_t mcu_pix_y = mcu_y * mcu_h;
    uint16_t mcu_w_curr = (mcu_pix_x + mcu_w <= max_x) ? mcu_w : min_w;
    uint16_t mcu_h_curr = (mcu_pix_y + mcu_h <= max_y) ? mcu_h : min_h;
    {
      tft.pushImage(mcu_pix_x + xpos + xShift, mcu_pix_y + ypos + yShift, mcu_w_curr, mcu_h_curr, pImg);
    }
  }
}

//##########################################################################################################################//

void JPEGWrapper(int cycle) {  // JPG wrapper, cycle = reload time in minutes

  while (true) {
    connectWIFI();
    getimage(false);
    WiFi.disconnect();
    displayJpeg();
    for (long i = 0; i < cycle * 600; i++) {
      get_Touch();
      if (pressed)
        ESP.restart();
      delay(100);
    }
  }
}
//##########################################################################################################################//

void PNGWrapper(int cycle) {  

  while (true) {
    connectWIFI();
    getimage(false);
    WiFi.disconnect();
    displayPNG();
    for (long i = 0; i < cycle * 600; i++) {
      get_Touch();
      if (pressed)
        ESP.restart();
      delay(100);
    }
  }
}
//##########################################################################################################################//

void reportWrapper(int cycle) {  

  encLockedtoSynth = false;
  static long offset = 0, oldOffset = 0;

  while (true) {
    getReport();
    displayReport(offset);
    for (long i = 0; i < cycle * 600; i++) {
      if (clw)  // change page
        offset += 400;
      if (cclw)
        offset -= 400;
      if (offset < 0)
        offset = 0;

      clw = 0;
      cclw = 0;

      if (oldOffset != offset) {
        displayReport(offset);
        oldOffset = offset;
      }
      get_Touch();
      if (pressed)
        ESP.restart();
      delay(100);
    }
  }
}

//##########################################################################################################################//


void getReport(void) {  //writes JSON or text page to file. Takes looooong....

  connectWIFI();
  tft.println("Preparing report...please be patient");
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();  //do not use https certificate

  if (reportSelector == 0) { //NOAA forecast uses text file
    client.connect(fr, 443);
    http.begin(client, fr);
  }

  if (reportSelector == 1) {
    client.connect(al, 443); // NOAA alert uses JSON file
    http.begin(client, al);
  }


  uint16_t httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK)
    tft.println("Connection error");
  if (!SPIFFS.begin()) {
    tft.printf("\n\nSPIFFS Mount Failed");
    SPIFFS.format();
    ESP.restart();
  } else
    tft.printf("\n\nSPIFFS mounted\n\nDownloading...");
  File file = SPIFFS.open("/fc.txt", "w");
  http.writeToStream(&file);
  http.end();
  WiFi.disconnect();
}


//##########################################################################################################################//

void displayReport(size_t offset) {

  int bufferSize = 1000;
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    tft.printf("SPIFFS Mount Failed\n");
    return;
  }

  // Open file
  File file = SPIFFS.open("/fc.txt", FILE_READ);
  if (!file) {
    tft.printf("Failed to open file:fc.txt");
    return;
  }

  // Check if offset is inside file size
  size_t fileSize = file.size();
  if (offset >= fileSize) {
    tft.printf("End");
    offset = fileSize;
  }

  // goto offset position
  file.seek(offset, SeekSet);

  //read into the buffer
  size_t bytesRead = file.readBytes(txtbuf, bufferSize);
  txtbuf[bytesRead] = '\0';
  file.close();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 0);

  uint16_t ccounter = 0;
  uint16_t cursorX;
  uint8_t cb;

  while (txtbuf[ccounter]) {
    cb = txtbuf[ccounter];
    cursorX = tft.getCursorX();

    if (cursorX > 479) {
      tft.setCursor(0, tft.getCursorY() + 23);  // new line
    }

    if (cb == '{')
      tft.setCursor(0, tft.getCursorY() + 46);  // new JSON


    // Suppress control characters
    if (!(cb == 195 || cb == '{' || cb == '}' || cb == '\\' || cb == '[')) {
      tft.write(cb);
    }

    if (reportSelector == 1) {
      if (cb == '\\') {  // remove \r\n sequence in alerts
        ccounter += 3;
        tft.write(' ');
      }
    }
    ccounter++;
  }
}

//##########################################################################################################################//


