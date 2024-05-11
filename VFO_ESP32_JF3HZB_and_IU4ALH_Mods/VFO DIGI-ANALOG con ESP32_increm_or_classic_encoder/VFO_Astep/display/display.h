/* 
 * File:   display.h
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2019/02/10, 22:13
 */

#ifndef _display_
#define _display_

/*----------------------------------------
 Select Display controller
   ST7735 : LCD  (1.8inch 128x160 TFT)
   SEPS525: OLED ( NHD-1.69-160128UGC3 ) 
-----------------------------------------*/

//#define SEPS525
#define ST7735

/*---------------------------------------*/


#define Xw 128
#define Yw 160

#define Nx Yw
#define Ny Xw

void display_init(void);
void Transfer_Image(void);
void trans65k(void);
void d_Command(uint8_t d);

#endif
