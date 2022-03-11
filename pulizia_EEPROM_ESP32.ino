/**************************************************************
   Pulizia EEPROM su dispositivi ESP32 - ESP8266
   Versione 1.0   by Nino IU4ALH   

   26 maggio 2021
/**************************************************************/




#include <EEPROM.h>  

void setup() {
  EEPROM.begin(512);  // 512 è la grandezza della Eeprom del ESP32 e ESP8266
  
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);      // porta al valore  0 tutti i 512 bytes della EEPROM
  }
  EEPROM.end();  // finalizza la EEPROM e chiude
}

void loop() {
}
