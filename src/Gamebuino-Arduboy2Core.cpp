/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Gamebuino-Arduboy-Compat.h"
#include "Gamebuino-Arduboy2Core.h"
#include <Gamebuino-Meta.h>
#include <Gamebuino-EEPROM.h>
#include "Gamebuino-Arduboy-Compat.h"

const uint8_t PROGMEM lcdBootProgram[] = {
  // boot defaults are commented out but left here in case they
  // might prove useful for reference
  //
  // Further reading: https://www.adafruit.com/datasheets/SSD1306.pdf
  //
  // Display Off
  // 0xAE,

  // Set Display Clock Divisor v = 0xF0
  // default is 0x80
  0xD5, 0xF0,

  // Set Multiplex Ratio v = 0x3F
  // 0xA8, 0x3F,

  // Set Display Offset v = 0
  // 0xD3, 0x00,

  // Set Start Line (0)
  // 0x40,

  // Charge Pump Setting v = enable (0x14)
  // default is disabled
  0x8D, 0x14,

  // Set Segment Re-map (A0) | (b0001)
  // default is (b0000)
  0xA1,

  // Set COM Output Scan Direction
  0xC8,

  // Set COM Pins v
  // 0xDA, 0x12,

  // Set Contrast v = 0xCF
  0x81, 0xCF,

  // Set Precharge = 0xF1
  0xD9, 0xF1,

  // Set VCom Detect
  // 0xDB, 0x40,

  // Entire Display ON
  // 0xA4,

  // Set normal/inverse display
  // 0xA6,

  // Display On
  0xAF,

  // set display mode = horizontal addressing mode (0x00)
  0x20, 0x00,

  // set col address range
  // 0x21, 0x00, COLUMN_ADDRESS_END,

  // set page address range
  // 0x22, 0x00, PAGE_ADDRESS_END
};


Arduboy2Core::Arduboy2Core() { }

void Arduboy2Core::boot()
{
  gb.begin();
  EEPROM.begin(1024);
  Gamebuino_Arduboy::gba.init();
  // and now let's draw the arduboy background screen stuffs
  
  Gamebuino_Arduboy::gba.drawScreenBackground();
}

#ifdef ARDUBOY_SET_CPU_8MHZ
// If we're compiling for 8MHz we need to slow the CPU down because the
// hardware clock on the Arduboy is 16MHz.
// We also need to readjust the PLL prescaler because the Arduino USB code
// likely will have incorrectly set it for an 8MHz hardware clock.
void Arduboy2Core::setCPUSpeed8MHz()
{
  // trash
}
#endif

// Pins are set to the proper modes and levels for the specific hardware.
// This routine must be modified if any pins are moved to a different port
void Arduboy2Core::bootPins()
{
  // trash
}

void Arduboy2Core::bootOLED()
{
  // trash
}

void Arduboy2Core::LCDDataMode()
{
  // trash
}

void Arduboy2Core::LCDCommandMode()
{
  // trash
}

// Initialize the SPI interface for the display
void Arduboy2Core::bootSPI()
{
  // trash
}

// Write to the SPI bus (MOSI pin)
void Arduboy2Core::SPItransfer(uint8_t data)
{
  SPI.transfer(data);
}

void Arduboy2Core::safeMode()
{
  // trash, no need for this
}


/* Power Management */

void Arduboy2Core::idle()
{
  // do nothing
}

void Arduboy2Core::bootPowerSaving()
{
  // trash
}

// Shut down the display
void Arduboy2Core::displayOff()
{
  // trash
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
  // trash
}

uint8_t Arduboy2Core::width() { return WIDTH; }

uint8_t Arduboy2Core::height() { return HEIGHT; }


/* Drawing */

void Arduboy2Core::paint8Pixels(uint8_t pixels)
{
  // TODO: implement
//  SPItransfer(pixels);
}


void Arduboy2Core::paintScreen(const uint8_t *image)
{
  Gamebuino_Arduboy::gba.paintScreen(image);
}

// paint from a memory buffer, this should be FAST as it's likely what
// will be used by any buffer based subclass
void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{
  paintScreen((const uint8_t *)image);
  if (clear) {
    memset(image, 0, (WIDTH*HEIGHT) / 8);
  }
}

void Arduboy2Core::blank()
{
  // trash
}

void Arduboy2Core::sendLCDCommand(uint8_t command)
{
  // trash
}

// invert the display or set to normal
// when inverted, a pixel set to 0 will be on
void Arduboy2Core::invert(bool inverse)
{
  // trash
}

// turn all display pixels on, ignoring buffer contents
// or set to normal buffer display
void Arduboy2Core::allPixelsOn(bool on)
{
  // trash
}

// flip the display vertically or set to normal
void Arduboy2Core::flipVertical(bool flipped)
{
  // trash
}

// flip the display horizontally or set to normal
void Arduboy2Core::flipHorizontal(bool flipped)
{
  // trash
}

/* RGB LED */

void Arduboy2Core::setRGBled(uint8_t r, uint8_t g, uint8_t b)
{
  digitalWriteRGB(r, g, b);
}

uint8_t gamebuino_red = 0;
uint8_t gamebuino_green = 0;
uint8_t gamebuino_blue = 0;

void Arduboy2Core::digitalWriteRGB(uint8_t r, uint8_t g, uint8_t b)
{
  gamebuino_red = r;
  gamebuino_green = g;
  gamebuino_blue = b;
  gamebuino_updateNeoPixels();
}

void Arduboy2Core::digitalWriteRGB(uint8_t color, uint8_t param)
{
  uint8_t val = 0;
  if (param == RGB_ON) {
    val = 0xFF;
  }
  if (color == RED_LED)
  {
    gamebuino_red = val;
  }
  else if (color == GREEN_LED)
  {
    gamebuino_green = val;
  }
  else if (color == BLUE_LED)
  {
    gamebuino_blue = val;
  }
  gamebuino_updateNeoPixels();
}

void Arduboy2Core::gamebuino_updateNeoPixels() {
  gb.lights.fill(gb.createColor(gamebuino_red, gamebuino_green, gamebuino_blue));
}

/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  uint8_t buttons = 0;
  if (gb.buttons.repeat(BUTTON_LEFT, 0)) {
    buttons |= LEFT_BUTTON;
  }
  if (gb.buttons.repeat(BUTTON_RIGHT, 0)) {
    buttons |= RIGHT_BUTTON;
  }
  if (gb.buttons.repeat(BUTTON_UP, 0)) {
    buttons |= UP_BUTTON;
  }
  if (gb.buttons.repeat(BUTTON_DOWN, 0)) {
    buttons |= DOWN_BUTTON;
  }
  if (gb.buttons.repeat(BUTTON_A, 0)) {
    buttons |= A_BUTTON;
  }
  if (gb.buttons.repeat(BUTTON_B, 0)) {
    buttons |= B_BUTTON;
  }
  return buttons;
}

// delay in ms with 16 bit duration
void Arduboy2Core::delayShort(uint16_t ms)
{
  delay((unsigned long) ms);
}

void Arduboy2Core::exitToBootloader()
{
  gb.changeGame();
}

// Replacement main() that eliminates the USB stack code.
// Used by the ARDUBOY_NO_USB macro. This should not be called
// directly from a sketch.

void Arduboy2Core::mainNoUSB()
{

  init();
  
  setup();

  for (;;) {
    loop();
  }
}
