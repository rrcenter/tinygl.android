
#include "kkImage.h"
#include <android/log.h>
#include <string.h>


// prototype
void swapAlphaChannel(unsigned int *pImageMemory, unsigned int numPixels);

BitmapDC::BitmapDC()
        : m_pData(NULL)
        , m_nWidth(0)
        , m_nHeight(0)
{
}

BitmapDC::~BitmapDC(void)
{
    if (m_pData)
    {
        delete [] m_pData;
    }
}

// ARGB -> RGBA
unsigned int BitmapDC::swapAlpha(unsigned int value)
{
    return ((value << 8 & 0xffffff00) | (value >> 24 & 0x000000ff));
}

BitmapDC& sharedBitmapDC()
{
    static BitmapDC s_BmpDC;
    return s_BmpDC;
}

// swap the alpha channel in an 32 bit image (from ARGB to RGBA)
void swapAlphaChannel(unsigned int *pImageMemory, unsigned int numPixels)
{
    for(int c = 0; c < numPixels; ++c, ++pImageMemory)
    {
        // copy the current pixel
        unsigned int currenPixel =  (*pImageMemory);
        // swap channels and store back
        unsigned char *pSource = (unsigned char *) 	&currenPixel;
        *pImageMemory = (pSource[0] << 24) | (pSource[3]<<16) | (pSource[2]<<8) | pSource[1];
    }
}

//int32_t convertArgbToInt(ARGB argb)
//{
//    return (argb.alpha) | (argb.red << 24) | (argb.green << 16)
//           | (argb.blue << 8);
//}
//
//void convertIntToArgb(uint32_t pixel, ARGB* argb)
//{
//    argb->red = ((pixel >> 24) & 0xff);
//    argb->green = ((pixel >> 16) & 0xff);
//    argb->blue = ((pixel >> 8) & 0xff);
//    argb->alpha = (pixel & 0xff);
//}