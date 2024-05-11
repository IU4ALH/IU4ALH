/*------------------------------------------------------------------
   VFO System for ESP32-DevKitC  Ver 1.00
        by T.Uebo / JF3HZB
          Feb. 10, 2019
--------------------------------------------------------------------*/

/*------Hard ware Configuration ---------------------
<<ESP32-DevKitC>>
pin No.  Connection
  16 :  Rotary Encoder A
  17 :  Rotary Encoder B

  22 :  si5351A SCL
  21 :  si5351A SDA

  18 :  SCK    / ST7735,SEPS525(128x160 display)  
  23 :  MOSI   / ST7735,SEPS525(128x160 display)
   5 :  CS     / ST7735,SEPS525(128x160 display)  
   2 :  DC(A0) / ST7735,SEPS525(128x160 display)
  15 :  RESET  / ST7735,SEPS525(128x160 display)

<<si5351A>>
CLK0 : Car Signal (I)
CLK1 : Car Signal (Q)
CLK2 : Lo Signal
------------------------------------------------*/

// Libraries
#include <EEPROM.h>  // https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM

/*-------------------------------------------------------
   Frequency settings
--------------------------------------------------------*/
  #define init_freq  14230000    // Initial Frequncy[Hz]

  int32_t offset_frq = 0;    // Offset Frequency[Hz]in + o in - per oscillatore locale LO
  int32_t    car_frq = 0;   // 19000000 frequenza BFO  calcolare + o - 1500 Hz per usb o lsb  // Carrier Frequency[Hz]
  unsigned char f_carON = 1; // ON/OFF Car signal
  int32_t  freq_step;
  float readParam;
  
#define fmax 160000000       // Max frequency[160 MHz]
#define fmin   100000        // Min frequency[100 KHz]
#define EEPROM_SIZE 50       // 20 

/*----------------------------------------------------------------------------------
    Control flags
-----------------------------------------------------------------------------------*/
uint8_t f_fchange;  // if frequency changed, set this flag to 1
uint8_t f_cchange;  // if Car frequency and/or "f_carON" changed, set this flag to 1
uint8_t f_dchange;  // if need to renew display, set this flag to 1

/*--------------------------------------------------------
   pin assign
----------------------------------------------------------*/
//#define LED_BUILTIN 13
  #define PULSE_INPUT_PIN 17 // Rotaty Encoder A  // orig16
  #define PULSE_CTRL_PIN 16  // Rotaty Encoder B  // orig17

//------------------------------------------------------------------------------
#define NAME "VFO System"
#define VERSION "Ver. 1.00"
#define ID "by JF3HZB"

#include "driver/pcnt.h"
#include "display.h"
#include "graph.h"
#include "dial.h"
#include "si5351.h"

/*-----------------------------------------------------------------------------
 *       Global
-----------------------------------------------------------------------------*/
float dacc=0.0;
float Maxdacc=500.0;
long frq=init_freq;
int16_t RE_Count = 0;
uint8_t f_redraw; 
extern char f_rev;
extern uint32_t cl_BG;
int press;
int cnt=2;
int mem, mem_old;
int cnt_old;
long step;
int address;

/*--------------------------------------------------------------------------
        Timer ISR
---------------------------------------------------------------------------*/
hw_timer_t * timer = NULL;
void IRAM_ATTR onTimer(){}

//-----------------------------------------------------------------------------------------------
void loop() {    // (core1)
//-----------------------------------------------------------------------------------------------
//  press = digitalRead(27);  // con pulsante  Step -
//  if(press==LOW)
//  {
//  cnt = cnt-1; 
//  if(cnt<0) { cnt=0 ; }
//  Serial.println(cnt);
//  delay(300);
//  }
 
 press = digitalRead(26);  //  pulsante Step +
  if(press==LOW)
  {
  cnt = cnt+1; 
  if(cnt>6) { cnt=0 ; }  // cnt=6 se si usano i due pulsanti S+ e S-.... cnt=0 se si una un solo pulsante Step +
  Serial.println(cnt);
  delay(300);
  }

 press = digitalRead(25);  //  pulsante Memorie
  if(press==LOW)
  {
  mem = mem+1; 
  if(mem>15) { mem=0 ; }  // numero memorie
//  Serial.println(mem);
  delay(300);
  }

  if(cnt==0) { step=10; freq_step=2.5;         }
  else
  if(cnt==1)  { step=100; freq_step=25;        }
  else
  if(cnt==2) { step=1000; freq_step=250;       }
  else
  if(cnt==3)  { step=10000; freq_step=2500;    }
  else
  if(cnt==4) { step=100000; freq_step=25000;   }
  else
  if(cnt==5) { step=1000000; freq_step=250000; }
  else
  if(cnt==6) { step=10000000; freq_step=2500000; }
  
  if(mem!=mem_old)
  {  
  if(mem==0) { frq=init_freq;      }
  else
  if(mem==1) { frq=1830000;   }
  else
  if(mem==2) { frq=3600000;   }
  else
  if(mem==3) { frq=3850000;   }
  else
  if(mem==4) { frq=5505000;   }
  else
  if(mem==5) { frq=6070000;   }
  else
  if(mem==6) { frq=7130000;   }
  else
  if(mem==7) { frq=7878100;   }
  else
  if(mem==8) { frq=8957000;   }
  else
  if(mem==9) { frq=10100000;  }
   else
  if(mem==10) { frq=14100000;  }
 else
  if(mem==11) { frq=17100000;  }
   else
  if(mem==12) { frq=21100000;  }
   else
  if(mem==13) { frq=24100000;  }
 else
  if(mem==14) { frq=27100000;  }
 else
  if(mem==15) { frq=28100000;  }
  

  set_freq( frq + offset_frq );  // akt wenn frq nicht von Si5351 uebernommen         

//  Serial.println(mem);
//  Serial.println(frq);
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
  double param = frq;
  Serial.print("Write to  EEPROM = ");
  Serial.println(param,0);
  EEPROM.writeFloat(address, param); //EEPROM.put(address, param);
  EEPROM.commit();
  delay(300);
  }

    char str[64];
    if(f_dchange==1||cnt!=cnt_old||mem!=mem_old)
    {
        f_dchange=0;        
        //GRAM_clr();
        boxfill(0,0,Nx-1,Ny-1,cl_BG);     
        
        //Display Dial
        Dial(frq);
        box(0,100,160,126, 0xa0a0a0); // 0x00efff   - 0xa0a0a0 colore riquadri display frequenza
        box(1,99,159,127, 0xa0a0a0);
  
  sprintf(str, "STEP"); disp_str8(str,73, 85, 0xFF0000);
  
  if(cnt==0) { sprintf(str, " 10 Hz"); disp_str12(str,100, 85, 0xffff00); }
  else
  if(cnt==1) { sprintf(str, " 100 Hz"); disp_str12(str,100, 85, 0xffff00); }
  else
  if(cnt==2) { sprintf(str, " 1 KHz" ); disp_str12(str,100, 85, 0xffff00); }
  else 
  if(cnt==3) { sprintf(str, " 10 KHz" ); disp_str12(str,100, 85, 0xffff00); }
  else
  if(cnt==4) { sprintf(str, "100 KHz" ); disp_str12(str,100, 85, 0xffff00); }
  else
  if(cnt==5) { sprintf(str, " 1 MHz" ); disp_str12(str,100, 85, 0xffff00); }
   else
  if(cnt==6) { sprintf(str, " 10 MHz" ); disp_str12(str,100, 85, 0xffff00); }
  
  if(mem==0) { sprintf(str, "VFO"); disp_str16(str,8, 80, 0x00bb2d); }
  else
  if(mem==1) { sprintf(str, "Mem.01");  disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==2) { sprintf(str, "Mem.02" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==3) { sprintf(str, "Mem.03" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==4) { sprintf(str, "Mem.04" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==5) { sprintf(str, "Mem.05" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==6) { sprintf(str, "Mem.06" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==7) { sprintf(str, "Mem.07" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==8) { sprintf(str, "Mem.08" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==9) { sprintf(str, "Mem.09" ); disp_str12(str,1, 85, 0xff8000); }
else
  if(mem==10) { sprintf(str, "Mem.10" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==11) { sprintf(str, "Mem.11" ); disp_str12(str,1, 85, 0xff8000); }
else
  if(mem==12) { sprintf(str, "Mem.12" ); disp_str12(str,1, 85, 0xff8000); }
else
  if(mem==13) { sprintf(str, "Mem.13" ); disp_str12(str,1, 85, 0xff8000); }
else
  if(mem==14) { sprintf(str, "Mem.14" ); disp_str12(str,1, 85, 0xff8000); }
  else
  if(mem==15) { sprintf(str, "Mem.15" ); disp_str12(str,1, 85, 0xff8000); }
  

        //-------- Display Digital Frquency ---------------------------------------           
        sprintf(str, "%3d.%03d,%02d",  frq/1000000, (frq/1000)%1000, (frq/10)%100 );
        disp_str20(str,7, 105, 0x00ffff);        // 0xffd080  - 0x00efff   colore cifre freq 
        sprintf(str, "MHz" );

        disp_str12(str,127, 106, 0xf80000); // 0xffd080     colore Mhz
        if(f_redraw==0){
          trans65k();
          f_redraw=1;
        cnt_old=cnt;
        mem_old=mem;
        }            
    } 
    //digitalWrite(LED_BUILTIN, 1^digitalRead(LED_BUILTIN) ); //  Toggle LED     
}

/*-----------------------------------------------------------------------------------------------
        Alternative Loop (core0)
------------------------------------------------------------------------------------------------*/
void task0(void* arg)
{
     while (1)
     {    
         pcnt_get_counter_value(PCNT_UNIT_0, &RE_Count);          
         int count=RE_Count;
         pcnt_counter_clear(PCNT_UNIT_0);      
         if(f_rev==1) count=-count;                  
         if(count!=0){                           
            f_dchange=1;
            f_fchange=1;
            
            frq+= count * freq_step;  
            if(frq>fmax) frq=fmax;
            if(frq<fmin) frq=fmin;
         }
         
         //-------------------------------------
         if(f_fchange==1){
            f_fchange=0;
            // Output Lo freq
            set_freq( frq + offset_frq );                
         }
         
         //-------------------------------------
         if(f_cchange==1){
            f_cchange=0;
            // Output Car freq            
            set_car_freq(car_frq, f_carON, 0);                         
         }

         if(f_redraw==1){
            Transfer_Image();
            f_redraw=0;
         }                   
         delay(1);
     }
}

//-----------------------------------------------------------------------------------------------
void setup() {
//-----------------------------------------------------------------------------------------------
    Serial.begin(9600);   
    Serial.println("Start");
    delay(100);
    pinMode(25,INPUT_PULLUP);  
    pinMode(26,INPUT_PULLUP);  
//  pinMode(27,INPUT_PULLUP);  
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
  if(readParam!=0) { frq=readParam; } 
      
    char str[64];
    //--------- create tasks on core0 --------------------------------
    xTaskCreatePinnedToCore(task0, "Task0", 4096, NULL, 1, NULL, 0);
        
    //--------- Set up Interrupt Timer -------------------------------
    timer = timerBegin(0, 80, true); //use Timer0, div80 for 1us clock
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 10000, true); // T=10000us
    timerAlarmEnable(timer); // Start Timer

    //pinMode(LED_BUILTIN, OUTPUT);   //LED
    
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
  
    display_init();    
    GRAM_clr();  
    sprintf(str,  NAME  ); disp_str16(str,20, 90, 0xefff00);
    sprintf(str,    ID  ); disp_str12(str,40, 70, 0x00ff2a);
    sprintf(str, "- Mod. By IU4ALH -"); disp_str8(str,28, 5, 0xffd080);
    trans65k();
    f_redraw=1;
    delay(1500); 
    init_Dial();
    GRAM_clr();
    si5351_init();         
    f_fchange = 1;
    f_cchange = 1;
    f_dchange = 1;
    f_redraw=0;    
}
