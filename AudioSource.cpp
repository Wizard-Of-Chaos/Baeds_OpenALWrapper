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
#include "AudioSource.h"
#include <iostream>

AudioSource::AudioSource()
{
	alGetError();
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, m_pitch);
	alSourcef(source, AL_GAIN, m_gain);
	alSource3f(source, AL_POSITION, m_position[0], m_position[1], m_position[2]);
	alSource3f(source, AL_VELOCITY, m_velocity[0], m_velocity[1], m_velocity[2]);
	alSourcei(source, AL_LOOPING, m_loop);
	alSourcei(source, AL_BUFFER, buf);

	alSourcef(source, AL_REFERENCE_DISTANCE, m_maxDist);
	alSourcef(source, AL_MAX_DISTANCE, m_refDist);
	//alSourcef(source, AL_ROLLOFF_FACTOR, .5f);
	alSourcei(source, AL_SOURCE_RELATIVE, false);
}
AudioSource::~AudioSource()
{
	alGetError();
	alSourcei(source, AL_BUFFER, 0); //detach the buffer, if it exists
	alDeleteSources(1, &source); //get rid of the source
}
void AudioSource::play(const ALuint bufToPlay)
{
	if (buf != 0) stop();

	buf = bufToPlay;
	auto err = alGetError();
	alSourcei(source, AL_BUFFER, buf);
	if (err = alGetError() != AL_NO_ERROR) {
		std::cerr << "Could not attach buffer to source - error " << err << std::endl;
	}
	setPitch(m_pitch);
	setGain(m_gain);
	alSource3f(source, AL_POSITION, m_position[0], m_position[1], m_position[2]);
	alSource3f(source, AL_VELOCITY, m_velocity[0], m_velocity[1], m_velocity[2]);
	alSourcei(source, AL_LOOPING, m_loop);

	//alSourcef(source, AL_MAX_DISTANCE, 100.f);
	//alSourcef(source, AL_REFERENCE_DISTANCE, 100.f);

	err = alGetError();
	alSourcePlay(source);
	if (err = alGetError() != AL_NO_ERROR) {
		std::cerr << "Could not play source!\n";
	}
}

void AudioSource::stop()
{
	buf = 0;
	alGetError();
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
}

void AudioSource::setPos(const AlVec3f pos) {
	m_position[0] = pos.x;
	m_position[1] = pos.y;
	m_position[2] = -pos.z;
	auto err = alGetError();
	alSourcefv(source, AL_POSITION, m_position);
}
void AudioSource::setVel(const AlVec3f vel) {
	m_velocity[0] = vel.x;
	m_velocity[1] = vel.y;
	m_velocity[2] = -vel.z;
	alGetError();
	alSourcefv(source, AL_VELOCITY, m_velocity);
}
void AudioSource::setPitch(const float pitch)
{
	m_pitch = pitch;
	alSourcef(source, AL_PITCH, m_pitch);

}
void AudioSource::setGain(const float gain)
{
	m_gain = gain;
	alSourcef(source, AL_GAIN, m_gain);
}
void AudioSource::setLoop(const bool loop)
{
	m_loop = loop;
	alSourcei(source, AL_LOOPING, m_loop);
}
const bool AudioSource::isLooping()
{
	return m_loop;
}
void AudioSource::setMaxDist(const float dist)
{
	m_maxDist = dist;
	alSourcef(source, AL_MAX_DISTANCE, m_maxDist);
}

void AudioSource::setRefDist(const float dist)
{
	m_refDist = dist;
	alSourcef(source, AL_REFERENCE_DISTANCE, m_refDist);
}

bool AudioSource::isFinished()
{
	if (buf == 0) return true;

	alGetError();
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING) return true;

	return false;
}