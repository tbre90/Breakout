#ifndef BACKBUFFER_H

struct backbuffer
{
    int width;
    int height;

    HDC device_context;
    HBITMAP handle_bitmap;

    COLORREF background_color;

    int error;
};

struct backbuffer
initialize_backbuffer(HWND main_window_handle)
{
    struct backbuffer buffer;

    if (!main_window_handle)
    {
        goto early_return;
    }

    RECT client_dimensions;
    if (!GetClientRect(main_window_handle, &client_dimensions))
    {
        goto early_return;
    }

    buffer.width = client_dimensions.right - client_dimensions.left;
    buffer.height = client_dimensions.bottom - client_dimensions.top;

    // get the main windows device context
    HDC main_window_dc = GetDC(main_window_handle); 
    if (!main_window_dc) { goto early_return; }

    COLORREF wnd_bkgn = GetBkColor(main_window_dc);

    if (wnd_bkgn == CLR_INVALID)
    {
        goto early_return;
    }

    buffer.background_color = wnd_bkgn;

    // now create a compatible memory device context for our backbuffer
    buffer.device_context = CreateCompatibleDC(main_window_dc);
    if (!buffer.device_context) { goto error_release_main_window_dc; }

    // now create a bitmap so that we have something we can actually draw on
    buffer.handle_bitmap =
        CreateCompatibleBitmap(
            main_window_dc,
            buffer.width,
            buffer.height
        );
    if (!buffer.handle_bitmap) { goto error_release_main_window_dc; }

    // select bitmap into device_context
    SelectObject(buffer.device_context, buffer.handle_bitmap);

    goto success;

error_release_main_window_dc:
    ReleaseDC(main_window_handle, main_window_dc);
early_return:
    buffer.error = 1;
    return buffer;

success:
    ReleaseDC(main_window_handle, main_window_dc);
    buffer.error = 0;
    return buffer;
}

#define BACKBUFFER_H
#endif
