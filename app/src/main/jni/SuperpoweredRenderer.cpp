#include "SuperpoweredRenderer.h"
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <jni.h>
#include <android/log.h>
#include <malloc.h>

#define TAG "SuperPoweredRenderer"

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "File loaded succesfully!");
    };
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

    audioPlayer = new SuperpoweredAdvancedAudioPlayer(&audioPlayer , playerEventCallbackA, samplerate, 2 * buffersize);
    audioPlayer->open(path, 0, fileLength);

    audioRecorder = new SuperpoweredRecorder("/sdcard/test.wav", samplerate);

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, true, audioRecordingProcessing, this);
}

SuperpoweredRenderer::~SuperpoweredRenderer() {
    delete audioSystem;
    delete audioPlayer;
    delete audioRecorder;
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

bool SuperpoweredRenderer::process(short int *buffer, unsigned int numberOfSamples, bool isRecorded) {
    SuperpoweredShortIntToFloat(buffer, stereoBufferRecording, numberOfSamples);
    bool silence = !audioPlayer->process(stereoBufferRecording, true, numberOfSamples);
    audioRecorder->process(stereoBufferRecording, NULL, numberOfSamples);
    SuperpoweredFloatToShortInt(stereoBufferRecording, buffer, numberOfSamples);

    return silence;
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
