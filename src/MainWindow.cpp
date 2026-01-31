#include "MainWindow.h"

using namespace Gdiplus;

MainWindow::MainWindow() : angleX(0), angleY(0), angleZ(0) {}

void MainWindow::OnPaint(HDC hdc)
{
    Graphics g(hdc);
    g.Clear(Color(255, 255, 255));
    wireframe.Draw(g);
}

void MainWindow::DrawCoordinates(HDC hdc)
{
    Point3D pos = wireframe.GetPosition();
    std::wstring coordinates = L"X = " + std::to_wstring(pos.x) +
                               L", Y = " + std::to_wstring(pos.y) +
                               L", Z = " + std::to_wstring(pos.z);

    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, 10, 550, coordinates.c_str(), coordinates.size());
}

void MainWindow::Translate(float dx, float dy, float dz)
{
    wireframe.Translate(dx, dy, dz);
}

void MainWindow::Rotate(float dx, float dy, float dz)
{
    angleX += dx;
    angleY += dy;
    angleZ += dz;
    wireframe.Rotate(dx, dy, dz);
}

void MainWindow::Scale(float sx, float sy, float sz)
{
    wireframe.Scale(sx, sy, sz);
}

void MainWindow::Reset()
{
    wireframe.Reset();
}