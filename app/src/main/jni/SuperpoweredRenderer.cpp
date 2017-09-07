#include "SuperpoweredRenderer.h"
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <jni.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <malloc.h>

#define TAG "SuperPoweredRenderer"

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "File loaded succesfully!");
    };
}

static bool audioPlaybackProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int __unused samplerate) {
	return ((SuperpoweredRenderer *)clientdata)->process(audioIO, (unsigned int)numberOfSamples, false);
}

static bool audioRecordingProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int __unused samplerate) {
    return ((SuperpoweredRenderer *)clientdata)->process(audioIO, (unsigned int)numberOfSamples, true);
}

SuperpoweredRenderer::SuperpoweredRenderer(unsigned int samplerate, unsigned int buffersize, const char *path, int fileLength) {
    /*
     * According to the SuperpoweredAdvancedAudioPlayer::process method, the size of our buffer should be: numberOfSamples * 8 + 64 bytes big
     */
    stereoBufferOutput = (float *)memalign(16, (buffersize * 8) + 64);
    stereoBufferRecording = (float *)memalign(16, (buffersize * 8) + 64);
    mixerOutput = (float *)memalign(16, (buffersize * 8) + 64);

    audioPlayer = new SuperpoweredAdvancedAudioPlayer(&audioPlayer , playerEventCallbackA, samplerate, 2 * buffersize);
    audioPlayer->open(path, 0, fileLength);

    audioRecorder = new SuperpoweredRecorder("/sdcard/test.wav", samplerate);

    mixer = new SuperpoweredStereoMixer();

    audioRecordingSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, false, audioRecordingProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, 0);
    audioPlaybackSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioPlaybackProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, 0);
}

SuperpoweredRenderer::~SuperpoweredRenderer() {
    delete audioRecordingSystem;
    delete audioPlaybackSystem;
    delete audioPlayer;
    delete audioRecorder;
    delete mixer;
    free(stereoBufferOutput);
    free(stereoBufferRecording);
}

void SuperpoweredRenderer::onPlayPause(bool play) {
    if (!play) {
        audioRecorder->stop();
        audioPlayer->pause();
    } else {
        audioRecorder->start("/sdcard/test.wav");
        audioPlayer->play(false);
    };
    SuperpoweredCPU::setSustainedPerformanceMode(play); // <-- Important to prevent audio dropouts.
}

bool SuperpoweredRenderer::processRecording(short int *output, unsigned int numberOfSamples) {
//    isRecording = true;
//
//    while (isPlaying);
//    if (!isPlaying) {
        SuperpoweredShortIntToFloat(output, stereoBufferRecording, numberOfSamples);
//    }
//    isRecording = false;

    return true;
}

bool SuperpoweredRenderer::processPlayback(short int *output, unsigned int numberOfSamples) {
//    isPlaying = true;
//
//    while(isRecording);
//
//    if (!isRecording) {
        bool silence = !audioPlayer->process(stereoBufferOutput, false, numberOfSamples);
        SuperpoweredFloatToShortInt(stereoBufferOutput, output, numberOfSamples);

        // Mix signals
        mixerInputs[0] = stereoBufferRecording;
        mixerInputs[1] = stereoBufferOutput;
        mixerInputs[2] = NULL;
        mixerInputs[3] = NULL;

        mixerOutputs[0] = mixerOutput;
        mixerOutputs[1] = NULL;

        float inputLevels[] = {0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        float outputLevels[] = {1.0f, 1.0f};

        mixer->process(mixerInputs, mixerOutputs, inputLevels, outputLevels, NULL, NULL, numberOfSamples);

        audioRecorder->process(mixerOutput, NULL, numberOfSamples);
//    }
//    // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
//
//    isPlaying = false;
    return true;
}

bool SuperpoweredRenderer::process(short int *buffer, unsigned int numberOfSamples, bool isRecorded) {
    pthread_mutex_lock(mutex);
    if (isRecorded) {
        SuperpoweredShortIntToFloat(buffer, stereoBufferRecording, numberOfSamples);
    } else {
        bool silence = !audioPlayer->process(stereoBufferOutput, false, numberOfSamples);
        SuperpoweredFloatToShortInt(stereoBufferOutput, buffer, numberOfSamples);

        // Mix signals
        mixerInputs[0] = stereoBufferRecording;
        mixerInputs[1] = NULL;//stereoBufferOutput;
        mixerInputs[2] = NULL;
        mixerInputs[3] = NULL;

        mixerOutputs[0] = mixerOutput;
        mixerOutputs[1] = NULL;

        float inputLevels[] = {0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        float outputLevels[] = {1.0f, 1.0f};

        mixer->process(mixerInputs, mixerOutputs, inputLevels, outputLevels, NULL, NULL, numberOfSamples);

        audioRecorder->process(mixerOutput, NULL, numberOfSamples);
    }
    pthread_mutex_unlock(mutex);
    return true;
}

static SuperpoweredRenderer *renderer = NULL;

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_SuperpoweredRenderer(JNIEnv *javaEnvironment, jobject instance, jint samplerate,  jint buffersize, jstring filePath, jint fileLengthBytes)
{
    const char *path = javaEnvironment->GetStringUTFChars(filePath, JNI_FALSE);
    renderer = new SuperpoweredRenderer((unsigned int)samplerate, (unsigned int)buffersize, path, fileLengthBytes);
    javaEnvironment->ReleaseStringUTFChars(filePath, path);
}

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_onPlayPause(JNIEnv *env, jobject instance, jboolean play)
{
    renderer->onPlayPause(play);
}

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_setVibratoDepth(JNIEnv *env, jobject instance, jfloat depth)
{
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "setVibratoDepth(): %f", depth);
}

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_setVibratoRate(JNIEnv *env, jobject instance, jint rate)
{
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "setVibratoRate(): %f", (float)rate);
}