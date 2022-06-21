/*
 * glGeLists & glCallList
 */

#include "gl_hello2.h"

#include "kk_log.h"

extern int winSizeX;
extern int winSizeY;
extern ZBuffer *frameBuffer;

static GLuint Object;
static GLuint tex = 0;
static double time_passed = 0.0;

void hello2_draw() {
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D,tex);
    time_passed += 0.0166666 * 10;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();

    glRotatef(time_passed, 0, 0, 1);

    glCallList(Object);

    glPopMatrix();

    glDrawText((const GLubyte *) "glGenLists\nglCallList\nFrom TinyGL", 0, 0, 0x00FFFFFF);
}

void hello2_initGL() {
    frameBuffer = NULL;
    frameBuffer = ZB_open(winSizeX, winSizeY, ZB_MODE_RGBA, 0);

    LOGI("fb:%p w=%d,h=%d\n", frameBuffer, winSizeX, winSizeY);

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
}

void hello2_initScene() {

    // glLightfv( GL_LIGHT0, GL_AMBIENT, white);
    // glLightfv( GL_LIGHT0, GL_SPECULAR, white);
    // glEnable( GL_CULL_FACE );
    glDisable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    // glEnable( GL_LIGHT0 );
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glTextSize(GL_TEXT_SIZE24x24);
    /*
    {
        int sw = 0, sh = 0, sc = 0; //sc goes unused.
        uchar* source_data = stbi_load("texture.png", &sw, &sh, &sc, 3);
        if(source_data){
            tex = loadRGBTexture(source_data, sw, sh);
            free(source_data);
        } else {
            printf("\nCan't load texture!\n");
        }
    }*/
    glEnable(GL_NORMALIZE);


    // make object
    {
        Object = glGenLists(1);

        glNewList(Object, GL_COMPILE);

        glBegin(GL_TRIANGLES);
        glColor3f(0.2, 0.2, 1.0); // BLUE!
        // glColor3f(1.0, 0.2, 0.2); //RED!
        glVertex3f(-0.8, -0.8, 0.2);

        glColor3f(0.2, 1.0, 0.2); // GREEN!
        // glColor3f(1.0, 0.2, 0.2); //RED!
        glVertex3f(0.8, -0.8, 0.2);

        glColor3f(1.0, 0.2, 0.2); // RED!
        glVertex3f(0, 1.2, 0.2);
        glEnd();

        glEndList();
    }

}