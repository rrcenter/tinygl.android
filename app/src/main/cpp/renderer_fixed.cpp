
#include "renderer_fixed.h"
#include "tinygl.hpp"

void Set2DF(void)
{
#if 1
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

#else
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    glOrthof(-winSizeX, winSizeX, -winSizeY, winSizeY, -1, 1);
    //glOrthof(-1.0f, 1.0f, -1.5f, 1.5f, -1.0f, 1.0f);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
#endif
}