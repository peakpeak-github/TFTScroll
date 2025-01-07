
Sample code to demonstrate non-repeating dynamic scrolling text on a TFT display.
Optionally have fixed areas without scrolling at top and/or bottom of the display.
Uses display hardware scrolling.
Handles ASCII extended characters (=ANSI) for åäöéü and more.

See scroll.jpg and https://youtube.com/shorts/lmKyIJF2Dw8?feature=share

Tested with ESP32 and TFT 1.8" SPI ST7735 128 x 160 display.

Currently scrolling only works in portrait mode.

Note: Encoding of this file should be Windows-1252 (8 bit) to display extended characters correctly.

Successfully built with:
Arduino IDE 2, Visual Studio Code with PlatformIO and Visual Studio with Visual Micro extension.
