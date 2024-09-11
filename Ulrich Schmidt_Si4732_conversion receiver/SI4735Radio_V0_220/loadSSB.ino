

//=======================================================================================
void loadSSB() {
  //=======================================================================================

  if (!ssbLoaded) {
    delay(25);
    reset();
    si4735.queryLibraryId();
    si4735.patchPowerUp();
    delay(50);
    si4735.setI2CFastModeCustom(PATCHLOADSPEED);  // 2MHz seems to work just fine
    si4735.downloadPatch(ssb_patch_content, size_content);
    si4735.setSSBConfig(bandWidth, 0, 0, 1, 0, 1);
    delay(25);
    si4735.setI2CFastMode();
    si4735.setSSBAudioBandwidth(bandWidth); 
    if (bandWidth <= 2) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
    ssbLoaded = true;
    
    /*
setSSBConfig()
AUDIOBW	SSB Audio bandwidth; 0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz; 4=500Hz; 5=1kHz.
SBCUTFLT	SSB side band cutoff filter for band passand low pass filter if 0, the band pass filter to cutoff both the unwanted side band and high frequency component > 2kHz of the wanted side band (default).
AVC_DIVIDER	set 0 for SSB mode; set 3 for SYNC mode.
AVCEN	SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
SMUTESEL	SSB Soft-mute Based on RSSI or SNR.
DSP_AFCDIS	DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.

*/
  }
}
//##########################################################################################################################//
void setAM() {

  si4735.setAM(520, 29990, SI4735TUNE_FREQ, 1);
}  

//##########################################################################################################################//

void reset() {

  si4735.reset();

}

//##########################################################################################################################//


