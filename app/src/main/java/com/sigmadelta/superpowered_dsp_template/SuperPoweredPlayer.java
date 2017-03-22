package com.sigmadelta.superpowered_dsp_template;


import android.content.Context;
import android.net.Uri;

import java.io.File;


class SuperPoweredPlayer {

    private static final String TAG = "SuperPoweredPlayer";
    private boolean _playState = false;

    SuperPoweredPlayer(Uri uri, Context ctx) {

        final int sampleRate = 44100;
        final int bufferSize = 512;

        final int fileLengthBytes = (int) new File(UriHelper.getPath(ctx, uri)).length();

        SuperpoweredRenderer(sampleRate, bufferSize, UriHelper.getPath(ctx, uri), fileLengthBytes);
    }

    void startPlayback() {
        _playState = true;
        onPlayPause(_playState);
    }

    void togglePlayback() {
        _playState = !_playState;
        onPlayPause(_playState);
    }

    private native void SuperpoweredRenderer(int samplerate, int buffersize, String filePath, int fileLengthBytes);
    private native void onPlayPause(boolean play);
    native void setVibratoDepth(float depth);
    native void setVibratoRate(int rate);
}
