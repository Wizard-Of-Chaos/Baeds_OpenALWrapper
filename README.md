# OpenALWrapper

This is an [OpenAL](https://www.openal.org/) wrapper meant for use with game environments in C++17 to make using OpenAL a little easier.

This project uses the [OpenAL sources](https://www.openal.org/downloads/), the [OggVorbis sources](https://xiph.org/), and a chunk of code from a random [Github gist](https://gist.github.com/tilkinsc/f91d2a74cff62cc3760a7c9291290b29) to function.

Notes:

-Supports .OGG files only

-Only supports *one* music track playing at any given time

-Amount of valid sources is limited by OpenAL and your hardware

-Requires definitions from your code for functions to determine position, velocity, and validity of any given entity

-Requires the OpenAL32.dll

Example entity functions:
```cpp
struct dummyEntity
{
	float velocity[3];
	float position[3];
	bool alive = true;
};

AlVec3f dummyVel(dummyEntity ent)
{
	return AlVec3f(ent.velocity[0], ent.velocity[1], ent.velocity[2]);
}

AlVec3f dummyPos(dummyEntity ent)
{
	return AlVec3f(ent.position[0], ent.position[1], ent.position[2]);
}

bool dummyValid(dummyEntity ent)
{
	return ent.alive;
}
```

Example initialization with these functions:
```cpp
	AudioDriver<dummyEntity> driver(dummyVel, dummyPos, dummyValid);
    driver.playMenuSound("impact_1.ogg");
```

## Use
Include AudioDriver.h for the entire library.

In your main game loop, you should be calling setListenerPosition, gameSoundUpdate, and menuSoundUpdate to make sure that the audio sources move with their entities.
