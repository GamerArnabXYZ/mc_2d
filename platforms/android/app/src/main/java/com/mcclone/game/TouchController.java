package com.mcclone.game;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.view.MotionEvent;
import android.view.View;

public class TouchController extends View {

    private Paint paint;
    private float joystickX, joystickY;
    private boolean jumpPressed, attackPressed;
    private static final float JOYSTICK_SIZE = 100f;
    private static final float BUTTON_SIZE = 60f;

    // Touch zones
    private float leftJoystickX, leftJoystickY;
    private float jumpButtonX, jumpButtonY;
    private float attackButtonX, attackButtonY;
    private int leftTouchId = -1;

    public TouchController(Context context, GameSurfaceView gameView) {
        super(context);
        paint = new Paint();
        paint.setAntiAlias(true);

        // Position buttons
        post(() -> {
            int w = getWidth();
            int h = getHeight();
            if (w == 0) w = 1080;
            if (h == 0) h = 1920;

            leftJoystickX = w * 0.2f;
            leftJoystickY = h * 0.75f;
            jumpButtonX = w * 0.8f;
            jumpButtonY = h * 0.75f;
            attackButtonX = w * 0.85f;
            attackButtonY = h * 0.55f;
        });
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        // Draw semi-transparent joystick base
        paint.setAlpha(80);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(0xFF555555);
        canvas.drawCircle(leftJoystickX, leftJoystickY, JOYSTICK_SIZE, paint);

        // Draw joystick thumb
        paint.setColor(0xFF888888);
        canvas.drawCircle(leftJoystickX + joystickX * 50,
                          leftJoystickY + joystickY * 50,
                          JOYSTICK_SIZE * 0.5f, paint);

        // Draw jump button
        paint.setColor(jumpPressed ? 0xFF00AA00 : 0xFF006600);
        canvas.drawCircle(jumpButtonX, jumpButtonY, BUTTON_SIZE, paint);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(30);
        paint.setTextAlign(Paint.Align.CENTER);
        canvas.drawText("↑", jumpButtonX, jumpButtonY + 10, paint);

        // Draw attack button
        paint.setColor(attackPressed ? 0xFFAA0000 : 0xFF660000);
        canvas.drawCircle(attackButtonX, attackButtonY, BUTTON_SIZE, paint);
        canvas.drawText("⚔", attackButtonX, attackButtonY + 10, paint);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getActionMasked();
        int index = event.getActionIndex();
        float x = event.getX(index);
        float y = event.getY(index);
        int pointerId = event.getPointerId(index);

        switch (action) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                // Check left side for joystick
                if (x < getWidth() * 0.4f) {
                    leftTouchId = pointerId;
                    joystickX = (x - leftJoystickX) / 50f;
                    joystickY = (y - leftJoystickY) / 50f;
                    ((MainActivity)getContext()).nativeTouchStart(pointerId, x, y);
                }
                // Check right side for buttons
                else if (x > getWidth() * 0.6f) {
                    if (y > getHeight() * 0.65f) {
                        jumpPressed = true;
                    } else if (y > getHeight() * 0.45f) {
                        attackPressed = true;
                    }
                }
                invalidate();
                return true;

            case MotionEvent.ACTION_MOVE:
                if (pointerId == leftTouchId) {
                    joystickX = (x - leftJoystickX) / 50f;
                    joystickY = (y - leftJoystickY) / 50f;
                    ((MainActivity)getContext()).nativeTouchMove(pointerId, x, y);
                    invalidate();
                }
                return true;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                if (pointerId == leftTouchId) {
                    leftTouchId = -1;
                    joystickX = 0;
                    joystickY = 0;
                }
                jumpPressed = false;
                attackPressed = false;
                ((MainActivity)getContext()).nativeTouchEnd(pointerId);
                invalidate();
                return true;
        }

        return super.onTouchEvent(event);
    }

    public float getJoystickX() { return joystickX; }
    public float getJoystickY() { return joystickY; }
    public boolean isJumpPressed() { return jumpPressed; }
    public boolean isAttackPressed() { return attackPressed; }
}