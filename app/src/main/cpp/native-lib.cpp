#include <jni.h>
//extern "C" {
#include <android/bitmap.h>
//}
#include <string>

#include "tinygl.hpp"
#include "kk_log.h"
#include "kkImage.h"
#include "gl_hello.h"
#include "gl_gears.h"
#include "gl_texture.h"
#include "gl_menu.h"
#include "gl_model.h"
#include "gl_touch_event.h"
#include "renderer_fixed.h"
#include "gl_hello2.h" // glGenLists demo

#include "kkFileUtils.h"

#include "../include-demo/openimgui.h"

extern int winSizeX;
extern int winSizeY;
extern ZBuffer *frameBuffer;

typedef void (*gl_function_t)(void);

gl_function_t gl_init = NULL;
gl_function_t gl_init_scene = NULL;
gl_function_t gl_draw = NULL;

void draw_touch_rect()
{   // draw mouse
    float w = 20.0 / winSizeX;
    float h = 20.0 / winSizeY;

    glColor3f(1.0, 1.0, 0.0);
    float touch_x = omg_cursorpos[0];
    float touch_y = omg_cursorpos[1];
    drawBox(touch_x - w * 0.5, touch_y - h * 0.5, w, h);
}

static void register_gl_func()
{
    // draw triangle
    gl_init = hello_initGL;
    gl_init_scene = hello_initScene;
    gl_draw = hello_draw;

    // draw gears
    gl_init = gears_initGL;
    gl_init_scene = gears_initScene;
    gl_draw = gears_draw;

    // draw texture
    gl_init = texture_initGL;
    gl_init_scene = texture_initScene;
    gl_draw = texture_draw;

    // draw menu
    gl_init = menu_initGL;
    gl_init_scene = menu_initScene;
    gl_draw = menu_draw;

    // draw model
    gl_init = model_initGL;
    gl_init_scene = model_initScene;
    gl_draw = model_draw;

    //
    gl_init = hello2_initGL;
    gl_init_scene = hello2_initScene;
    gl_draw = hello2_draw;

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_androidtinygl_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    const GLubyte* ret = glGetString(GL_VERSION);

    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    std::string hello = "Hello from C++";
    hello.assign((const char*)ret);



    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_initGL(
        JNIEnv* env,
        jclass clazz,
        jint width,
        jint height
        )
{
    register_gl_func();

    LOGI("%s\n", __FUNCTION__ );
//    winSizeX = width;
//    winSizeY = height;

    BitmapDC &dc = sharedBitmapDC();
    int count = winSizeX * winSizeY * sizeof(uint32_t);
    dc.m_nWidth = winSizeX;
    dc.m_nHeight = winSizeY;
    dc.m_pData = new unsigned char[count];

    //
    gl_init();
    gl_init_scene();
}


extern "C" JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_drawGL(
        JNIEnv* env,
        jclass clazz)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gl_draw();

    // draw main menu and mouse
    Set2DF();

    if (omg_textbox(0.01, 0.9, "main menu", 24, 1, 0.0, 0.0, 0x00FF00, 0) && omg_cb == 2)
    {
        LOGE("main menu");
    }
    draw_touch_rect();

    BitmapDC &dc = sharedBitmapDC();
    ZB_copyFrameBuffer(frameBuffer, dc.m_pData, frameBuffer->linesize);
//    ZB_copyFrameBuffer(frameBuffer, screen->pixels, screen->pitch);


}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_nativeSetBitmap(
        JNIEnv *env,
        jclass clazz,
        jobject bitmap
        )
{

    // TODO: implement nativeSetBitmap()
    AndroidBitmapInfo bitmapInfo;
    uint32_t* storedBitmapPixels = NULL;
    //LOGD("reading bitmap info...");
    int ret;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo)) < 0)
    {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return ;
    }

    BitmapDC& bitmapDc = sharedBitmapDC();
//    bitmapDc.m_nWidth = bitmapInfo.width;
//    bitmapDc.m_nHeight = bitmapInfo.height;
    bitmapDc.m_nWidth = winSizeX;
    bitmapDc.m_nHeight = winSizeY;
    LOGE("width:%d height:%d stride:%d", bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride);
    if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
    {
        LOGE("Bitmap format is not RGBA_8888!");
        return ;
    }
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_androidtinygl_MainActivity_nativeGetBitmapFromBitmapData(JNIEnv *env, jclass clazz)
{
    BitmapDC& dc = sharedBitmapDC();
    auto height = dc.m_nHeight;
    auto width = dc.m_nWidth;
    unsigned int count = width * height;
    unsigned int pixelsCount = count * sizeof(uint32_t);
    //
    //creating a new bitmap to put the pixels into it - using Bitmap Bitmap.createBitmap (int width, int height, Bitmap.Config config) :
    //
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapFunction = env->GetStaticMethodID(bitmapCls,
                                                            "createBitmap",
                                                            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(
            bitmapConfigClass, "valueOf",
            "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass,
                                                       valueOfBitmapConfigFunction, configName);
    jobject newBitmap = env->CallStaticObjectMethod(bitmapCls,
                                                    createBitmapFunction,
                                                    width,
                                                    height,
                                                    bitmapConfig);
    //
    // putting the pixels into the new bitmap:
    //
    int ret;
    void* bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, newBitmap, &bitmapPixels)) < 0)
    {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }

    uint32_t* newBitmapPixels = (uint32_t*) bitmapPixels;
    unsigned int *tempPtr = (unsigned int*)dc.m_pData;
    unsigned int tempdata = 0;

    memcpy(newBitmapPixels, dc.m_pData, pixelsCount);
    // convert rgba -> argb for java
    for (unsigned int i = 0; i < count; i++)
    {
        tempdata = *tempPtr;
        *tempPtr++ = dc.swapAlpha(tempdata);
    }
    AndroidBitmap_unlockPixels(env, newBitmap);
    return newBitmap;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_nativeSyncBitmapData(JNIEnv *env, jclass clazz,
                                                                 jobject bitmap) {
    // TODO: implement nativeSyncBitmapData()

    BitmapDC& dc = sharedBitmapDC();
    auto height = dc.m_nHeight;
    auto width = dc.m_nWidth;
    unsigned int count = width * height;
    unsigned int pixelsCount = count * sizeof(uint32_t);

    //
    // putting the pixels into the new bitmap:
    //
    int ret;
    void* bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0)
    {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return ;
    }
    uint32_t* newBitmapPixels = (uint32_t*) bitmapPixels;
    unsigned int *tempPtr = (unsigned int*)dc.m_pData;
    unsigned int tempdata = 0;

    memcpy(newBitmapPixels, dc.m_pData, pixelsCount);
    // convert rgba -> argb for java
    for (unsigned int i = 0; i < count; i++)
    {
        tempdata = *tempPtr;
        *tempPtr++ = dc.swapAlpha(tempdata);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_nativeSetContext(JNIEnv *env, jclass clazz,
                                                             jobject p_context,
                                                             jobject p_asset_manager) {
    // TODO: implement nativeSetContext()

    kkFileUtils::setAssetManager(AAssetManager_fromJava(env, p_asset_manager));
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_nativeTouchesBegin(JNIEnv *env, jclass clazz,
                                                               jfloat p_x, jfloat p_y) {
    // TODO: implement nativeTouchesBegin()

    gl_set_touchevent(kTouchBegin, p_x, p_y);
    omg_update_mcursor(p_x, p_y, 1);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_nativeTouchesEnd(JNIEnv *env, jclass clazz, jfloat p_x,
                                                             jfloat p_y) {
    // TODO: implement nativeTouchesEnd()
    gl_set_touchevent(kTouchEnd, p_x, p_y);
    omg_update_mcursor(p_x, p_y, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_androidtinygl_MainActivity_nativeTouchesMove(JNIEnv *env, jclass clazz,
                                                              jfloat p_xs, jfloat p_ys) {
    // TODO: implement nativeTouchesMove()
    gl_set_touchevent(kTouchMove, p_xs, p_ys);

    omg_update_mcursor(p_xs, p_ys, 1);
}