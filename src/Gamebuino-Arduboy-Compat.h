#ifndef _GAMEBUINO_ARDUBOY_COMPAT_H_
#define _GAMEBUINO_ARDUBOY_COMPAT_H_

#include <Arduino.h>
#include "Gamebuino-Arduboy2.h"


namespace Gamebuino_Arduboy {

class Gamebuino_Arduboy {
public:
	void init();
	void setFrameRate(uint8_t rate, Arduboy2Base* a);
	bool nextFrame(Arduboy2Base* a);
	void displayPrework(Arduboy2Base* a);
	void paintScreen(const uint8_t* image);
	void updateNeoPixels();
	bool frameskip();
	void drawScreenBackground();
private:
	void settings();
	void adjustFramerate();
	bool dispCpu;
	bool dispFrameskip;
	bool dispUseDMA;
	uint8_t frameskip_counter;
	uint8_t frameskip_max;
	uint8_t want_framerate;
	uint8_t is_framerate;
	int8_t frameskip_index;
	uint8_t framerate_bad_counter;
	uint8_t framerate_good_counter;
};

extern Gamebuino_Arduboy gba;

}; // namespace Gamebuino_Arduboy


#endif // _GAMEBUINO_ARDUBOY_COMPAT_H_
