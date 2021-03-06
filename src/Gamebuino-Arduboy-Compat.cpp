#include "Gamebuino-Arduboy-Compat.h"
#include <Gamebuino-Meta.h>
#include <utility/Adafruit_ZeroDMA.h>
extern Gamebuino gb;

#include "language.h"


namespace Gamebuino_Arduboy {
bool is_in_settings = false;
}; // namespace Gamebuino_Arduboy

namespace Gamebuino_Meta {
#define DMA_DESC_COUNT (3)
extern volatile uint32_t dma_desc_free_count;

static inline void wait_for_desc_available(const uint32_t min_desc_num) {
	while (dma_desc_free_count < min_desc_num);
}

static inline void wait_for_transfers_done(void) {
	while (dma_desc_free_count < DMA_DESC_COUNT);
}

static SPISettings tftSPISettings = SPISettings(24000000, MSBFIRST, SPI_MODE0);
void Hook_ExitHomeMenu() {
	if (!Gamebuino_Arduboy::is_in_settings) {
		Gamebuino_Arduboy::gba.drawScreenBackground();
	}
}
}; // namespace Gamebuino_Meta

namespace Gamebuino_Arduboy {

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

void Gamebuino_Arduboy::init() {
	dispCpu = false;
	dispFrameskip = false;
	dispUseDMA = true;
}

void Gamebuino_Arduboy::setFrameRate(uint8_t rate, Arduboy2Base* a) {
	frameskip_max = 1;
	framerate_bad_counter = 0;
	framerate_good_counter = 0;
	frameskip_index = 0;
	is_framerate = want_framerate = rate;
	gb.setFrameRate(rate);
}

const uint8_t frameskip_rates_60[] = {60, 30, 20, 15, 12, 10};
const uint8_t frameskip_counters_60[] = {1, 2, 3, 4, 5, 6};

const uint8_t frameskip_rates_30[] = {30, 15, 10, 6, 5};
const uint8_t frameskip_counters_30[] = {1, 2, 3, 5, 6};

uint8_t frame_cpu_devider = 80;
uint8_t justChangedFrameRate = 0;

void Gamebuino_Arduboy::adjustFramerate() {
	uint8_t cpu = gb.getCpuLoad();
	if (want_framerate == is_framerate && cpu < 100) {
		return;
	}
	bool usageChange = false;
	if (cpu > 100) {
		framerate_bad_counter++;
		usageChange = true;
	}
	if (cpu < frame_cpu_devider) {
		framerate_good_counter++;
		usageChange = true;
	}
	bool changed = false;
	if (framerate_bad_counter > 5) {
		frameskip_index++;
		justChangedFrameRate++;
		changed = true;
	}
	if (framerate_good_counter > 5) {
		frameskip_index--;
		justChangedFrameRate++;
		changed = true;
	}
	if (!changed) {
		if (!usageChange) {
			justChangedFrameRate = 0;
		}
		return;
	}
	if (justChangedFrameRate > 2) {
		justChangedFrameRate = 0;
		frame_cpu_devider--;
	}
	framerate_bad_counter = framerate_good_counter = 0;
	if (frameskip_index < 0) {
		frameskip_index = 0;
	}
	if (want_framerate == 60) {
		if (frameskip_index > 5) {
			frameskip_index = 5;
		}
		is_framerate = frameskip_rates_60[frameskip_index];
		frameskip_max = frameskip_counters_60[frameskip_index];
		
		gb.setFrameRate(is_framerate);
		return;
	}
	if (want_framerate == 30) {
		if (frameskip_index > 5) {
			frameskip_index = 5;
		}
		is_framerate = frameskip_rates_30[frameskip_index];
		frameskip_max = frameskip_counters_30[frameskip_index];
		gb.setFrameRate(is_framerate);
		return;
	}
	is_framerate = want_framerate / (frameskip_index + 1);
	frameskip_max = frameskip_index + 1;
	gb.setFrameRate(is_framerate);
}

bool Gamebuino_Arduboy::nextFrame(Arduboy2Base* a) {
	frameskip_counter++;
	if (frameskip_counter >= frameskip_max) {
		frameskip_counter--; // if we return false we want to land here again
		if (!gb.update()) {
			return false;
		}
		frameskip_counter = 0;
		a->gamebuino_updateNeoPixels();
		a->frameCount++;
		if (gb.buttons.pressed(BUTTON_C)) {
			settings();
		}
		adjustFramerate();
		return true;
	}
	a->gamebuino_updateNeoPixels();
	if (!frameskip_counter) {
		gb.buttons.update();
		gb.checkHomeMenu();
		if (gb.buttons.pressed(BUTTON_C)) {
			settings();
		}
	}
	a->frameCount++;
	return true;
}

void Gamebuino_Arduboy::displayPrework(Arduboy2Base* a) {
	a->setTextColor(1);
	a->setTextBackground(0);
	a->setTextSize(1);
	if (dispCpu) {
		a->setCursor(0, 0);
		a->print(gb.getCpuLoad());
	}
	if (dispFrameskip) {
		uint8_t fs = frameskip_max - 1;
		uint8_t xpos = 123;
		if (fs >= 10) {
			xpos -= 6;
		}
		a->setCursor(xpos, 0);
		a->print(fs);
	}
}

const uint8_t gb_lookup[] = {
    ((uint8_t)INDEX_WHITE << 4) | (uint8_t)INDEX_WHITE,
    ((uint8_t)INDEX_WHITE << 4) | (uint8_t)INDEX_BLACK,
    ((uint8_t)INDEX_BLACK << 4) | (uint8_t)INDEX_WHITE,
    ((uint8_t)INDEX_BLACK << 4) | (uint8_t)INDEX_BLACK,
};

void Gamebuino_Arduboy::paintScreen(const uint8_t* image) {
	const uint8_t gb_display_width = 160;
	const uint8_t gb_display_height = 128;
	uint8_t x = (gb_display_width - WIDTH) / 2;
	uint8_t y = (gb_display_height - HEIGHT) / 2;
	if (!dispUseDMA) {
		uint8_t* buf = (uint8_t*)gb.display._buffer;
		uint8_t display_bytewidth = gb_display_width / 2;
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
		return;
	}
	gb.tft.setAddrWindow(
		x,
		y,
		x + WIDTH - 1,
		y + HEIGHT - 1
	);
	uint8_t bitshift = 0;
	
	SPI.beginTransaction(Gamebuino_Meta::tftSPISettings);
	gb.tft.dataMode();
	for (uint8_t y = 0; y < HEIGHT; y++) {
		if (bitshift >= 8) {
			bitshift = 0;
			image += WIDTH;
		}
		prepareLine(image, bitshift, preBufferLine);
		bitshift++;
		//swap buffers pointers
		uint16_t *temp = preBufferLine;
		preBufferLine = sendBufferLine;
		sendBufferLine = temp;
		
		gb.tft.sendBuffer(sendBufferLine, WIDTH); //start DMA send
		
		Gamebuino_Meta::wait_for_desc_available(2);
	}
	Gamebuino_Meta::wait_for_transfers_done();
	gb.tft.idleMode();
	SPI.endTransaction();
}

bool Gamebuino_Arduboy::frameskip() {
	return frameskip_counter > 0;
}

void Gamebuino_Arduboy::drawScreenBackground() {
	Gamebuino_Meta::Graphics* img;
	if (dispUseDMA) {
		gb.display.init(0, 0, ColorMode::index);
		img = &(gb.tft);
	} else {
		gb.display.init(160, 128, ColorMode::index);
		img = &(gb.display);
	}
	img->fill(INDEX_GREEN);
	img->setColor(INDEX_WHITE);
	img->setCursor(0, 0);
	img->fontSize = 2;
	img->print(gb.language.get(lang_arduboy_game));
	img->setCursor(0, 116);
	img->setColor(INDEX_BEIGE);
	img->print("\x17");
	img->setColor(INDEX_WHITE);
	img->print(gb.language.get(lang_settings));
}

void Gamebuino_Arduboy::settings() {
	is_in_settings = true;
	gb.display.init(160, 128, ColorMode::index);
	uint8_t cursor = 0;
	const uint8_t cursor_max = 3;
	while(1) {
		if (!gb.update()) {
			continue;
		}
		gb.display.clear(INDEX_GREEN);
		gb.display.fontSize = 2;
		gb.display.setColor(INDEX_WHITE);
		gb.display.print(lang_settings);
		gb.display.print("\n\n ");
		gb.display.print(lang_settings_cpu);
		gb.display.print(" ");
		if (dispCpu) {
			gb.display.setColor(INDEX_BLACK);
			gb.display.println(lang_settings_on);
		} else {
			gb.display.setColor(INDEX_RED);
			gb.display.println(lang_settings_off);
		}
		gb.display.setColor(INDEX_WHITE);
		gb.display.print(" ");
		gb.display.print(lang_settings_frameskip);
		gb.display.print(" ");
		if (dispFrameskip) {
			gb.display.setColor(INDEX_BLACK);
			gb.display.println(lang_settings_on);
		} else {
			gb.display.setColor(INDEX_RED);
			gb.display.println(lang_settings_off);
		}
		gb.display.setColor(INDEX_WHITE);
		gb.display.print(" ");
		gb.display.print(lang_settings_displaymode);
		gb.display.setColor(INDEX_BLACK);
		if (dispUseDMA) {
			gb.display.print(" ");
			gb.display.println(lang_settings_displaymode_fast);
		} else {
			gb.display.println(lang_settings_displaymode_recordable);
		}
		gb.display.setColor(INDEX_WHITE);
		gb.display.print(" ");
		gb.display.println(lang_settings_back);
		
		if ((gb.frameCount%10) < 5) {
			gb.display.setColor(INDEX_BEIGE);
			gb.display.setCursor(0, cursor*6*2 + 24);
			gb.display.print(">");
		}
		
		if (gb.buttons.repeat(BUTTON_UP, 8)) {
			if (cursor == 0) {
				cursor = cursor_max;
			} else {
				cursor--;
			}
			gb.sound.playTick();
		}
		
		if (gb.buttons.repeat(BUTTON_DOWN, 8)) {
			cursor++;
			if (cursor > cursor_max) {
				cursor = 0;
			}
			gb.sound.playTick();
		}
		
		if (gb.buttons.pressed(BUTTON_A)) {
			if (cursor == cursor_max) {
				break; // can't handle this in a switch
			}
			gb.sound.playTick();
			switch(cursor) {
				case 0: // toggle cpu
					dispCpu = !dispCpu;
					break;
				case 1: // toggle frameskip
					dispFrameskip = !dispFrameskip;
					break;
				case 2: // toggle display mode
					dispUseDMA = !dispUseDMA;
					break;
			}
		}
		
		if (gb.buttons.pressed(BUTTON_C)) {
			break;
		}
	}
	is_in_settings = false;
	drawScreenBackground();
}

Gamebuino_Arduboy gba;

}; // namespace Gamebuino_Arduboy
