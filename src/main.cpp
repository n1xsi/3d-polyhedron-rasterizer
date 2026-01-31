#define UNICODE

#include <algorithm>
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <cmath>
#include <string>
#include "resource.h"

#pragma comment(lib, "gdiplus.lib")

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

using namespace Gdiplus;

class Point3D {
public:
    float x, y, z;
    Point3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

class Wireframe {
private:
    std::vector<Point3D> vertices;
    std::vector<Point3D> copy_vertices;
    std::vector<std::vector<int>> faces;

public:
    Wireframe() {
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
            {1, 2, 6, 5} // Правая грань
        };


    }

    void Translate(float dx, float dy, float dz) {
        for (auto &vertex: vertices) {
            vertex.x += dx;
            vertex.y += dy;
            vertex.z += dz;
        }
    }

    void Rotate(float angleX, float angleY, float angleZ) {
        // Вращение вокруг трех осей
        float cosX = cos(angleX);
        float sinX = sin(angleX);

        float cosY = cos(angleY);
        float sinY = sin(angleY);

        float cosZ = cos(angleZ);
        float sinZ = sin(angleZ);

        for (auto &vertex: vertices) {
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

    void Scale(float sx, float sy, float sz) {
        for (auto &vertex: vertices) {
            vertex.x *= sx;
            vertex.y *= sy;
            vertex.z *= sz;
        }
    }

    void Reset() {
        vertices = copy_vertices;
    }

    static void FillQuadrilateral(Graphics &g, const PointF &p0, const PointF &p1, const PointF &p2, const PointF &p3,
                                  Color color) {
        /* Алгоритм закрашивает с помощью метода сканирующей строки, который проходит по
            горизонтальным линиям от верхней до нижней границы четырёхугольника, вычисляя пересечения
            сторон четырёхугольника с каждой строкой. Потом закрашивает отрезки между попарными пересечениями */

        SolidBrush brush(color);
        std::vector<PointF> points = {p0, p1, p2, p3};

        // Находим минимальный и максимальный y для ограничивающей рамки
        float minY = std::min({p0.Y, p1.Y, p2.Y, p3.Y});
        float maxY = std::max({p0.Y, p1.Y, p2.Y, p3.Y});

        // Проходим по каждой строке в ограничивающей рамке
        for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y) {
            std::vector<float> intersections;

            // Находим точки пересечения стороны четырёхугольника с текущей строкой y
            for (size_t i = 0; i < points.size(); ++i) {
                PointF pStart = points[i];
                PointF pEnd = points[(i + 1) % points.size()];

                if ((y >= pStart.Y && y < pEnd.Y) || (y >= pEnd.Y && y < pStart.Y)) {
                    float x = pStart.X + (y - pStart.Y) * (pEnd.X - pStart.X) / (pEnd.Y - pStart.Y);
                    intersections.push_back(x);
                }
            }

            std::sort(intersections.begin(), intersections.end());
            for (size_t j = 0; j + 1 < intersections.size(); j += 2) {
                float xStart = intersections[j];
                float xEnd = intersections[j + 1];
                float width = xEnd - xStart;

                if (width > 0) {
                    // Вместо вызова DrawLine используем FillRectangle для заполнения горизонтального отрезка
                    g.FillRectangle(&brush, (int) xStart, (int) y, (int) width, 1);
                }
            }
        }
    }

    void Draw(Graphics &g) {
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

        for (size_t i = 0; i < faces.size(); ++i) {
            const auto &face = faces[i];
            std::vector<PointF> projectedPoints;
            bool isVisible = true;

            // Проверка на видимость грани
            Point3D p1 = vertices[face[0]];
            Point3D p2 = vertices[face[1]];
            Point3D p3 = vertices[face[2]];

            Point3D normal = {
                (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y),
                (p2.z - p1.z) * (p3.x - p1.x) - (p2.x - p1.x) * (p3.z - p1.z),
                (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x)
            };

            float viewDirection = p1.x * normal.x + p1.y * normal.y + (p1.z + distance) * normal.z;
            if (viewDirection >= 0) {
                isVisible = false;
            }

            // Проекция вершин грани
            if (isVisible) {
                for (int idx: face) {
                    Point3D vertex = vertices[idx];
                    if (vertex.z + distance > nearPlane) {
                        float xProj = (vertex.x / (vertex.z + distance)) * 200 + 400;
                        float yProj = (vertex.y / (vertex.z + distance)) * 200 + 300;
                        projectedPoints.emplace_back(xProj, yProj);
                    }
                }

                // Закраска четырёхугольника с помощью сканирующей строки
                if (projectedPoints.size() == 4) {
                    FillQuadrilateral(g, projectedPoints[0], projectedPoints[1], projectedPoints[2],
                        projectedPoints[3], faceColors[i]);

                    // Рисуем границы только один раз для всего четырехугольника
                    for (size_t j = 0; j < projectedPoints.size(); ++j) {
                        g.DrawLine(&pen, projectedPoints[j], projectedPoints[(j + 1) % projectedPoints.size()]);
                    }
                }
            }
        }
    }

    [[nodiscard]] Point3D GetPosition() const {
        return vertices[0];
    }
};

class MainWindow {
private:
    Wireframe wireframe;
    float angleX, angleY, angleZ;

public:
    MainWindow() : angleX(0), angleY(0), angleZ(0) {}

    void OnPaint(HDC hdc) {
        Graphics g(hdc);
        g.Clear(Color(255, 255, 255));
        wireframe.Draw(g);
    }

    void DrawCoordinates(HDC hdc) {
        Point3D pos = wireframe.GetPosition();
        std::wstring coordinates = L"X = " + std::to_wstring(pos.x) + L", Y = " + std::to_wstring(pos.y) + L", Z = " +
                                   std::to_wstring(pos.z);

        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 10, 550, coordinates.c_str(), coordinates.size());
    }

    void Translate(float dx, float dy, float dz) {
        wireframe.Translate(dx, dy, dz);
    }

    void Rotate(float dx, float dy, float dz) {
        angleX += dx;
        angleY += dy;
        angleZ += dz;
        wireframe.Rotate(dx, dy, dz);
    }

    void Scale(float sx, float sy, float sz) {
        wireframe.Scale(sx, sy, sz);
    }

    void Reset() {
        wireframe.Reset();
    }
};

// Глобальные переменные
MainWindow *mainWindow;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            mainWindow->OnPaint(hdc);
            mainWindow->DrawCoordinates(hdc);
            EndPaint(hwnd, &ps);
        }
        break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_MOVE_UP: mainWindow->Translate(0, -0.1f, 0); break;
                case ID_MOVE_DOWN: mainWindow->Translate(0, 0.1f, 0); break;
                case ID_MOVE_LEFT: mainWindow->Translate(-0.1f, 0, 0); break;
                case ID_MOVE_RIGHT: mainWindow->Translate(0.1f, 0, 0); break;
                case ID_MOVE_FORWARD: mainWindow->Translate(0, 0, -0.1f); break;
                case ID_MOVE_BACKWARD: mainWindow->Translate(0, 0, 0.1f); break;

                case ID_ROTATE_PLUS_X: mainWindow->Rotate(0.1f, 0, 0); break;
                case ID_ROTATE_MINUS_X: mainWindow->Rotate(-0.1f, 0, 0); break;
                case ID_ROTATE_PLUS_Y: mainWindow->Rotate(0, 0.1f, 0); break;
                case ID_ROTATE_MINUS_Y: mainWindow->Rotate(0, -0.1f, 0); break;
                case ID_ROTATE_PLUS_Z: mainWindow->Rotate(0, 0, 0.1f); break;
                case ID_ROTATE_MINUS_Z: mainWindow->Rotate(0, 0, -0.1f); break;

                case ID_SCALE_UP: mainWindow->Scale(1.2f, 1.2f, 1.2f); break;
                case ID_SCALE_DOWN: mainWindow->Scale(0.8f, 0.8f, 0.8f); break;

                case ID_RESET: mainWindow->Reset(); break;
            }
            InvalidateRect(hwnd, nullptr, TRUE); break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    mainWindow = new MainWindow();

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"WireframeWindow";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Управление РАСКРАШЕННОЙ трёхмерной каркасной моделью",
                               WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT,
                               800, 600, nullptr, nullptr, hInstance, nullptr);

    // Создание кнопок
    CreateWindow(L"BUTTON", L"Вверх ↑", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 10, 110, 30, hwnd, (HMENU)ID_MOVE_UP, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вниз ↓", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 50, 110, 30, hwnd, (HMENU)ID_MOVE_DOWN, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Влево ←", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 90, 110, 30, hwnd, (HMENU)ID_MOVE_LEFT, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вправо →", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 130, 110, 30, hwnd, (HMENU)ID_MOVE_RIGHT, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вперед ⟰", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 170, 110, 30, hwnd, (HMENU)ID_MOVE_FORWARD, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Назад ⟱", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 210, 110, 30, hwnd, (HMENU)ID_MOVE_BACKWARD, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Вращение X↻", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 675, 10, 110, 30, hwnd, (HMENU)ID_ROTATE_PLUS_X, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение X↺", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 675, 50, 110, 30, hwnd, (HMENU)ID_ROTATE_MINUS_X, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Y↻", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 675, 100, 110, 30, hwnd, (HMENU)ID_ROTATE_PLUS_Y, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Y↺", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 675, 140, 110, 30, hwnd, (HMENU)ID_ROTATE_MINUS_Y, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Z↻", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 675, 190, 110, 30, hwnd, (HMENU)ID_ROTATE_PLUS_Z, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Z↺", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 675, 230, 110, 30, hwnd, (HMENU)ID_ROTATE_MINUS_Z, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Увеличить 🔍➕", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 270, 110, 30, hwnd, (HMENU)ID_SCALE_UP, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Уменьшить 🔍➖", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 310, 110, 30, hwnd, (HMENU)ID_SCALE_DOWN, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Сбросить ❌", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 370, 110, 30, hwnd, (HMENU)ID_RESET, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    delete mainWindow;
    return 0;
}
