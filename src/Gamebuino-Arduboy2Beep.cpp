/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Gamebuino-Arduboy2Beep.h"
#include <Gamebuino-Meta.h>

// Speaker pin 1, Timer 3A, Port C bit 6, Arduino pin 5

uint8_t BeepPin1::duration = 0;

int8_t tone1channel = -1;
int8_t tone2channel = -1;

void BeepPin1::begin()
{
  return; // we are already inited
}

void BeepPin1::tone(uint16_t count)
{
  if (tone1channel != -1) {
    gb.sound.stop(tone1channel);
  }
  tone1channel = gb.sound.tone(1000000/(count+1));
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
  if (tone1channel != -1) {
    gb.sound.stop(tone1channel);
  }
  tone1channel = gb.sound.tone(1000000/(count+1), dur);
}

void BeepPin1::timer()
{
  return; // junk for us, handled in gb.update();
}

void BeepPin1::noTone()
{
  gb.sound.stop(tone1channel);
  tone1channel = -1;
}


// Speaker pin 2, Timer 4A, Port C bit 7, Arduino pin 13

uint8_t BeepPin2::duration = 0;

void BeepPin2::begin()
{
  return; // we are already inited
}

void BeepPin2::tone(uint16_t count)
{
  if (tone2channel != -1) {
    gb.sound.stop(tone2channel);
  }
  tone2channel = gb.sound.tone(1000000/(count+1));
}

void BeepPin2::tone(uint16_t count, uint8_t dur)
{
  if (tone2channel != -1) {
    gb.sound.stop(tone2channel);
  }
  tone2channel = gb.sound.tone(1000000/(count+1), dur);
}

void BeepPin2::timer()
{
  return; // junk for us, handled in gb.update();
}

void BeepPin2::noTone()
{
  gb.sound.stop(tone2channel);
  tone2channel = -1;
}
