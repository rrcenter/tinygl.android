
#pragma once

//#ifdef __cplusplus
//extern "C" {
//#endif

#include <jni.h>
#include <android/bitmap.h>

//#ifdef __cplusplus
//}
//#endif

// prototype
void swapAlphaChannel(unsigned int *pImageMemory, unsigned int numPixels);

class BitmapDC
{
public:
    BitmapDC();

    ~BitmapDC(void);

    // ARGB -> RGBA
    unsigned int swapAlpha(unsigned int value);

public:
    int m_nWidth;
    int m_nHeight;
    unsigned char *m_pData;
    JNIEnv *env;
};

BitmapDC& sharedBitmapDC();
//
//#pragma mark - JNIBitmap
//class JniBitmap
//{
//public:
//    uint32_t* _storedBitmapPixels;
//    AndroidBitmapInfo _bitmapInfo;
//    JniBitmap()
//    {
//        _storedBitmapPixels = NULL;
//    }
//};
//
//typedef struct
//{
//    uint8_t alpha, red, green, blue;
//} ARGB;
//
//int32_t convertArgbToInt(ARGB argb);
//
//void convertIntToArgb(uint32_t pixel, ARGB* argb);