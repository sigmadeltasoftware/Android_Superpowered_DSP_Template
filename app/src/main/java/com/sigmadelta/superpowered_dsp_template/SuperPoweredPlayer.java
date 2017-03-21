package com.sigmadelta.superpowered_dsp_template;


import android.content.Context;
import android.net.Uri;
import android.util.Log;

import java.io.File;


class SuperPoweredPlayer {

    private static final String TAG = "SuperPoweredPlayer";
    private Uri _uri;
    private Context _ctx;
    private boolean _playState = false;

    SuperPoweredPlayer(Uri uri, Context ctx) {
        _uri = uri;
        _ctx = ctx;

        final int sampleRate = 44100;
        final int bufferSize = 512;

        File file = new File(UriHelper.getPath(_ctx, _uri));
        int fileAlength = (int) file.length();

        SuperpoweredExample(sampleRate, bufferSize, UriHelper.getPath(_ctx, _uri), fileAlength);
    }

    void startPlayback() {
        _playState = true;
        onPlayPause(_playState);
    }

    void togglePlayback() {
        _playState = !_playState;
        onPlayPause(_playState);
    }

    void setVibratoDepth(float depth) {
        setVibratoDepthNative(depth);
    }

    void setVibratoRate(int rate) {
        setVibrateRateNative(rate);
    }

    private native void SuperpoweredExample(int samplerate, int buffersize, String apkPath, int fileAlength);
    private native void onPlayPause(boolean play);
    private native void setVibratoDepthNative(float depth);
    private native void setVibrateRateNative(int rate);
}
