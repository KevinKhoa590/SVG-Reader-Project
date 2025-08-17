#include "stdafx.h"
#include "rapidxml.hpp"
#include "SVGParser.h"
#include "SVGRenderer.h"
#include "SVGElement.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
using namespace std;
using namespace rapidxml;
using namespace Gdiplus;

#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

SVGParseResult svgResult;
vector<unique_ptr<SVGElement>>& elements = svgResult.elements;

VOID OnPaint(HDC hdc)
{
    Graphics graphics(hdc);
    static SVGRenderer renderer;
    renderer.render(svgResult, graphics);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int main()
{
    // GDI+ init
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Ask user for SVG path until valid
    string svgPath;
    while (true) {
        cout << "Enter path to SVG file: ";
        getline(cin, svgPath);

        if (svgPath.empty()) {
            cerr << "Path cannot be empty. Try again.\n";
            continue;
        }

        ifstream test(svgPath);
        if (test.good()) {
            break;
        }
        else {
            cerr << "File not found: " << svgPath << "\nTry again.\n";
        }
    }

    // Parse SVG file
    svgResult = SVGParser::parse(svgPath);

    // Register window class
    WNDCLASS wndClass = {};
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = GetModuleHandle(NULL);
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszClassName = TEXT("SVGWindow");

    RegisterClass(&wndClass);

    HWND hWnd = CreateWindow(
        TEXT("SVGWindow"), TEXT("SVG Viewer"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        OnPaint(hdc);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
