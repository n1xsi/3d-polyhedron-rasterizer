#pragma once

#include <vector>
#include <windows.h>
#include <gdiplus.h>
#include "Point3D.h"

class Wireframe
{
private:
    std::vector<Point3D> vertices;
    std::vector<Point3D> copy_vertices;
    std::vector<std::vector<int>> faces;

    static void FillQuadrilateral(Gdiplus::Graphics &g, const Gdiplus::PointF &p0, const Gdiplus::PointF &p1,
                                  const Gdiplus::PointF &p2, const Gdiplus::PointF &p3, Gdiplus::Color color);

public:
    Wireframe();

    void Translate(float dx, float dy, float dz);
    void Rotate(float angleX, float angleY, float angleZ);
    void Scale(float sx, float sy, float sz);
    void Reset();
    void Draw(Gdiplus::Graphics &g);

    [[nodiscard]] Point3D GetPosition() const;
};