#include "gl_menu.h"

/* OPENIMGUI STANDARD DEMO

Demo of Gek's proposed Open Immediate Mode Gui Standard



*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl_touch_event.h"

#include "../include/GL/gl.h"

#include "../include/zbuffer.h"
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "../include-demo/3dMath.h"

typedef unsigned char uchar;

// Gek's OpenIMGUI standard.
#define OPENIMGUI_IMPL
#include "../include-demo/openimgui.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif


extern int winSizeX;
extern int winSizeY;
extern ZBuffer* frameBuffer;

double tpassed = 0;
int isRunning = 1;
int dirbstates[4] = {0, 0, 0, 0}; // up,down,left,right
int mousepos[2] = {0, 0};
int using_cursorkeys = 0; // Switches to cursor keys upon pressing a key.
int mb = 0;				  // cursor button
int mb2 = 0;			  // cursor second button.

#define BEGIN_EVENT_HANDLER                                                                                                                                    \
	void events(SDL_Event* e) {                                                                                                                                \
		switch (e->type) {
#define E_KEYSYM e->key.keysym.sym

#define END_EVENT_HANDLER                                                                                                                                      \
	}                                                                                                                                                          \
	}
#define EVENT_HANDLER events
#define E_MOTION e->motion
#define E_BUTTON e->button.button
#define E_WINEVENT e->window.event
#define E_WINW e->window.data1
#define E_WINH e->window.data2

vec3 mouse_to_normal(int mx, int my) {
    vec3 r;
    r.d[0] = mx / (float)winSizeX;
    r.d[1] = my / (float)winSizeY;
    return r;
}

void drawBox(GLfloat x, GLfloat y, GLfloat xdim, GLfloat ydim) { // 0,0 is top left, 1,1 is bottom right

    x *= 2;
    xdim *= 2;
    y *= 2;
    ydim *= 2;
    glBegin(GL_TRIANGLES);
    // TRIANGLE 1,
    glTexCoord2f(0, 0);
    glVertex3f(-1 + x, 1 - y - ydim, 0.5); // Bottom Left Corner

    glTexCoord2f(1, -1);
    glVertex3f(-1 + x + xdim, 1 - y, 0.5); // Top Right Corner

    glTexCoord2f(0, -1);
    glVertex3f(-1 + x, 1 - y, 0.5); // Top Left
    // TRIANGLE 2
    glTexCoord2f(0, 0);
    glVertex3f(-1 + x, 1 - y - ydim, 0.5); // Bottom Left Corner

    glTexCoord2f(1, 0);
    glVertex3f(-1 + x + xdim, 1 - y - ydim, 0.5);

    glTexCoord2f(1, -1);
    glVertex3f(-1 + x + xdim, 1 - y, 0.5); // Top Right Corner
    glEnd();
    return;
}

void drawMouse() {
    if (!omg_cb)
        glColor3f(0.7, 0.7, 0.7);
    else
        glColor3f(1.0, 0.1, 0.1);
    // if(!using_cursorkeys)
    drawBox(omg_cursorpos[0], omg_cursorpos[1], 0.03, 0.03);
    // else
    //	drawBox(omg_cursorpos_presuck[0],omg_cursorpos_presuck[1], 0.03, 0.03);
}

void drawTB(const char* text, GLuint textcolor, GLfloat x, GLfloat y, GLint size, float* tw, float* th) {
    size = (size > 64) ? 64 : ((size < 8) ? 8 : size);
    size >>= 3; // divide by 8 to get the GLTEXTSIZE
    if (!size || !text)
        return;
    int mw = 0, h = 1, cw = 0; // max width, height, current width
    for (int i = 0; text[i] != '\0' && (text[i] & 127); i++) {
        if (text[i] != '\n')
            cw++;
        else {
            cw = 0;
            h++;
        }
        if (mw < cw)
            mw = cw;
    }
    float w = (size)*8 * (mw) / (float)winSizeX;
    float bw = 3 * size / (float)winSizeX;
    float h_ = (size)*8 * (h) / (float)winSizeY;
    float bh = 3 * size / (float)winSizeY;
    drawBox(x, y, w, h_);
    *tw = w + bw;
    *th = h_ + bh;
    glTextSize((GLTEXTSIZE)size);
    glDrawText((unsigned char*)text, x * winSizeX, y * winSizeY, textcolor);
    return;
}

int omg_box(float x, float y, float xdim, float ydim, int sucks, float buttonjumpx, float buttonjumpy, int hints) {
    // hints is the color of the box.
    float r = ((hints & 0xFF0000) >> 16) / 255.0;
    float g = ((hints & 0xFF00) >> 8) / 255.0;
    float b = ((hints & 0xFF)) / 255.0;
    glColor3f(r, g, b);
    drawBox(x, y, xdim, ydim);
    omg_box_suck(x, y, xdim, ydim, sucks, buttonjumpx, buttonjumpy);
    return omg_box_retval(x, y, xdim, ydim);
}

int omg_textbox(float x, float y, const char* text, int textsize, int sucks, float buttonjumpx, float buttonjumpy, int hints, int hintstext) {
    float r = ((hints & 0xFF0000) >> 16) / 255.0;
    float g = ((hints & 0xFF00) >> 8) / 255.0;
    float b = ((hints & 0xFF)) / 255.0;
    glColor3f(r, g, b);
    float xdim = 0, ydim = 0;
    drawTB(text, (GLuint)hintstext, x, y, textsize, &xdim, &ydim);
    omg_box_suck(x, y, xdim, ydim, sucks, buttonjumpx, buttonjumpy);
    return omg_box_retval(x, y, xdim, ydim);
}

int haveclicked = 0; // For our toggleable movable button.
vec3 tbcoords = (vec3){{0.4, 0.4, 0}};
vec3 slidcoords = (vec3){{0.1, 0.8, 0}};
float slidmoffset = 0;
int slidersliding = 0; // Is the slider being slid?
void menu_draw() {
    if (mb2) { // Use an additional input to move gui elements for testing- right click moves the button.
        tbcoords.d[0] = omg_cursorpos[0];
        tbcoords.d[1] = omg_cursorpos[1];
        haveclicked = 0;
    }

    if (omg_textbox(0.01, 0, "\nEntry 1\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2)
        puts("Entry 1");
    if (omg_textbox(0.01, 0.2, "\nEntry 2\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2)
        puts("Entry 2");
    if (omg_textbox(0.01, 0.4, "\nEntry 3\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2)
        puts("Entry 3");
    if (omg_textbox(0.01, 0.6, "\nQuit\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2) {
        puts("Quitting...");
        isRunning = 0;
    }

    if (omg_textbox(tbcoords.d[0], tbcoords.d[1], "\nClick me and I toggle color!\n", 16, 1, 0.4, 0.3, 0xFFFFFF, haveclicked ? 0xFF0000 : 0x00) &&
        omg_cb == 1) {
        puts("Detected click! EVENT FIRED!\n");
        haveclicked = !haveclicked;
    }
    // A slider element
    if (omg_textbox(slidcoords.d[0], slidcoords.d[1], "\n Slider \n", 16, 1, 0.4, 0.3, 0xFFFFFF, haveclicked ? 0xFF0000 : 0x00) && omg_cb == 1) {
        slidersliding = 1;
        slidmoffset = omg_cursorpos[0] - slidcoords.d[0];
    }
    if (omg_cb == 2)
        slidersliding = 0;
    // Handle the slider sliding behavior.
    if (slidersliding) {
        if (using_cursorkeys) {
            if (omg_udlr[3]) {
                slidcoords.d[0] = clampf(slidcoords.d[0] + 0.05, 0.1, 0.7);
            }
            if (omg_udlr[2]) {
                slidcoords.d[0] = clampf(slidcoords.d[0] - 0.05, 0.1, 0.7);
            }
            omg_cursorpos[0] = slidcoords.d[0] + slidmoffset;
            omg_cursorpos[1] = slidcoords.d[1];
        } else {
            // Move the element to the cursorposition's x.
            slidcoords.d[0] = clampf(omg_cursorpos[0] - slidmoffset, 0.1, 0.7);
        }
        printf("Slider's value is %f\n", slidcoords.d[0]);
    }
    drawMouse();
}

void menu_initScene() {
    static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0};

    static GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    // glLightfv( GL_LIGHT0, GL_AMBIENT, white);
    // glLightfv( GL_LIGHT0, GL_SPECULAR, white);
    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    // glEnable(GL_LIGHTING);
    // glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    // glEnable( GL_LIGHT0 );
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glShadeModel(GL_SMOOTH);
    glTextSize(GL_TEXT_SIZE24x24);
    glEnable(GL_NORMALIZE);
}
//
//BEGIN_EVENT_HANDLER
//        case SDL_KEYDOWN:
//            using_cursorkeys = 1;
//            switch (E_KEYSYM) {
//                case SDLK_ESCAPE:
//                case SDLK_q:
//                    isRunning = 0;
//                    break;
//                case SDLK_UP:
//                    dirbstates[0] = 1;
//                    break;
//                case SDLK_DOWN:
//                    dirbstates[1] = 1;
//                    break;
//                case SDLK_LEFT:
//                    dirbstates[2] = 1;
//                    break;
//                case SDLK_RIGHT:
//                    dirbstates[3] = 1;
//                    break;
//                case SDLK_SPACE:
//                case SDLK_RETURN:
//                    mb = 1;
//                    break;
//                default:
//                    break;
//            }
//            break;
//        case SDL_KEYUP:
//            using_cursorkeys = 1;
//            switch (E_KEYSYM) {
//                case SDLK_SPACE:
//                case SDLK_RETURN:
//                    mb = 0;
//                    break;
//                case SDLK_UP:
//                    dirbstates[0] = 0;
//                    break;
//                case SDLK_DOWN:
//                    dirbstates[1] = 0;
//                    break;
//                case SDLK_LEFT:
//                    dirbstates[2] = 0;
//                    break;
//                case SDLK_RIGHT:
//                    dirbstates[3] = 0;
//                    break;
//                default:
//                    break;
//            }
//            break;
//        case SDL_QUIT:
//            isRunning = 0;
//            break;
//        case SDL_MOUSEBUTTONDOWN:
//            if (E_BUTTON == SDL_BUTTON_LEFT)
//                mb = 1;
//            if (E_BUTTON == SDL_BUTTON_RIGHT) {
//                mb2 = 1;
//            }
//            break;
//        case SDL_MOUSEBUTTONUP:
//            if (E_BUTTON == SDL_BUTTON_LEFT)
//                mb = 0;
//            if (E_BUTTON == SDL_BUTTON_RIGHT)
//                mb2 = 0;
//            break;
//        case SDL_MOUSEMOTION:
//            using_cursorkeys = 0;
//            mousepos[0] = E_MOTION.x;
//            mousepos[1] = E_MOTION.y;
//            break;
//END_EVENT_HANDLER

void menu_initGL() {
    // initialize TinyGL:

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

