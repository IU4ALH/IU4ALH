  /*

  Functions listed by tab

Radio:

RotaryEncFreq()
setup()
loop()
restartSynth() 
encoderChangeFREQ() 
FREQCheck()
displayFREQ(long freq)
displaySTEP()
setSTEP()
readTouchCoordinates()
drawButton(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color1, uint32_t color2)
draw8Buttons(uint16_t color1, uint16_t color2)
draw16Buttons()
fillBackground()
Smeter() 
DrawSmeter()
loopTimer() 
readSquelchPot()
setSquelch()
displayText(int x, int y, in length, int height, const char* text)
indicatorTouch() 


//##########################################################################################################################//

SecondScreen:

SecScreen()
drawSecBtns() 
readSecBtns()
setClockMode()
setAGCMode()
writeAGCMode(uint8_t AGCDIS, uint8_t AGCIDX)
printAGC()
setBFO()
SelectButtonStyle() 


//##########################################################################################################################//

Keypad:
freqScreen() 
drawKeypadButtons()
readKeypadButtons()
void keyPadErr() 
redrawMainScreen()
tDoublePress()
tPress()
drawNumPad()
void showTouch (int row, int column)
longPress()
get_Touch()
//##########################################################################################################################//

MainScreen
mainScreen() 
drawMainButtons()
readMainBtns()
setBandwith()
printBandWidth()
setTinySAMmode()
printTinySAMode()
setMode()
int getButtonID()
low_High_Injection()
waterFall()
uint16_t interpolate(uint16_t color1, uint16_t color2, float factor)
uint16_t valueToWaterfallColor(int value)
addLineToFramebuffer(uint16_t* newLine) 
updateDisplay() 
uint16_t getInterpolatedColor(int x, int y)
uint16_t interpolateColor(uint16_t color1, uint16_t color2, float fraction)
drawIBtns()
readIBtns()
displayJpeg()
drawJpeg()
renderJPEG(int xpos, int ypos)
connectWIFI()
loadJPEG( int repeatTime) 

//##########################################################################################################################//

Init:

bootScreen()
spriteBorder()
drawBigBtns()
SI5351_Init()
radioInit()
loadLastSettings()

//##########################################################################################################################//

SetBand:

setBand()

//##########################################################################################################################//

SI4735Controls:
loadSSB()
setAM()
reset()


//##########################################################################################################################//

ThirdScreen:

ThirdScreen()
drawThiBtns()
readThiBtns()
calibSI5351()
setVol()
setIF()
setAvcAmMaxGain()
printAvcAmMaxGain(uint8_t AvcAmMaxGain)
setSMute() 
printSmute(uint8_t AMSoftMuteSnrThreshold)


//##########################################################################################################################//

FourthScreen:
drawFoBtns()
readFoBtns()
setAmAGCRelRate()
setAmAGCAttRate() 
gpioTest()

//##########################################################################################################################//

VFO:

tuneSI5351()
settleSynth()
fineTune()
saveCurrentSettings()
setFreqSI4735()
clockSelector()
getFilterNumber() 
showFilterNumber() 
void switchFilter () 

//##########################################################################################################################//

Mode:

setMode() 
loadModulation4735()
selectModulation()
printModulation() 
setScanRange() 

//##########################################################################################################################//

Scan:

setScanRange() 
printScanRange()
freq_UP_DOWN();
SCANfreq_UP_DOWN
getFreqLimits(); 

//##########################################################################################################################//
Memo:

printMemoName();
showMemo()
writeMemo()
readMemo() 
memoAction(bool isWrite)
tuneMemo()
memoScanner()
drawMarker(int buttonID, uint16_t COLOR)
//##########################################################################################################################//
Waterfall:
waterFall()
wIntro();
rebuildMainScreen();
buffErr();
uint16_t interpolate(uint16_t color1, uint16_t color2, float factor)
uint16_t valueToWaterfallColor(int value)
addLineToFramebuffer(uint16_t* newLine) 
updateDisplay() 
uint16_t getInterpolatedColor(int x, int y)
uint16_t interpolateColor(uint16_t color1, uint16_t color2, float fraction)

//##########################################################################################################################//
Inet:

connectWIFI()
getImage()
displayPNG()
myOpen()
myClose()
myRead()
mySeek()
PNGDraw()
drawIBtns()
readIBtns()
displayJPpeg()
drawJpeg()
renderJPEG()
JPEGWrapper(int cycle)
PNGWrapper(int cycle)
alertWrapper(int cycle)
getReport()
displayReport()
getForecast()
displayForecast()


//##########################################################################################################################//

FFT:

audioSpectrum()
displayBand() 
getBandVal()
displayBand(int band, int dsize)
tuneCWDecoder()
CWDecoder() 
AddCharacter()
CodeToChar() 


*/
