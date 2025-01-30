#include "Overlay.h"
#include <dwmapi.h>
#include <iostream>
#pragma comment(lib, "dwmapi.lib")

// Static member initialization
const std::vector<std::string> Overlay::KEYBOARD_ROWS = {
    "QWERTYUIOP",
    "ASDFGHJKL;",
    "ZXCVBNM,."};

const int Overlay::SQUARE_SIZE = 25;
const int Overlay::FONT_SIZE = 13;
const UINT Overlay::TOGGLE_KEY = VK_OEM_3; // Backquote
const UINT Overlay::TOGGLE_MODIFIER = MOD_ALT;

std::map<char, POINT> Overlay::keyboardPositions;
std::map<std::string, std::pair<double, double>> Overlay::pairPositions;
std::string Overlay::selectedPair;
char Overlay::lastKeyPressed = '\0';
bool Overlay::isShowingKeyboard = false;
bool Overlay::isVisible = true;
HWND Overlay::hwnd = NULL;
HFONT Overlay::gridFont = NULL;
HFONT Overlay::layoutFont = NULL;
HHOOK Overlay::keyboardHook = NULL;
UINT_PTR Overlay::clickTimer = 0;
POINT Overlay::pendingClickPos = {0, 0};
int Overlay::clickCount = 0;

bool Overlay::Initialize()
{
    // Register window class
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MouselessOverlay";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // Ensure background is black
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassExW(&wc))
    {
        return false;
    }

    // Create transparent window
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        L"MouselessOverlay",
        L"Mouseless",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (!hwnd)
    {
        return false;
    }

    // Modified transparency setup
    SetLayeredWindowAttributes(hwnd, 0, 200, LWA_ALPHA); // Use only alpha for overlap

    // Create fonts
    gridFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");

    layoutFont = CreateFontW(FONT_SIZE, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");

    // Install keyboard hook
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc,
                                    GetModuleHandle(NULL), 0);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);

    return true;
}

void Overlay::Shutdown()
{
    if (keyboardHook)
    {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }

    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = NULL;
    }

    if (gridFont)
    {
        DeleteObject(gridFont);
        gridFont = NULL;
    }

    if (layoutFont)
    {
        DeleteObject(layoutFont);
        layoutFont = NULL;
    }
}

LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER:
    {
        if (wParam == clickTimer)
        {
            KillTimer(hwnd, clickTimer);
            clickTimer = 0;

            // Log total clicks before hiding
            // Perform the click after delay
            for (int i = 0; i < clickCount; i++)
            {
                SetCursorPos(pendingClickPos.x, pendingClickPos.y);
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
            std::cout << "Total clicks performed: " << clickCount << std::endl;
            clickCount = 0; // Reset counter

            // Now hide overlay and cleanup

            // lastKeyPressed = '\0';
            ToggleVisibility();
        }
        return 0;
    }
    // case WM_ERASEBKGND:
    //     return 1; // Prevent background erasing
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

// LRESULT CALLBACK Overlay::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
// {
//     if (nCode == HC_ACTION)
//     {
//         KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
//         bool isAltPressed = ((GetAsyncKeyState(VK_LMENU) & 0x8000) != 0) ||
//         ((GetAsyncKeyState(VK_RMENU) & 0x8000) != 0);
//         std::cout << "Key pressed: " << p->vkCode << std::endl;
//         if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
//         {
//             std::cout << "Is alt pressed: " << isAltPressed << std::endl;
//             if (p->vkCode == TOGGLE_KEY && isAltPressed)
//             {
//                 std::cout << "Toggle key pressed" << std::endl;
//                 ToggleVisibility();
//                 return 1;
//             }

//             if (isVisible)
//             {
//                 char key = MapVirtualKey(p->vkCode, MAPVK_VK_TO_CHAR);
//                 ProcessKeyPress(key, isAltPressed);
//                 return 1;
//             }
//         }
//         else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
//         {
//             if (p->vkCode == VK_MENU)
//             {
//                 keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
//             }
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }
LRESULT CALLBACK Overlay::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        bool isAltPressed = (p->flags & LLKHF_ALTDOWN) != 0;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            if (p->vkCode == TOGGLE_KEY && isAltPressed)
            {
                ToggleVisibility();
                return 1;
            }

            if (isVisible)
            {
                char key = MapVirtualKey(p->vkCode, MAPVK_VK_TO_CHAR);
                ProcessKeyPress(key, isAltPressed);
                return 1;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
void Overlay::Draw(HDC hdc)
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Fill the entire window with black first:
    // HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    // RECT fullRect = {0, 0, screenWidth, screenHeight};
    // FillRect(hdc, &fullRect, blackBrush);
    // DeleteObject(blackBrush);
    int cols = screenWidth / (SQUARE_SIZE * 3);
    int rows = screenHeight / (SQUARE_SIZE * 3);
    cols = cols % 2 == 0 ? cols : cols + 1;
    rows = rows % 2 == 0 ? rows : rows + 1;
    std ::cout << "Cols: " << cols << " Rows: " << rows << std::endl;
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col += 2)
        {
            char pairKey1 = (char)('A' + (col / 2) % 26);
            char pairKey2 = (char)('A' + row % 26);

            int x = col * SQUARE_SIZE * 3;
            int y = row * SQUARE_SIZE * 3;

            // Draw rectangle background
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            RECT rect = {x, y, x + SQUARE_SIZE * 6, y + SQUARE_SIZE * 3};
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            // Draw border
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(hdc, hPen);

            // Select a null brush so Rectangle won't fill it again
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, x, y, x + SQUARE_SIZE * 6, y + SQUARE_SIZE * 3);
            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            DeleteObject(hPen);
            // Draw character grids
            DrawCharacterGrid(hdc, pairKey1, x, y);
            DrawCharacterGrid(hdc, pairKey2, x + SQUARE_SIZE * 3, y);

            // Store pair position
            std::string pair;
            pair += pairKey1;
            pair += pairKey2;
            pairPositions[pair] = std::make_pair(
                x + (SQUARE_SIZE * 3),
                y + (SQUARE_SIZE * 1.5));
        }
    }

    if (isShowingKeyboard && !selectedPair.empty())
    {
        std::cout << "Selected pair: " << selectedPair << std::endl;
        auto pos = pairPositions[selectedPair];
        DrawKeyboardLayout(hdc, (int)pos.first, (int)pos.second);
    }
}

void Overlay::DrawKeyboardLayout(HDC hdc, int centerX, int centerY)
{
    int rectWidth = SQUARE_SIZE * 6;
    int rectHeight = SQUARE_SIZE * 3;
    int startX = centerX - (rectWidth / 2);
    int startY = centerY - (rectHeight / 2);
    keyboardPositions.clear();

    int maxKeysInRow = KEYBOARD_ROWS[0].length();
    int keyWidth = rectWidth / maxKeysInRow;
    int keyHeight = rectHeight / KEYBOARD_ROWS.size();

    for (int row = 0; row < KEYBOARD_ROWS.size(); row++)
    {
        std::string keys = KEYBOARD_ROWS[row];
        int rowOffset = (maxKeysInRow - keys.length()) * keyWidth / 2;

        for (int col = 0; col < keys.length(); col++)
        {
            char key = keys[col];
            int x = startX + rowOffset + (col * keyWidth);
            int y = startY + (row * keyHeight);

            // Draw key background
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            RECT rect = {x, y, x + keyWidth, y + keyHeight};
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            // // Draw key border
            // HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            // SelectObject(hdc, hPen);
            // Rectangle(hdc, x, y, x + keyWidth, y + keyHeight);
            // DeleteObject(hPen);

            // Draw key character
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, layoutFont);
            int stringX = x + keyWidth / 2 - FONT_SIZE / 3;
            int stringY = y + keyHeight / 2 + FONT_SIZE / 3;
            wchar_t wch = static_cast<wchar_t>(key);
            TextOutW(hdc, stringX, stringY, &wch, 1);

            // Store key position
            keyboardPositions[key] = {x + keyWidth / 2, y + keyHeight / 2};
        }
    }
}

void Overlay::DrawCharacterGrid(HDC hdc, char c, int x, int y)
{
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, RGB(0, 0, 0));
    SelectObject(hdc, gridFont);
    int stringX = x + SQUARE_SIZE * 1.5 - 8;
    int stringY = y + SQUARE_SIZE * 1.5 - 8;
    wchar_t wch = static_cast<wchar_t>(c);
    TextOutW(hdc, stringX, stringY, &wch, 1);
}

void Overlay::ToggleVisibility()
{
    isVisible = !isVisible;
    isShowingKeyboard = false;
    selectedPair.clear();
    if (isVisible)
    {
        ShowWindow(hwnd, SW_SHOW);
        SetWindowLong(hwnd, GWL_EXSTYLE,
                      GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
    else
    {
        ShowWindow(hwnd, SW_HIDE);
    }

    // Release modifier keys
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
}

void Overlay::ProcessKeyPress(char key, bool isAltPressed)
{
    if (isShowingKeyboard)
    {
        if (keyboardPositions.find(key) != keyboardPositions.end())
        {
            // Perform immediate click if alt key is pressed
            if (isAltPressed)
            {
                SetCursorPos(keyboardPositions[key].x, keyboardPositions[key].y);
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                ToggleVisibility();
                return;
            }
            // Check if there is a pending click
            if (pendingClickPos.x != 0 && pendingClickPos.y != 0)
            {
                // Check if position has changed
                if (pendingClickPos.x != keyboardPositions[key].x || pendingClickPos.y != keyboardPositions[key].y)
                {
                    // Reset click count
                    clickCount = 0;
                }
            }
            pendingClickPos = keyboardPositions[key];

            clickCount++; // Increment click counter

            // Start/reset timer if not already running
            if (clickTimer)
            {
                KillTimer(hwnd, clickTimer);
            }
            clickTimer = SetTimer(hwnd, 1, CLICK_DELAY, NULL);

            // Don't hide overlay yet - wait for timer
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
            return;
        }
    }

    if (lastKeyPressed != '\0')
    {
        std::string pairKey = std::string(1, lastKeyPressed) + key;
        if (pairPositions.find(pairKey) != pairPositions.end())
        {
            selectedPair = pairKey;
            isShowingKeyboard = true;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        lastKeyPressed = '\0';
    }
    else
    {
        lastKeyPressed = key;
    }
}