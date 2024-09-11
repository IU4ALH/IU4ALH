// Select Band button, selects a predefined band. Does not use an upper limit

void setBand (bool isWaterfall) { // Use Select Band button to set start freq to a specific band, also used for range view 
    uint16_t yb = 58;
    int h = 8, row, column;
    int page = 0;

    int x_positions[] = { 15, 98, 181, 264 };
    int y_positions[] = { 70, 128, 186, 244 };

    tRel();
    while (true) {
        if (altStyle == 0)
            tft.fillRect(2, 53, 337, 238, TFT_BLACK); // overwrite previous content
        else
            drawButton(2, 50, 337, 240, TFT_YELLOW, TFT_MAROON); // background

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                drawButton(h + j * 83, (i + 1) * yb, TILE_WIDTH, TILE_HEIGHT, TFT_BTNBDR, TFT_BTNCTR);
            }
        }

        tft.setTextSize(2);
        int label_index = page * 16;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                etft.setCursor(x_positions[j] + 8, y_positions[i]);
                const BandInfo& band = bands[label_index];
                if (strcmp(band.bandName, "EXIT") == 0 || strcmp(band.bandName, "NEXT") == 0) {
                     etft.setCursor(x_positions[j] + 5, y_positions[i]);
                    etft.setTextColor(TFT_WHITE); // color for Exit and Next 
                } else if (band.isAmateurRadioBand) {
                    etft.setTextColor(TFT_GREEN); // color for amater radio 
                } else {
                    etft.setTextColor(TFT_ORANGE);  // color for boadcast 
                }
                etft.setTTFFont(Arial_14);
                etft.print(band.bandName);               
                if (band.startFreqKHz){
                  etft.setTTFFont(Arial_8);
                   etft.setCursor(x_positions[j] + 6, y_positions[i] + 24);
                   etft.printf("%lu KHz", band.startFreqKHz);           
                }
                label_index++;
            }
        }

    tPress(); // wait until pressed
    column = 1 + (tx / HorSpacing); // get row and column
    row = 1 + ((ty - 20) / VerSpacing);
   
    int selected_index = (row - 1) * 4 + (column - 1) + page * 16;
    BandInfo& sel = bands[selected_index];

 if (strcmp(sel.bandName, "EXIT") == 0)
    return; 

if (strcmp(sel.bandName, "NEXT") == 0)               
   page = (page + 1); // load pages
    
else {
      if (! isWaterfall) {
      FREQ = 1000ULL * sel.startFreqKHz;
      settleSynth();     
      break;
      }
     else {
      lim1 = 1000ULL * sel.stopFreqKHz;;
      lim2 = 1000ULL * sel.startFreqKHz;
      settleSynth();  
      waterFall(false); // call waterFall range view, do not use keypad
      return;
     }      
    }
  }
}
