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
  // trash, we want people to do this via the gamebuino home menu
}

void Arduboy2Audio::off()
{
  // trash, we want people to do this via the gamebuino home menu
}

void Arduboy2Audio::toggle()
{
  // trash, we want people to do this via the gamebuino home menu
}

void Arduboy2Audio::saveOnOff()
{
  // trash, we want people to do this via the gamebuino home menu
}

void Arduboy2Audio::begin()
{
  // trash, we already called gb.begin()
}

bool Arduboy2Audio::enabled()
{
  return true;
}
