/*
 * Tardis.exe - primeiro protótipo Windows CE / ARMv4.
 * Dependência prevista: COREDLL.DLL via coredll.lib do SDK CE.
 * Sem .NET, MFC, ATL, imagens ou bibliotecas externas.
 */
#include <windows.h>

#pragma comment(lib, "coredll.lib")

#define IDC_TARDIS 100
#define BTN_TOUCH  1
#define BTN_INFO   2
#define BTN_EXIT   3

static HWND g_hwnd;
static RECT g_touch, g_info, g_exit;
static int g_touches;
static BOOL g_showInfo;

static void SetButtonRects(int width, int height)
{
    int margin = 12;
    int bh = 42;
    int gap = 8;
    int y = height - margin - bh;
    int w = (width - (margin * 2) - (gap * 2)) / 3;
    SetRect(&g_touch, margin, y, margin + w, y + bh);
    SetRect(&g_info, margin + w + gap, y, margin + (w + gap) + w, y + bh);
    SetRect(&g_exit, margin + (w + gap) * 2, y, margin + (w + gap) * 2 + w, y + bh);
}

static BOOL InRect(RECT *r, int x, int y)
{
    return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

static void DrawCentered(HDC dc, RECT *r, LPCWSTR text, HFONT font)
{
    HFONT old = (HFONT)SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    DrawTextW(dc, text, -1, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
}

static void DrawButton(HDC dc, RECT *r, LPCWSTR text, HFONT font)
{
    Rectangle(dc, r->left, r->top, r->right, r->bottom);
    DrawCentered(dc, r, text, font);
}

static void DrawScreen(HDC dc, RECT *client)
{
    HBRUSH bg = CreateSolidBrush(RGB(24, 32, 45));
    HFONT title = CreateFontW(34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                              DEFAULT_PITCH | FF_SWISS, L"Arial");
    HFONT body = CreateFontW(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, L"Arial");
    HFONT button = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_SWISS, L"Arial");
    RECT r = *client;
    WCHAR line[256];

    FillRect(dc, &r, bg);
    SetTextColor(dc, RGB(240, 245, 250));
    SetRect(&r, 0, 18, client->right, 70);
    DrawCentered(dc, &r, L"TARDIS", title);
    SetRect(&r, 0, 70, client->right, 102);
    DrawCentered(dc, &r, L"Windows CE operacional", body);

    if (g_showInfo) {
        MEMORYSTATUS ms;
        OSVERSIONINFO vi;
        DWORD pathLen;
        WCHAR path[MAX_PATH];
        ZeroMemory(&ms, sizeof(ms));
        ms.dwLength = sizeof(ms);
        GlobalMemoryStatus(&ms);
        ZeroMemory(&vi, sizeof(vi));
        vi.dwOSVersionInfoSize = sizeof(vi);
        pathLen = GetModuleFileNameW(NULL, path, MAX_PATH);
        if (pathLen == 0) path[0] = 0;
        wsprintfW(line, L"Tela: %dx%d   CE: %u.%u   Memória: %lu KB",
                  GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                  vi.dwMajorVersion, vi.dwMinorVersion,
                  (unsigned long)(ms.dwAvailPhys / 1024));
        SetRect(&r, 8, 108, client->right - 8, 142);
        DrawCentered(dc, &r, line, body);
        SetRect(&r, 8, 142, client->right - 8, 176);
        DrawCentered(dc, &r, path, body);
    } else {
        wsprintfW(line, L"Toques: %d", g_touches);
        SetRect(&r, 0, 112, client->right, 150);
        DrawCentered(dc, &r, line, body);
    }

    SetTextColor(dc, RGB(240, 245, 250));
    DrawButton(dc, &g_touch, L"TESTAR TOUCH", button);
    DrawButton(dc, &g_info, L"INFORMAÇÕES", button);
    DrawButton(dc, &g_exit, L"SAIR", button);

    DeleteObject(bg);
    DeleteObject(title);
    DeleteObject(body);
    DeleteObject(button);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_SIZE:
        SetButtonRects(LOWORD(lp), HIWORD(lp));
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_LBUTTONUP:
        {
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            if (InRect(&g_touch, x, y)) {
                ++g_touches;
                g_showInfo = FALSE;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (InRect(&g_info, x, y)) {
                g_showInfo = TRUE;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (InRect(&g_exit, x, y)) {
                DestroyWindow(hwnd);
            }
        }
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT r;
            HDC dc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &r);
            DrawScreen(dc, &r);
            EndPaint(hwnd, &ps);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmd, int show)
{
    WNDCLASSW wc;
    MSG msg;
    RECT r;
    (void)prev; (void)cmd;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"TardisCEWindow";
    if (!RegisterClassW(&wc)) return 1;
    g_hwnd = CreateWindowExW(0, wc.lpszClassName, L"TARDIS",
                             WS_POPUP, 0, 0, 480, 272, NULL, NULL, inst, NULL);
    if (!g_hwnd) return 2;
    GetClientRect(g_hwnd, &r);
    SetButtonRects(r.right, r.bottom);
    ShowWindow(g_hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(g_hwnd);
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
