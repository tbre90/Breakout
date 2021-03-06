﻿#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <strsafe.h>
#include <wchar.h>

#include "..\..\Include\backbuffer.h"
#include "..\..\Include\common.h"
#include "..\..\Include\game_api.h"
#include "..\..\Include\platform_api.h"

// restart game custom message
// according to msdn:
// Message numbers in the [...] range (0x8000 through 0xBFFF)
// are available for applications to use as private messages.
// Messages in this range do not conflict with system messages.
//
// sent after window is created, and whenever the user wants to restart @ game over
#define WM_RESTART_GAME 0x8FFF

struct platform
{
    struct backbuffer backbuffer;
    struct platform_data pd;
    struct window
    {
        HDC device_context;
        int width;
        int height;
    } window;
    HFONT font;
};

static struct platform g_platform;

LRESULT CALLBACK
WndProc(HWND, UINT, WPARAM, LPARAM);

static struct backbuffer *
get_backbuffer(void)
{
    return &g_platform.backbuffer;
}

static void
render(HDC device_context,
       int client_width,
       int client_height,
       struct backbuffer *buffer)
{
    StretchBlt(
        device_context,
        0, 0,
        client_width, client_height,
        buffer->device_context,
        0, 0,
        buffer->width,
        buffer->height,
        SRCCOPY
    );

    GdiFlush();
}

int WINAPI
WinMain(HINSTANCE h_instance,
        HINSTANCE h_prev_instance,
        LPSTR cmd_line,
        int cmd_show)
{
    // silence compiler about unused function parameters
    UNREFERENCED_PARAMETER(h_instance);
    UNREFERENCED_PARAMETER(h_prev_instance);
    UNREFERENCED_PARAMETER(cmd_line);
    UNREFERENCED_PARAMETER(cmd_show);

    char const * const app_name = "Breakout";

    WNDCLASSEX wndclass = {0};
    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.hInstance      = h_instance;
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName  = app_name;

    if (!RegisterClassEx(&wndclass))
    {
        MessageBox(NULL, "Failed to register class.", "Error", MB_OK);
        goto early_exit;
    }

    HWND hwnd =
        CreateWindowEx(
            WS_EX_OVERLAPPEDWINDOW,
            app_name,
            "Breakout",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, h_instance, NULL
        );

    if (!hwnd)
    {
        MessageBox(NULL, "Failed to create main window.", "Error", MB_OK);
        goto early_exit;
    }

    g_platform.backbuffer = initialize_backbuffer(hwnd);
    if (g_platform.backbuffer.error)
    {
        MessageBox(NULL, "Failed to initialize backbuffer.", "Error", MB_OK);
        goto early_exit;
    }

    // don't want to automatically paint background
    SetBkMode(g_platform.backbuffer.device_context, TRANSPARENT);

    g_platform.font = 
            CreateFont(
                80, 50, 0, 0,
                FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                DEFAULT_PITCH | FF_ROMAN,
                "Times New Roman"
            );

    if (!g_platform.font)
    {
        MessageBox(NULL, "CreateFont failed.", "Error", MB_OK);
        goto early_exit;
    }

    g_platform.window.device_context = GetDC(hwnd);

    SelectObject(g_platform.backbuffer.device_context, g_platform.font);

    // start the game
    SendMessage(hwnd, WM_RESTART_GAME, 0, 0);

    // show 
    ShowWindow(hwnd, cmd_show);
    UpdateWindow(hwnd);

    MSG msg = {0};

    LARGE_INTEGER freq          = {0};
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER currentTicks  = {0};
    LARGE_INTEGER previousTicks = {0};

#ifdef DEBUG_PRINT
    char       old_window_text[1024] = {0};
    GetWindowText(hwnd, old_window_text, 1024);

    char       new_window_text[1024] = {0};

    LARGE_INTEGER seconds = {0};
    LARGE_INTEGER fps_counter = {0};

#endif

    double ms_per_frame = game_request_ms_per_frame();

    QueryPerformanceCounter(&previousTicks);
    double lag = 0.0;
    for (;;)
    {
        QueryPerformanceCounter(&currentTicks);
        LARGE_INTEGER elapsedTicks;
        elapsedTicks.QuadPart = currentTicks.QuadPart - previousTicks.QuadPart;
        previousTicks = currentTicks;

        lag += ((double)(1000 * elapsedTicks.QuadPart)) / (double)freq.QuadPart;

#ifdef DEBUG_PRINT

        seconds.QuadPart += elapsedTicks.QuadPart;

        if (((seconds.QuadPart * 1000) / freq.QuadPart) >= 250)
        {
            StringCchPrintf(
                new_window_text,
                1024,
                "%s | FPS: %I64d",
                old_window_text,
                fps_counter.QuadPart
            );
            SetWindowText(hwnd, new_window_text);

            seconds.QuadPart -= freq.QuadPart;
            fps_counter.QuadPart = 0;
        }
#endif

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
        {
            if (msg.message == WM_QUIT) { goto exit; }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        while (lag >= ms_per_frame)
        {
            game_main(&g_platform.pd);

            lag -= ms_per_frame;
        }

        render(
            g_platform.window.device_context,
            g_platform.window.width,
            g_platform.window.height,
            &(g_platform.backbuffer)
        );

#ifdef DEBUG_PRINT
        fps_counter.QuadPart++;
#endif
    }

early_exit:
    return -1;

exit:
    return (int)msg.wParam;
}

LRESULT CALLBACK
WndProc(HWND handle_window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    short key_state = 0;

    struct platform_data *pd = &g_platform.pd;

    switch (message)
    {
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc;
            hdc = BeginPaint(handle_window, &ps);

            EndPaint(handle_window, &ps);
        } break;

        case WM_ERASEBKGND:
        {
            result = 1;
        } break;

        case WM_MOUSEMOVE:
        {
            pd->movement.mouse_x = GET_X_LPARAM(lparam);

        } break;

        case WM_LBUTTONDOWN:
        {
            pd->movement.mouse_enabled = !pd->movement.mouse_enabled;
        } break;

        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            switch (wparam)
            {
                // A (left)
                case 0x41:
                {
                    if ((1<<31) & lparam) { pd->movement.keyboard_left = 0; }
                    else                  { pd->movement.keyboard_left = 1; }
                } break;

                // D (right)
                case 0x44:
                {
                    if ((1<<31) & lparam) { pd->movement.keyboard_right = 0; }
                    else                  { pd->movement.keyboard_right = 1; }
                } break;
            }

            // restart game if user presses ctrl + r
            if (wparam == 'R' && !(lparam & (1 << 30)))
            {
                key_state = GetKeyState(VK_CONTROL);
                if (key_state & (1 << 15))
                {
                    SendMessage(handle_window, WM_RESTART_GAME, 0, 0);
                }
            }
        } break;
        
        case WM_RESTART_GAME:
        {
            memset(&pd->movement, 0, sizeof(pd->movement));
            game_initialize();

            RedrawWindow(handle_window, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        } break;

        case WM_SIZE:
        {
            g_platform.window.width = LOWORD(lparam);
            g_platform.window.height = HIWORD(lparam);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        default:
        {
            return DefWindowProc(handle_window, message, wparam, lparam);
        } break;
    }

    return result;
}

void
request_window_dimensions(int *width, int *height)
{
    struct backbuffer const * const buffer = get_backbuffer();

    *width = buffer->width;
    *height = buffer->height;
}

int
set_background(int color)
{
    struct backbuffer * const buffer = get_backbuffer();

    buffer->background_color = (COLORREF) color;

    return 1;
}

int
paint_background(void)
{
    struct backbuffer * const buffer = get_backbuffer();

    RECT back_buffer;
    back_buffer.left   = 0;
    back_buffer.top    = 0;
    back_buffer.right  = buffer->width;
    back_buffer.bottom = buffer->height;

    HBRUSH brush = CreateSolidBrush(buffer->background_color);

    FillRect(
        buffer->device_context,
        &back_buffer, 
        brush
    );

    DeleteObject(brush);

    return 1;
}

int
draw_filled_rect(int x, int y, int width, int height, int color)
{
    struct backbuffer * const buffer = get_backbuffer();

    HBRUSH fill_brush = CreateSolidBrush((COLORREF)color);

    RECT r;
    r.left   = x;
    r.top    = y;
    r.right  = x + width;
    r.bottom = y + height;

    FillRect(
        buffer->device_context,
        &r,
        fill_brush
    );

    DeleteObject(fill_brush);

    return 1;
}

int
draw_rect_frame(int x, int y, int width, int height, int color)
{
    struct backbuffer * const buffer = get_backbuffer();

    HBRUSH tint_brush = CreateSolidBrush((COLORREF)color);

    RECT r;
    r.left   = x;
    r.top    = y;
    r.right  = x + width;
    r.bottom = y + height;

    FrameRect(
        buffer->device_context,
        &r,
        tint_brush
    );

    DeleteObject(tint_brush);

    return 1;
}

int
remove_rectangle(int x, int y, int width, int height)
{
    struct backbuffer * const buffer = get_backbuffer();

    RECT r;
    r.left = x;
    r.top  = y;
    r.right = x + width;
    r.bottom = y + height;

    HBRUSH brush = CreateSolidBrush(buffer->background_color);
    FillRect(
        buffer->device_context,
        &r,
        brush
    );

    DeleteObject(brush);

    return 1;
}

int
draw_circle(int x, int y, int width, int height, int fill, int tint)
{
    struct backbuffer * const buffer = get_backbuffer();

    HBRUSH fill_brush = CreateSolidBrush((COLORREF)fill);
    HPEN old_brush = SelectObject(buffer->device_context, fill_brush);

    HPEN tint_pen   = CreatePen(PS_SOLID, 1, (COLORREF)tint);
    HPEN old_pen = SelectObject(buffer->device_context, tint_pen);

    Ellipse(
        buffer->device_context,
        x,
        y,
        x + width,
        y + height 
    );

    DeleteObject(SelectObject(buffer->device_context, old_brush));
    DeleteObject(SelectObject(buffer->device_context, old_pen));

    return 1;
}

int
put_text(int x, int y, char const * const text, size_t length)
{
    struct backbuffer * const buffer = get_backbuffer();

    SetTextAlign(
        buffer->device_context,
        TA_CENTER
    );

    return (int)TextOutA(
                buffer->device_context,
                x, y,
                text, (int)length
            );
}

void
request_text_dimension(int * const width, int * const height)
{
    struct backbuffer * const buffer = get_backbuffer();

    TEXTMETRICA tm = {0};

    GetTextMetricsA(
        buffer->device_context,
        &tm
    );

    *width = tm.tmAveCharWidth;
    *height = tm.tmHeight;
}
