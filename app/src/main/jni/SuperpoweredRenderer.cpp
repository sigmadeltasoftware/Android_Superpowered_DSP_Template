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

static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int __unused samplerate) {
	return ((SuperpoweredRenderer *)clientdata)->process(audioIO, (unsigned int)numberOfSamples);
}

SuperpoweredRenderer::SuperpoweredRenderer(unsigned int samplerate, unsigned int buffersize, const char *path, int fileLength) {
    /*
     * According to the SuperpoweredAdvancedAudioPlayer::process method, the size of our buffer should be: numberOfSamples * 8 + 64 bytes big
     */
    stereoBuffer = (float *)memalign(16, (buffersize * 8) + 64);

    audioPlayer = new SuperpoweredAdvancedAudioPlayer(&audioPlayer , playerEventCallbackA, samplerate, 0);
    audioPlayer->open(path, 0, fileLength);

    audioRecorder = new SuperpoweredRecorder("/sdcard/test.wav", samplerate);
    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, SL_ANDROID_RECORDING_PRESET_GENERIC, SL_ANDROID_STREAM_MEDIA, 0);
}

SuperpoweredRenderer::~SuperpoweredRenderer() {
    delete audioSystem;
    delete audioPlayer;
    delete audioRecorder;
    free(stereoBuffer);
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

bool SuperpoweredRenderer::process(short int *output, unsigned int numberOfSamples) {
    bool silence = !audioPlayer->process(stereoBuffer, false, numberOfSamples);
    audioRecorder->process(stereoBuffer, NULL, numberOfSamples);
    if (!silence) {
        /*****************************
        *  APPLY PROCESSING BELOW
        */

        /*****************************
         *  APPLY PROCESSING ABOVE
         */

        // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
        SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    }

    return !silence;
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