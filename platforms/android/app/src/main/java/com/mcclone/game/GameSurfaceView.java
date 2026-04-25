package com.mcclone.game;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.SurfaceHolder;

public class GameSurfaceView extends GLSurfaceView {

    private GameRenderer renderer;

    public GameSurfaceView(Context context) {
        super(context);

        // OpenGL ES 2.0
        setEGLContextClientVersion(2);

        // Create renderer
        renderer = new GameRenderer(context);
        setRenderer(renderer);

        // Render continuously
        setRenderMode(RENDERMODE_CONTINUOUSLY);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        super.surfaceCreated(holder);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
    }

    public void resume() {
        onResume();
    }

    public void pause() {
        onPause();
    }
}