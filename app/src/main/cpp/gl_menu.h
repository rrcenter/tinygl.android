#pragma once

#include "tinygl.hpp"

void menu_draw();
void menu_initGL();
void menu_initScene();

int omg_textbox(float x, float y, const char* text, int textsize, int sucks, float buttonjumpx, float buttonjumpy, int hints, int hintstext);
void drawBox(GLfloat x, GLfloat y, GLfloat xdim, GLfloat ydim);