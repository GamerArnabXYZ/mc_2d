package com.mcclone.game;

import android.content.Context;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GameRenderer implements GLSurfaceView.Renderer {

    private Context context;
    private int screenWidth, screenHeight;
    private boolean initialized = false;

    public GameRenderer(Context context) {
        this.context = context;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        // Initialize game engine
        nativeInit(context.getAssets());
        initialized = true;
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        screenWidth = width;
        screenHeight = height;
        gl.glViewport(0, 0, width, height);
        nativeResize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if (initialized) {
            nativeRender();
        }
    }

    // Native methods
    public native void nativeInit(Object assetManager);
    public native void nativeResize(int width, int height);
    public native void nativeRender();
    public native void nativeSetInput(float joystickX, float joystickY,
                                       boolean jump, boolean attack);
}