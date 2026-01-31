#define UNICODE
#include <windows.h>
#include <gdiplus.h>
#include "MainWindow.h"
#include "resource.h"

using namespace Gdiplus;

MainWindow *mainWindow;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (mainWindow)
        {
            mainWindow->OnPaint(hdc);
            mainWindow->DrawCoordinates(hdc);
        }
        EndPaint(hwnd, &ps);
    }
    break;

    case WM_COMMAND:
        if (mainWindow)
        {
            switch (LOWORD(wParam))
            {
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
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
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

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"3D Polyhedron Rasterizer",
                               WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT,
                               800, 600, nullptr, nullptr, hInstance, nullptr);

    // Кнопки (оставлены как есть для краткости, они работают через ID)
    CreateWindow(L"BUTTON", L"Вверх ↑", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 10, 110, 30, hwnd, (HMENU)ID_MOVE_UP, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вниз ↓", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 50, 110, 30, hwnd, (HMENU)ID_MOVE_DOWN, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Влево ←", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 90, 110, 30, hwnd, (HMENU)ID_MOVE_LEFT, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вправо →", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 130, 110, 30, hwnd, (HMENU)ID_MOVE_RIGHT, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вперед ⟰", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 170, 110, 30, hwnd, (HMENU)ID_MOVE_FORWARD, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Назад ⟱", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 210, 110, 30, hwnd, (HMENU)ID_MOVE_BACKWARD, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Вращение X↻", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 675, 10, 110, 30, hwnd, (HMENU)ID_ROTATE_PLUS_X, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение X↺", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 675, 50, 110, 30, hwnd, (HMENU)ID_ROTATE_MINUS_X, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Y↻", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 675, 100, 110, 30, hwnd, (HMENU)ID_ROTATE_PLUS_Y, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Y↺", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 675, 140, 110, 30, hwnd, (HMENU)ID_ROTATE_MINUS_Y, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Z↻", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 675, 190, 110, 30, hwnd, (HMENU)ID_ROTATE_PLUS_Z, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Вращение Z↺", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 675, 230, 110, 30, hwnd, (HMENU)ID_ROTATE_MINUS_Z, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Увеличить 🔍➕", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 270, 110, 30, hwnd, (HMENU)ID_SCALE_UP, hInstance, nullptr);
    CreateWindow(L"BUTTON", L"Уменьшить 🔍➖", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 310, 110, 30, hwnd, (HMENU)ID_SCALE_DOWN, hInstance, nullptr);
    
    CreateWindow(L"BUTTON", L"Сбросить ❌", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 370, 110, 30, hwnd, (HMENU)ID_RESET, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete mainWindow;
    GdiplusShutdown(gdiplusToken);
    return 0;
}