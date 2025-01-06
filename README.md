
Sample code to demonstrate non-repeating dynamic scrolling text on a TFT display
Tested with ESP32 and TFT 1.8" SPI ST7735 128 x 160 display
Currently scrolling only works in portrait mode
Note: Encoding of this file should be Windows-1252 (8 bit) to display extended characters correctly

Successfully built with:
Arduino IDE 2.0
Visual Studio Code with PlatformIO
Visual Studio with VisualMicro

// User parameter definitions here ONLY
//
// Set your SPI pins for the screen
//
#define TFT_CS 25	// Chip Select pin (CS)
#define TFT_RST 26	// Reset pin (RESET)
#define TFT_DC 27	// Data/Command pin (A0)
#define TFT_SCK 32	// SCK pin
#define TFT_MOSI 33 // MOSI pin (SDA)
// #define TFT_LED 2   // (LED) 3.3V or optional backlight through a transistor to handle the required current
//
//  Set your orientation here
//
#define ORIENTATION CONNECTOR_TOP // Screen connector at the top, portrait mode
//
// Set your scroll and font parameters
// Fonts converted with fontconvert.c from the Adafruit-GFX-library to a GFX font will have it's origin at the upper left corner
// Build-in GLCD font in glcdfont.c has its origin at the lower left corner (default here)
//
#define FONTUPPERLEFTORIGIN 1	// Set to -1 if the font has its origin at the upper left corner, 1 if at the lower left corner
#define FONTHEIGHT 8			// Desired height of one line of text on the screen in pixels
#define FONTOFFSET 0		   	// Adjust if GFX font prints partially on top of screen
#define TOPFIXEDLINES 2		   	// Set number of lines of non-scrollable top area, set to 0 to scroll from top of screen
#define BOTTOMFIXEDLINES 3	   	// Set number of lines of non-scrollable bottom area to use
#define SOFTSCOLLING 1		   	// Scroll pixelwise if 1, linewise if 0
//
// Set the text- and background color here
//
#define FOREGROUNDCOL ST77XX_YELLOW // Choose from ST7735_WHITE, ST7735_BLACK, ST7735_RED, ST7735_GREEN, ST7735_BLUE,
#define BACKGROUNDCOL ST77XX_BLACK	// and ST7735_CYAN, ST7735_MAGENTA, ST7735_YELLOW, ST7735_ORANGE
//
// End of user parameter definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
