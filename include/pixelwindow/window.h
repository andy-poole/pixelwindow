
/**
 * PixelWindow - A simple callback-based window abstraction.
 *
 * Copyright (C) 2025 Andy Poole
 *
 * This code is licensed under the MIT licence. Please see
 * LICENSE.md in http://www.github.com/andy-poole/pixelwindow
 */

#ifndef AP_PIXELWINDOW_H
#define AP_PIXELWINDOW_H

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)

// Remove unnecessary functionality
// from the windows.h header file
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

// Enable strict Windows type checks
#ifndef STRICT
#define STRICT
#endif // STRICT

// Removes the awful min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <timeapi.h>

#pragma comment(lib, "winmm.lib")

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

namespace ap {

/**
 * Keyboard key enumerations
 */
enum class Key
{
    NONE,
    ENTER,
    ESC,
    SPACE,
    LEFT,
    UP,
    RIGHT,
    DOWN,
    NUM0,
    NUM1,
    NUM2,
    NUM3,
    NUM4,
    NUM5,
    NUM6,
    NUM7,
    NUM8,
    NUM9,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
};

/**
 * Mouse button enumerations
 */
enum class Mouse
{
    NONE,
    LEFT,
    MIDDLE,
    RIGHT,
    X1,
    X2,
};

/**
 * 2D Point with x and y coordinates
 */
struct Point
{
    int x;
    int y;

    constexpr bool operator==(const Point& other) const
    {
        return (x == other.x) && (y == other.y);
    }

    constexpr bool operator!=(const Point& other) const
    {
        return !(*this == other);
    }
};

/**
 * 2D Size with width and height dimensions
 */
struct Size
{
    unsigned w;
    unsigned h;

    constexpr bool operator==(const Size& other) const
    {
        return (w == other.w) && (h == other.h);
    }

    constexpr bool operator!=(const Size& other) const
    {
        return !(*this == other);
    }
};

/**
 * Rect with a top-left origin and a size
 */
struct Rect
{
    Point origin;
    Size size;

    constexpr bool operator==(const Rect& other) const
    {
        return (origin == other.origin) && (size == other.size);
    }

    constexpr bool operator!=(const Rect& other) const
    {
        return !(*this == other);
    }
};

/**
 * 32-bit RGBA Pixel structure
 *
 * The internal format of the pixel is BGRA
 * and stored in memory as [ BB GG RR AA ] on
 * on a little-endian systems.
 */
union Pixel
{
    uint32_t raw = 0;
    struct { uint8_t b, g, r, a; } bgra;

    constexpr Pixel() = default;

    constexpr Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        // Internally stored as BGRA
        raw = (uint32_t(a) << 24) |
              (uint32_t(r) << 16) |
              (uint32_t(g) <<  8) |
              (uint32_t(b));
    }

    constexpr bool operator==(const Pixel& other) const
    {
        return raw == other.raw;
    }

    constexpr bool operator!=(const Pixel& other) const
    {
        return !(*this == other);
    }
};

// Ensure no extra padding is added
static_assert(sizeof(Pixel) == 4);

/**
 * PixelWindow callback interface
 *
 * Provides callbacks that a derived class may override.
 * In addition, this class provides the basic enumerations
 * and structures that derived classes can use.
 *
 * Note that derived classes do not have to implement
 * the interfaces defined here.
 */
class IPixelWindow
{
public:
    /**
     * Version: v0.2.0
     * Format:  major=[15..12], minor=[11..8], patch[7..0]
     */
    static constexpr uint16_t VERSION = 0x0200;

    /**
     * Public IPixelWindow destructor
     */
    virtual ~IPixelWindow() = default;

protected:
    /**
     * Protected IPixelWindow constructor
     */
    IPixelWindow() = default;

    /**
     * Callback called on window creation
     *
     * This callback is called after the window was created
     * but before it is displayed. The function is called
     * with the window width and height.
     *
     * The size of the canvas can be queried in this
     * function.
     *
     * @param width Width of the window in pixels
     * @param height Height of the window in pixels
     * @return true if successful, otherwise false
     */
    virtual bool OnCreate(unsigned width, unsigned height) { (void)width; (void)height; return true; }

    /**
     * Callback called on window destroy
     *
     * This callback is called just before the window has
     * been destroyed.
     */
    virtual void OnDestroy() {}

    /**
     * Callback called when the focus changes
     *
     * A window in the foreground has focus whereas a
     * window in the background loses focus.
     *
     * @param hasFocus Focus state of the window
     */
    virtual void OnFocusChanged(bool hasFocus) { (void)hasFocus; }

    /**
     * Callback called when a file is dropped
     *
     * When a user drags and drops a file onto the window,
     * this function will be called with the path.
     *
     * Note: This is Windows-only
     *
     * @param path Path of the dropped file
     */
    virtual void OnDropFile(const std::string& path) { (void)path; }

    /**
     * Callback called on a frame update
     *
     * This function is called in the message loop when
     * the canvas's contents should be updated. Use this
     * this function to "render" any graphical content to
     * the canvas by using the SetPixel and DrawRect
     * functions.
     *
     * This function is called as close to the update rate
     * as possible. The actual amount of time elapsed since
     * the last call to this function is passed as a
     * parameter.
     *
     * Returning true from this function signals that the
     * canvas has been updated and should be presented to
     * the window. Returning false from this function
     * signals that the window should not be updated from
     * the canvas.
     *
     * @param millis Time since last call in milliseconds
     * @return true if the canvas should be presented
     */
    virtual bool OnUpdateFrame(double millis) { (void)millis; return false; }

    /**
     * Callback called on a mouse button event
     *
     * This function is called synchronously as part of the
     * window's message loop when checking window mouse
     * events - it will not be called asynchronously and
     * interrupt another callback.
     *
     * The function is passed a boolean to signal whether
     * the key was pressed (true) or released (false). The
     * window coordinates of mouse's location is also
     * passed to the function. Please note these are
     * window coordinates and not the coordinates on the
     * canvas.
     *
     * @param btn Button event enumeration
     * @param x x-coordinate of the button event
     * @param y y-coordinate of the button event
     * @param down true if pressed, false if released
     */
    virtual void OnMouseClick(Mouse btn, int x, int y, bool down) { (void)btn; (void)x; (void)y; (void)down; }

    /**
     * Callback called on a keyboard key event
     *
     * This function is called synchronously as part of the
     * window's message loop when checking window key
     * events - it will not be called asynchronously and
     * interrupt another callback.
     *
     * The function is passed a boolean to signal whether
     * the key was pressed (true) or released (false).
     *
     * @param key Key event enumeration
     * @param down true if pressed, false if released
     */
    virtual void OnKeyPress(Key key, bool down) { (void)key; (void)down; }

    /**
     * Callback called on window resize event
     *
     * This function is called synchronously as part of the
     * window's message loop when checking window resize
     * events - it will not be called asynchronously and
     * interrupt another callback.
     *
     * The function is passed the new window size.
     *
     * @param width Width of the window in pixels
     * @param height Height of the window in pixels
     */
    virtual void OnResize(unsigned width, unsigned height) { (void)width; (void)height; }
};

/**
 * PixelWindowBase0 abstract class
 *
 * This class provides common utility functionality
 * for the PixelWindowBase class.
 */
class PixelWindowBase0
{
public:
    /**
     * Public PixelWindowBase0 destructor
     */
    ~PixelWindowBase0() = default;

protected:
    /**
     * Protected PixelWindowBase0 constructor
     */
    PixelWindowBase0() = default;

    /**
     * Returns the size and position for where to blit a
     * source area to a destination area. The source area
     * will be scaled so that it fits inside the destination
     * and maintains its aspect ratio. It will also be
     * positioned central to the destination area by adding
     * horizontal or vertical bars (as appropriate).
     *
     * @param srcSize Size of the src rectangle
     * @param dstSize Size of the dst rectangle
     * @return Rectangle of the scaled area's size and position
     */
    static Rect GetScaledRect(const Size& srcSize, const Size& dstSize)
    {
        // Fast exit for edge cases
        if (srcSize.w == 0 || srcSize.h == 0 ||
            dstSize.w == 0 || dstSize.h == 0) {
            return {};
        }

        const double sx = static_cast<double>(dstSize.w) / srcSize.w;
        const double sy = static_cast<double>(dstSize.h) / srcSize.h;

        // Use minimum scale to preserve aspect ratio
        const double scale = (sx < sy) ? sx : sy;

        // Calculate final dimensions size
        const unsigned width  = static_cast<unsigned>(srcSize.w * scale);
        const unsigned height = static_cast<unsigned>(srcSize.h * scale);

        Rect rect = {};
        rect.size.w = width;
        rect.size.h = height;
        rect.origin.x = static_cast<int>(dstSize.w - width) / 2;
        rect.origin.y = static_cast<int>(dstSize.h - height) / 2;

        return rect;
    }
};

#if defined(_WIN32)

/**
 * PixelWindowBase abstract class
 *
 * The PixelWindowBase class provides the base, and the
 * implementation-specific functionality for the
 * PixelWindow class.
 *
 * This base class is implemented specifically to use the
 * Win32 API for the Windows operating system
 */
template <typename Derived>
class PixelWindowBase : public IPixelWindow, private PixelWindowBase0
{
    // Allow the Derived class to access private data of
    // this class to prevent further derived classes from
    // accessing unnecessary private state.
    // Making internal state 'protected' would allow all
    // subclasses access to the implementation details.
    friend Derived;

public:
    /**
     * Win32-specific native window handle.
     */
    struct NativeHandle {
        HINSTANCE hinstance;
        HWND hwnd;
    };

    /**
     * Public PixelWindowBase destructor
     */
    ~PixelWindowBase() override = default;

protected:
    /**
     * Protected PixelWindowBase constructor
     */
    PixelWindowBase() = default;

private:
    static constexpr std::array<Key,256> BuildKeyMap()
    {
        // Compile-time construction of
        // the keyboard key map consisting
        // of 8-bit Win32 Virtual Key Codes
        std::array<Key, 256> map = {};
        map[0x0d] = Key::ENTER;
        map[0x1b] = Key::ESC;
        map[0x20] = Key::SPACE;
        map[0x25] = Key::LEFT;
        map[0x26] = Key::UP;
        map[0x27] = Key::RIGHT;
        map[0x28] = Key::DOWN;
        map[0x30] = Key::NUM0;
        map[0x31] = Key::NUM1;
        map[0x32] = Key::NUM2;
        map[0x33] = Key::NUM3;
        map[0x34] = Key::NUM4;
        map[0x35] = Key::NUM5;
        map[0x36] = Key::NUM6;
        map[0x37] = Key::NUM7;
        map[0x38] = Key::NUM8;
        map[0x39] = Key::NUM9;
        map[0x41] = Key::A;
        map[0x42] = Key::B;
        map[0x43] = Key::C;
        map[0x44] = Key::D;
        map[0x45] = Key::E;
        map[0x46] = Key::F;
        map[0x47] = Key::G;
        map[0x48] = Key::H;
        map[0x49] = Key::I;
        map[0x4a] = Key::J;
        map[0x4b] = Key::K;
        map[0x4c] = Key::L;
        map[0x4d] = Key::M;
        map[0x4e] = Key::N;
        map[0x4f] = Key::O;
        map[0x50] = Key::P;
        map[0x51] = Key::Q;
        map[0x52] = Key::R;
        map[0x53] = Key::S;
        map[0x54] = Key::T;
        map[0x55] = Key::U;
        map[0x56] = Key::V;
        map[0x57] = Key::W;
        map[0x58] = Key::X;
        map[0x59] = Key::Y;
        map[0x5a] = Key::Z;
        return map;
    }

    void WindowTitleUpdated(double fps, bool showFps)
    {
        if (hwnd_) {
            char str[256] = {};
            if (showFps) {
                snprintf(str, sizeof(str), "%s (fps: %.1f)", windowTitle_.c_str(), fps);
            } else {
                snprintf(str, sizeof(str), "%s", windowTitle_.c_str());
            }
            SetWindowTextA(hwnd_, str);
        }
    }

    void ClearWindow()
    {
        if (!hwnd_ || !hdc_) {
            return;
        }

        // Clear areas of the window not covered by the canvas
        const RECT rect = {
            0, 0,
            static_cast<LONG>(windowSize_.w),
            static_cast<LONG>(windowSize_.h)
        };

        FillRect(hdc_, &rect, hbrush_);
    }

    void WindowSizeUpdated()
    {
        // Clear areas of the window not covered by the canvas
        ClearWindow();

        // Cache for blitting the canvas to window
        scaledCanvasRect_ = GetScaledRect(canvasSize_, windowSize_);
    }

    void CanvasSizeUpdated()
    {
        // Clear areas of the window not covered by the canvas
        ClearWindow();

        auto pixels = size_t(canvasSize_.w) * size_t(canvasSize_.h);
        canvasData_.resize(pixels);

        canvasBitmapInfo_ = {};
        BITMAPINFOHEADER& h = canvasBitmapInfo_.bmiHeader;
        h.biSize = sizeof(BITMAPINFOHEADER);
        h.biWidth = static_cast<LONG>(canvasSize_.w);
        h.biHeight = -static_cast<LONG>(canvasSize_.h); // -ve for top-left origin
        h.biPlanes = 1;
        h.biBitCount = sizeof(Pixel) * 8;
        h.biCompression = BI_RGB;

        // Cache for blitting the canvas to window
        scaledCanvasRect_ = GetScaledRect(canvasSize_, windowSize_);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Will be null before WM_CREATE message
        auto window = reinterpret_cast<PixelWindowBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!window && msg != WM_CREATE) {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
            return 0;
        }
        case WM_DESTROY: {
            // Window being destroyed: call callback
            window->OnDestroy();
            ReleaseDC(hwnd, window->hdc_);
            PostQuitMessage(0); // sends WM_QUIT
            return 0;
        }
        case WM_SIZE: {
            if (wParam == SIZE_MINIMIZED) {
                return 0;
            }
            window->windowSize_ = { LOWORD(lParam), HIWORD(lParam) };
            window->WindowSizeUpdated();
            window->OnResize(window->windowSize_.w, window->windowSize_.h);
            return 0;
        }
        case WM_DROPFILES: {
            // +1 for null terminator
            const auto size = DragQueryFileA((HDROP)wParam, 0, nullptr, 0) + 1;
            auto buf = std::vector<CHAR>(size);
            DragQueryFileA((HDROP)wParam, 0, buf.data(), size);

            window->OnDropFile(std::string(buf.data()));
            DragFinish((HDROP)wParam);
            return 0;
        }
        case WM_SETFOCUS: {
            window->OnFocusChanged(true);
            return 0;
        }
        case WM_KILLFOCUS: {
            window->OnFocusChanged(false);
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const auto idx = wParam;
            window->OnKeyPress((idx < window->keyMap_.size()) ? window->keyMap_[idx] : Key::NONE, true);
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const auto idx = wParam;
            window->OnKeyPress((idx < window->keyMap_.size()) ? window->keyMap_[idx] : Key::NONE, false);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(Mouse::LEFT, x, y, true);
            return 0;
        }
        case WM_LBUTTONUP: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(Mouse::LEFT, x, y, false);
            return 0;
        }
        case WM_MBUTTONDOWN: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(Mouse::MIDDLE, x, y, true);
            return 0;
        }
        case WM_MBUTTONUP: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(Mouse::MIDDLE, x, y, false);
            return 0;
        }
        case WM_RBUTTONDOWN: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(Mouse::RIGHT, x, y, true);
            return 0;
        }
        case WM_RBUTTONUP: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(Mouse::RIGHT, x, y, false);
            return 0;
        }
        case WM_XBUTTONDOWN: {
            const auto btn = (HIWORD(wParam) == XBUTTON1) ? Mouse::X1 : Mouse::X2;
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(btn, x, y, true);
            return 0;
        }
        case WM_XBUTTONUP: {
            const auto btn = (HIWORD(wParam) == XBUTTON1) ? Mouse::X1 : Mouse::X2;
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            window->OnMouseClick(btn, x, y, false);
            return 0;
        }
        default:
            break;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    bool BeginMainLoop()
    {
        // Set the resolution of the Windows
        // timer to 1ms
        timeBeginPeriod(1);

        // Must be destroyed with DeleteObject
        hbrush_ = CreateSolidBrush(RGB(0, 0, 0));

        wndclass_.style = CS_OWNDC;
        wndclass_.lpfnWndProc = WndProc;
        wndclass_.cbClsExtra = 0;
        wndclass_.cbWndExtra = 0;
        wndclass_.hInstance = GetModuleHandle(nullptr);
        wndclass_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wndclass_.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndclass_.hbrBackground = hbrush_;
        wndclass_.lpszMenuName = nullptr;
        wndclass_.lpszClassName = "pixelwindow_class";

        if (RegisterClassA(&wndclass_) == 0) {
            return false;
        };

        DWORD dwStyle = WS_OVERLAPPEDWINDOW;

        // Window size accounting for the style
        // (includes title bar and borders)
        RECT windowRect = { 0, 0, (LONG)windowSize_.w, (LONG)windowSize_.h };
        AdjustWindowRect(&windowRect, dwStyle, false);

        hwnd_ = CreateWindowA(
            wndclass_.lpszClassName,
            windowTitle_.c_str(),
            dwStyle,
            CW_USEDEFAULT, // x
            CW_USEDEFAULT, // y
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, // parent
            nullptr, // menu
            GetModuleHandle(nullptr),
            this // lpParam
        );

        if (!hwnd_) {
            return false;
        }

        DragAcceptFiles(hwnd_, true);

        // Device/Graphics context for the window
        hdc_ = GetDC(hwnd_);

        WindowSizeUpdated();
        CanvasSizeUpdated();

        native_ = {};
        native_.hinstance = GetModuleHandle(nullptr);
        native_.hwnd = hwnd_;

        // Window created: call callback
        if (!OnCreate(windowSize_.w, windowSize_.h)) {
            DestroyWindow(hwnd_);
            return false;
        }

        ShowWindow(hwnd_, SW_SHOWNORMAL);

        return true;
    }

    bool HandleMessages()
    {
        bool quit = false;

        // Handle queued messages
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                quit = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return !quit;
    }

    void WaitForVSync()
    {
        // TODO
    }

    void Present(const Rect& dstRect)
    {
        if (!hwnd_ || !hdc_ || !dstRect.size.w || !dstRect.size.h) {
            return;
        }

        // Windows can perform the scaling, so we don't
        // need additional scaled canvas data or size

        StretchDIBits(
            hdc_,
            dstRect.origin.x, // dest x
            dstRect.origin.y, // dest y
            dstRect.size.w,   // dest width
            dstRect.size.h,   // dest height
            0, 0,             // src x,y
            canvasSize_.w,    // src width
            canvasSize_.h,    // src height
            canvasData_.data(),
            &canvasBitmapInfo_,
            DIB_RGB_COLORS,
            SRCCOPY
        );

        // Block until the compositor has finished
        // compositing this frame.
        WaitForVSync();
    }

    void EndMainLoop()
    {
        // Window handle no longer valid
        native_ = {};

        // DC already destroyed
        hdc_ = {};

        // Window already destroyed
        hwnd_ = {};

        // Unregister the window class
        UnregisterClassA("pixelwindow_class", GetModuleHandle(nullptr));

        // Free the brush object
        DeleteObject(hbrush_);
        hbrush_ = {};

        // Restore timer resolution
        timeEndPeriod(1);
    }

    HBRUSH hbrush_ = {};
    WNDCLASSA wndclass_ = {};
    HWND hwnd_ = {};
    HDC hdc_ = {};

    static constexpr auto keyMap_ = BuildKeyMap();

    std::string windowTitle_;
    Size windowSize_ = {};
    NativeHandle native_ = {};

    std::chrono::duration<double> updateInterval_ = {}; // 0: run as fast as possible

    std::vector<Pixel> canvasData_ = {};
    Size canvasSize_ = {};
    BITMAPINFO canvasBitmapInfo_ = {};

    Rect scaledCanvasRect_ = {}; // in window coordinates
};

#elif defined(__linux__)

/**
 * PixelWindowBase abstract class
 *
 * The PixelWindowBase class provides the base, and the
 * implementation-specific functionality for the
 * PixelWindow class.
 *
 * This base class is implemented specifically to use the
 * XLib API for the Linux operating system
 */
template <typename Derived>
class PixelWindowBase : public IPixelWindow, private PixelWindowBase0
{
    // Allow the Derived class to access private data of
    // this class to prevent further derived classes from
    // accessing unnecessary private state.
    // Making internal state 'protected' would allow all
    // subclasses access to the implementation details.
    friend Derived;

public:
    /**
     * Xlib-specific native window handle.
     */
    struct NativeHandle {
        Display* display;
        Window window;
    };

    /**
     * Public PixelWindowBase destructor
     */
    ~PixelWindowBase() override = default;

protected:
    /**
     * Protected PixelWindowBase constructor
     */
    PixelWindowBase() = default;

private:
    static constexpr std::array<Key,256> BuildCtrlKeyMap()
    {
        // Compile-time construction of
        // the keyboard key map consisting
        // of 16-bit control key symbols that
        // all start with 0xff.
        // We only store the lower byte
        std::array<Key, 256> map = {};
        map[XK_Return & 0xff] = Key::ENTER;
        map[XK_Escape & 0xff] = Key::ESC;
        map[XK_Left & 0xff] = Key::LEFT;
        map[XK_Up & 0xff] = Key::UP;
        map[XK_Right & 0xff] = Key::RIGHT;
        map[XK_Down & 0xff] = Key::DOWN;
        map[XK_KP_Enter & 0xff] = Key::ENTER;
        map[XK_KP_Left & 0xff] = Key::LEFT;
        map[XK_KP_Up & 0xff] = Key::UP;
        map[XK_KP_Right & 0xff] = Key::RIGHT;
        map[XK_KP_Down & 0xff] = Key::DOWN;
        return map;
    }

    static constexpr std::array<Key,256> BuildKeyMap()
    {
        // Compile-time construction of
        // the keyboard key map consisting
        // of 16-bit key symbols that
        // all start with 0x00.
        // We only store the lower byte
        std::array<Key, 256> map = {};
        map[XK_space] = Key::SPACE;
        map[XK_0] = Key::NUM0;
        map[XK_1] = Key::NUM1;
        map[XK_2] = Key::NUM2;
        map[XK_3] = Key::NUM3;
        map[XK_4] = Key::NUM4;
        map[XK_5] = Key::NUM5;
        map[XK_6] = Key::NUM6;
        map[XK_7] = Key::NUM7;
        map[XK_8] = Key::NUM8;
        map[XK_9] = Key::NUM9;
        map[XK_a] = Key::A;
        map[XK_b] = Key::B;
        map[XK_c] = Key::C;
        map[XK_d] = Key::D;
        map[XK_e] = Key::E;
        map[XK_f] = Key::F;
        map[XK_g] = Key::G;
        map[XK_h] = Key::H;
        map[XK_i] = Key::I;
        map[XK_j] = Key::J;
        map[XK_k] = Key::K;
        map[XK_l] = Key::L;
        map[XK_m] = Key::M;
        map[XK_n] = Key::N;
        map[XK_o] = Key::O;
        map[XK_p] = Key::P;
        map[XK_q] = Key::Q;
        map[XK_r] = Key::R;
        map[XK_s] = Key::S;
        map[XK_t] = Key::T;
        map[XK_u] = Key::U;
        map[XK_v] = Key::V;
        map[XK_w] = Key::W;
        map[XK_x] = Key::X;
        map[XK_y] = Key::Y;
        map[XK_z] = Key::Z;
        return map;
    }

    static constexpr std::array<Mouse, 8> BuildMouseMap()
    {
        // Compile-time construction of
        // the mouse button map consisting
        // of X11/Xlib Button codes.
        std::array<Mouse, 8> map = {};
        map[Button1] = Mouse::LEFT;
        map[Button2] = Mouse::MIDDLE;
        map[Button3] = Mouse::RIGHT;
        return map;
    }

    void WindowTitleUpdated(double fps, bool showFps)
    {
        if (display_ && window_ != 0) {
            char str[256] = {};
            if (showFps) {
                snprintf(str, sizeof(str), "%s (fps: %.1f)", windowTitle_.c_str(), fps);
            } else {
                snprintf(str, sizeof(str), "%s", windowTitle_.c_str());
            }
            XStoreName(display_, window_, str);
        }
    }

    void ClearWindow()
    {
        if (!display_ || window_ == 0) {
            return;
        }

        XClearWindow(display_, window_);
    }

    void WindowSizeUpdated()
    {
        // Clear areas of the window not covered by the canvas
        ClearWindow();

        // Cache for blitting the canvas to window
        scaledCanvasRect_ = GetScaledRect(canvasSize_, windowSize_);
    }

    void CanvasSizeUpdated()
    {
        // Clear areas of the window not covered by the canvas
        ClearWindow();

        auto pixels = size_t(canvasSize_.w) * size_t(canvasSize_.h);
        canvasData_.resize(pixels);

        // Cache for blitting the canvas to window
        scaledCanvasRect_ = GetScaledRect(canvasSize_, windowSize_);
    }

    bool BeginMainLoop()
    {
        display_ = XOpenDisplay(nullptr);
        if (!display_) {
            return false;
        }

        screen_ = DefaultScreen(display_);
        visual_ = XDefaultVisual(display_, screen_);

        const unsigned black = BlackPixel(display_, screen_);

        XSetWindowAttributes attributes = {};
        attributes.background_pixel = black;
        attributes.border_pixel = black;
        attributes.event_mask =
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask |
            FocusChangeMask |
            StructureNotifyMask;

        window_ = XCreateWindow(
            display_,
            DefaultRootWindow(display_),
            0, // x
            0, // y
            windowSize_.w,
            windowSize_.h,
            0, // border size
            DefaultDepth(display_, screen_), // depth
            InputOutput,
            visual_,
            CWBackPixel | CWBorderPixel | CWEventMask,
            &attributes
        );

        if (window_ == 0) {
            return false;
        }

        XStoreName(display_, window_, windowTitle_.c_str());

        // Register a client event to respond
        // to when the window is closed
        WM_DELETE_WINDOW = XInternAtom(display_, "WM_DELETE_WINDOW", 0);
        XSetWMProtocols(display_, window_, &WM_DELETE_WINDOW, 1);

        WindowSizeUpdated();
        CanvasSizeUpdated();

        native_= {};
        native_.display = display_;
        native_.window = window_;

        // Window created: call callback
        if (!OnCreate(windowSize_.w, windowSize_.h)) {
            return false;
        }

        XMapWindow(display_, window_);
        XFlush(display_);

        return true;
    }

    bool HandleMessages()
    {
        bool quit = false;

        while (XPending(display_)) {
            XEvent event;
            XNextEvent(display_, &event);

            // Client Events
            if (event.type == ClientMessage) {
                Atom protocol = event.xclient.data.l[0];
                if(protocol == WM_DELETE_WINDOW) {
                    quit = true;
                    break;
                }
                continue;
            }

            // Input Events
            switch (event.type) {
            case ConfigureNotify: {
                const auto w = static_cast<unsigned>(event.xconfigure.width);
                const auto h = static_cast<unsigned>(event.xconfigure.height);
                if (w != windowSize_.w || h != windowSize_.h) {
                    windowSize_.w = w;
                    windowSize_.h = h;
                    WindowSizeUpdated();
                    OnResize(windowSize_.w, windowSize_.h);
                }
                break;
            }
            case FocusIn: {
                OnFocusChanged(true);
                break;
            }
            case FocusOut: {
                OnFocusChanged(false);
                break;
            }
            case KeyPress: {
                const KeySym sym = XLookupKeysym(&event.xkey, 0);
                const size_t idx = sym & 0xff;
                if ((sym & 0xff00) == 0xff00) {
                    OnKeyPress((idx < ctrlkeyMap_.size()) ? ctrlkeyMap_[idx] : Key::NONE, true);
                } else {
                    OnKeyPress((idx < keyMap_.size()) ? keyMap_[idx] : Key::NONE, true);
                }
                break;
            }
            case KeyRelease: {
                // X11's auto-repeat always sends a KeyRelease
                // after a KeyPress event:
                //    KeyPress
                //    KeyRelease
                //    KeyPress
                //    KeyRelease
                // To avoid the extra KeyRelease we ignore
                // this event when we detect auto-repeat
                if (XEventsQueued(display_, QueuedAfterReading)) {
                    XEvent next;
                    XPeekEvent(display_, &next);

                    // Detect auto-repeat by checking the
                    // timestamp of the next KeyPress event
                    if (next.type == KeyPress &&
                        next.xkey.keycode == event.xkey.keycode &&
                        next.xkey.time == event.xkey.time) {
                        // Don't process this event
                        break;
                    }
                }
                const KeySym sym = XLookupKeysym(&event.xkey, 0);
                const size_t idx = sym & 0xff;
                if ((sym & 0xff00) == 0xff00) {
                    OnKeyPress((idx < ctrlkeyMap_.size()) ? ctrlkeyMap_[idx] : Key::NONE, false);
                } else {
                    OnKeyPress((idx < keyMap_.size()) ? keyMap_[idx] : Key::NONE, false);
                }
                break;
            }
            case ButtonPress: {
                const unsigned idx = event.xbutton.button;
                const int x = event.xbutton.x;
                const int y = event.xbutton.y;
                OnMouseClick(idx < mouseMap_.size() ? mouseMap_[idx] : Mouse::NONE, x, y, true);
                break;
            }
            case ButtonRelease: {
                const unsigned idx = event.xbutton.button;
                const int x = event.xbutton.x;
                const int y = event.xbutton.y;
                OnMouseClick(idx < mouseMap_.size() ? mouseMap_[idx] : Mouse::NONE, x, y, false);
                break;
            }
            default:
                break;
            }
        }

        return !quit;
    }

    /**
     * Scales image data using nearest neighbour filtering
     * TODO: Replace with XRender solution
     */
    static void ScaleNearest(const std::vector<Pixel>& srcData, const Size& srcSize,
                             std::vector<Pixel>& dstData, const Size& dstSize)
    {
        for (unsigned dy = 0; dy < dstSize.h; ++dy) {
            unsigned sy = (dy * srcSize.h) / dstSize.h;
            for (unsigned dx = 0; dx < dstSize.w; ++dx) {
                unsigned sx = (dx * srcSize.w) / dstSize.w;
                dstData[(dy * dstSize.w) + dx] = srcData[(sy * srcSize.w) + sx];
            }
        }
    }

    void WaitForVSync()
    {
        // TODO
    }

    void Present(const Rect& dstRect)
    {
        if (!display_ || window_ == 0 || !dstRect.size.w || !dstRect.size.h) {
            return;
        }

        // If the destination (presented) rectangle is
        // different to the scaled canvas, we need to update
        // the scaled canvas's data buffer and XImage
        if (dstRect.size != scaledCanvasSize_) {
            scaledCanvasSize_ = dstRect.size;

            auto pixels = size_t(scaledCanvasSize_.w) * size_t(scaledCanvasSize_.h);
            scaledCanvasData_.resize(pixels);

            if (scaledCanvasImageInfo_) {
                // The XImage doesn't own the data
                scaledCanvasImageInfo_->data = nullptr;
                XDestroyImage(scaledCanvasImageInfo_);
            }

            scaledCanvasImageInfo_ = XCreateImage(
                display_,
                visual_,
                DefaultDepth(display_, screen_),
                ZPixmap, // uses 4 bytes per pixel
                0,
                reinterpret_cast<char*>(scaledCanvasData_.data()),
                scaledCanvasSize_.w,
                scaledCanvasSize_.h,
                32, 0
            );
        }

        // Scale the canvas using "nearest" filtering
        ScaleNearest(canvasData_, canvasSize_, scaledCanvasData_, scaledCanvasSize_);

        XPutImage(
            display_,
            window_,
            DefaultGC(display_, screen_),
            scaledCanvasImageInfo_,
            0, 0,
            dstRect.origin.x,
            dstRect.origin.y,
            dstRect.size.w,
            dstRect.size.h
        );
        XFlush(display_);

        // Block until the compositor has finished
        // compositing this frame.
        WaitForVSync();
    }

    void EndMainLoop()
    {
        if (display_ && window_ != 0) {
            // Hide the window before OnDestroy
            XUnmapWindow(display_, window_);
            XFlush(display_);
        }

        // Window being destroyed: call callback
        OnDestroy();

        // Free only if canvas created
        if (scaledCanvasImageInfo_) {
            // The XImage doesn't own the data
            scaledCanvasImageInfo_->data = nullptr;
            XDestroyImage(scaledCanvasImageInfo_);

            scaledCanvasImageInfo_ = nullptr;
        }

        // Window handle no longer valid
        native_ = {};

        // Now destroy the window
        if (display_ && window_ != 0) {
            XDestroyWindow(display_, window_);
            window_ = 0;
        }

        // Now close the display connection
        if (display_) {
            XCloseDisplay(display_);
            display_ = {};
        }
    }

    Display* display_ = nullptr;
    int screen_ = 0;
    Visual* visual_ = nullptr;
    Window window_ = 0;
    Atom WM_DELETE_WINDOW = {};

    static constexpr auto ctrlkeyMap_ = BuildCtrlKeyMap();
    static constexpr auto keyMap_ = BuildKeyMap();
    static constexpr auto mouseMap_ = BuildMouseMap();

    std::string windowTitle_;
    Size windowSize_ = {};
    NativeHandle native_ = {};

    std::chrono::duration<double> updateInterval_ = {}; // 0: run as fast as possible

    std::vector<Pixel> canvasData_ = {};
    Size canvasSize_ = {};

    std::vector<Pixel> scaledCanvasData_ = {};
    Size scaledCanvasSize_ = {};
    XImage* scaledCanvasImageInfo_ = nullptr;

    Rect scaledCanvasRect_ = {}; // in window coordinates
};

#endif

/**
 * PixelWindow abstract class
 */
class PixelWindow : public PixelWindowBase<PixelWindow>
{
public:
    static constexpr Pixel COLOR_NONE    = Pixel(  0,   0,   0,   0);
    static constexpr Pixel COLOR_BLACK   = Pixel(  0,   0,   0, 255);
    static constexpr Pixel COLOR_GREY    = Pixel(128, 128, 128, 255);
    static constexpr Pixel COLOR_WHITE   = Pixel(255, 255, 255, 255);
    static constexpr Pixel COLOR_RED     = Pixel(255,   0,   0, 255);
    static constexpr Pixel COLOR_GREEN   = Pixel(  0, 255,   0, 255);
    static constexpr Pixel COLOR_BLUE    = Pixel(  0,   0, 255, 255);
    static constexpr Pixel COLOR_CYAN    = Pixel(  0, 255, 255, 255);
    static constexpr Pixel COLOR_MAGENTA = Pixel(255,   0, 255, 255);
    static constexpr Pixel COLOR_YELLOW  = Pixel(255, 255,   0, 255);
    static constexpr Pixel COLOR_ORANGE  = Pixel(255, 127,   0, 255);
    static constexpr Pixel COLOR_PURPLE  = Pixel(127,   0, 255, 255);

    /**
     * Public PixelWindow destructor
     */
    ~PixelWindow() override = default;

    /**
     * Set the title string of the Window
     *
     * @param title Title text string to display
     */
    void SetWindowTitle(const std::string& title)
    {
        windowTitle_ = title;
        WindowTitleUpdated(0.0, false);
    }

    /**
     * Set OnUpdateFrame callback update interval
     *
     * This is the amount of time between frame updates.
     * The OnUpdateFrame will be called as close to
     * this value as possible; however, the actual timing
     * can vary.
     *
     * @param ms The update interval in milliseconds
     */
    void SetUpdateInterval(double ms)
    {
        updateInterval_ = std::chrono::duration<double>(ms / 1000.0);
    }

    /**
     * Create and displays the window on screen
     *
     * Calling this function will result in creating a
     * window on screen and running its main event loop.
     * The event loop is run on the calling thread.
     *
     * With this function, the window and canvas are
     * created with the same dimensions.
     *
     * @param width Width of the window in pixels
     * @param height Height of the window in pixels
     * @param title Title of the window as a string
     * @return true if successful, otherwise false
     */
    bool Show(unsigned width, unsigned height, const std::string& title = "")
    {
        windowSize_ = { width, height };

        // Set the canvas size to the window
        // size if it hasn't already been set
        if ((canvasSize_.w == 0) && (canvasSize_.h == 0)) {
            canvasSize_ = { width, height };
        }

        // Set the window title only if it
        // hasn't already been set
        if (windowTitle_.empty()) {
            windowTitle_ = title;
        }

        // This will block the thread
        return RunMainLoop();
    }

    /**
     * Returns a reference to the window's native handle
     *
     * @return Reference to platform-specific handle
     */
    const NativeHandle& GetNative() const
    {
        return native_;
    }

    // Disable copying
    PixelWindow(const PixelWindow&) = delete;
    PixelWindow& operator=(const PixelWindow&) = delete;

    // Disable moving
    PixelWindow(PixelWindow&&) = delete;
    PixelWindow& operator=(PixelWindow&&) = delete;

protected:
    /**
     * Protected PixelWindow constructor
     */
    PixelWindow() : PixelWindowBase() {}

    /**
     * Sets the size of the window's canvas
     *
     * @param width Width of the canvas in pixels
     * @param height Height of the canvas in pixels
     */
    void SetCanvasSize(unsigned width, unsigned height)
    {
        if ((width == canvasSize_.w) && (height == canvasSize_.h)) {
            return;
        }

        canvasSize_ = { width, height };
        CanvasSizeUpdated();
    }

    /**
     * Gets the size of the window
     *
     * @return size in pixels
     */
    const Size& GetWindowSize() const
    {
        return windowSize_;
    }

    /**
     * Gets the size of the window's canvas
     *
     * @return size in pixels
     */
    const Size& GetCanvasSize() const
    {
        return canvasSize_;
    }

    /**
     * Converts window to canvas coordinates
     *
     * When calculating the canvas coordinate, the scale
     * of the canvas is taken into account. Because the
     * canvas is positioned central to window, this
     * function can return a negative position if the
     * input coordinate is above or to the left of the
     * scaled canvas.
     *
     * @param x x-position in window coordinates
     * @param y y-position in window coordinates
     * @return pair containing canvas coordinates
     */
    std::pair<int, int> WindowPosToCanvas(int x, int y) const
    {
        // Destination rectangle of the scaled canvas in the window
        const Rect& rect = scaledCanvasRect_;
        if ((rect.size.w == 0) || (rect.size.h == 0)) {
            return {};
        }

        double sx = canvasSize_.w / static_cast<double>(rect.size.w);
        double sy = canvasSize_.h / static_cast<double>(rect.size.h);
        double cx = std::floor((x - rect.origin.x) * sx);
        double cy = std::floor((y - rect.origin.y) * sy);

        return { static_cast<int>(cx), static_cast<int>(cy) };
    }

    /**
     * Clears the window's canvas with a pixel value
     *
     * @param pixel Value used when clearing
     */
    void ClearCanvas(Pixel pixel = COLOR_NONE)
    {
        std::fill(canvasData_.begin(), canvasData_.end(), pixel);
    }

    /**
     * Sets a pixel on the window's canvas
     *
     * Note that if a pixel's alpha value is 0, it will
     * not be set.
     *
     * @param x x-position in canvas coordinates
     * @param y y-position in canvas coordinates
     * @param pixel Value to set
     * @param force Ignore alpha check if true
     */
    void SetPixel(unsigned x, unsigned y, Pixel pixel, bool force = false)
    {
        // Don't set the pixel if transparent
        if ((pixel.bgra.a == 0) && (!force)) {
            return;
        }

        const unsigned offset = (y * canvasSize_.w) + x;

        if (offset < canvasData_.size()) {
            canvasData_[offset] = pixel;
        }
    }

    /**
     * Gets a pixel on the window's canvas
     *
     * @param x x-position in canvas coordinates
     * @param y y-position in canvas coordinates
     * @return Value of the requested pixel
     */
    Pixel GetPixel(unsigned x, unsigned y) const
    {
        const unsigned offset = (y * canvasSize_.w) + x;

        Pixel pixel = {};
        if (offset < canvasData_.size()) {
            pixel = canvasData_[offset];
        }

        return pixel;
    }

    /**
     * Draws a rectangle on the window's canvas
     *
     * If the fill option is set, the rectangle will be
     * filled with a solid colour. Otherwise it will have
     * a 1 pixel (internal) border.
     *
     * The rectangle is clipped to the canvas size.
     *
     * Note that if a pixel's alpha value is 0, this
     * function has no effect.
     *
     * @param x Top-left x-position in canvas coordinates
     * @param y Top-left y-position in canvas coordinates
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param pixel Value used for the rectangle's pixel data
     * @param fill Set to true for a solid-fill
     */
    void DrawRect(unsigned x, unsigned y, unsigned w, unsigned h, Pixel pixel, bool fill = false)
    {
        // Don't set the pixel if transparent
        if (pixel.bgra.a == 0) {
            return;
        }

        // Clip to canvas size
        const unsigned xEnd = std::min(x + w, canvasSize_.w);
        const unsigned yEnd = std::min(y + h, canvasSize_.h);

        if ((x >= xEnd) || (y >= yEnd)) {
            return;
        }

        // Update w and h from clipped size
        w = xEnd - x;
        h = yEnd - y;

        // Start at top-left of rectangle
        unsigned offset = (y * canvasSize_.w) + x;

        // A border only makes sense if there is
        // at least a 3 pixel width and height
        if (!fill && w > 2 && h > 2) {
            std::fill_n(&canvasData_[offset], w, pixel);
            offset += canvasSize_.w;
            for (unsigned i = 1; i < (h - 1); ++i) {
                canvasData_[offset] = pixel;
                canvasData_[offset + w - 1] = pixel;
                offset += canvasSize_.w;
            }
            std::fill_n(&canvasData_[offset], w, pixel);
        } else {
            for (unsigned i = 0; i < h; ++i) {
                std::fill_n(&canvasData_[offset], w, pixel);
                offset += canvasSize_.w;
            }
        }
    }

    /**
     * Draws a rectangle on the window's canvas
     *
     * The rectangle is filled with the given array of
     * pixel data. It is filled one row at a time, from
     * top to bottom of the rectangle. If the array of
     * data is incomplete, the remaining pixels of the
     * rectangle will be filled with COLOR_NONE.
     *
     * The rectangle is clipped to the canvas size.
     *
     * Note that if a pixel's alpha value is 0, it will
     * not be set.
     *
     * @param x Top-left x-coordinate of the rectangle
     * @param y Top-left y-coordinate of the rectangle
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param pixels Values used for the rectangle's pixels
     */
    void DrawRect(unsigned x, unsigned y, unsigned w, unsigned h, const std::vector<Pixel>& pixels)
    {
        // Clip to canvas size
        const unsigned xEnd = std::min(x + w, canvasSize_.w);
        const unsigned yEnd = std::min(y + h, canvasSize_.h);

        if ((x >= xEnd) || (y >= yEnd)) {
            return;
        }

        // Update w and h from clipped size
        w = xEnd - x;
        h = yEnd - y;

        // Start at top-left of rectangle
        unsigned offset = (y * canvasSize_.w) + x;

        size_t p = 0;

        for (unsigned j = 0; j < h; ++j) {
            for (unsigned i = 0; i < w; ++i) {
                const auto pixel = (p < pixels.size()) ? pixels[p] : COLOR_NONE;
                ++p;

                // Don't set the pixel if transparent
                if (pixel.bgra.a != 0) {
                    canvasData_[offset + i] = pixel;
                }
            }
            offset += canvasSize_.w;
        }
    }

private:
    bool RunMainLoop()
    {
        if (!BeginMainLoop()) {
            EndMainLoop();
            return false;
        }

        using clock = std::chrono::steady_clock;

        // The time we want to next call OnUpdateFrame
        auto targetUpdateTime = clock::now();

        auto fpsTimer = targetUpdateTime;
        unsigned fpsFrameCount = 0;

        while (true) {
            // Handle window events for this iteration
            if (!HandleMessages()) {
                break;
            }

            auto now = clock::now();

            if (now >= targetUpdateTime) {
                // Calculate time since last frame update.
                // The difference between now and target
                // update time gives the drift from the
                // desired update time. Adding the
                // fixed update interval gives the time to
                // when the last OnFrameUpdate was last called.
                auto drift = now - targetUpdateTime;
                std::chrono::duration<double> update_dt = drift + updateInterval_;

                // Update and optionally present
                bool present = OnUpdateFrame(update_dt.count() * 1000.0);
                if (present) {
                    Present(scaledCanvasRect_);
                }

                // Update the time to next call OnUpdateFrame
                targetUpdateTime += std::chrono::duration_cast<clock::duration>(updateInterval_);

                // If we are running slowly due to a heavy
                // update, we need to reset the timer.
                // Reset timer if the drift is larger than the
                // fixed update interval.
                if (drift > updateInterval_) {
                    targetUpdateTime = now;
                }

                // Update the FPS counter and window title
                ++fpsFrameCount;
                auto fps_dt = std::chrono::duration<double>(now - fpsTimer);
                if (fps_dt.count() >= 0.5) {
                    double fps = fpsFrameCount / fps_dt.count();
                    WindowTitleUpdated(fps, true);
                    fpsTimer = clock::now();
                    fpsFrameCount = 0;
                }
            } else {
                // If there is more than 1ms to the target update
                // time, we can sleep for a bit to save CPU
                auto sleepUntil = targetUpdateTime - std::chrono::milliseconds(1);
                if (now < sleepUntil) {
                    std::this_thread::sleep_until(sleepUntil);
                }
            }
        }

        EndMainLoop();
        return true;
    }
};

} // namespace ap

#endif // AP_PIXELWINDOW_H
