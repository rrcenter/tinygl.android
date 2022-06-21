#pragma once

#define kTouchBegin 1
#define kTouchMove  2
#define kTouchEnd   3

void gl_set_touchevent(int type, float x, float y);
void gl_get_touchevent(int *type, float *x, float *y);