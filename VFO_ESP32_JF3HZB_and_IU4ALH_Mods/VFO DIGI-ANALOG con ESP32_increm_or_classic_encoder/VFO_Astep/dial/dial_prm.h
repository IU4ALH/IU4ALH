/*----------------------------------------------------------------------------------
       Dial parameter
-----------------------------------------------------------------------------------*/
char f_dispmode = 0;   // 0:Normal / 1: Rotate 90 degrees
char f_main_outside = 0; // 0:Main dial is inside / 1:Main dial is outside
char f_FONT = 0;   // Font   0, 1, or 2
char f_rev = 0;   // Increasing Direction of dial rotation   0: CW / 1: CCW

char f_maintick1 = 1;  // Main Tick(1) display on/off
char f_maintick5 = 1;  // Main Tick(5) display on/off 
char f_maintick10 = 1; // Main Tick(10) display on/off
char f_mainnum = 1;    // Main Number display on/off

char f_subtick1 = 1;  // Sub Tick(1) display on/off
char f_subtick5 = 1;  // Sub Tick(5) display on/off
char f_subtick10 = 1; // Sub Tick(10) display on/off
char f_subnum = 1;    // Sub Number display on/off


//メインダイアル1目盛あたりの周波数 ------------------------------------------------------------
int freq_tick_main = 10000;  // Frequency/tikcs of Main dial (if 10000: 10kHz / else： 100kHz)

//ダイアル表示エリア------------------------------------------------------------
int D_height = 80;   // Display position

//ダイアル半径------------------------------------------------------------------
int D_R = 160;   //Dial radius (if 45000, Linear scale)

//メイン，サブダイアルの間隔----------------------------------------------------
int Dial_space = 40;   // Space bitween Main and Sub Dial

//目盛り間隔--------------------------------------------------------------------
int tick_pitch_main = 50; // Main Tick Pitch
int tick_pitch     = 60;  // Sub Tick Pitch

//目盛りの太さ------------------------------------------------------------------
int tick_width = 1; // Width of Tick( 0 or 1 )

//目盛りの長さ(main)------------------------------------------------------------
int tick_main1 = 4;  // Length of Main Tick(1)
int tick_main5 = 9;  // Length of Main Tick(5)
int tick_main10 = 12; // Length of Main Tick(10)

//目盛りの長さ(sub)-------------------------------------------------------------
int tick1 = 8;    // Length of Sub Tick(1)
int tick5 = 11;   // Length of Sub Tick(5)
int tick10 = 15;  // Length of Sub Tick(10)

//目盛り・数字間のスペース------------------------------------------------------
int TNCL_main = 13;  // Space bitween Number and Tick (Main)
int TNCL = 16;      // Space bitween Number and Tick (Sub)

// 指針長さ，幅-----------------------------------------------------------------
int DP_len = 90;   // Length of Dial pointer
int DP_width = 1;  // Width of Dial pointer
int DP_pos = 12;   // Position of Dial pointer


//Coloring  0xRRGGBB -----------------------------------------------------------
// 背景
unsigned long cl_BG = 0x000000;  // Background
// 指針色
unsigned long cl_POINTER = 0xFF0000; // Dial pointer
// main目盛
unsigned long cl_TICK_main = 0x00FF00;  // Main Tick
// main数字
unsigned long cl_NUM_main = 0xFFFFFF;   //Main Number
// sub目盛
unsigned long cl_TICK = 0x00FFFF;  // Sub Tick
// sub数字
unsigned long cl_NUM = 0xFFFFFF;  // Sub Number
// ダイアルベース
unsigned long cl_DIAL_BG = 0x000000;  // Dial base
/*---------------------------------------------------------------------------------

---------------------------------------------------------------------------------*/
