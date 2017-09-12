/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Gamebuino-Arduboy2Core.h"
#include <Gamebuino-Meta.h>
#include <Gamebuino-EEPROM.h>

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
  // and now let's draw the arduboy background screen stuffs
  
  gb.display.fill(INDEX_GREEN);
  gb.display.setColor(INDEX_WHITE, INDEX_GREEN);
  gb.display.setCursors(0, 0);
  gb.display.fontSize = 2;
  gb.display.println("Arduboy Game");
  gb.updateDisplay();
  gb.display.init(0, 0, ColorMode::index);
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

const uint8_t gb_lookup[] = {
    ((uint8_t)INDEX_WHITE << 4) | (uint8_t)INDEX_WHITE,
    ((uint8_t)INDEX_WHITE << 4) | (uint8_t)INDEX_BLACK,
    ((uint8_t)INDEX_BLACK << 4) | (uint8_t)INDEX_WHITE,
    ((uint8_t)INDEX_BLACK << 4) | (uint8_t)INDEX_BLACK,
};

#include <utility/Adafruit_ZeroDMA.h>
namespace Gamebuino_Meta {
extern Adafruit_ZeroDMA myDMA;
extern volatile bool transfer_is_done;

static SPISettings mySPISettings = SPISettings(24000000, MSBFIRST, SPI_MODE0);
}; // namespace Gamebuino_Meta

void prepareLine(const uint8_t* buf, uint8_t bitshift, uint16_t* destination) {
  for (uint8_t x = 0; x < WIDTH; x++) {
    uint8_t b = *(buf++);
    b >>= bitshift;
    *(destination++) = (b & 0x01) ? 0xFFFF : 0;
  }
}

uint16_t preBufferLineArray[WIDTH];
uint16_t sendBufferLineArray[WIDTH];
uint16_t *preBufferLine = preBufferLineArray;
uint16_t *sendBufferLine = sendBufferLineArray;

void Arduboy2Core::paintScreen(const uint8_t *image)
{
  uint8_t x = (gb.display.width() - WIDTH) / 2;
  uint8_t y = (gb.display.height() - HEIGHT) / 2;
  gb.tft.setAddrWindow(
    x,
    y,
    x + WIDTH - 1,
    y + HEIGHT - 1
  );
  prepareLine(image, 0, preBufferLine);
  uint8_t bitshift = 0;
  
  SPI.beginTransaction(Gamebuino_Meta::mySPISettings);
  gb.tft.dataMode();
  for (uint8_t y = 1; y < HEIGHT; y++) {
    //swap buffers pointers
    uint16_t *temp = preBufferLine;
    preBufferLine = sendBufferLine;
    sendBufferLine = temp;
    
    PORT->Group[0].OUTSET.reg = (1 << 17); // set PORTA.17 high	"digitalWrite(13, HIGH)"
    gb.tft.sendBuffer(sendBufferLine, WIDTH); //start DMA send
    
    bitshift++;
    if (bitshift >= 8) {
      bitshift = 0;
      image += WIDTH;
    }
    prepareLine(image, bitshift, preBufferLine);
    PORT->Group[0].OUTCLR.reg = (1 << 17); // clear PORTA.17 high "digitalWrite(13, LOW)"
    while (!Gamebuino_Meta::transfer_is_done); // chill
    Gamebuino_Meta::myDMA.free();
  }
  gb.tft.sendBuffer(preBufferLine, WIDTH);
  while (!Gamebuino_Meta::transfer_is_done); // chill
  Gamebuino_Meta::myDMA.free();
  gb.tft.idleMode();
  SPI.endTransaction();
  
  /*
  uint8_t x = (gb.display.width() - WIDTH) / 2;
  uint8_t y = (gb.display.height() - HEIGHT) / 2;
  uint8_t* buf = (uint8_t*)gb.display._buffer;
  uint8_t display_bytewidth = gb.display.width() / 2;
  buf += y*(display_bytewidth) + (x / 2);
  uint8_t* _buf = buf;
  uint8_t* __buf = buf;
  
  uint8_t w = WIDTH;
  uint8_t h = HEIGHT;
  uint8_t byteHeight = (h + 7) / 8;
  for (uint8_t j = 0; j < byteHeight; j++) {
    for (uint8_t i = 0; i < w; i += 2) {
      uint8_t b1 = *(image++);
      uint8_t b2 = *(image++);
      for (uint8_t k = 0; k < 8; k++) {
        uint8_t index = 0;
        if (!(b1 & 0x01)) {
          index += 2;
        }
        if (!(b2 & 0x01)) {
          index++;
        }
        *buf = gb_lookup[index];
        b1 >>= 1;
        b2 >>= 1;
        buf += display_bytewidth;
      }
      _buf++;
      buf = _buf;
    }
    __buf += display_bytewidth*8;
    _buf = __buf;
    buf = _buf;
  }
  */
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
  gb.light.fill(gb.createColor(gamebuino_red, gamebuino_green, gamebuino_blue));
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
