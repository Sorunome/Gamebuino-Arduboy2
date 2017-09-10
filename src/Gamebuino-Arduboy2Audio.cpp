/**
 * @file Arduboy2Audio.cpp
 * \brief
 * The Arduboy2Audio class for speaker and sound control.
 */

#include "Gamebuino-Arduboy2.h"
#include "Gamebuino-Arduboy2Audio.h"
#include <Gamebuino-Meta.h>

bool Arduboy2Audio::audio_enabled = false;

void Arduboy2Audio::on()
{
  gb.sound.unmute();
}

void Arduboy2Audio::off()
{
  gb.sound.mute();
}

void Arduboy2Audio::toggle()
{
  if (gb.sound.isMute()) {
    gb.sound.unmute();
  } else {
    gb.sound.mute();
  }
}

void Arduboy2Audio::saveOnOff()
{
  gb.settings.set(SETTING_VOLUME_MUTE, (int32_t)gb.sound.isMute());
}

void Arduboy2Audio::begin()
{
  // trash, we already called gb.begin()
}

bool Arduboy2Audio::enabled()
{
  return !gb.sound.isMute();
}
