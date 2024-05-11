/* 
 * File:   si5351.h
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2019/02/11, 23:07
 */


#ifndef __si5351H___
#define __si5351H___

void si5351_init(void);
void set_freq(unsigned long freq);
void set_car_freq(unsigned long freq, unsigned char EN, unsigned char RST);
void cmd_si5351_block(unsigned char reg_No, unsigned char *d);

#endif
