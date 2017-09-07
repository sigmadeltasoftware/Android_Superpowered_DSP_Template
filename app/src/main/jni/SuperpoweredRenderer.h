#ifndef Header_SuperpoweredExample
#define Header_SuperpoweredExample

#include <math.h>
#include <pthread.h>

#include "SuperpoweredRenderer.h"
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <AndroidIO/SuperpoweredAndroidAudioIO.h>
#include <Vibrato-effect/BerVibrato/BerVibrato.h>
#include <Superpowered/SuperpoweredRecorder.h>
#include <Superpowered/SuperpoweredMixer.h>


class SuperpoweredRenderer {
public:

	SuperpoweredRenderer(unsigned int samplerate, unsigned int buffersize, const char *path, int fileLength);
	~SuperpoweredRenderer();

	bool processRecording(short int *output, unsigned int numberOfSamples);
	bool processPlayback(short int *output, unsigned int numberOfSamples);
	void onPlayPause(bool play);

private:
    SuperpoweredAndroidAudioIO *audioRecordingSystem;
    SuperpoweredAndroidAudioIO *audioPlaybackSystem;
    SuperpoweredAdvancedAudioPlayer *audioPlayer;
    SuperpoweredRecorder *audioRecorder;
	SuperpoweredStereoMixer *mixer;

	float *mixerInputs[4];
	float *mixerOutputs[2];
    float *stereoBufferOutput;
    float *stereoBufferRecording;
    float *mixerOutput;

    bool isRecording = false;
    bool isPlaying = true;
};

#endif
