#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include "Wireframe.h"

// ID кнопок
#define ID_MOVE_UP 1
#define ID_MOVE_DOWN 2
#define ID_MOVE_LEFT 3
#define ID_MOVE_RIGHT 4
#define ID_MOVE_FORWARD 5
#define ID_MOVE_BACKWARD 6
#define ID_ROTATE_PLUS_X 7
#define ID_ROTATE_MINUS_X 8
#define ID_ROTATE_PLUS_Y 9
#define ID_ROTATE_MINUS_Y 10
#define ID_ROTATE_PLUS_Z 11
#define ID_ROTATE_MINUS_Z 12
#define ID_SCALE_UP 13
#define ID_SCALE_DOWN 14
#define ID_RESET 15

class MainWindow
{
private:
    Wireframe wireframe;
    float angleX, angleY, angleZ;

public:
    MainWindow();
    void OnPaint(HDC hdc);
    void DrawCoordinates(HDC hdc);
    void Translate(float dx, float dy, float dz);
    void Rotate(float dx, float dy, float dz);
    void Scale(float sx, float sy, float sz);
    void Reset();
};