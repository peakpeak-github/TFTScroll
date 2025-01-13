////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sample code to demonstrate non-repeating scrolling text on a TFT display
// Tested with ESP32 and TFT 1.8" SPI ST7735 128 x 160 display
// Currently scrolling only works in portrait mode
// Note: Encoding of this file should be Windows-1252 (8 bit) to display extended characters correctly
//
// Successfully built with:
// Arduino IDE 2.0
// Visual Studio Code with PlatformIO
// Visual Studio with VisualMicro
//
#include <stdint.h> // uint8_t etc ...
#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "display.h"
//
// Using a GFX font. Make sure you set FONTUPPERLEFTORIGIN to -1 and FONTHEIGHT to the font's height in pixels
// Also possibly set FONTOFFSET to a small positive value if the font prints partially on top of the screen
//
//#include "..\..\..\GFXFonts\MyLucidaConsole.h" // Converted with fontconvert.c from the Adafruit-GFX-library
//const GFXfont *g_fontName = &LUCON8pt8b;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

#define SCREENLINES (SCREENHEIGHT / FONTHEIGHT)		// Number of complete lines on the screen
#define TOPFIXEDHEIGHT (TOPFIXEDLINES * FONTHEIGHT) // Number of pixel rows to use for non-scrollable areas
//
// If SCREENHEIGH is not even divisible with FONTHEIGHT then (SCREENHEIGHT % FONTHEIGHT) pixels in height
// of the bottom area will be non-scrollable
//
#define BOTTOMFIXEDHEIGHT ((BOTTOMFIXEDLINES * FONTHEIGHT) + (SCREENHEIGHT % FONTHEIGHT))

#if ((ORIENTATION == CONNECTOR_BOTTOM) || (ORIENTATION == CONNECTOR_TOP)) // Portrait orientations
#define SCREENWIDTH 128
#define SCREENHEIGHT 160
#endif

#if ((ORIENTATION == CONNECTOR_LEFT) || (ORIENTATION == CONNECTOR_RIGHT)) // Landscape orientations
#define SCREENWIDTH 160
#define SCREENHEIGHT 128
#endif

#define LINECOUNT (SCREENHEIGHT / FONTHEIGHT)				// Number of lines on the screen
#define FONTRECTANGLE (FONTUPPERLEFTORIGIN * FONTHEIGHT)	// Erasing previous line
#define SCROLLABLEHEIGHT (SCREENHEIGHT - TOPFIXEDHEIGHT)
#define LASTPIXELLINE (SCREENHEIGHT - BOTTOMFIXEDHEIGHT)
#define LASTTEXTLINE (LASTPIXELLINE / FONTHEIGHT)

#if (FONTUPPERLEFTORIGIN == -1)
#define SCROLLSTARTPOS FONTHEIGHT // Start scrolling one line below the top as the font has its origin at the upper left corner
#else
#define SCROLLSTARTPOS 0 // Font has its origin at the lower left corner (built-in font)
#endif

#ifdef OTHER
SPIClass mySPI(VSPI);													  // Create an SPI object with custom pins with VSPI (you can also use HSPI if needed)
Adafruit_ST7735 g_tft = Adafruit_ST7735(&mySPI, TFT_CS, TFT_DC, TFT_RST); // Create an instance of the display with the custom SPI object
#else																	  // ESP32, ESP8266
Adafruit_ST7735 g_tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST); // Create an instance of the display with the default SPI object
#endif

#if (FONTUPPERLEFTORIGIN == 1)									 // Font is bottoms-up (built-in GLCD font in glcdfont.c)
int16_t g_scrollPos = TOPFIXEDHEIGHT;							 // Current scroll position, start offset in frame buffer
int16_t g_textPos = -FONTHEIGHT + (TOPFIXEDHEIGHT - FONTOFFSET); // Current text position
#else															 // Font is top-down (GFX fonts)
int16_t g_scrollPos = TOPFIXEDHEIGHT;			   // Current scroll position, start offset in frame buffer
int16_t g_textPos = (TOPFIXEDHEIGHT - FONTOFFSET); // Current text position
#endif
uint16_t g_lineCounter = 0; // Counter for line numbers
uint8_t g_data[6];			// Commands to send to the display
bool g_scroll = false;		// Scroll flag
//
// Translation table from ANSI extended characters to bult-in GLCD font characters
// You can add more characters if needed, see Built-In_Font.png
//
constexpr const char *g_ansiChars = "ÅåÄäÖöÉéÜüÇç";
// Character codes in GLCD font                 Å     å     Ä     ä     Ö     ö     É     é     Ü     ü     Ç     ç
constexpr unsigned const char g_glcdChars[] = {0x8f, 0x86, 0x8e, 0x84, 0x99, 0x94, 0x90, 0x82, 0x9a, 0x81, 0x80, 0x87};

void setup()
{
	Serial.begin(115200);
	while (!Serial)
		delay(10);
	Serial.println("\nStarting ...");
	delay(500);

	// pinMode(TFT_LED, OUTPUT);
	// digitalWrite(TFT_LED, HIGH);  // Enable backlight (must be through a transistor). With PWM dimming is possible

#ifdef OTHER
	mySPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS); // Begin the SPI bus with custom pins
#endif
	g_tft.initR(INITR_BLACKTAB);										  // Initialization for ST7735 with black tab
	g_tft.setRotation(ORIENTATION);										  // Set our screen orientation
#if ((ORIENTATION == CONNECTOR_RIGHT) || (ORIENTATION == CONNECTOR_LEFT)) // ): Scroll direction wrong for landscape orientations
	g_tft.writeCommand(ST77XX_MADCTL);
	g_tft.spiWrite(ST77XX_MADCTL_MV | ST77XX_MADCTL_MX | ST77XX_MADCTL_MY); // Does not work, scrolls in the wrong direction
#endif
	g_tft.fillScreen(BACKGROUNDCOL); // Clear the screen, set background color
//	g_tft.setFont(g_fontName);		 // Load optional GFX font
	g_tft.setTextWrap(false);
	g_tft.setTextSize(1);												  // Size 1 -> the font's pixel height will match the TEXTHEIGHT value
	SetScrollRegion(SCROLLABLEHEIGHT, TOPFIXEDHEIGHT, BOTTOMFIXEDHEIGHT); // Set the scrollable region
																		  //
																		  // If unscrollable areas are used, fill them with some text
																		  //
#if (TOPFIXEDHEIGHT > 0)
	g_tft.setTextColor(ST7735_CYAN); // Set text color

#if (FONTUPPERLEFTORIGIN == -1) // Font is top-down (GFX fonts)
	for (int i = 1; i <= TOPFIXEDLINES; i++)
#else
	for (int i = 0; i < TOPFIXEDLINES; i++)
#endif
	{
		g_tft.setCursor(0, (i * FONTHEIGHT) - FONTOFFSET);
		g_tft.print("Top fixed area");
	}
#endif
#if (BOTTOMFIXEDHEIGHT - (SCREENHEIGHT % FONTHEIGHT) > 0)
	g_tft.setTextColor(ST7735_GREEN); // Set text color
#if (FONTUPPERLEFTORIGIN == -1)		  // Font is top-down (GFX fonts)
	for (int i = 1 + (SCREENLINES - BOTTOMFIXEDLINES); i <= SCREENLINES; i++)
#else
	for (int i = SCREENLINES - BOTTOMFIXEDLINES; i < SCREENLINES; i++)
#endif
	{
		g_tft.setCursor(0, (i * FONTHEIGHT) - FONTOFFSET);
		g_tft.print("Bottom fixed area");
	}
#endif
	g_tft.setTextColor(FOREGROUNDCOL); // Set main text color
	Serial.printf("LASTPIXELLINE: %d, LASTTEXTLINE: %d, textpos %d, scrollPos %d\n", LASTPIXELLINE, LASTTEXTLINE, g_textPos, g_scrollPos);
}
void loop()
{
	unsigned char buf[32];

	sprintf((char *)buf, "Line %d: ÄäÉéÜüÇç", g_lineCounter++); // Test to output some extended characters
	// delay(500);
	PrintText(buf);
	myGetch();
}
int myGetch()
{
	while (Serial.available() == 0) // Wait for data
		delay(50);
	return Serial.read();
}
///////////////////////////////////////////////////////////////////
// Function to print text and scroll when screen is full
// Currently scrolling only works in portrait mode
//
void PrintText(const unsigned char *text)
{
	unsigned char *translatedText;

	if (g_scroll == false)
	{
		g_textPos += FONTHEIGHT;		// Assume screen not filled, print at the next line
#if (FONTUPPERLEFTORIGIN == 1)			// Using GLCD font
		if (g_textPos >= LASTPIXELLINE) // Screen filled, set scroll flag
#else									// Using GFX font
		if (g_textPos > LASTPIXELLINE) // Screen filled, set scroll flag
#endif
			g_scroll = true;
	}
	if (g_scroll)
#if (FONTUPPERLEFTORIGIN == 1) // Font is bottoms-up (built-in font)
	{
		g_scrollPos += FONTHEIGHT; // Scroll one line
		if (g_scrollPos >= SCROLLABLEHEIGHT)
			g_scrollPos = FONTHEIGHT + SCROLLSTARTPOS + TOPFIXEDHEIGHT; // Restart at the top
		g_textPos = g_scrollPos - (FONTHEIGHT + FONTOFFSET);
	}
#else // Font is top-down (GFX fonts)
	{
		g_scrollPos += SCROLLSTARTPOS; // Scroll one line
		if (g_scrollPos >= SCROLLABLEHEIGHT)
			g_scrollPos = SCROLLSTARTPOS + TOPFIXEDHEIGHT; // Restart at the top
		g_textPos = g_scrollPos - FONTOFFSET;
	}
#endif
	Serial.printf("Scroll: %d. Printing %s at textPos %d, scrollPos %d\n", g_scroll, text, g_textPos, g_scrollPos);
	g_tft.fillRect(0, g_textPos, SCREENWIDTH, FONTRECTANGLE, BACKGROUNDCOL); // Erase location for new text
	g_tft.setCursor(0, g_textPos);
#if (FONTUPPERLEFTORIGIN == 1)				  // Font is bottoms-up (built-in font)
	translatedText = TranslateExtended(text); // Translate possibly extended characters
	g_tft.print((char*)translatedText);			  // Print the text on the TFT display
	free(translatedText);					  // Free the copy
#else
	g_tft.print(text); // Print the text on the TFT display
#endif
	SetScrollStartAddress();
}
////////////////////////////////////////////////////////////////////////////////////////////////
// Translate extended ASCII characters to corresponding GLCD built-in font characters
//
unsigned char *TranslateExtended(const unsigned char *text)
{
	int tlen, alen, n, i;
	unsigned char *translatedText = (unsigned char*)strdup((char*)text); // Make a writable copy of the text
	unsigned char *p = translatedText;

	tlen = strlen((char*)translatedText);
	alen = strlen(g_ansiChars);

	for (n = 0; n < tlen; n++)
	{
		for (i = 0; i < alen; i++)
		{
			if (*p == g_ansiChars[i])
			{
				*p = g_glcdChars[i];
				break;
			}
		}
		p++;
	}
	return translatedText; // Caller must do a free() on the returned pointer
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scroll one line
// Set the vertical scroll start address which specifies which line in the framebuffer corresponds to the top of the visible screen
//
void SetScrollStartAddress()
{
#if (SOFTSCOLLING == 1)
	if (g_scroll)
	{
		int i;
		int n = g_scrollPos - FONTHEIGHT;
		for (i = n; i <= g_scrollPos; i++)
		{
			g_data[0] = i >> 8;						// High byte of scroll start address
			g_data[1] = i & 0xFF;					// Low byte of scroll start address
			g_tft.sendCommand(VSCRSADD, g_data, 2); // Send Vertical Scroll Start Address command
			delay(20);
		}
	}
	else
#endif
	{
		g_data[0] = g_scrollPos >> 8;			// High byte of scroll start address
		g_data[1] = g_scrollPos & 0xFF;			// Low byte of scroll start address
		g_tft.sendCommand(VSCRSADD, g_data, 2); // Send Vertical Scroll Start Address command
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Define the scrollable region
//
void SetScrollRegion(uint16_t scrollHeight, uint16_t topFixedArea, uint16_t bottomFixedArea)
{
	g_data[0] = topFixedArea >> 8;		   // Top fixed area (high byte)
	g_data[1] = topFixedArea & 0xFF;	   // Top fixed area (low byte)
	g_data[2] = scrollHeight >> 8;		   // Scrollable area height (high byte)
	g_data[3] = scrollHeight & 0xFF;	   // Scrollable area height (low byte)
	g_data[4] = bottomFixedArea >> 8;	   // Bottom fixed area (high byte)
	g_data[5] = bottomFixedArea & 0xFF;	   // Bottom fixed area (low byte)
	g_tft.sendCommand(VSCRDEF, g_data, 6); // Send Vertical Scroll Definition command
}
