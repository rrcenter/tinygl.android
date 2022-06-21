
#include "gl_texture.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "kkFileUtils.h"
#include "kk_log.h"

#include "../include/GL/gl.h"

#include "../include/zbuffer.h"
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "../include-demo/3dMath.h"

typedef unsigned char uchar;

#define STB_IMAGE_IMPLEMENTATION
#include "../include-demo/stb_image.h"
int doPostProcess = 0;

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265
#endif

extern ZBuffer *frameBuffer;
extern int winSizeX, winSizeY;

static GLuint tex = 0;
GLuint tex2 = 0;
GLuint do1D = 0;
GLint Row1D = 30;
GLint dorect = 0;
GLfloat texture_mult = 1.0;
GLuint loadRGBTexture(unsigned char* buf, unsigned int w, unsigned int h) {
    GLuint t = 0;
    glGenTextures(1, &t);
    // for(unsigned int i = 0; i < w * h; i++)
    // {
    // unsigned char t = 0;
    // unsigned char* r = buf + i*3;
    // // unsigned char* g = buf + i*3+1;
    // unsigned char* b = buf + i*3+2;
    // t = *r;
    // *r = *b;
    // *b = t;
    // }
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if (!do1D)
        glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
    else
        glTexImage1D(GL_TEXTURE_1D, 0, 3, w, 0, GL_RGB, GL_UNSIGNED_BYTE, buf + Row1D * w * 3);
    return t;
}

GLuint postProcessingStep(GLint x, GLint y, GLuint pixel, GLushort z) {
#if TGL_FEATURE_RENDER_BITS == 32
    // 32 bit pixel
    return pixel & 0x8F8F8F; // Half color mode.
#else
    // 16 bit mode
	return pixel & (63 << 5); // Solid green
#endif
}

unsigned int frames_notreset = 0;
void texture_draw()
{
    frames_notreset++;
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    if (dorect)
        glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    // TRIANGLE 1,
    glTexCoord2f(0, 0);
    glVertex3f(-1, -1, 0.5);

    glTexCoord2f(1 * texture_mult, -1 * texture_mult);
    glVertex3f(1, 1, 0.5);

    glTexCoord2f(0, -1 * texture_mult);
    glVertex3f(-1, 1, 0.5);
    // TRIANGLE 2
    glTexCoord2f(0, 0);
    glVertex3f(-1, -1, 0.5);

    glTexCoord2f(1 * texture_mult, 0);
    glVertex3f(1, -1, 0.5);

    glTexCoord2f(1 * texture_mult, -1 * texture_mult);
    glVertex3f(1, 1, 0.5);
    glEnd();
    if (dorect) {
        // glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0, 0, 0);
        glRectf(0, 0, 1, 1);
        // glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);
    }
    if (doPostProcess)
        glPostProcess(postProcessingStep); // do a post processing step on the rendered geometry.

    {
        glDrawText((unsigned char*)"\nBlitting text\nto the screen!", 0, 0, 0x000000FF);
        glPixelZoom(2.0, 0.7);
        glRasterPos3f(0, -1, fabs(sinf(frames_notreset / 100.0)));
        {
            GLint xsize, ysize;
            void* data = glGetTexturePixmap(tex2, 0, &xsize, &ysize);
            // make noise
//             for(int i = 0; i < winSizeX; i++)
//             for(int j = 0; j < winSizeY; j++){
//             glPlotPixel(i,j,rand() & 0xFFFFFF);
//            }
            if (data)
                glDrawPixels(xsize, ysize, GL_RGB, (TGL_FEATURE_RENDER_BITS == 32) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, data);
        }
    }
}
void texture_initGL()
{
    int winSizeX = 640;
    int winSizeY = 480;
    frameBuffer = ZB_open(winSizeX, winSizeY, ZB_MODE_RGBA, 0);
    glInit(frameBuffer);

    // initialize GL:
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glViewport(0, 0, winSizeX, winSizeY);
    glEnable(GL_DEPTH_TEST);
    // GLfloat  h = (GLfloat) winSizeY / (GLfloat) winSizeX;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glFrustum( -1.0, 1.0, -h, h, 5.0, 60.0 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // glTranslatef( 0.0, 0.0, -45.0 );
}

void texture_initScene()
{
    glEnable(GL_CULL_FACE);
    // glDisable( GL_CULL_FACE );
    glDisable(GL_BLEND);
    // glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    // glEnable( GL_LIGHT0 );
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glTextSize(GL_TEXT_SIZE24x24);
    {
        int sw = 0, sh = 0, sc = 0; // sc goes unused.
        std::string filedata = kkFileUtils::sharedFileUtils()->getFileData("texture.png", "rb");
        uchar* source_data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(filedata.c_str()), filedata.length(), &sw, &sh, &sc, 3);
//        uchar* source_data = stbi_load("texture.png", &sw, &sh, &sc, 3);
        if (source_data) {
            tex = loadRGBTexture(source_data, sw, sh);
            free(source_data);
        } else {
            LOGE("\nCan't load texture!\n");
        }
    }
    {
        int sw = 0, sh = 0, sc = 0; // sc goes unused.
        std::string filedata = kkFileUtils::sharedFileUtils()->getFileData("tex.jpg", "rb");
        uchar* source_data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(filedata.c_str()), filedata.length(), &sw, &sh, &sc, 3);
//        uchar* source_data = stbi_load("tex.jpg", &sw, &sh, &sc, 3);
        if (source_data) {
            tex2 = loadRGBTexture(source_data, sw, sh);
            free(source_data);
        } else {
            LOGE("\nCan't load texture!\n");
        }
    }
    glEnable(GL_NORMALIZE);
}
