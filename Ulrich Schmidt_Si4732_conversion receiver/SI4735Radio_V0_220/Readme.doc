
Wideband receiver:

Here is the description of a wideband receiver project which covers 0.1-500 MHz. It is intended as an alternative to a SDR receiver. It fits in a small box and does not require
a PC or RPi for signal processing.
It uses ESP32, ILI9488 touch display, SI4732, SI5351 and AD831 as main components. A TinySA is (optionally) used as panorama adapter. It uses potentiometers for volume, squelch and fine tune.
The software is a bit rough since I am not a programmer. It does include some nice features though, such as morse decoder, slow scan waterfall, memory bank scanning, web tools, etc.

To build the hardware you must have experience with RF circuits and the appropiate toolset. For building the filters you need a network analyzer or nanoVNA.
There is no schematics available, a lot of the hardware is based on try and error, but if you have experience with RF circuits you will be able to build it.

I used Arduino IDE for the software. It should work right away, but there are some details (for examle the right version of "arduinoFFT.h") that need to be adjusted before compiling. At least a modest understanding of C/C++ is required.
SI4735Radio_V0_220.ino is the main code file.



Brief hardware description:

1.
RF enters a +25dB wideband LNA. I used an MSA-0886 gain block, but there are better ones with lower noise available.
Important is to select a LNA that goes down to 0 MHz.

2. WBFM uses a seperate antenna connector and no LNA or filters. There is basic support in the software, but no features.

3.
RF then enters the filter bank. The filters are crucial and the most difficult part of the project. The RF gets filtered by one of 6 filters:
0-30MHz lowpass = Filter 1, 25 - 65 bandpass = 2, 65 - 135 bandpass = 3, 116 - 148 steep bandpass = 4 , 148 - 220 = bandpass 5, above 220 highpass = 6
Filters use hand wound inductors and were tuned with a NanoVNA.This is not a good solution, there is not enough dampening for far off frequencies and losses are high.
Better would be to use smd inductors.
Frequency ranges are described in struct FilterInfo and can be adjusted as needed. I had to adjust several frequencies, since the real life filter characteristics
did not exactly meet the designed frequencies. The software activates the filter in use.
Filters are Chebyshev 5th order filters.
Filters were designed with an online filter tool and a ripple of 1db.  The selected filter gets connected to the output of the LNA and the input of the mixer
through a pair of pin diodes (BAR64). Current for each pin diode is about 5ma. For low frequencies you could also use 1N4148.
A 74LS138 3:8 decoder  is connected to 3 GPIO's of the ESP32 and its outputs drive the bases of 6 PNP transistor through 680 Ohms resistors.
Their emitters are connected to +5V and the collectors drive the respective pair of pin diodes through 1K resistors.


4. The filtered RF then gets mixed down in an AD831 to the IF of 21.4MHz. I used the AD831 schematics from the application note with a 9V supply.
This is the only component that requires +9V supply. I also tried an ADE-1 diode ring mixer, but it required a much higher oscillator level, producing more spurs and interferences.
The AD 831 LO input is connected to two -6dB attenuators. One of them is connected directly to CLK2 of the SI5351 and is used for 0-200 MHz reception. The other one is connected
to CLK0 through a Chebyshev 7th order high pass. It lets pass the 3rd harmonics which is used for 200-500MHz reception. This should better be a band pass,since it lets also pass the 5th harmonics
which still produces mirrors of digital television. If CLK0 is selected in software, the VFO gets programmed with 1/3 of its frequency.

CLK2 uses mainly high injection (LO above RF), but this can be configured in struct FilterInfo. If high injection produces too many mirrors, try low injection and viceversa.
This can be done in segments, so you can for example have too many mirrors in the 2m amateur band, define the frequencies in the structure and change injection mode.
CLK0 MHz uses mainly low injection (LO below RF). 500MHz RF is not the limit, I have tried up to 650MHz, but sensitivity drops rapidly.

5. The 3rd clock from the SI5351 (CLK1) can be selected in software and used as clock source for a transmitter.

6. The IF side of the AD831 is terminated with 50 Ohms and enters a crystal filter which is made of 5 cheap 21.4 MHz 2 pole crystal filters (Aliexpress) in series and ground capacitors
in between. The bandwith is +- 5 KHz, with around 70dB dampening at +- 10 KHz. There is a depression of around 2dB at the center frequency.
This filter unexpectedly allows excellent FM narrowband flank demodulation, so I did not build a seperate NBFM demodulator like originally planned. The ground capacitors were hand picked with a
nanoVNA. The filters flanks should not be too steep, otherwise FM demodulation will get distorted. It would even be better to use 2 separate filters, a narrow one for SSB and a wider one for AM/NBFM.
My filter's center frequency is off around 3KHz (21.397MHz) for some reason, but this can be compensated in software. The filter is critical for not overloading the SI4732.
The IF can be configured in the software, anything up to 30MHz is possible, so you can also use filters for different frequencies. The higher the IF, the better.

7. A TinySA is used as optional panorama adapter. Software connects it through pin diodes to either the IF output of AD831 (through a 20dB attenuator), or to the output of the MSA-0886 preamplifier.
   Please be aware that the TinySA is not sufficient for a detailed image, it is too slow.

8. The crystal filter output connects to a SI4732, terminated with 50 Ohms. There is quite a loss (10 - 15 db estimated) through the crystal filter, since it does not use impendance transformers,
but the active mixer provides enough amplification to overcome it.
The SI4732 is controlled by an ESP32 development board. SI4732 is in the standard configuration with a 32.768 KHz crystal,
but it's frequency stability is not great. WBFM reception is possible and a seperate antenna connector is used for the FM 64-108MHz input. The software for FM radio reception is quite basic.
The audio outputs are connected together and go to a squelch transistor which is driven by the one of the ESP32 GPIO's. It grounds the audio signal when triggered and  eliminates noise and the hard cracks when switching mode.
From the squelch transistor the signal goes to the volume potentiomenter and then to a LM386 audioamplifier which drives headphones and speaker. DAC_CHANNEL_2  is connected via 330KOhms
to the input of the audio amplifier and provides a short touch sound.

9. The ESP32 drives the SI4732 and SI5351 on the same I2C bus. A squelch potentiometer and a fine tune potentiometer are connected to the ESP32.
The fine tune potentiometer is also used to adjust color spectrum in waterfall mode. The fine tune pot needs to be of good quality, since any noise causes frequency jumps.
It is important to use a 38pin version of the ESP32 board since almost all of the GPIO's will be used.
The ESP32 also drives the 3.5" ILI9488 touch display in a standard configutation for the tft eSPI library.
This code will only work with ILI9488 480*320 pixel displays and is not adaptable for smaller displays.

10. 12V DC input input gets regulated through two linear regulators down to +5 and +9V. Power consumption is about 400ma.

11. For the FFT analysis and the morse decoder, audio output goes via 4.7K and 1 microfarad to the base of a npn transistor amplifier. Emitter via 100 Ohms to ground, base via 47K to collector and collector via 1K to +3.3V.
Collector then goes to pin 36 of the ESP which does the sampling. Gain is about 20 dB. Both applications require the volume of the SI4732 to be set to a fixed level, in this case 50,
setVolume(50).

12. I have not designed a PCB, nor drawn full schematics. The ESP board is directly attached to the backside of the display. The HF part is build upon the copper sides of two seperate
(unetched) PCB's with the SI4732, AD831, crystal filter and audio amp on one and the LNA and the filters on the other one.
SI5351 is placed in a metal box and connected through coax cables with the mixer.There is a shielding plate between the display and the RF boards.


