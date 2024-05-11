/* 
 * File:   si5351.cpp
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2019/02/11, 23:07
 */


#include <arduino.h>

#define SDA    21
#define SCL    22
#define fxtal (25000000)




volatile uint32_t oMf=0; 
volatile uint32_t oMc=0; 

void wr_I2C(unsigned char d){
    int k;
    for(k=0;k<8;k++){
        if(d & 0x80) digitalWrite(SDA, HIGH); else digitalWrite(SDA, LOW);
        delayMicroseconds(1);
        digitalWrite(SCL, HIGH);
        delayMicroseconds(1);
        digitalWrite(SCL, LOW);
        delayMicroseconds(1);
        digitalWrite(SDA, LOW);
        d <<= 1;
    }
    digitalWrite(SCL, HIGH);
    delayMicroseconds(1);    
    digitalWrite(SCL, LOW);       
}


void cmd_si5351(unsigned char reg_No, unsigned char d){
    digitalWrite(SDA, LOW);  // start condition
    delayMicroseconds(1);
    digitalWrite(SCL, LOW);  //
    delayMicroseconds(1);
    
    wr_I2C(0xC0);
    wr_I2C(reg_No);
    wr_I2C(d);
    
    delayMicroseconds(1);    
    digitalWrite(SCL, HIGH);  // stop condition
    delayMicroseconds(1);
    digitalWrite(SDA, HIGH);  //
    delayMicroseconds(10);
}



void set_car_freq(unsigned long freq, unsigned char EN, unsigned char RST){
//    freq [Hz]
//
//    fvco= fxtal*(a+b/c)  ( a:15 -- 90,   b:0 -- 1048575, c:1 -- 1048575 )
//    freq= fvco /(a+b/c)  ( a:4, 6--1800, b:0 -- 1048575, c:1 -- 1048575 )
//
//    P1= 128*a +   floor(128*b/c) - 512
//    P2= 128*b - c*floor(128*b/c)
//    P3= c
//
	
	int k;
	uint32_t M;
	uint32_t R;

  if(EN==1){
      cmd_si5351(16,0x4F);
      cmd_si5351(17,0x4F);
      
    	if(freq<1500) freq=1500; else if(freq>280000000) freq=280000000;
    	
    	if(     freq> 150000000){M=4; R=0;}
    	else if(freq>=63000000){M=6; R=0;}
    	else if(freq>=27500000){M=14; R=0;}
    	else if(freq>=13000000){M=30; R=0;}
    	else if(freq>= 6500000){M=62; R=0;}
    	else if(freq>= 3000000){M=126; R=0;}
    	else if(freq>= 1500000){M=280; R=0;}	
    	else if(freq>=  700000){M=600; R=0;}
    	else if(freq>=  330000){M=1280; R=0;}
    	else if(freq>=  150000){M=1300; R=1;}
    	else if(freq>=   67000){M=1500; R=2;}
    	else if(freq>=   30300){M=1600; R=3;}	
    	else if(freq>=   14000){M=1800; R=4;}
    	else if(freq>=    7000){M=1800; R=5;}	
    	else if(freq>=    3500){M=1800; R=6;}	
    	else{M=1800; R=7;}
    	
    	freq*=M;
    	freq<<=R;
    	
    	unsigned long c=0xFFFFF;	
    	unsigned long a=freq/fxtal;
    	unsigned long b=(long)((float)(freq-a*fxtal)*(float)c/(float)fxtal);
    	unsigned long dd=(128*b)/c;
    	unsigned long P1=128*a+dd-512;
    	unsigned long P2=128*b-c*dd;
    	unsigned long P3=c;
    	
    	
    	//Set fvco of PLL_A
        cmd_si5351(26,(P3>>8)&0xFF);        //MSNA_P3[15:8]
        cmd_si5351(27,P3&0xFF);             //MSNA_P3[7:0]
        cmd_si5351(28,(P1>>16)&0x03);       //MSNA_P1[17:16]
        cmd_si5351(29,(P1>>8)&0xFF);        //MSNA_P1[15:8]
        cmd_si5351(30,P1&0xFF);              //MSNA_P1[7:0]
        cmd_si5351(31,(P3>>12)&0xF0|(P2>>16)&0x0F);//MSNA_P3[19:16], MSNA_P2[19:16]
        cmd_si5351(32,(P2>>8)&0xFF);        //MSNA_P2[15:8]
        cmd_si5351(33,P2&0xFF);             //MSNA_P2[7:0]
    	
    	// Set MS0, MS1
    	// a=M, b=0, c=1 ---> P1=128*M-512, P2=0, P3=1	
    	if(M==4){
    		P1=0;
    		cmd_si5351(42,0);                   //MS0_P3[15:8]
    		cmd_si5351(43,1);                   //MS0_P3[7:0]
    		cmd_si5351(44,0b00001100);  		//0,R0_DIV[2:0],MS0_DIVBY4[1:0],MS0_P1[17:16]
    		cmd_si5351(45,0);        			//MS0_P1[15:8]
    		cmd_si5351(46,0);              		//MS0_P1[7:0]
    		cmd_si5351(47,0);                   //MS0_P3[19:16], MS0_P2[19:16]
    		cmd_si5351(48,0);                   //MS0_P2[15:8]
    		cmd_si5351(49,0);                   //MS0_P2[7:0]
    		
    		cmd_si5351(50,0);                   //MS1_P3[15:8]
    		cmd_si5351(51,1);                   //MS1_P3[7:0]
    		cmd_si5351(52,0b00001100);  		//0,R1_DIV[2:0],MS1_DIVBY4[1:0],MS1_P1[17:16]
    		cmd_si5351(53,0);        			//MS1_P1[15:8]
    		cmd_si5351(54,0);              		//MS1_P1[7:0]
    		cmd_si5351(55,0);                   //MS1_P3[19:16], MS0_P2[19:16]
    		cmd_si5351(56,0);                   //MS1_P2[15:8]
    		cmd_si5351(57,0);                   //MS1_P2[7:0]	
    	}else{
    		P1=128*M-512;
    		cmd_si5351(42,0);                    //MS0_P3[15:8]
    		cmd_si5351(43,1);                    //MS0_P3[7:0]
    		cmd_si5351(44,(R<<4)&0x70|(P1>>16)&0x03);//0,R0_DIV[2:0],MS0_DIVBY4[1:0],MS0_P1[17:16]
    		cmd_si5351(45,(P1>>8)&0xFF);        //MS0_P1[15:8]
    		cmd_si5351(46,P1&0xFF);              //MS0_P1[7:0]
    		cmd_si5351(47,0);                    //MS0_P3[19:16], MS0_P2[19:16]
    		cmd_si5351(48,0);                    //MS0_P2[15:8]
    		cmd_si5351(49,0);                    //MS0_P2[7:0]
    		
    		cmd_si5351(50,0);                    //MS1_P3[15:8]
    		cmd_si5351(51,1);                    //MS1_P3[7:0]
    		cmd_si5351(52,(R<<4)&0x70|(P1>>16)&0x03);//0,R1_DIV[2:0],MS1_DIVBY4[1:0],MS1_P1[17:16]
    		cmd_si5351(53,(P1>>8)&0xFF);        //MS1_P1[15:8]
    		cmd_si5351(54,P1&0xFF);              //MS1_P1[7:0]
    		cmd_si5351(55,0);                    //MS1_P3[19:16], MS0_P2[19:16]
    		cmd_si5351(56,0);                    //MS1_P2[15:8]
    		cmd_si5351(57,0);                    //MS1_P2[7:0]
    	}
    	cmd_si5351(165,0);
    	cmd_si5351(166,M); 
    
      if( (oMc!=M)||(RST==1) ){
    	  cmd_si5351(177,0x20);   // Reset PLLA
      }
      oMc=M;
      
  }else{
      cmd_si5351(16,0x80);
      cmd_si5351(17,0x80);        
  }
  
}



void set_freq(unsigned long freq){
//    freq [Hz]
//
//    fvco= fxtal*(a+b/c)  ( a:15 -- 90,   b:0 -- 1048575, c:1 -- 1048575 )
//    freq= fvco /(a+b/c)  ( a:4, 6--1800, b:0 -- 1048575, c:1 -- 1048575 )
//
//    P1= 128*a +   floor(128*b/c) - 512
//    P2= 128*b - c*floor(128*b/c)
//    P3= c
//
  
  int k;
  uint32_t M;
  uint32_t R;

      if(freq<1500) freq=1500; else if(freq>280000000) freq=280000000;
      
      if(     freq> 150000000){M=4; R=0;}
      else if(freq>=63000000){M=6; R=0;}
      else if(freq>=27500000){M=14; R=0;}
      else if(freq>=13000000){M=30; R=0;}
      else if(freq>= 6500000){M=62; R=0;}
      else if(freq>= 3000000){M=126; R=0;}
      else if(freq>= 1500000){M=280; R=0;}  
      else if(freq>=  700000){M=600; R=0;}
      else if(freq>=  330000){M=1280; R=0;}
      else if(freq>=  150000){M=1300; R=1;}
      else if(freq>=   67000){M=1500; R=2;}
      else if(freq>=   30300){M=1600; R=3;} 
      else if(freq>=   14000){M=1800; R=4;}
      else if(freq>=    7000){M=1800; R=5;} 
      else if(freq>=    3500){M=1800; R=6;} 
      else{M=1800; R=7;}
      
      freq*=M;
      freq<<=R;
      
      unsigned long c=0xFFFFF;  
      unsigned long a=freq/fxtal;
      unsigned long b=(long)((float)(freq-a*fxtal)*(float)c/(float)fxtal);
      unsigned long dd=(128*b)/c;
      unsigned long P1=128*a+dd-512;
      unsigned long P2=128*b-c*dd;
      unsigned long P3=c;
      
      
      //Set fvco of PLL_B
        cmd_si5351(34,(P3>>8)&0xFF);        //MSNB_P3[15:8]
        cmd_si5351(35,P3&0xFF);             //MSNB_P3[7:0]
        cmd_si5351(36,(P1>>16)&0x03);       //MSNB_P1[17:16]
        cmd_si5351(37,(P1>>8)&0xFF);        //MSNB_P1[15:8]
        cmd_si5351(38,P1&0xFF);              //MSNB_P1[7:0]
        cmd_si5351(39,(P3>>12)&0xF0|(P2>>16)&0x0F);//MSNB_P3[19:16], MSNB_P2[19:16]
        cmd_si5351(40,(P2>>8)&0xFF);        //MSNB_P2[15:8]
        cmd_si5351(41,P2&0xFF);             //MSNB_P2[7:0]
      
      // Set MS2
      // a=M, b=0, c=1 ---> P1=128*M-512, P2=0, P3=1  
      if(M==4){
        P1=0;
        cmd_si5351(58,0);                   //MS2_P3[15:8]
        cmd_si5351(59,1);                   //MS2_P3[7:0]
        cmd_si5351(60,0b00001100);      //0,R0_DIV[2:0],MS2_DIVBY4[1:0],MS2_P1[17:16]
        cmd_si5351(61,0);             //MS2_P1[15:8]
        cmd_si5351(62,0);                 //MS2_P1[7:0]
        cmd_si5351(63,0);                   //MS2_P3[19:16], MS2_P2[19:16]
        cmd_si5351(64,0);                   //MS2_P2[15:8]
        cmd_si5351(65,0);                   //MS2_P2[7:0]
      }else{
        P1=128*M-512;
        cmd_si5351(58,0);                    //MS2_P3[15:8]
        cmd_si5351(59,1);                    //MS2_P3[7:0]
        cmd_si5351(60,(R<<4)&0x70|(P1>>16)&0x03);//0,R0_DIV[2:0],MS2_DIVBY4[1:0],MS2_P1[17:16]
        cmd_si5351(61,(P1>>8)&0xFF);        //MS2_P1[15:8]
        cmd_si5351(62,P1&0xFF);              //MS2_P1[7:0]
        cmd_si5351(63,0);                    //MS2_P3[19:16], MS2_P2[19:16]
        cmd_si5351(64,0);                    //MS2_P2[15:8]
        cmd_si5351(65,0);                    //MS2_P2[7:0]

      }
          
      if(oMf!=M){
        cmd_si5351(177,0x80);   // Reset PLLB
      }
      oMf=M;      
}



void si5351_init(void){
    pinMode(SDA, OUTPUT);
    pinMode(SCL, OUTPUT);
    digitalWrite(SDA, HIGH);
    digitalWrite(SCL, HIGH);
    delay(10);
	  cmd_si5351(183,0b10010010); // CL=8pF
	  cmd_si5351(16,0x80);     	// Disable CLK0
    cmd_si5351(17,0x80);     	// Disable CLK1
    cmd_si5351(18,0x80);      // Disable CLK2
    cmd_si5351(177,0xA0);    	// Reset PLL_A and B	  
    cmd_si5351(16,0x80);     	// Disable CLK0 (MS0=Integer Mode, Source=PLL_A)	
    cmd_si5351(17,0x80);     	// Disable CLK1 (MS1=Integer Mode, Source=PLL_A)
    cmd_si5351(18,0x6F);      // Enable CLK2 (MS2=Integer Mode, Source=PLL_B) 
}
