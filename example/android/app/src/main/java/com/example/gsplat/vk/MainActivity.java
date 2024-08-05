package com.example.gsplat.vk;

import com.google.androidgamesdk.GameActivity;
import android.os.Bundle;

public class MainActivity extends GameActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    static {
        System.loadLibrary("vkrenderer");
    }
}
