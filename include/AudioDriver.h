#pragma once
#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H
#include "AudioBuffer.h"
#include "AudioSource.h"
#include <alc.h>
#include <random>
#include <functional>
#include <memory>
#include <stdio.h>
/*
* The audio driver class does what you think it does and handles the audio for the game itself, including the loading of files, playing of audio,
* and management of various sound sources within a scene. It keeps track of anything that is currently making noise in the game, be that a menu sound
* effect, the music, or in-game effects.
*/
template<class T>
class AudioDriver
{
	public:
		//utility structure for managing sound instances
		struct _SoundInstance {
			T id;
			std::shared_ptr<AudioSource> src;
			bool overrideValidLoop = false;
		};
		/*
		Initializes the audio driver.
		velocityFunc, positionFunc, and validityFunc are all necessary functions that should take in whatever your game's "entity" construct
		is for sound and check the velocity of it, the position of it, and whether or not it's valid (alive).
		Example functions:
			bool myValidityFunction(myEntity entity) {
				return entity.isAlive();
			}

			AlVec3f myPositionFunction(myEntity entity) {
				myInternalVec3f pos = entity.position();
				return AlVec3f(pos.X, pos.Y, pos.Z);
			}

			AlVec3f myVelocityFunction(myEntity entity) {
				myInternalVec3f vel = entity.velocity();
				return AlVec3f(vel.X, vel.Y, vel.Z);
			}
		*/
		AudioDriver(std::function<AlVec3f(T)> velocityFunc, std::function<AlVec3f(T)> positionFunc, std::function<bool(T)> validityFunc, 
			float speedOfSound = 331.5f, float dopplerFactor = 1.f) 
			: m_velocityFunc(velocityFunc), m_positionFunc(positionFunc), m_validityFunc(validityFunc)
		{
			device = alcOpenDevice(nullptr);
			if (device) {
				context = alcCreateContext(device, nullptr);
				if (context) {
					alcMakeContextCurrent(context);
				}
			}
			const ALCchar* name = nullptr;
			if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
				name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
			if (!name || alcGetError(device) != AL_NO_ERROR)
				name = alcGetString(device, ALC_DEVICE_SPECIFIER);

			alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
			alSpeedOfSound(speedOfSound);
			alDopplerFactor(dopplerFactor);

			printf("Opened audio device: %s \n", name);
			musicSource = new AudioSource;
			musicSource->setGain(musicGain);
			musicSource->setLoop(true);
		}


		//This plays a sound from the given source in the game and registers the source. Returns the source if you need to track it.
		//This sound is attached to an entity, and will stop if the validityFunc for this entity fails.
		std::shared_ptr<AudioSource> playGameSound(T ent, std::string fname, float gain = 1.f, float refDist = 20.f, float maxDist = 1200.f, bool loop = false)
		{
			AlVec3f srcPos = m_positionFunc(ent);
			ALfloat lPos[3] = { 0.f, 0.f, 0.f };
			alGetListener3f(AL_POSITION, &lPos[0], &lPos[1], &lPos[2]);
			AlVec3f listener(lPos[0], lPos[1], -lPos[2]);

			if (m_useMaximumDistance) {
				if (std::abs((srcPos - listener).length()) >= m_maximumDistance) {
					return std::shared_ptr<AudioSource>(); //returns a null if the sound is more than a kilometer away
				}
			}

			ALuint buf = 0;
			if (loadedGameSounds.find(fname) != loadedGameSounds.end()) {
				buf = loadedGameSounds.at(fname);
			}
			else {
				buf = gameSounds.loadAudio(m_gameSoundPath + fname);
				if (buf == 0) return nullptr;
				loadedGameSounds[fname] = buf;
			}
			//also needs to register the audio source
			AudioSource* src = new AudioSource;

			src->setVel(m_velocityFunc(ent));

			src->setRefDist(refDist);
			src->setMaxDist(maxDist);
			src->setGain(gain * gameGain);
			src->setLoop(loop);

			src->setPitch(m_randomPitchOnGameSounds ? std::uniform_real_distribution<float>(.75f, 1.25f)(randGen) : 1.f);

			src->play(buf);
			_SoundInstance inst;
			inst.id = ent;
			inst.src = std::shared_ptr<AudioSource>(src);

			curGameSounds.push_back(inst);
			return inst.src;
		}

		//This plays a sound from the given source in the game and registers the source. Returns the source if you need to track it.
		//This plays the sound explicitly from the given position, and is not attached to an entity.
		std::shared_ptr<AudioSource> playGameSound(AlVec3f position, std::string fname, float gain = 1.f, float refDist = 20.f, float maxDist = 1200.f, bool loop = false)
		{
			AlVec3f srcPos = position;
			ALfloat lPos[3] = { 0.f, 0.f, 0.f };
			alGetListener3f(AL_POSITION, &lPos[0], &lPos[1], &lPos[2]);
			AlVec3f listener(lPos[0], lPos[1], -lPos[2]);

			if (m_useMaximumDistance) {
				if (std::abs((srcPos - listener).length()) >= m_maximumDistance) {
					return std::shared_ptr<AudioSource>(); //returns a null if the sound is more than a kilometer away
				}
			}

			ALuint buf = 0;
			if (loadedGameSounds.find(fname) != loadedGameSounds.end()) {
				buf = loadedGameSounds.at(fname);
			}
			else {
				buf = gameSounds.loadAudio(m_gameSoundPath + fname);
				if (buf == 0) return nullptr;
				loadedGameSounds[fname] = buf;
			}
			//also needs to register the audio source
			AudioSource* src = new AudioSource;

			src->setRefDist(refDist);
			src->setMaxDist(maxDist);
			src->setGain(gain * gameGain);
			src->setLoop(loop);

			src->setPitch(m_randomPitchOnGameSounds ? std::uniform_real_distribution<float>(.75f, 1.25f)(randGen) : 1.f);

			src->play(buf);
			_SoundInstance inst;
			inst.src = std::shared_ptr<AudioSource>(src);
			inst.overrideValidLoop = loop;

			curGameSounds.push_back(inst);
			return inst.src;
		}

		//Plays a menu sound effect.
		void playMenuSound(std::string fname)
		{
			ALuint buf = 0;
			if (loadedMenuSounds.find(fname) != loadedMenuSounds.end()) {
				buf = loadedMenuSounds.at(fname);
			}
			else {
				buf = menuSounds.loadAudio(m_menuSoundPath + fname);
				if (buf == 0) return;
				loadedMenuSounds[fname] = buf;
			}
			AudioSource* src = new AudioSource;
			src->setGain(menuGain);
			src->play(buf);
			curMenuSounds.push_back(src);
		}
		//Plays music. Will halt any present music.
		void playMusic(std::string fname)
		{
			if (currentMusic != 0) {
				musicSource->stop();
				menuSounds.removeAudio(currentMusic);
			}

			auto music = menuSounds.loadAudio(m_musicPath + fname);
			currentMusic = music;
			musicSource->play(music);
		}
		//Updates all the sounds in the game to be deleted and shuffled around.
		//ALWAYS CALL setListenerPosition PRIOR TO USING THIS UPDATE
		void gameSoundUpdate() {
			auto it = curGameSounds.begin();
			while (it != curGameSounds.end()) {
				if (it->src->isFinished()) { //if the sound is finished we're done here
					//std::cout << "erasing sound\n";
					delete it->src;
					it = curGameSounds.erase(it);
					continue;
				}
				if (m_validityFunc(it->id)) { //if the entity is still alive we need to update the sound accordingly

					it->src->setPos(m_positionFunc(it->id));
					it->src->setVel(m_velocityFunc(it->id));
				}
				else { //if it's not alive, we need to waste anything that's looping still, but if it's a regular effect just let it play out
					if (it->src->isLooping() && !it->src->overrideValidLoop) {
						it->src->setLoop(false); //this will make it finished on the next iteration
					}
				}
				++it;
			}
		}
		//Wipes the data buffer for in-game sound effects. Useful for ending a scene and returning to menus.
		void cleanupGameSounds()
		{
			setListenerPosition(AlVec3f(0, 0, 0));
			for (auto inst : curGameSounds) {
				inst.src->stop();
				delete inst.src;
			}
			curGameSounds.clear();

			gameSounds.removeAllAudio();
			loadedGameSounds.clear();
		}
		std::list<_SoundInstance> curGameSounds;
		std::list<AudioSource*> curMenuSounds;

		//Runs an update for menu sounds. Unlike the game sounds, this does not track entities or position.
		//The "ingame" is meant to help for whether or not you're trying to play menu sounds while actively in game.
		void menuSoundUpdate(bool inGame = false)
		{
			auto it = curMenuSounds.begin();
			while (it != curMenuSounds.end()) {
				AudioSource* src = *it;
				if (src->isFinished()) {
					delete src;
					it = curMenuSounds.erase(it);
				}
				if (it == curMenuSounds.end()) break;

				++it;
			}
			if (!inGame) {
				setListenerPosition(AlVec3f(0, 0, 0));
				for (auto src : curMenuSounds) {
					src->setPos(AlVec3f(0, 0, 0));
					src->setVel(AlVec3f(0, 0, 0));
				}
			}
		}
		//Sets the listener position, including up values and forward velocity.
		void setListenerPosition(AlVec3f pos, AlVec3f up = AlVec3f(0.f, 1.f, 0.f), AlVec3f forward = AlVec3f(0.f, 0.f, -1.f), AlVec3f vel = AlVec3f(0.f, 0.f, 0.f))
		{
			auto err = alGetError();
			ALfloat orient[] = { forward.x, forward.y, -forward.z, up.x, up.y, -up.z };
			alListener3f(AL_POSITION, pos.x, pos.y, -pos.z);
			alListener3f(AL_VELOCITY, vel.x, vel.y, -vel.z);
			alListenerfv(AL_ORIENTATION, orient);
			musicSource->setPos(pos);
			musicSource->setVel(vel);
			for (auto src : curMenuSounds) {
				src->setPos(pos);
				src->setVel(vel);
			}
		}
		//Sets the global gains for sound effects and the various types of sound.
		void setGains(float master, float music, float game, float menu)
		{
			masterGain = master;
			musicGain = music;
			gameGain = game;
			menuGain = menu;
			m_updateGains();
		}

		//Sets the paths to look for the various types of sound - music, menu, and gains. Default is no path.
		void setPaths(std::string music, std::string menus, std::string game) { m_musicPath = music; m_menuSoundPath = menus; m_gameSoundPath = game; }
		//If this is enabled, the game's sounds will vary in pitch by ~.5f to make them all sound less monotonous.
		//Default: True
		void setRandomPitch(bool random = true) { m_randomPitchOnGameSounds = random; }
		//Sets the maximum distance a new sound can be spawned at. Default: 1500
		//If the sound is further away from the listener than this distance, it will not play. Only works if useMaximumDistance is set to true.
		void setMaximumDistance(float max) { m_maximumDistance = max; }
		//Should this driver use a maximum distance to allow sounds to be played at? Default: True
		void useMaximumDistance(bool maxDist = true) { m_useMaximumDistance = maxDist; }
	private:
		void m_updateGains() {
			auto err = alGetError();
			alListenerf(AL_GAIN, masterGain);
			musicSource->setGain(musicGain);
			for (auto src : curMenuSounds) {
				src->setGain(menuGain);
			}
			for (auto snd : curGameSounds) {
				snd.src->setGain(gameGain);
			}
		}
		std::unordered_map<std::string, ALuint> loadedGameSounds;
		std::unordered_map<std::string, ALuint> loadedMenuSounds;

		std::string m_musicPath = "";
		std::string m_menuSoundPath = "";
		std::string m_gameSoundPath = "";

		AudioBuffer gameSounds;
		AudioBuffer menuSounds;
		ALuint currentMusic;

		AudioSource* musicSource; //should always be on top of the listener
		//AudioSource* menuSource; //ditto - plays menu noises
		ALCcontext* context;
		ALCdevice* device;
		float masterGain = 1.f;
		float musicGain = 1.f;
		float gameGain = 1.f;
		float menuGain = 1.f;

		//for use in random pitches
		std::random_device rd;
		std::mt19937 randGen;

		std::function<AlVec3f(T)> m_positionFunc;
		std::function<AlVec3f(T)> m_velocityFunc;
		std::function<bool(T)> m_validityFunc;

		float m_maximumDistance = 1500.f;

		bool m_useMaximumDistance = true;
		bool m_randomPitchOnGameSounds = true;
};

#endif