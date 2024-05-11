/* 
 * File:   display.cpp
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2019/02/10, 22:13
 * 
 * ------------------------------------------- 
 *  Dedicated Display Interface for ESP32
 *-------------------------------------------- 
 *
 */

#include <arduino.h>
#include <SPI.h>
#include "soc/spi_reg.h"
#include "display.h"

#define VSPI 3
#define HSPI 2

#define sel_SPI VSPI

#define SCLK  18 // SPI clock pin
#define SDI -1 // MISO(master input slave output) not using
#define SDO 23 // MOSI(master output slave input) pin
#define CS  5 // Chip Select pin
#define DC  2 // Data/Command pin
#define RST 15 // Reset pin


uint8_t R_GRAM[Nx][Ny];
uint8_t G_GRAM[Nx][Ny];
uint8_t B_GRAM[Nx][Ny];
uint16_t GRAM65k[Nx][Ny];



//-------------------------------------------------------
#ifdef ST7735
//-------------------------------------------------------
void Transfer_Image() {
  
  #define block_size 512
  int kxy, pt=0, xp, yp;
  uint32_t col_data;

  d_Command(0x2A);
  SPI.write(0); SPI.write(0);
  SPI.write((Xw-1)>>8); SPI.write(Xw-1);
  d_Command(0x2B);             
  SPI.write(0); SPI.write(0);             
  SPI.write((Yw-1)>>8); SPI.write(Yw-1); 
  d_Command(0x2C);


    
    SET_PERI_REG_BITS(  SPI_MOSI_DLEN_REG(sel_SPI),
            SPI_USR_MOSI_DBITLEN,
            (block_size-1),
            SPI_USR_MOSI_DBITLEN_S
          );
      
    for(kxy=0; kxy<(Nx*Ny*16/block_size); kxy++) {  
      while (READ_PERI_REG(SPI_CMD_REG(sel_SPI)) & SPI_USR); 
         
      for (uint32_t i = 0; i < 16; i++){        
    xp=pt/Ny; yp=pt%Ny;      
    col_data=GRAM65k[xp][yp];
    pt++;      
    xp=pt/Ny; yp=pt%Ny;      
    col_data|=(GRAM65k[xp][yp]<<16);
    pt++;      
    WRITE_PERI_REG((SPI_W0_REG(sel_SPI) + (i << 2)), col_data);
      }
    
      SET_PERI_REG_MASK(SPI_CMD_REG(sel_SPI), SPI_USR);   
    }     
}




/*-----------------------------------------------------------------
   Initialize ST7735
------------------------------------------------------------------ */
void display_init(void){  
  pinMode(RST, OUTPUT); //Set RESET pin
  pinMode(DC, OUTPUT); //Set Data/Command pin 
  SPI.begin(SCLK, SDI, SDO, CS); //VSPI setting  
  
  SPI.setBitOrder(MSBFIRST);
  //SPI.setFrequency(10000000);
  //SPI.setFrequency(20000000);
  SPI.setFrequency(27000000);
  //SPI.setFrequency(40000000);  
  SPI.setDataMode(SPI_MODE0);  
  SPI.setHwCs(true); //Set Hardware CS pin
  
  //Hardware Reset------------
  digitalWrite(RST, HIGH);
  delay(10);
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(100);
  
  digitalWrite(DC, HIGH);
  
  d_Command(0x01);  // Software reset
  delay (150); //Delay 150ms
  d_Command(0x11); //Sleep out
  delay (300);
  
  d_Command(0xB1); SPI.write(0x01); SPI.write(0x2C); SPI.write(0x2D);
  d_Command(0xB2); SPI.write(0x01); SPI.write(0x2C); SPI.write(0x2D);
  d_Command(0xB3); SPI.write(0x01); SPI.write(0x2C); SPI.write(0x2D);
                   SPI.write(0x01); SPI.write(0x2C); SPI.write(0x2D);

  d_Command(0xB4); SPI.write(0x07); //Column inversion
    
  d_Command(0xC0); SPI.write(0xA2); SPI.write(0x02); SPI.write(0x84);  
  d_Command(0xC1); SPI.write(0xC5);  
  d_Command(0xC2); SPI.write(0x0A); SPI.write(0x00); 
  d_Command(0xC3); SPI.write(0x8A); SPI.write(0x2A); 
  d_Command(0xC4); SPI.write(0x8A); SPI.write(0xEE); 
  d_Command(0xC5); SPI.write(0x0E); //VCOM
   
  d_Command(0x20); // not invert display,
  d_Command(0x36); SPI.write(0xC8); //Mem access ctl (directions),
  
  d_Command(0x3A);  //  set color mode
  SPI.write(0x05);  //65k mode

  d_Command(0x2A);  //  Column addr set x=0 to 127
  SPI.write(0x00); SPI.write(0x00); SPI.write(0x00); SPI.write(0x7F);  
  d_Command(0x2B);  //  Raw addr set y=0 to 159
  SPI.write(0x00); SPI.write(0x00); SPI.write(0x00); SPI.write(0x9F);  
  
  d_Command(0xE0);
  SPI.write(0x02); SPI.write(0x1C); SPI.write(0x07); SPI.write(0x12);
  SPI.write(0x37); SPI.write(0x32); SPI.write(0x29); SPI.write(0x2D);
  SPI.write(0x29); SPI.write(0x25); SPI.write(0x2B); SPI.write(0x39);
  SPI.write(0x00); SPI.write(0x01); SPI.write(0x03); SPI.write(0x10);
  d_Command(0xE1);
  SPI.write(0x03); SPI.write(0x1D); SPI.write(0x07); SPI.write(0x06);
  SPI.write(0x2E); SPI.write(0x2C); SPI.write(0x29); SPI.write(0x2D);
  SPI.write(0x2E); SPI.write(0x2E); SPI.write(0x37); SPI.write(0x3F);
  SPI.write(0x00); SPI.write(0x00); SPI.write(0x02); SPI.write(0x10);

  d_Command(0x13);
  delay(10);
  d_Command(0x29);
  delay(100); 
  d_Command(0x36); SPI.write(0xC0);
  delay(10);   
}
#endif




//---------------------------------------------------------------------------------
#ifdef SEPS525
//---------------------------------------------------------------------------------
void Transfer_Image() {
 int pt=0, xp, yp;

    d_Command(0x17);
    SPI.write(0);
    d_Command(0x18);
    SPI.write(0x9F);
    d_Command(0x19);
    SPI.write(0);
    d_Command(0x1A);
    SPI.write(0x7F);
    d_Command(0x22);
 
    SET_PERI_REG_BITS(  SPI_MOSI_DLEN_REG(sel_SPI),
            SPI_USR_MOSI_DBITLEN,
            15,
            SPI_USR_MOSI_DBITLEN_S
          );                    
    for(pt=0; pt<Nx*Ny; pt++) {   
      while (READ_PERI_REG(SPI_CMD_REG(sel_SPI)) & SPI_USR);               
      xp=pt%Nx; yp=pt/Nx;      
      WRITE_PERI_REG( SPI_W0_REG(sel_SPI), GRAM65k[xp][yp]);
      SET_PERI_REG_MASK(SPI_CMD_REG(sel_SPI), SPI_USR);           
    } 
}

/*-----------------------------------------------------------------
   Initialize SEPS525
------------------------------------------------------------------ */
void display_init(void){  
  pinMode(RST, OUTPUT); //Set RESET pin
  pinMode(DC, OUTPUT); //Set Data/Command pin 
  SPI.begin(SCLK, SDI, SDO, CS); //VSPI setting  
  
  SPI.setBitOrder(MSBFIRST);
  //SPI.setFrequency(10000000);
  //SPI.setFrequency(20000000);
  SPI.setFrequency(27000000);
  //SPI.setFrequency(40000000);
  SPI.setDataMode(SPI_MODE3);  //OLED;
  
  SPI.setHwCs(true); //Set Hardware CS pin
  
  //Hardware Reset------------
  digitalWrite(RST, HIGH);
  delay(100);
  digitalWrite(RST, LOW);
  delay(500);
  digitalWrite(RST, HIGH);
  delay(500);
  
  digitalWrite(DC, HIGH);

  d_Command(0x04);// Set Normal Driving Current
  SPI.write(0x03);// Disable Oscillator Power Down
  delay(10);
  d_Command(0x04); // Enable Power Save Mode
  SPI.write(0x00); // Set Normal Driving Current
  delay(10); // Disable Oscillator Power Down


  d_Command(0x3B);
  SPI.write(0x00);
  d_Command(0x02);
  SPI.write(0x01); // Set EXPORT1 Pin at Internal Clock
                // Oscillator operates with external resister.
                // Internal Oscillator On

  d_Command(0x03);
  SPI.write(0x90); // Set Frame Rate as 120Hz
  //SPI.write(0x30);  // default 40Hz

  d_Command(0x80);
  SPI.write(0x01); // Set Reference Voltage Controlled by External Resister
  
  d_Command(0x08);// Set Pre-Charge Time of Red
  SPI.write(0x08); //(0x04);
  d_Command(0x09);// Set Pre-Charge Time of Green
  SPI.write(0x0A); //(0x05);
  d_Command(0x0A);// Set Pre-Charge Time of Blue
  SPI.write(0x0A); //(0x05);
  
  d_Command(0x0B);// Set Pre-Charge Current of Red
  SPI.write(0xC0); //(0x9D);
  d_Command(0x0C);// Set Pre-Charge Current of Green
  SPI.write(0xB0); //(0x8C);
  d_Command(0x0D);// Set Pre-Charge Current of Blue
  SPI.write(0x98); //(0x57);
  
  d_Command(0x10);// Set Driving Current of Red
  SPI.write(0xA0); //(0x56);
  d_Command(0x11);// Set Driving Current of Green
  SPI.write(0x93); //(0x4D);
  d_Command(0x12);// Set Driving Current of Blue
  SPI.write(0x8C); //(0x46);
  
  d_Command(0x13);
  SPI.write(0x20); // Set Color Sequence
  d_Command(0x14);
  SPI.write(0x11); // Set MCU Interface Mode, 65k
  d_Command(0x16);
  SPI.write(0x66);
  d_Command(0x20);
  SPI.write(0x00); // Shift Mapping RAM Counter
  d_Command(0x21);
  SPI.write(0x00); // Shift Mapping RAM Counter
  d_Command(0x28);
  SPI.write(0x7F); // 1/128 Duty (0x0F~0x7F)
  d_Command(0x29);
  SPI.write(0x00); // Set Mapping RAM Display Start Line (0x00~0x7F)
  d_Command(0x06);
  SPI.write(0x01); // Display On (0x00/0x01)
  d_Command(0x05); // Disable Power Save Mode
  SPI.write(0x00); // Set All Internal Register Value as Normal Mode
  d_Command(0x15);
  SPI.write(0x00); // Set RGB Interface Polarity as Active Low
  delay(10);  
}
#endif


/*------------------------------------------
       Command write
-------------------------------------------*/ 
void d_Command(uint8_t d){
  digitalWrite(DC, LOW);
  SPI.write(d);
  digitalWrite(DC, HIGH);
}


/*------------------------------------------
     Convert 24bit image to 16bit image
-------------------------------------------*/ 
void trans65k(void){
    int xps, yps;
    uint16_t col16;
    for(xps=0; xps<Nx; xps++){
        for(yps=0; yps<Ny; yps++){ 
          col16=(0xf800&(R_GRAM[xps][yps]<<8)) | (0x07E0&(G_GRAM[xps][yps]<<3)) | (0x001F&(B_GRAM[xps][yps]>>3));
          GRAM65k[xps][yps]=(col16 >> 8) | (col16 << 8);           
        }
    }
}
