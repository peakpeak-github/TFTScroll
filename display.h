#pragma once

// Orientation
#define CONNECTOR_BOTTOM 0
#define CONNECTOR_RIGHT 1
#define CONNECTOR_TOP 2
#define CONNECTOR_LEFT 3

// ST7735 commands
// https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf
//
enum TFTCommands
{
    NOP = 0x00,
    SWRESET = 0x01,
    SLPOUT = 0x11,  // Sleep Out
    NORON = 0x13,   // Normal Display Mode On
    INVOFF = 0x20,  // Display Inversion Off
    DISPOFF = 0x28, // Display Off
    DISPON = 0x29,
    CASET = 0x2A,   // Column Address Set
    RASET = 0x2B,   // Row Address Set
    RAMWR = 0x2C,   // Memory Write
    RAMRD = 0x2E,   // Memory Read
    MADCTL = 0x36,  // Memory Data Access Control
    COLMOD = 0x3A,  // Interface Pixel Format
    FRMCTR1 = 0xB1, // Frame Rate Control (In normal mode/Full colors)
    FRMCTR2 = 0xB2, // Frame Rate Control (In Idle mode/8-colors)
    FRMCTR3 = 0xB3, // Frame Rate control (In Partial mode/Full colors)
    INVCTR = 0xB4,  // Display Inversion Control
    PWCTR1 = 0xC0,  // Power Control 1
    PWCTR2 = 0xC1,
    PWCTR3 = 0xC2,
    PWCTR4 = 0xC3,
    PWCTR5 = 0xC4,
    VMCTR1 = 0xC5,  // VCOM Control 1
    GMCTRP1 = 0xE0, // Gamma '+'Polarity Correction Characteristics Setting
    GMCTRN1 = 0xE1,
    DELAY = 0xFFFF, // End of TFT command list ?
    TEST = 0xF0,
    PWRSAV = 0xF6,
    VSCRDEF = 0x33, // Vertical Scrolling Definition
    VSCRSADD = 0x37 // Vertical Scrolling Start Address
};

void SetScrollStartAddress();
void SetScrollRegion(uint16_t scrollHeight, uint16_t topFixedArea = 0, uint16_t bottomFixedArea = 0);
void PrintText(const unsigned char *text);
unsigned char *TranslateExtended(const unsigned char *text);
int myGetch();
