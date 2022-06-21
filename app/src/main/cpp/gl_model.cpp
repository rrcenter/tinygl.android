
#include "gl_model.h"

extern int winSizeX;
extern int winSizeY;
extern ZBuffer* frameBuffer;

#include "kkFileUtils.h"
//#define PLAY_MUSIC

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/GL/gl.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "../include-demo/stb_image.h"
#define CHAD_MATH_IMPL
#include "../include-demo/3dMath.h"
#include "../include-demo/tobjparse.h"
#define CHAD_API_IMPL
#include "../include/zbuffer.h"
#ifdef PLAY_MUSIC
#include "../include-demo/api_audio.h"
#else
typedef unsigned char uchar;
#endif

static GLuint tex = 0;
static int noSDL = 0;
static int doblend = 0;
static int dlExists = 0;
static int doTextures = 1;
static unsigned int count = 40;
static GLuint modelDisplayList = 0;

#ifndef M_PI
#define M_PI 3.14159265
#endif

vec3 campos = (vec3){.d[0] = 0, .d[1] = 0, .d[2] = -3};
vec3 camforw = (vec3){.d[0] = 0, .d[1] = 0, .d[2] = -1};
vec3 camup = (vec3){.d[0] = 0, .d[1] = 1, .d[2] = 0};
uint wasdstate[4] = {0, 0, 0, 0};
const float mouseratiox = 1.0 / 300.0f;
const float mouseratioy = 1.0 / 300.0f;
int mousex = 0, mousey = 0;

int ModelArrayLoaded = 0;
int testingModelArrays = 0;
int testingCopyImage2D = 0;
struct {
    float* points;
    uint npoints;
    float* normals;
    float* colors;
    float* texcoords;
} ModelArray;

void FreeModelArray() {
    if (!ModelArrayLoaded) {
        ModelArray.points = NULL;
        ModelArray.normals = NULL;
        ModelArray.npoints = 0;
        ModelArray.colors = NULL;
        ModelArray.texcoords = NULL;
        return;
    }
    ModelArrayLoaded = 0;
    if (ModelArray.points)
        free(ModelArray.points);
    if (ModelArray.normals)
        free(ModelArray.normals);
    if (ModelArray.texcoords)
        free(ModelArray.texcoords);
    if (ModelArray.colors)
        free(ModelArray.colors);
    ModelArray.npoints = 0;
    return;
}

void rotateCamera() {
    vec3 a;
    a.d[1] = (float)mousex * mouseratiox;
    a.d[2] = (float)mousey * mouseratioy;

    vec3 right = normalizev3(crossv3(camforw, camup));
    right.d[1] = 0;
    // vec3 forward = glm::vec3(glm::normalize(glm::rotate(angle, right) * glm::vec4(forward, 0.0)));

    camforw = normalizev3(rotatev3(camforw, right, -a.d[2]));
    // up = glm::normalize(glm::cross(forward, right));
    camup = normalizev3(crossv3(right, camforw));

    // Perform the rotation about the Y axis last.
    static const vec3 UP = (vec3){{0, 1, 0}};
    static const vec3 DOWN = (vec3){{0, -1, 0}};
    if (dotv3(UP, camup) < 0) {
        camforw = normalizev3(rotatev3(camforw, DOWN, -a.d[1]));
        camup = normalizev3(rotatev3(camup, DOWN, -a.d[1]));
    } else {
        camforw = normalizev3(rotatev3(camforw, UP, -a.d[1]));
        camup = normalizev3(rotatev3(camup, UP, -a.d[1]));
    }
}
static
GLuint loadRGBTexture(unsigned char* buf, unsigned int w, unsigned int h) {
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
    return t;
}

void LoadModelArrays(
        // HUGE important note! these depend on the math library using
        // f_ as float and not double!
        // Remember that!
        vec3* points, uint npoints, vec3* colors, vec3* normals, vec3* texcoords) {
    if (!points)
        return;
    FreeModelArray();
    ModelArrayLoaded = 1;
    ModelArray.npoints = npoints;
    ModelArray.points = (float *)(malloc(sizeof(float) * npoints * 3));
    if (normals)
        ModelArray.normals = (float *)(malloc(sizeof(float) * npoints * 3));
    if (texcoords)
        ModelArray.texcoords = (float *)(malloc(sizeof(float) * npoints * 2));
    if (colors)
        ModelArray.colors = static_cast<float *>(malloc(sizeof(float) * npoints * 3));
    for (uint i = 0; i < npoints; i++) {
        if (colors) { // Fix for TinyGL color interpolation.
            ModelArray.colors[i * 3 + 0] = colors[i].d[0];
            ModelArray.colors[i * 3 + 1] = colors[i].d[1];
            ModelArray.colors[i * 3 + 2] = colors[i].d[2];
        }
        if (texcoords) {
            ModelArray.texcoords[i * 2 + 0] = texcoords[i].d[0];
            ModelArray.texcoords[i * 2 + 1] = texcoords[i].d[1];
        }
        if (normals) {
            ModelArray.normals[i * 3 + 0] = normals[i].d[0];
            ModelArray.normals[i * 3 + 1] = normals[i].d[1];
            ModelArray.normals[i * 3 + 2] = normals[i].d[2];
        }
        ModelArray.points[i * 3 + 0] = points[i].d[0];
        ModelArray.points[i * 3 + 1] = points[i].d[1];
        ModelArray.points[i * 3 + 2] = points[i].d[2];
    }
}

// Without display list
void drawModelArrays(
        // HUGE important note! these depend on the math library using
        // f_ as float and not double!
        // Remember that!
        vec3* points, uint npoints, vec3* colors, vec3* normals, vec3* texcoords) {
    if (!points)
        return;
    glBegin(GL_TRIANGLES);
    for (uint i = 0; i < npoints; i++) {
        if (colors) { // Fix for TinyGL color interpolation.
            glColor3f(colors[i].d[0], colors[i].d[1], colors[i].d[2]);
        }
        if (texcoords)
            glTexCoord2f(texcoords[i].d[0], texcoords[i].d[1]);
        if (normals)
            glNormal3f(normals[i].d[0], normals[i].d[1], normals[i].d[2]);
        glVertex3f(points[i].d[0], points[i].d[1], points[i].d[2]);
    }
    glEnd();
}

GLuint createModelDisplayList(
        // HUGE important note! these depend on the math library using
        // f_ as float and not double!
        // Remember that!
        vec3* points, uint npoints, vec3* colors, vec3* normals, vec3* texcoords) {
    GLuint ret = 0;
    if (!points)
        return 0;
    ret = glGenLists(1);
    glNewList(ret, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (uint i = 0; i < npoints; i++) {
        if (colors) {
            glColor3f(colors[i].d[0], colors[i].d[1], colors[i].d[2]);
        }
        if (texcoords)
            glTexCoord2f(texcoords[i].d[0], texcoords[i].d[1]);
        if (normals)
            glNormal3f(normals[i].d[0], normals[i].d[1], normals[i].d[2]);
        glVertex3f(points[i].d[0], points[i].d[1], points[i].d[2]);
    }
    // printf("\ncreateModelDisplayList is not the problem.\n");
    glEnd();
    glEndList();
    return ret;
}

static
GLubyte stipplepattern[128] = {0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                               0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};

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

static int isRunning = 1;

/*
mousex = (kHeld & KEY_Y)?-30:0 + (kHeld & KEY_A)?30:0;
		mousey = (kHeld & KEY_X)?-30:0 + (kHeld & KEY_B)?30:0;
*/
#if 0
BEGIN_EVENT_HANDLER
        case SDL_KEYDOWN:
            switch (E_KEYSYM) {
                case SDLK_w:
                    wasdstate[0] = 1;
                    break;
                case SDLK_a:
                    wasdstate[1] = 1;
                    break;
                case SDLK_s:
                    wasdstate[2] = 1;
                    break;
                case SDLK_d:
                    wasdstate[3] = 1;
                    break;
                case SDLK_UP:
                    mousey = -30;
                    break;
                case SDLK_DOWN:
                    mousey = 30;
                    break;
                case SDLK_LEFT:
                    mousex = -30;
                    break;
                case SDLK_RIGHT:
                    mousex = 30;
                    break;
                case SDLK_ESCAPE:
                case SDLK_q:
                    isRunning = 0;
                    break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (E_KEYSYM) {
                case SDLK_w:
                    wasdstate[0] = 0;
                    break;
                case SDLK_a:
                    wasdstate[1] = 0;
                    break;
                case SDLK_s:
                    wasdstate[2] = 0;
                    break;
                case SDLK_d:
                    wasdstate[3] = 0;
                    break;
                case SDLK_UP:
                case SDLK_DOWN:
                    mousey = 0;
                    break;
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    mousex = 0;
                    break;
                default:
                    break;
            }
            break;
        case SDL_QUIT:
            isRunning = 0;
            break;
END_EVENT_HANDLER
#endif

void model_initScene() {

}

std::vector<std::string> string_split(const std::string& s, char seperator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}


void model_initGL() {
    char needsRGBAFix = 0;


    GLuint buffers[4]; // pos,color,normal,texcoord

    const char* modelName = "extrude.obj";

    frameBuffer = ZB_open(winSizeX, winSizeY, ZB_MODE_RGBA, 0);
    glInit(frameBuffer);

//    srand(time(NULL));
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glSetEnableSpecular(0);
    static GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};
    static GLfloat pos[4] = {5, 5, 10, 0.0}; // Light at infinity.

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    // glLightfv( GL_LIGHT0, GL_AMBIENT, white);
    // glLightfv( GL_LIGHT0, GL_SPECULAR, white);
    glEnable(GL_CULL_FACE);

    // glDisable( GL_LIGHTING );
    glEnable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);

    glClearColor(0, 0, 0.3, 0);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    // glDisable(GL_DEPTH_TEST);
    double t = 0;
    glViewport(0, 0, winSizeX, winSizeY);
    // glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(stipplepattern);
    // initScene();
    {
        objraw omodel;
        model m = initmodel();
        std::string buf = kkFileUtils::sharedFileUtils()->getFileData(modelName, "r");
//        std::vector<std::string> array = string_split(buf, '\n');
//        omodel = tobj_load(modelName);
        omodel = tobj_load_buf((char*)buf.c_str());

        if (!omodel.positions) {
            puts("\nERROR! No positions in model. Aborting...\n");
        } else {
            m = tobj_tomodel(&omodel);
            printf("\nHas %ld points.\n", m.npoints);
            if (!testingModelArrays) {
                modelDisplayList = createModelDisplayList(m.d, m.npoints, m.c, m.n, m.t);
                dlExists = 1;
            } else {
                LoadModelArrays(m.d, m.npoints, m.c, m.n, m.t);
                /*
                if(ModelArray.colors)glEnableClientState(GL_COLOR_ARRAY);
                if(ModelArray.points)glEnableClientState(GL_VERTEX_ARRAY);
                if(ModelArray.normals)glEnableClientState(GL_NORMAL_ARRAY);
                if(ModelArray.texcoords)glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                if(ModelArray.points)glVertexPointer(3,GL_FLOAT,0,ModelArray.points);
                if(ModelArray.normals)glNormalPointer(GL_FLOAT,0,ModelArray.normals); //Must be 3!
                if(ModelArray.colors)glColorPointer(3,GL_FLOAT,0,ModelArray.colors);
                if(ModelArray.texcoords)glTexCoordPointer(2,GL_FLOAT,0,ModelArray.texcoords);
                */
                glGenBuffers(4, buffers);
                for (int i = 0; i < 4; i++) {
                    printf("\nBuffer %d is %d", i, buffers[i]);
                    if (buffers[i] == 0) {
                        printf("\nBuffer allocation failed for buffer %d!\n", i);
                        return ;
                    }
                }
                glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
                /*Done multiple times to test and make sure that data isn't leaked*/
                puts("\nTesting glBufferData data integrity\n");
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ModelArray.npoints, ModelArray.points, GL_STATIC_DRAW);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ModelArray.npoints, ModelArray.points, GL_STATIC_DRAW);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ModelArray.npoints, ModelArray.points, GL_STATIC_DRAW);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ModelArray.npoints, ModelArray.points, GL_STATIC_DRAW);
                if (glMapBuffer(GL_ARRAY_BUFFER, 0) == NULL)
                    printf("\nglBufferData failed for buffer %d!\n", 0);
                glBindBufferAsArray(GL_VERTEX_BUFFER, buffers[0], GL_FLOAT, 3, 0);
                if (ModelArray.colors) {
                    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ModelArray.npoints, ModelArray.colors, GL_STATIC_DRAW);
                    glBindBufferAsArray(GL_COLOR_BUFFER, buffers[1], GL_FLOAT, 3, 0);
                }
                if (ModelArray.normals) {
                    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ModelArray.npoints, ModelArray.normals, GL_STATIC_DRAW);
                    glBindBufferAsArray(GL_NORMAL_BUFFER, buffers[2], GL_FLOAT, 3, 0);
                }
                if (ModelArray.texcoords) {
                    glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * ModelArray.npoints, ModelArray.texcoords, GL_STATIC_DRAW);
                    glBindBufferAsArray(GL_TEXTURE_COORD_BUFFER, buffers[3], GL_FLOAT, 2, 0);
                }
            }
            freemodel(&m);
        }
        freeobjraw(&omodel);
    }

//    GLuint tex = 0;
    if (doTextures) {
        int sw = 0, sh = 0, sc = 0; // sc goes unused.
#if TGL_FEATURE_NO_DRAW_COLOR == 1
        uchar* source_data = stbi_load("tex_hole.png", &sw, &sh, &sc, 3);
#else
//        uchar* source_data = stbi_load("tex.jpg", &sw, &sh, &sc, 3);
        std::string filedata = kkFileUtils::sharedFileUtils()->getFileData("tex.jpg", "rb");
        uchar* source_data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(filedata.c_str()), filedata.length(), &sw, &sh, &sc, 3);
#endif
        if (source_data) {
            tex = loadRGBTexture(source_data, sw, sh);
            free(source_data);
        }
        // tex =
    }
    // glDisable(GL_LIGHTING);
    // glEnable( GL_NORMALIZE );
    // variables for timing:
}

void model_draw() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // gluPerspective(70,(float)WIDTH/(float)HEIGHT,1,512);
    mat4 matrix = perspective(70, (float)winSizeX / (float)winSizeY, 1, 512);
    glLoadMatrixf(matrix.d);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix(); // Pushing on the LookAt Matrix.

    vec3 right = normalizev3(crossv3(camforw, camup));
    // right.d[1] = 0;
    matrix = (lookAt(campos, addv3(campos, camforw), camup)); // Using right vector to correct for screen rotation.
    glLoadMatrixf(matrix.d);
    if (wasdstate[0])
        campos = addv3(campos, scalev3(0.1, camforw));
    if (wasdstate[2])
        campos = addv3(campos, scalev3(-0.1, camforw));
    if (wasdstate[1])
        campos = addv3(campos, scalev3(-0.1, right));
    if (wasdstate[3])
        campos = addv3(campos, scalev3(0.1, right));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    if (doTextures)
        glBindTexture(GL_TEXTURE_2D, tex);
    // glDisable(GL_BLEND);
    // Testing blending for textured triangles.
    // glDisable(GL_DEPTH_TEST);
    if (doblend) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_BLEND);
    }
    glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR);
    glBlendEquation(GL_FUNC_ADD);
    // glDisable(GL_TEXTURE_2D);
    // printf("\nNew triangle!\n");
    if (!dlExists) {
        if (!testingModelArrays) {
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_TRIANGLES);
            // glColor3f(0,0,1);
            glColor3f(1, 0, 0);
            glTexCoord2f(0, 0);
            glVertex3f(-1, -1, -10);
            glColor3f(0, 1, 0);
            glTexCoord2f(1, 0);
            glVertex3f(1, -1, -10);
            glColor3f(0, 0, 1);
            glTexCoord2f(0.5, 1);
            glVertex3f(0, 1, -10);
            // glColor3f(0,1,0);
            glEnd();
        } else {
            if (doTextures)
                glEnable(GL_TEXTURE_2D);
            // glDisable(GL_TEXTURE_2D);
            glEnable(GL_POLYGON_STIPPLE);
            // puts("\nUSING ARRAYS!");
            // glDisable(GL_COLOR_MATERIAL);
            for (unsigned int i = 0; i < count; i++) {
                glPushMatrix();
                mat4 horiz_translation = translate((vec3){{float(8.0 * (i % 10)), 0.0, 0.0}});
                mat4 vert_translation = translate((vec3){{0.0, float(8.0 * (i / 10)), 0.0}});
                const mat4 ztranslation = translate((vec3){{0, 0, -10}});
                mat4 total_translation = multm4(multm4(horiz_translation, vert_translation), ztranslation);
                glMultMatrixf(total_translation.d);
                // glTranslatef((float)(i % 10) * 8.0, (float)(i / 10) * 8.0, -10);
                glBegin(GL_TRIANGLES);
                for (uint j = 0; j < ModelArray.npoints; j++)
                    glArrayElement(j);
                glEnd();
                glPopMatrix();
            }
            glDisable(GL_POLYGON_STIPPLE);
            glDisable(GL_TEXTURE_2D);
        }
    } else {
        if (doTextures)
            glEnable(GL_TEXTURE_2D);
        // glDisable(GL_TEXTURE_2D);
        glEnable(GL_POLYGON_STIPPLE);
        // glDisable(GL_COLOR_MATERIAL);
        for (unsigned int i = 0; i < count; i++) {
            glPushMatrix();
            mat4 horiz_translation = translate((vec3){{float(8.0 * (i % 10)), 0.0, 0.0}});
            mat4 vert_translation = translate((vec3){{0.0, float(8.0 * (i / 10)), 0.0}});
            const mat4 ztranslation = translate((vec3){{0, 0, -10}});
            mat4 total_translation = multm4(multm4(horiz_translation, vert_translation), ztranslation);
            glMultMatrixf(total_translation.d);
            // glTranslatef((float)(i % 10) * 8.0, (float)(i / 10) * 8.0, -10);
            glCallList(modelDisplayList);
            // drawModel(
            // m.d, m.npoints,
            // m.c,
            // m.n,
            // m.t
            // );
            glPopMatrix();
        }
        glDisable(GL_POLYGON_STIPPLE);
        if (doTextures)
            glDisable(GL_TEXTURE_2D);
    }
    // draw();
    glPopMatrix(); // The view transform.

    rotateCamera();
    glTextSize(GL_TEXT_SIZE16x16);
    glDrawText((unsigned char*)"\nModel Viewer Demo-\nTinyGL\n", 0, 0, 0x000000FF);

}