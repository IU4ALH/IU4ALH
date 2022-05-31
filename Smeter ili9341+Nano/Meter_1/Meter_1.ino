/*
 An example analogue meter using a ILI9341 TFT LCD screen

 This example uses the hardware SPI only and a board based
 on the ATmega328

 Needs Fonts 2, and 7 (also Font 4 if using large scale label)

 Comment out lines 153 and 197 to reduce needle flicker and
 to remove need for Font 4 (which uses ~8k of FLASH!)
 
 Alan Senior 23/2/2015
 */

// These are the connections for the UNO to display
#define sclk 13  // Don't change
#define mosi 11  // Don't change
#define cs   10  // If cs and dc pin allocations are changed then 
#define dc   9   // comment out #define F_AS_T line in "Adafruit_ILI9341_FAST.h"
                 // (which is inside "Adafriit_ILI9341_AS" library)

#define rst  7  // Can alternatively connect this to the Arduino reset

#include <Adafruit_GFX_AS.h>     // Core graphics library
#include <Adafruit_ILI9341_AS.h> // Fast hardware-specific library
#include <SPI.h>

#define ILI9341_GREY 0x5AEB

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(cs, dc, rst); // Invoke custom library

float ltx = 0;    // Saved x coord of bottom of needle
uint16_t osx = 120, osy = 120; // Saved x & y coords
uint32_t updateTime = 0;       // time for next update

int old_analog =  -999; // Value last displayed
int old_digital = -999; // Value last displayed


void setup(void) {
  tft.init();
  tft.setRotation(2);

  tft.fillScreen(ILI9341_BLACK);

  analogMeter(); // Draw analogue meter
  
  digitalMeter(); // Draw digital meter

  updateTime = millis(); // Next update time
}


void loop() {
  if (updateTime <= millis()) {
    updateTime = millis() + 500;

    int reading = 0;
    reading = random(-50, 151); // Test with random value
    reading = map(analogRead(A0),0,1023,0,100); // Test with value form Analogue 0

    showDigital(reading); // Update digital reading
    
    plotNeedle(reading, 8); // Update analogue meter, 8ms delay per needle increment
    
  }
}


// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter()
{
  // Meter outline
  tft.fillRect(0, 0, 239, 126, ILI9341_GREY);
  tft.fillRect(5, 3, 230, 119, ILI9341_WHITE);
  
  tft.setTextColor(ILI9341_BLACK);  // Text colour
  
  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;
    
    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (100 + tl) + 120;
    uint16_t y0 = sy * (100 + tl) + 140;
    uint16_t x1 = sx * 100 + 120;
    uint16_t y1 = sy * 100 + 140;
    
    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (100 + tl) + 120;
    int y2 = sy2 * (100 + tl) + 140;
    int x3 = sx2 * 100 + 120;
    int y3 = sy2 * 100 + 140;
    
    // Yellow zone limits
    //if (i >= -50 && i < 0) {
    //  tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_YELLOW);
    //  tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_YELLOW);
    //}
    
    // Green zone limits
    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_GREEN);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_ORANGE);
    }
    
    // Short scale tick length
    if (i % 25 != 0) tl = 8;
    
    // Recalculate coords incase tick lenght changed
    x0 = sx * (100 + tl) + 120;
    y0 = sy * (100 + tl) + 140;
    x1 = sx * 100 + 120;
    y1 = sy * 100 + 140;
    
    // Draw tick
    tft.drawLine(x0, y0, x1, y1, ILI9341_BLACK);
    
    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (100 + tl + 10) + 120;
      y0 = sy * (100 + tl + 10) + 140;
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("25", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("50", x0, y0 - 6, 2); break;
        case 1: tft.drawCentreString("75", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("100", x0, y0 - 12, 2); break;
      }
    }
    
    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * 100 + 120;
    y0 = sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, ILI9341_BLACK);
  }
  
  tft.drawString("%RH", 5 + 230 - 40, 119 - 20, 2); // Units at bottom right
  tft.drawCentreString("%RH", 120, 70, 4); // Comment out to avoid font 4
  tft.drawRect(5, 3, 230, 119, ILI9341_BLACK); // Draw bezel line
  
  plotNeedle(0,0); // Put meter needle at 0
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(int value, byte ms_delay)
{
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  char buf[8]; dtostrf(value, 4, 0, buf);
  tft.drawRightString(buf, 40, 119 - 20, 2);

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 110) value = 110;

  // Move the needle util new value reached
  while (!(value == old_analog)) {
    if (old_analog < value) old_analog++;
    else old_analog--;
    
    if (ms_delay == 0) old_analog = value; // Update immediately id delay is 0
    
    float sdeg = map(old_analog, -10, 110, -150, -30); // Map value to angle 
    // Calcualte tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg+90) * 0.0174532925);
    
    // Erase old needle image
    tft.drawLine(120 + 20 * ltx - 1, 140 - 20, osx - 1, osy, ILI9341_WHITE);
    tft.drawLine(120 + 20 * ltx, 140 - 20, osx, osy, ILI9341_WHITE);
    tft.drawLine(120 + 20 * ltx + 1, 140 - 20, osx + 1, osy, ILI9341_WHITE);
    
    // Re-plot text under needle
    tft.setTextColor(ILI9341_BLACK);
    tft.drawCentreString("%RH", 120, 70, 4); // // Comment out to avoid font 4
    
    // Store new needle end coords for next erase
    ltx = tx;
    osx = sx * 98 + 120;
    osy = sy * 98 + 140;
    
    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(120 + 20 * ltx - 1, 140 - 20, osx - 1, osy, ILI9341_RED);
    tft.drawLine(120 + 20 * ltx, 140 - 20, osx, osy, ILI9341_MAGENTA);
    tft.drawLine(120 + 20 * ltx + 1, 140 - 20, osx + 1, osy, ILI9341_RED);
    
    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;
    
    // Wait before next update
    delay(ms_delay);
  }
}

// #########################################################################
// Draw 3 digit digital meter with faint 7 segment image
// #########################################################################
void digitalMeter()
{
  int xpos = 118, ypos = 134; // was 134
  tft.fillRect(xpos - 52, ypos - 5, 2 * 54, 59, ILI9341_GREY);
  tft.fillRect(xpos - 49, ypos - 2, 2 * 51, 53, ILI9341_BLACK);
  tft.setTextColor(7 << 11, ILI9341_BLACK); // Plot over numbers in dim red
  tft.drawString("888", xpos - 48, ypos+1, 7);
}

// #########################################################################
// Update digital meter reading
// #########################################################################
void showDigital(int value)
{
  if (value==old_digital) return; // return if no change to prevent flicker
  if (value < 0) value = 0; //Constrain lower limit to 0
  if (value > 999) value = 999; //Constrain upper limit to 999
  
  int xpos = 118, ypos = 134+1; // Position with location tweak
  tft.setTextColor(7 << 11, ILI9341_BLACK); // Plot over numbers in dim red
  tft.drawString("888", xpos - 47, ypos, 7); //Erase old value
  
  // Nb. 32 pixels wide +2 gap per digit
  
  // Update with new value
  tft.setTextColor(ILI9341_RED); // Dont draw background to leave dim segments
  if (value < 10) tft.drawNumber(value, xpos+19, ypos, 7);
  else if (value < 100) tft.drawNumber(value, xpos - 14, ypos, 7);
  else tft.drawNumber(value, xpos - 47, ypos, 7);
  old_digital = value;
}
