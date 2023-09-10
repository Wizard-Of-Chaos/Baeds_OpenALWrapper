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
#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H
#include <al.h>
#include <cmath>

#pragma comment(lib, "libogg.lib")
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")
#pragma comment(lib, "OpenAL32.lib")

//A basic vector of 3 values for convenience. If you want to use your own vector classes by all means edit this.
struct AlVec3f {
	AlVec3f() {}
	AlVec3f(float x, float y, float z) : x(x), y(y), z(z) {}
	AlVec3f operator-(const AlVec3f& other) const { return AlVec3f(x - other.x, y - other.y, z - other.z); }
	float length() const { return sqrt(x*x + y*y + z*z); }
	float x = 0;
	float y = 0;
	float z = 0;
};

/*
* An audio source is a wrapper class around an actual audio source as specified by the OpenAL docs. If you want a thorough description of what
* exactly a source is, go read up there, but for our purposes an audio source represents a single sound that is currently playing. Emphasis on SINGLE-
* multiple sounds cannot play concurrently through one source. It includes several helpful functions to adjust values about the sound like
* pitch, max distance, and whether or not it's looping.
*/
class AudioSource
{
	public:
		AudioSource();
		~AudioSource();

		//Plays the sound from the buffer given.
		void play(const ALuint bufToPlay);
		//Stops the sound.
		void stop();
		//Sets the position of the source.
		void setPos(const AlVec3f pos);
		//Sets the velocity of the source.
		void setVel(const AlVec3f vel);
		//Sets the pitch of the source.
		void setPitch(const float pitch);
		//Sets the gain of the source.
		void setGain(const float gain);
		//Sets the source to loop.
		void setLoop(const bool loop);
		//Returns whether or not the source is looping the current sound.
		const bool isLooping();

		//Sets the maximum distance this sound can be heard from.
		void setMaxDist(const float dist);
		//Sets the distance for scaling on the sound.
		void setRefDist(const float dist);

		//Returns if the sound is finished or not.
		bool isFinished();
	private:
		float m_pitch = 1.f;
		float m_gain = .7f;
		float m_maxDist = 100.f;
		float m_refDist = 10.f;
		float m_position[3] = { 0,0,0 };
		float m_velocity[3] = { 0,0,0 };
		float m_direction[3] = { 0,0,0 };
		bool m_loop = false;
		ALuint source; //the identifier of the source, do not touch this
		//a source has exactly ONE attached buffer - this means that a source plays ONE sound.
		ALuint buf = 0;
};

#endif 