#include "Wireframe.h"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace Gdiplus;

Wireframe::Wireframe()
{
    vertices = {
        {-1, -1, -1}, {1, -1, -1},
        {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1},
        {1, 1, 1}, {-1, 1, 1}
    };

    copy_vertices = vertices;

    faces = {
        {3, 2, 1, 0}, // Задняя грань
        {4, 5, 6, 7}, // Передняя грань
        {0, 1, 5, 4}, // Нижняя грань
        {2, 3, 7, 6}, // Верхняя грань
        {3, 0, 4, 7}, // Левая грань
        {1, 2, 6, 5}  // Правая грань
    };
}

void Wireframe::Translate(float dx, float dy, float dz)
{
    for (auto &vertex : vertices)
    {
        vertex.x += dx;
        vertex.y += dy;
        vertex.z += dz;
    }
}

void Wireframe::Rotate(float angleX, float angleY, float angleZ)
{
    float cosX = cos(angleX);
    float sinX = sin(angleX);
    float cosY = cos(angleY);
    float sinY = sin(angleY);
    float cosZ = cos(angleZ);
    float sinZ = sin(angleZ);

    for (auto &vertex : vertices)
    {
        // Вращение вокруг оси X
        float yNew = vertex.y * cosX - vertex.z * sinX;
        float zNew = vertex.y * sinX + vertex.z * cosX;
        vertex.y = yNew;
        vertex.z = zNew;

        // Вращение вокруг оси Y
        float xNew = vertex.x * cosY + vertex.z * sinY;
        zNew = -vertex.x * sinY + vertex.z * cosY;
        vertex.x = xNew;
        vertex.z = zNew;

        // Вращение вокруг оси Z
        xNew = vertex.x * cosZ - vertex.y * sinZ;
        yNew = vertex.x * sinZ + vertex.y * cosZ;
        vertex.x = xNew;
        vertex.y = yNew;
    }
}

void Wireframe::Scale(float sx, float sy, float sz)
{
    for (auto &vertex : vertices)
    {
        vertex.x *= sx;
        vertex.y *= sy;
        vertex.z *= sz;
    }
}

void Wireframe::Reset()
{
    vertices = copy_vertices;
}

void Wireframe::FillQuadrilateral(Graphics &g, const PointF &p0, const PointF &p1,
                                  const PointF &p2, const PointF &p3, Color color)
{
    /* Алгоритм закрашивает с помощью метода сканирующей строки, который проходит по горизонтальным
        линиям от верхней до нижней границы четырёхугольника, вычисляя пересечения сторон четырёхугольника
        с каждой строкой. После нахождения закрашивает отрезки между попарными пересечениями. */

    SolidBrush brush(color);
    std::vector<PointF> points = {p0, p1, p2, p3};

    // Поиск минимального и максимального Y для ограничивающей рамки
    float minY = std::min({p0.Y, p1.Y, p2.Y, p3.Y});
    float maxY = std::max({p0.Y, p1.Y, p2.Y, p3.Y});

    // Проход по каждой строке в ограничивающей рамке
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y)
    {
        std::vector<float> intersections;

        // Поиск точки пересечения стороны четырёхугольника с текущей строкой Y
        for (size_t i = 0; i < points.size(); ++i)
        {
            PointF pStart = points[i];
            PointF pEnd = points[(i + 1) % points.size()];

            if ((y >= pStart.Y && y < pEnd.Y) || (y >= pEnd.Y && y < pStart.Y))
            {
                float x = pStart.X + (y - pStart.Y) * (pEnd.X - pStart.X) / (pEnd.Y - pStart.Y);
                intersections.push_back(x);
            }
        }
        std::sort(intersections.begin(), intersections.end());
        for (size_t j = 0; j + 1 < intersections.size(); j += 2)
        {
            float width = intersections[j + 1] - intersections[j];
            if (width > 0)
            {
                // Вместо вызова DrawLine обязательно использовать FillRectangle для заполнения горизонтального отрезка
                g.FillRectangle(&brush, (int)intersections[j], y, (int)width, 1);
            }
        }
    }
}

void Wireframe::Draw(Graphics &g)
{
    /* Этот алгоритм рисует трёхмерную каркасную модель, определяя видимость граней и проецируя их на 2D-плоскость.
        Для каждой грани вычисляется перпендикуляр к ней и проверяется видимость грани относительно направления взгляда.
        Если грань не видна, она пропускается. Если видна, то выполняется проекция вершин грани на 2D-плоскость с учётом
        перспективы, чтобы получить координаты для отображения. Если проекция даёт четырёхугольник, он закрашивается с
        помощью функции `FillQuadrilateral`. Затем рисуются границы четырёхугольника, чтобы показать его контур. */

    Color faceColors[6] = {
        Color(255, 255, 0, 0), // Красная грань
        Color(255, 0, 255, 0), // Зелёная грань
        Color(255, 0, 0, 255), // Синяя грань
        Color(255, 255, 255, 0), // Жёлтая грань
        Color(255, 255, 0, 255), // Фиолетовая грань
        Color(255, 0, 255, 255) // Голубая грань
    };

    float nearPlane = 0.1f;
    float distance = 5.0f;
    Pen pen(Color(0, 0, 0));

    for (size_t i = 0; i < faces.size(); ++i)
    {
        const auto &face = faces[i];
        std::vector<PointF> projectedPoints;

        Point3D p1 = vertices[face[0]];
        Point3D p2 = vertices[face[1]];
        Point3D p3 = vertices[face[2]];

        Point3D normal = {
            (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y),
            (p2.z - p1.z) * (p3.x - p1.x) - (p2.x - p1.x) * (p3.z - p1.z),
            (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x)
        };

        float viewDirection = p1.x * normal.x + p1.y * normal.y + (p1.z + distance) * normal.z;

        // Проверка на видимость грани
        if (viewDirection < 0)
        {
            for (int idx : face)
            {
                Point3D vertex = vertices[idx];
                if (vertex.z + distance > nearPlane)
                {
                    float xProj = (vertex.x / (vertex.z + distance)) * 200 + 400;
                    float yProj = (vertex.y / (vertex.z + distance)) * 200 + 300;
                    projectedPoints.emplace_back(xProj, yProj);
                }
            }

            // Закраска четырёхугольника с помощью сканирующей строки
            if (projectedPoints.size() == 4)
            {
                FillQuadrilateral(g, projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3], faceColors[i]);
                
                // Отрисовка границы только один раз для всего четырёхугольника
                for (size_t j = 0; j < projectedPoints.size(); ++j)
                {
                    g.DrawLine(&pen, projectedPoints[j], projectedPoints[(j + 1) % projectedPoints.size()]);
                }
            }
        }
    }
}

Point3D Wireframe::GetPosition() const
{
    return vertices[0];
}