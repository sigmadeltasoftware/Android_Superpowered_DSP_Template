#include "SuperpoweredRenderer.h"
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <jni.h>
#include <stdio.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <Vibrato-effect/BerVibrato/BerVibrato.h>

#define TAG "SuperPoweredRenderer"

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerA = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        playerA->setPosition(playerA->firstBeatMs, false, false);
    };
}

static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int __unused samplerate) {
	return ((SuperpoweredRenderer *)clientdata)->process(audioIO, (unsigned int)numberOfSamples);
}

SuperpoweredRenderer::SuperpoweredRenderer(unsigned int samplerate, unsigned int buffersize, const char *path, int fileLength) {
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    playerA = new SuperpoweredAdvancedAudioPlayer(&playerA , playerEventCallbackA, samplerate, 0);
    playerA->open(path, 0, fileLength);

    playerA->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);

    vibrato.initialize(44100);
    vibrato.setDepth(0.0f);
    vibrato.setFrequency(0);
}

SuperpoweredRenderer::~SuperpoweredRenderer() {
    delete audioSystem;
    delete playerA;
    free(stereoBuffer);
}

void SuperpoweredRenderer::onPlayPause(bool play) {
    if (!play) {
        playerA->pause();
    } else {
        playerA->play(false);
    };
    SuperpoweredCPU::setSustainedPerformanceMode(play); // <-- Important to prevent audio dropouts.
}

bool SuperpoweredRenderer::process(short int *output, unsigned int numberOfSamples) {
    bool silence = !playerA->process(stereoBuffer, false, numberOfSamples, 1.0f, 0.0f, -1.0f);

//    processDelay(stereoBuffer, numberOfSamples);
    for (int i = 0; i < numberOfSamples * 2; ++i) {
        stereoBuffer[i] = vibrato.processOneSample(stereoBuffer[i]);
    }

    // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    return !silence;
}

static SuperpoweredRenderer *renderer = NULL;

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_SuperpoweredExample(JNIEnv *javaEnvironment, jobject instance, jint samplerate,  jint buffersize, jstring apkPath, jint fileLength)
{
    const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);
    renderer = new SuperpoweredRenderer((unsigned int)samplerate, (unsigned int)buffersize, path, fileLength);
    javaEnvironment->ReleaseStringUTFChars(apkPath, path);
}

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_onPlayPause(JNIEnv *env, jobject instance, jboolean play)
{
    renderer->onPlayPause(play);
}

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_setVibratoDepthNative(JNIEnv *env, jobject instance, jfloat depth)
{
    renderer->vibrato.setDepth(depth);
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "setVibratoDepthNative(): %f", depth);
}

extern "C" JNIEXPORT void JNICALL Java_com_sigmadelta_superpowered_1dsp_1template_SuperPoweredPlayer_setVibrateRateNative(JNIEnv *env, jobject instance, jint rate)
{
    renderer->vibrato.setFrequency((float)rate);
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "setVibratoRateNative(): %f", (float)rate);
}