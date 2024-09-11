 // draws and reads permanent buttons and  screen elements
void bootScreen(bool initHW) {
int16_t si4735Addr = 0;

bool fastBoot = preferences.getBool("fastBoot", 0); 

if (initHW) {
  tft.init();
  tft.setRotation(1);
  tft.setTextColor(TFT_FOREGROUND);
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  
  if (!fastBoot){
  tft.setTextSize(3);
  tft.setCursor(0, 30);
  tft.println("SI4732 + SI5351\n\nDSP Receiver\n");
  tft.setTextSize(2);
  si5351.update_status();
  tft.setCursor(0, 160);
  tft.print("SI5351 status: ");
  tft.print(si5351.dev_status.SYS_INIT);
  tft.print(si5351.dev_status.LOL_A);
  tft.print(si5351.dev_status.LOL_B);
  tft.print(si5351.dev_status.LOS);
  
  tft.setCursor(0, 200);
  if (si4735Addr == 0) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("Si4735 not detected");
    Serial.println("Si4735 not detected");
  } else {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(0, 220);
    tft.print("Si4732 found, Addr :  ");
    tft.println(si4735Addr, HEX);
  }

  tft.setCursor(0, 240);
  tft.print(ver);
  tft.setTextColor(textColor);
  delay(2000);

  }

  if (si4735Addr == 17) {
    si4735.setDeviceI2CAddress(0);
  } else {
    si4735.setDeviceI2CAddress(1);
  }

} // end if initHW


preferences.putBool("fastBoot", 0); // remove fastboot flag
  // draw frame
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.drawFastHLine(0, 46, DISP_WIDTH, TFT_GRID);
  tft.drawFastVLine(340, 46, 246, TFT_GRID);
  tft.fillCircle(162 + 13, 47, 3, TFT_GREEN);  //draw a circle to display finetune pot center
  spriteBorder();
  tft.setTextSize(2);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(405, 300);
  tft.print(ver); // version
  tft.setTextColor(textColor);
}

//##########################################################################################################################//

void spriteBorder() {

  spr.createSprite(480, 1);
  spr.pushImage(0, 0, 480, 1, (uint16_t *)border480);
  spr.pushSprite(0, 0);
  spr.pushSprite(0, 1);
  spr.pushSprite(0, 292);
  //spr.pushSprite(0, 293);
  spr.deleteSprite();
  
  spr.createSprite(1, 292);
  spr.pushImage(0, 0, 1, 292, (uint16_t *)border320);
  spr.pushSprite(0, 0);
  spr.pushSprite(1, 0);
  spr.pushSprite(479, 0);
  spr.pushSprite(478, 0);
  spr.deleteSprite();

}

//##########################################################################################################################//


void SI5351_Init() {

  tft.setTextSize(2);

for (int i = 0; i < 10; i++) {
    bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    if (!i2c_found) {
      tft.setCursor(20, 50 + 20 * i);
      tft.print("SI5351 not detected!");
      tft.println();
      delay(1000);
    }
  }
  si5351.set_correction(preferences.getLong("calib", 0), SI5351_PLL_INPUT_XO);  // read calibration from preferences
  si5351.set_ms_source(SI5351_CLK0, SI5351_PLLB);  // Clock for RX high band // assign to the other PLL to avoid 100 MHz issue
  si5351.output_enable(SI5351_CLK2, 1); 
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA); // Clock for RX low band
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA); // Clock for RX high band
  si5351.output_enable(SI5351_CLK1, 0);  //Clock for TX
  si5351.output_enable(SI5351_CLK0,0); 
}
//##########################################################################################################################//


void radioInit() {
 
  si4735.setAudioMuteMcuPin(MUTEPIN);
  digitalWrite(MUTEPIN, HIGH);
  si4735.setup(RESET_PIN, 1); 
  if (modType == USB || modType == LSB || modType == SYNC || modType == CW)  
    STEP = 1000;
  else if (modType == AM) 
    STEP = 5000; 
  loadModulation4735(); 
  si4735.setVolume(preferences.getChar("Vol", 45));
  si4735.setAMSoftMuteSnrThreshold(preferences.getChar("SMute",0));
  si4735.setAvcAmMaxGain( preferences.getChar("AVC", 0));
  digitalWrite(MUTEPIN, LOW);
}

//##########################################################################################################################//

void drawBigBtns() {

  drawButton(345, 51, 130, 78, TFT_BLUE, TFT_NAVY);   // UP button
  drawButton(345, 211, 130, 78, TFT_BLUE, TFT_NAVY);  // DOWN button
  drawButton(345, 131, 130, 78, TFT_BLUE, TFT_NAVY);  // MODE button
  tft.setTextSize(3);

  if (!scanMode) {
   
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(375, 158);
    tft.print("MODE");
    tft.setCursor(390, 80);
    tft.print("UP");
    tft.setCursor(375, 240);
    tft.print("DOWN");
  }

  else {
    tft.setTextColor(TFT_SKYBLUE);
    tft.setCursor(370, 145);
    tft.print("SET");
    tft.setCursor(370, 175);
    tft.print("RANGE");
    tft.setCursor(375, 65);
    tft.print("SEEK");
    tft.setCursor(375, 98);
    tft.print("UP");
    tft.setCursor(375, 220);
    tft.print("SEEK");
    tft.setCursor(375, 255);
    tft.print("DOWN");
    tft.setTextSize(2);
    tft.setTextColor(textColor);
  }
   tft.setTextSize(2);
}


//##########################################################################################################################//

void loadLastSettings() {

  FREQ = preferences.getLong("lastFreq", 0);          // load last Freq
  bandWidth = preferences.getInt("lastBw", 0);        // last bandwidth
  modType = preferences.getChar("lastMod", 0);        //last modulation type
  altStyle = preferences.getBool("lastStyle", 0);     //plain or sprite style
  pressSound = preferences.getBool("pressSound", 0);  // short beep when pressed
  sMode = preferences.getChar("spectr", 0);           // audio spectrum analyzer mode 
  sevenSeg = preferences.getBool("sevenSeg", 0);      // frequency display font
  showSNR = preferences.getBool("showSNR", 0);       // show SNR with Smeter
  SNRSquelch = preferences.getBool("SNRSquelch", 0); // SNR controls squelch, this is less sensible, but does not need squelchpot. Does not work with NBFM signals
  buttonSelected = preferences.getInt("sprite", 4); // load sprite for buttons
}

//##########################################################################################################################//