#include "gl_touch_event.h"


static int gl_touch_type = -1;
static float gl_touch_x = -1;
static float gl_touch_y = -1;

void gl_set_touchevent(int type, float x, float y)
{
    gl_touch_type = type;
    gl_touch_x = x;
    gl_touch_y = y;
}
void gl_get_touchevent(int *type, float *x, float *y)
{
    *type = gl_touch_type;
    *x = gl_touch_x;
    *y = gl_touch_y;
}
