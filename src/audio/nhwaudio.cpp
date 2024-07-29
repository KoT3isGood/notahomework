#include "nhwaudio.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "../nhw.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#include "stdlib.h"

ALCdevice* audiodevice;
ALCcontext* context;
void InitAudio()
{
    // Initialize OpenAL
    audiodevice = alcOpenDevice(nullptr); // Open default device
    if (!audiodevice) {
        Mayday("Failed to open audio device");
    }

    context = alcCreateContext(audiodevice, nullptr);
    if (!context) {
        alcCloseDevice(audiodevice);
        Mayday("Failed to create audio context");
    }

    alcMakeContextCurrent(context);
}

void DeinitAudio()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(audiodevice);
}

ALuint loadOgg(const char* filename) {
    int error;
    stb_vorbis* vorbis = stb_vorbis_open_filename(filename, &error, nullptr);

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);
    int numChannels = info.channels;
    int sampleRate = info.sample_rate;
    int numSamples = stb_vorbis_stream_length_in_samples(vorbis) * numChannels;

    short* samples = (short*)malloc(numSamples*sizeof(short));
    stb_vorbis_get_samples_short_interleaved(vorbis, numChannels, samples, numSamples);
    stb_vorbis_close(vorbis);

    ALenum format = (numChannels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, samples, numSamples * sizeof(short), sampleRate);
    free(samples);

    return buffer;
}

void* CreateSound(const char* sound)
{
    ALuint* buffer = new ALuint(loadOgg(sound));
    return buffer;
}

void* PlaySound(void* sound)
{
    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, *(ALuint*)sound);

    // Play the sound
    alSourcePlay(source);
    ALuint* sourceout = new ALuint(source);
    return sourceout;
}
void SetSoundPosition(void* source, void* pos)
{
    alSourcefv(*(ALuint*)source, AL_POSITION, (ALfloat*)pos);
}
void DestroySource(void* source)
{
    alDeleteSources(1, (ALuint*)source);
}
void DestroySound(void* sound)
{
    alDeleteBuffers(1, (ALuint*)sound);
    delete sound;
}
