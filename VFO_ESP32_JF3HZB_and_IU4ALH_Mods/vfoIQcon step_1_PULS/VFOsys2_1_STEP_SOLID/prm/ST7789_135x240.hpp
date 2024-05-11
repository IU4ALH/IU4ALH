#ifndef PRM_H
#define	PRM_H



/*----------------------------------------------------------
   Dial design
-----------------------------------------------------------*/
#define MAINDIAL_IS_OUTSIDE //Comment out this definition and Sub dial is outsaide
//#define REV_DIAL
//#define MAIN_UNIT_kHz

#define MODE0 // MODE0, MODE1, MODE2



//ダイアル数字のサイズ --------------------------------------------------------
//Font size of numbers
#define font_sizex_main 0.75
#define font_sizey_main 0.70
#define font_sizex_sub  0.90
#define font_sizey_sub  0.70



//メインダイアル1目盛あたりの周波数 ----------------------------------------------------------
#define  freq_tick_main 10000 // Frequency per tick of Main dial: 10000(10kHz) or 100000(100kHz)
#define  freq_tick_sub  1000  // Frequency per tick of Sub  dial: 1000(1kHz) or 10000(10kHz)



//ダイアル表示位置-----------------------------------------------
//Position of dial display
#define center_offset  0
#define top_position  10
#define top_margin    10


//ダイアル半径---------------------------------------------------
#define dial_radius 280.0  //Dial radius (if 10000, Linear scale)


//メイン，サブダイアル間の間隔------------------------------
#define dial_space  50.0  // Space bitween Main and Sub dial


//目盛り間隔---------------------------------------------
#define tick_pitch_main  10.0  // Tick pitch of main dial
#define tick_pitch_sub   12.0  // Tick pitch of sub dial


//目盛りの太さ----------------------------------------
#define tw_main1   3.0  // Tick width of main(1)
#define tw_main5   4.0  // Tick width of main(5)
#define tw_main10  4.5  // Tick width of main(10)
#define tw_sub1    3.0  // Tick width of sub(1)
#define tw_sub5    4.0  // Tick width of sub(5)
#define tw_sub10   4.5  // Tick width of sub(10)


//目盛りの長さ----------------------------------------
#define tl_main1    5.0  // Tick length of main(1)
#define tl_main5   14.0  // Tick length of main(5)
#define tl_main10  18.0  // Tick length of main(10)
#define tl_sub1     5.0  // Tick length of sub(1)
#define tl_sub5    14.0  // Tick length of sub(5)
#define tl_sub10   18.0  // Tick length of sub(10)


//目盛り・数字間のスペース-------------------------------------------
#define TNCL_main  20.0  // Space bitween Number and Tick (Main dial)
#define TNCL_sub   20.0  // Space bitween Number and Tick (Sub dial)


// 指針幅，長さ--------------------------
#define DP_width   1.0  // Needle width
#define DP_len   200.0  // Needle length


#define TickMainCol TFT_CYAN
#define TickSubCol  TFT_GREEN
#define NumMainCol  TFT_WHITE
#define NumSubCol   TFT_GOLD //TFT_ORANGE
#define PointerCol  TFT_RED
#define BGCol       TFT_BLACK //0x80FF80U
#define DialBGCol   BGCol

#define TFT_BLACK2  0x0020  //opaque black




#include <LovyanGFX.hpp>
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;

public:
  LGFX(void)
  {
    { // // SPIバスの設定
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。
      cfg.spi_host = VSPI_HOST;     // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 16000000;    // 受信時のSPIクロック
      cfg.spi_3wire  = false;        // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = 18;            // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 23;            // SPIのMOSIピン番号を設定
      cfg.pin_miso = -1;            // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = 2;            // SPIのD/Cピン番号を設定  (-1 = disable)
      _bus_instance.config(cfg);    // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。
      cfg.pin_cs           =    5;  // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          =    15;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.panel_width      =   135;  // 実際に表示可能な幅
      cfg.panel_height     =   240;  // 実際に表示可能な高さ
      cfg.offset_x         =    52;  // パネルのX方向オフセット量
      cfg.offset_y         =    40;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     1;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         = false;  // データ読出しが可能な場合 trueに設定
      cfg.invert           =  true;  // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定, false: 8bit
      cfg.bus_shared       = false;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
      //cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
      //cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ

      _panel_instance.config(cfg);
    }   
    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

#endif