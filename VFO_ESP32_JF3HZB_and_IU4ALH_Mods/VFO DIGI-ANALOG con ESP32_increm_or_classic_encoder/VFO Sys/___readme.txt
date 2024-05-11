VFOsysについて

T.UEBO / JF3HZB
Feb. 14, 2019

１．

LCDとOLEDの切り替えは，display.h の中の

  #define SEPS525 と,  #define ST7735

のどちらかをコメントアウトして，コンパイルし直してください．



２．
ダイアル表示は，dial_prm.h を編集すればカスタマイズできます．


以上