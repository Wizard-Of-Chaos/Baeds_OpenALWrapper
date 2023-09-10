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
#include "AudioBuffer.h"

#include <fstream>
#include <iostream>

#include <ogg.h>
#include <vorbisfile.h>
#include <vorbisenc.h>

//credit to https://gist.github.com/tilkinsc/f91d2a74cff62cc3760a7c9291290b29 for this loader
ALuint loadOgg(const char* path)
{
	ALenum error = 0;
	ALuint sound = 0;
	FILE* fp = 0;
	OggVorbis_File vf;
	vorbis_info* vi = 0;
	ALenum format = 0;
	short* pcmout = 0;

	size_t dataLength;

	fp = fopen(path, "rb");
	if (!fp) {
		std::cerr << "Could not open file: " << path << std::endl;
		return 0;
	}
	alGenBuffers(1, &sound);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		std::cerr << "Error creating buffer: " << path << ", buffer=" << sound << ", error=" << error << std::endl;
		fclose(fp);
		return 0;
	}

	if (ov_open_callbacks(fp, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0) {
		std::cerr << "Stream is not a valid OggVorbis stream.\n";
		goto fail;
	}

	vi = ov_info(&vf, -1);
	format = vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	dataLength = ov_pcm_total(&vf, -1) * vi->channels * 2;
	pcmout = (short*)malloc(dataLength);
	if (pcmout == 0) {
		std::cerr << "Out of memory. What the hell are you doing?\n";
		goto fail;
	}
	for (size_t size = 0, offset = 0, sel = 0;
		(size = ov_read(&vf, (char*)pcmout + offset, 4096, 0, 2, 1, (int*)&sel)) != 0;
		offset += size) {
		if (size < 0) {
			std::cerr << "This ogg file is faulty.\n";
		}
	}
	alBufferData(sound, format, pcmout, (ALsizei)dataLength, (ALsizei)vi->rate);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		std::cerr << "Failed to send audio info to OpenAL.\n";
		goto fail;
	}

	fclose(fp);
	ov_clear(&vf);
	free(pcmout);
	return sound;

fail:
	std::cerr << "Could not load .ogg file.\n";
	fclose(fp);
	ov_clear(&vf);
	free(pcmout);
	return 0;
}

ALuint AudioBuffer::loadAudio(std::string fname)
{
	if (buffers.find(fname) != buffers.end()) return buffers[fname];

	ALuint sound = loadOgg(fname.c_str());
	if (sound == 0) {
		std::cerr << "Error loading on " << fname << "!\n";
		return sound;
	}
	std::cout << "Loaded " << fname << std::endl;
	buffers[fname] = sound;
	return sound;
}
bool AudioBuffer::removeAudio(const ALuint& buf)
{
	std::string key = "";
	for (const auto& [str, val] : buffers) {
		if (val == buf) {
			alGetError();
			alDeleteBuffers(1, &val);
			auto err = alGetError();
			if (err != AL_NO_ERROR) {
				std::cerr << "Something went wrong on removing a buffer - error=" << err << std::endl;
			}
		}
		key = str;
		break;
	}
	if (key == "") return false;

	buffers.erase(key);
	return true;
}

void AudioBuffer::removeAllAudio()
{
	for (auto& [key, val] : buffers) {
		alDeleteBuffers(1, &val);
	}
	buffers.clear();
}