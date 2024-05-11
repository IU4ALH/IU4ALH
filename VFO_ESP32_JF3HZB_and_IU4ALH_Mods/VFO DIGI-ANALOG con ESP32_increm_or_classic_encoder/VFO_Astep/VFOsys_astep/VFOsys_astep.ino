/*------------------------------------------------------------------

   VFO System for ESP32-DevKitC  Ver 1.00

        by T.Uebo / JF3HZB

          Feb. 10, 2019

--------------------------------------------------------------------*/

/*------Hard ware Configuration ---------------------
<<ESP32-DevKitC>>
pin No.  Connection
  17 :  Rotary Encoder A
  16 :  Rotary Encoder B

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

/*-------------------------------------------------------
   Frequency settings
--------------------------------------------------------*/
#define init_freq 7100000      // Initial Frequncy[Hz]
int32_t offset_frq = 8998500;  // Offset Frequency[Hz]
int32_t car_frq = 8998500;     // Carrier Frequncy[Hz]
unsigned char f_carON = 1;     // ON/OFF Car signal

#define fmax 150000000         // Max frequency[Hz]
#define fmin 100000            // Min frequency[Hz]
#define freq_step 10           // Min step[Hz]

/*----------------------------------------------------------
   Adaptive step control parameters
-----------------------------------------------------------*/
uint32_t vth=2;   	  // Velocity threshold for acceleration
float Racc=0.002;     // Rate of acceleration
float Rdec=1.0;       // Rate of deceleration
float MaxL=1500.0;	  // Max Step[Hz] = freq_step + Racc*MaxL*MaxL


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

#define PULSE_INPUT_PIN 16 // Rotaty Encoder A
#define PULSE_CTRL_PIN 17  // Rotaty Encoder B







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
float L=0.0;
long afstp;
long frq=init_freq;
int16_t RE_Count = 0;
uint8_t f_redraw; 
extern char f_rev;
extern uint32_t cl_BG;

/*--------------------------------------------------------------------------
        Timer ISR
---------------------------------------------------------------------------*/
hw_timer_t * timer = NULL;
void IRAM_ATTR onTimer(){}



//-----------------------------------------------------------------------------------------------
void setup() {
//-----------------------------------------------------------------------------------------------
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
    sprintf(str,  NAME  ); disp_str16(str,20, 90, 0x00ffff);
    sprintf(str, VERSION); disp_str12(str,36, 50, 0x00ffff);   
    sprintf(str,    ID  ); disp_str8(str,50, 20, 0x00ffff);
    trans65k();
    f_redraw=1;


    delay(1000); 
    init_Dial();
    GRAM_clr();

    si5351_init();
         
    f_fchange = 1;
    f_cchange = 1;
    f_dchange = 1;
    f_redraw=0;    
}




  

//-----------------------------------------------------------------------------------------------
void loop() {    // (core1)
//-----------------------------------------------------------------------------------------------
    char str[64];
   
    if(f_dchange==1){
        f_dchange=0;        
        //GRAM_clr();
        boxfill(0,0,Nx-1,Ny-1,cl_BG); 		
        //Display Dial
        Dial(frq);
         
        box(7,100,153,126, 0xa0a0a0);
        box(6,99,154,127, 0xa0a0a0);
  
        //-------- Display Digital Frquency ---------------------------------------
        sprintf(str, "%3d.%03d,%02d",  frq/1000000, (frq/1000)%1000, (frq/10)%100 );
        disp_str16(str,17, 105, 0xffd080);              
        sprintf(str, "MHz" );
        disp_str12(str,120, 106, 0xffd080);
          
        if(f_redraw==0){
          trans65k();
          f_redraw=1;
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
          afstp=(long)(0.5 + (float)(count)*((float)freq_step + Racc*L*L) );         
          if(abs(count)>=vth) L+=1.0*(float)( abs(count) - vth );
          if(L>MaxL) L=MaxL;                                  
        }else{         
          L-=Rdec;
          if(L<0) L=0;
        }
         
    //---Update frequency -------------------------------
        if(afstp!=0){
          frq+=afstp;
          if(frq>fmax) frq=fmax;
          if(frq<fmin) frq=fmin;
          afstp=0;
          f_dchange=1;
          f_fchange=1;                                                     
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
