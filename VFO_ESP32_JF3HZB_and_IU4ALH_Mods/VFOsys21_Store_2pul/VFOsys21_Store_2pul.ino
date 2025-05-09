/* 
  Digital VFO with Rotation Dial
 
    File:   VFOsys2_1.ino
    Author: JF3HZB / IU4ALH mod.
 
    Created on 24/03/2024

  Board manager: esp32 ver. 2.0.9
  Board: ESP32 Dev Module
  Library: LovyanGFX ver. 1.1.7

*/

#define NAME "VFO System"
#define VERSION "Ver. 2.10"
#define ID "JF3HZB/IU4ALH"


#include "dial.hpp"
#include "si5351.h"
#include "driver/pcnt.h"
#include <EEPROM.h>  // https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM						

LGFX lcd;
LGFX_Sprite sp;
LGFX_Sprite sprites[2];
bool flip;
int sprite_height; 
DIAL dial;



// #define REV_ENCODER
#define PULSE_INPUT_PIN 16 // Rotaty Encoder A
#define PULSE_CTRL_PIN 17  // Rotaty Encoder B

/*-------------------------------------------------------
   Frequency settings
--------------------------------------------------------*/
#define init_frq 7100000       // Initial Frequncy[Hz]
int32_t offset_frq = -0000210;  // Offset Frequency[Hz] 
int32_t car_frq = 0000000;     // Carrier Frequncy[Hz]
#define max_frq 60000000      // Max frequency[Hz]
#define min_frq 10000         // Min frequency[Hz]
float readParam;

#define EEPROM_SIZE 50       // 20 

/*----------------------------------------------------------
   Adaptive step control parameters
-----------------------------------------------------------*/
// #define vth 4   	   // Velocity threshold for acceleration
#define Racc 200     // Rate of acceleration
#define Rdec 100     // Rate of deceleration
// #define freq_step 10  // Min step[Hz]
int32_t  freq_step;
#define MAX_Step 10000 // 5000 [Hz]

float L=0.0;
float inv_Tacc=Racc*1.0e-6;
float MaxL = sqrt( (MAX_Step - freq_step)/inv_Tacc );
int32_t afstp;
int16_t RE_Count = 0;


/*----------------------------------------------------------------------------------
    Control flags
-----------------------------------------------------------------------------------*/
uint8_t f_fchange = 1;  // if frequency changed, set this flag to 1
uint8_t f_cchange = 1;  // if Car frequency and/or "f_carON" changed, set this flag to 1
uint8_t f_carON = 0;     // ON(1)/OFF(0) Car signal

int32_t Dial_frq;
int press;
int cnt=2;															
int cnt_old;
int mem, mem_old;
int address;
long step;						


void setup(void)
{

  pinMode(26,INPUT_PULLUP);  
  pinMode(27,INPUT_PULLUP);  
  pinMode(32,INPUT_PULLUP);  

// Init EEPROM
  EEPROM.begin(EEPROM_SIZE);

// Read data from eeprom
  address = 0;
  int readId;
  readId = EEPROM.read(address);  // EEPROM.get(address,readId);
  address += sizeof(readId);      // update address value
  EEPROM.get(address, readParam); // readParam=EEPROM.readFloat(address);
  Serial.print("Read from EEPROM = ");
  Serial.println(readParam,0);
  EEPROM.end();
  if(readParam!=0) { Dial_frq=readParam; } 
      
	
  //--- Counter setup for Rotary Encoder ---------------------
  pcnt_config_t pcnt_config_A;// structure for A   
  pcnt_config_t pcnt_config_B;// structure for B
  //
  pcnt_config_A.pulse_gpio_num = PULSE_INPUT_PIN;
  pcnt_config_A.ctrl_gpio_num = PULSE_CTRL_PIN;
  pcnt_config_A.lctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config_A.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config_A.channel = PCNT_CHANNEL_0;
  pcnt_config_A.unit = PCNT_UNIT_0;
  pcnt_config_A.pos_mode = PCNT_COUNT_INC;
  pcnt_config_A.neg_mode = PCNT_COUNT_DEC;
  pcnt_config_A.counter_h_lim = 10000;
  pcnt_config_A.counter_l_lim = -10000;
  //
  pcnt_config_B.pulse_gpio_num = PULSE_CTRL_PIN;
  pcnt_config_B.ctrl_gpio_num = PULSE_INPUT_PIN;
  pcnt_config_B.lctrl_mode = PCNT_MODE_KEEP;
  pcnt_config_B.hctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config_B.channel = PCNT_CHANNEL_1;
  pcnt_config_B.unit = PCNT_UNIT_0;
  pcnt_config_B.pos_mode = PCNT_COUNT_INC;
  pcnt_config_B.neg_mode = PCNT_COUNT_DEC;
  pcnt_config_B.counter_h_lim = 10000;
  pcnt_config_B.counter_l_lim = -10000;
  //
  pcnt_unit_config(&pcnt_config_A);//Initialize A
  pcnt_unit_config(&pcnt_config_B);//Initialize B
  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_resume(PCNT_UNIT_0); //Start

  LCD_setup();
  lcd.setTextColor(TFT_CYAN);
  lcd.setFont(&fonts::Font0);
  lcd.setTextSize(1.0f*( lcd.height()/64.0f));
  lcd.setCursor( 0.5f*(lcd.width()-lcd.textWidth(NAME) ), 0.1f*lcd.height() );
  lcd.printf( NAME );
  lcd.setTextSize(1.0f*( lcd.height()/64.0f));
  lcd.setCursor( 0.5f*(lcd.width()-lcd.textWidth(VERSION) ), 0.4f*lcd.height());
  lcd.printf(VERSION); 
  lcd.setTextSize(1.0f*(lcd.height()/64.0f));
  lcd.setCursor( 0.5f*(lcd.width()-lcd.textWidth(ID) ), 0.7f*lcd.height());
  lcd.printf(ID);

  delay(2000);

  si5351_init();
    if(readParam!=0) { Dial_frq=readParam; }  
 //   else if(readParam=0)   { Dial_frq = init_frq;}
  set_freq(Dial_frq);
}

/*--------------------------------------------------------------------------
        Timer ISR
---------------------------------------------------------------------------*/
hw_timer_t * timer = NULL;
void IRAM_ATTR onTimer(){}


void loop(void)
{
 press = digitalRead(27);  //  pulsante Step -
  if(press==LOW)
  {
  cnt = cnt-1; 
  if(cnt<0) { cnt=0 ; }
  Serial.println(cnt);
  delay(300);
  }

 press = digitalRead(26);  //  pulsante Step +
  if(press==LOW)
  {
  cnt = cnt+1; 
  if(cnt>6) { cnt=6 ; }  // cnt=6 se si usano i due pulsanti S+ e S-.... cnt=0 se si una un solo pulsante Step +
  Serial.println(cnt);
  delay(300); // 300
  }
  if(cnt==0) { freq_step=2.5;     }
  else
  if(cnt==1) { freq_step=25;      }
  else
  if(cnt==2) { freq_step=250;     }
  else
  if(cnt==3) { freq_step=2500;    }
  else
  if(cnt==4) { freq_step=25000;   }
  else
  if(cnt==5) { freq_step=250000;  }
  else
  if(cnt==6) { freq_step=2500000; }


  // Encoder
  pcnt_get_counter_value(PCNT_UNIT_0, &RE_Count);          
  #ifdef REV_ENCODER
  int count=-RE_Count;
  #else if
  int count=RE_Count;
  #endif
  pcnt_counter_clear(PCNT_UNIT_0);

  // Step control     
  if(count!=0){                           
    afstp=(int32_t)( (count)*(freq_step ) );     
 //   if(abs(count)>= vth ) L+=1.0*(float)( abs(count) - vth );
    if(L>MaxL) L=MaxL;                                  
 // }else{         
  //  L-=(float)Rdec;
    if(L<0) L=0;
  }

  // Update frequency
  if(afstp!=0){
    int32_t tfrq=Dial_frq;
    tfrq+=afstp;
    
 //   tfrq = (tfrq/freq_step)*freq_step;
    
    if(tfrq>max_frq) tfrq=max_frq;
    if(tfrq<min_frq) tfrq=min_frq;
    Dial_frq=tfrq;
    afstp=0;
    f_fchange = 1;   
	
  }

  // Set frequency to Si5351A 
  if(f_fchange==1||cnt!=cnt_old){
    f_fchange=0;
    // Lo freq
    set_freq( Dial_frq + offset_frq );                
  }
  if(f_cchange==1){
    f_cchange=0;
    // Car freq            
    set_car_freq(car_frq, f_carON, 0);                         
  }

press = digitalRead(32);  // pulsante Store
  if(press==LOW)
																						
																
  {
  EEPROM.begin(EEPROM_SIZE);

				
							  

  // Write data into eeprom
  address = 0;
  int boardId = 18;
  EEPROM.write(address, boardId);    //EEPROM.put(address, boardId);
  address += sizeof(boardId);        //update address value
  double param = Dial_frq ;
  Serial.print("Write to  EEPROM = ");
  Serial.println(param,0);
  EEPROM.writeFloat(address, param); //EEPROM.put(address, param);
  EEPROM.commit();
  delay(300);
  }






  // Display process -------------------------------------------------------------------
  for (int yoff = 0; yoff < lcd.height(); yoff += sprite_height)
  {
    sprites[flip].clear(BGCol);

    // Draw dial
    dial.draw(Dial_frq, yoff);


    // Draw digital frequency display
    char str[12], strl[24];
    sprintf(str, "%03d.%03d.%02d",
	
                  Dial_frq/1000000, (Dial_frq%1000000)/1000, (Dial_frq%1000)/10 );
    int cc=0;
    if(str[0]=='0'){
      cc++;
      if(str[1]=='0') cc++;
    }
    for(int i=0; i<12; i++){
      strl[2*i]=str[i+cc];
      strl[2*i+1]=' ';
	   cnt_old=cnt;
    }
	
	  
  if(cnt==0) { lcd.fillRect(260,51,15,8, TFT_WHITE);} //  sprites[flip].drawRect(260,53,15,15, TFT_WHITE);
  else
  if(cnt==1) { lcd.fillRect(230,51,15,8, TFT_WHITE); }
  else
  if(cnt==2) { lcd.fillRect(185,51,15,8, TFT_WHITE); }
  else 
  if(cnt==3) { lcd.fillRect(155,51,15,8, TFT_WHITE); }
  else
  if(cnt==4) { lcd.fillRect(125,51,15,8, TFT_WHITE); }
  else
  if(cnt==5) { lcd.fillRect(80,51,15,8, TFT_WHITE); }
   else
  if(cnt==6) { lcd.fillRect(50,51,15,8, TFT_WHITE); }

  if (digitalRead(32)==LOW) { lcd.fillRect(295,23,15,15, TFT_ORANGE); }
  
	
    #ifdef MODE0
    int xdf=0, ydf=0;// position
    int hdf = top_position-2;
    if(hdf>=20)
    {
      sprites[flip].drawRoundRect(xdf, ydf -yoff, lcd.width(), hdf, 5, TFT_WHITE);
      sprites[flip].setFont(&fonts::Font7);
      sprites[flip].setTextSize(0.0022f*lcd.width(), 0.014f*hdf);
      sprites[flip].setTextColor(TFT_VIOLET);  // ORANGE
      sprites[flip].setCursor(xdf + 0.05f*lcd.width() + cc*sprites[flip].textWidth("0 "), 
                              ydf + 0.5f*(hdf - sprites[flip].fontHeight() ) -yoff    );
      sprites[flip].print(strl);
    }
    #endif

    // Add draw functions if you need. 
    // Note that the y-coordinate is applied with the offset -yoff added.
    //
    // ex.
     sprites[flip].setFont(&fonts::Font4);
     sprites[flip].setTextSize(1.0f);
     
    sprites[flip].setTextColor(TFT_BLACK);
    sprites[flip].setCursor(0, 60 -yoff); sprites[flip].print("USB");
    if (Dial_frq <= 10000000); else sprites[flip].setTextColor(TFT_RED); sprites[flip].setCursor(0, 60 -yoff); sprites[flip].print("USB");


    sprites[flip].setTextColor(TFT_BLACK);
    sprites[flip].setCursor(265, 60 -yoff); sprites[flip].print("LSB"); 
    if (Dial_frq > 10000000); else sprites[flip].setTextColor(TFT_RED);sprites[flip].setCursor(274, 60 -yoff); sprites[flip].print("LSB");
    //
    //
  

    sprites[flip].pushSprite(&lcd, 0, yoff);
    flip = !flip;
  }
  // End of Display process -------------------------------------------------------------

}
