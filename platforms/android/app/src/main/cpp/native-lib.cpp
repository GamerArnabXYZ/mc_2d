#include <android/log.h>
#include <pthread.h>
#include <stdlib.h>

#define LOG_TAG "MCClone"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// External game functions (defined in C99 code)
extern void game_init_android(void);
extern void game_resize_android(int width, int height);
extern void game_render_android(void);
extern void game_set_input_android(float jx, float jy, int jump, int attack);

static int screen_width = 0;
static int screen_height = 0;

JNIEXPORT void JNICALL
Java_com_mcclone_game_GameRenderer_nativeInit(JNIEnv *env, jobject thiz, jobject asset_manager) {
    LOGI("Initializing MC Clone engine on Android");

    // Initialize the game
    game_init_android();

    LOGI("Game engine initialized successfully");
}

JNIEXPORT void JNICALL
Java_com_mcclone_game_GameRenderer_nativeResize(JNIEnv *env, jobject thiz,
                                                  jint width, jint height) {
    screen_width = width;
    screen_height = height;

    LOGI("Screen resized to %dx%d", width, height);

    game_resize_android(width, height);
}

JNIEXPORT void JNICALL
Java_com_mcclone_game_GameRenderer_nativeRender(JNIEnv *env, jobject thiz) {
    game_render_android();
}

JNIEXPORT void JNICALL
Java_com_mcclone_game_GameRenderer_nativeSetInput(JNIEnv *env, jobject thiz,
                                                    jfloat jx, jfloat jy,
                                                    jboolean jump, jboolean attack) {
    game_set_input_android(jx, jy, jump ? 1 : 0, attack ? 1 : 0);
}

JNIEXPORT void JNICALL
Java_com_mcclone_game_MainActivity_nativeTouchStart(JNIEnv *env, jobject thiz,
                                                     jint touch_id, jfloat x, jfloat y) {
    // Forward touch to input system
    LOGI("Touch start: id=%d x=%.1f y=%.1f", touch_id, x, y);
}

JNIEXPORT void JNICALL
Java_com_mcclone_game_MainActivity_nativeTouchMove(JNIEnv *env, jobject thiz,
                                                    jint touch_id, jfloat x, jfloat y) {
    LOGI("Touch move: id=%d x=%.1f y=%.1f", touch_id, x, y);
}

JNIEXPORT void JNICALL
Java_com_mcclone_game_MainActivity_nativeTouchEnd(JNIEnv *env, jobject thiz,
                                                   jint touch_id) {
    LOGI("Touch end: id=%d", touch_id);
}