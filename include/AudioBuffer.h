/*
*
    An OpenAL wrapper in C++ meant for use with game environments.
    Copyright (C) 2023 Alexander Wiecking

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/
#pragma once
#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H
#include <unordered_map>
#include <al.h>
#include <string>
/*
* Audio buffers should absolutely never be seen outside of the base AudioDriver class. Their primary purpose is to both load and store
* .ogg files for use as game sounds or for music. They keep this data stored in an internal map so it can be re-called on demand.
* 
* There are a few audio buffers in use, one for game sounds and one for menu sounds. Sounds are pretty hefty, so the game sounds buffer gets wiped
* at the end of every scenario. The menu sound buffer should not be wiped.
*/
class AudioBuffer
{
public:
	//Loads audio from a filename into a buffer.
	ALuint loadAudio(std::string fname);
	//Removes the audio from a buffer.
	bool removeAudio(const ALuint& buf);
	//Removes all audio from the buffer.
	void removeAllAudio();
private:
	std::unordered_map<std::string, ALuint> buffers;
};

#endif 