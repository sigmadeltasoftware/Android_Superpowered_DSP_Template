package com.sigmadelta.superpowered_dsp_template;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private static final int FILECHOOSER_AUDIO_INTENT = 314;

    static{
        System.loadLibrary("superpoweredeffect");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        PermissionManager permMan = new PermissionManager(this);
        permMan.maybeRequestPermission(null, PermissionManager.Permissions.READ_EXTERNAL_STORAGE);

        final Button btnOpenFile = (Button) findViewById(R.id.btn_open_file);
        btnOpenFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(Intent.ACTION_GET_CONTENT, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI);
                i.setType("*/*");
                startActivityForResult(Intent.createChooser(i, "Select track"), FILECHOOSER_AUDIO_INTENT);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.e(TAG, "onActivityResult() - Req = " + requestCode + " || Res = " + resultCode + " || Data = " + data);

        // If a file has been selected through the filechooser-intent we launched with the 'Open File' button
        if (requestCode == FILECHOOSER_AUDIO_INTENT) {
            final SuperPoweredPlayer player = new SuperPoweredPlayer(data.getData(), this);
            player.startPlayback();

            findViewById(R.id.btn_play_pause).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    player.togglePlayback();
                }
            });

            ((SeekBar) findViewById(R.id.sb_vibrato_rate)).setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    player.setVibratoRate(progress);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });

            ((SeekBar) findViewById(R.id.sb_vibrato_decay)).setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    player.setVibratoDepth((float) progress/100);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        switch (requestCode) {
            case PermissionManager.READ_EXTERNAL_STORAGE_ID: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
                {
                    Log.d(TAG, "Permission READ_EXTERNAL_STORAGE granted!");
                } else {
                    Toast.makeText(this, "The app was not allowed to write to your storage. Hence, it cannot function properly. Please consider granting it this permission", Toast.LENGTH_LONG).show();
                    finish();
                }
            } break;
        }
    }
}
