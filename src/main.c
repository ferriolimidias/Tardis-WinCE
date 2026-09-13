/*
 * Tardis.exe - interface principal para GPS Aquarius / Windows CE.
 * Target: ARMv4/ARMv4T, PE32, Windows CE, 480x272.
 * Sem .NET/MFC/ATL e sem assets externos.
 */
#include <windows.h>

#pragma comment(lib, "coredll.lib")

#define PAGE_HOME       0
#define PAGE_SYSTEM     1
#define PAGE_MEDIA      2
#define PAGE_NETWORK    3
#define PAGE_CONTROLS   4

static HWND g_hwnd;
static int g_page = PAGE_HOME;
static RECT g_tile_system;
static RECT g_tile_media;
static RECT g_tile_network;
static RECT g_tile_controls;
static RECT g_back;
static RECT g_action1;
static RECT g_action2;
static RECT g_action3;
static WCHAR g_message[96] = L"";

static BOOL PtIn(const RECT *r, int x, int y)
{
    return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

static void DrawTextCentered(HDC dc, RECT r, LPCWSTR text)
{
    DrawTextW(dc, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static void DrawTextLeft(HDC dc, RECT r, LPCWSTR text)
{
    DrawTextW(dc, text, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

static void Fill(HDC dc, RECT r, COLORREF color)
{
    HBRUSH b = CreateSolidBrush(color);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

static void Frame(HDC dc, RECT r, COLORREF color)
{
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN old = (HPEN)SelectObject(dc, pen);
    HBRUSH oldb = (HBRUSH)SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, r.left, r.top, r.right, r.bottom);
    SelectObject(dc, oldb);
    SelectObject(dc, old);
    DeleteObject(pen);
}

static void Button(HDC dc, RECT r, LPCWSTR text, BOOL active)
{
    Fill(dc, r, active ? RGB(38, 76, 104) : RGB(28, 39, 54));
    Frame(dc, r, active ? RGB(95, 190, 230) : RGB(76, 95, 112));
    SetTextColor(dc, RGB(236, 244, 248));
    SetBkMode(dc, TRANSPARENT);
    DrawTextCentered(dc, r, text);
}

static void SetRects(int w, int h)
{
    int top = 72;
    int left = 12;
    int right = w - 12;
    int gap = 8;
    int tw = (right - left - gap) / 2;
    int th = 70;

    SetRect(&g_tile_system, left, top, left + tw, top + th);
    SetRect(&g_tile_media, left + tw + gap, top, right, top + th);
    SetRect(&g_tile_network, left, top + th + gap, left + tw, top + th + gap + th);
    SetRect(&g_tile_controls, left + tw + gap, top + th + gap, right, top + th + gap + th);

    SetRect(&g_back, 12, h - 44, 100, h - 10);
    SetRect(&g_action1, 12, 118, 150, 158);
    SetRect(&g_action2, 170, 118, 308, 158);
    SetRect(&g_action3, 328, 118, w - 12, 158);
}

static void DrawHeader(HDC dc, RECT client)
{
    SYSTEMTIME st;
    WCHAR clock[32];
    RECT r;

    Fill(dc, client, RGB(14, 20, 29));
    SetBkMode(dc, TRANSPARENT);

    SetTextColor(dc, RGB(92, 205, 242));
    SetRect(&r, 14, 8, 220, 42);
    DrawTextLeft(dc, r, L"TARDIS");

    SetTextColor(dc, RGB(178, 193, 205));
    SetRect(&r, 14, 38, 260, 61);
    DrawTextLeft(dc, r, L"TERMINAL WINDOWS CE");

    GetLocalTime(&st);
    wsprintfW(clock, L"%02u:%02u", st.wHour, st.wMinute);
    SetTextColor(dc, RGB(236, 244, 248));
    SetRect(&r, client.right - 92, 10, client.right - 12, 38);
    DrawTextCentered(dc, r, clock);

    SetTextColor(dc, RGB(232, 168, 70));
    SetRect(&r, client.right - 145, 38, client.right - 12, 61);
    DrawTextCentered(dc, r, L"TARDIS OFFLINE");

    SetRect(&r, 12, 64, client.right - 12, 66);
    Fill(dc, r, RGB(40, 58, 73));
}

static void DrawHome(HDC dc, RECT client)
{
    RECT r;

    Button(dc, g_tile_system, L"SISTEMA", TRUE);
    Button(dc, g_tile_media, L"MIDIA", TRUE);
    Button(dc, g_tile_network, L"REDE", TRUE);
    Button(dc, g_tile_controls, L"CONTROLES", TRUE);

    SetTextColor(dc, RGB(129, 149, 164));
    SetRect(&r, 12, client.bottom - 34, client.right - 12, client.bottom - 8);
    DrawTextCentered(dc, r, L"Interface local pronta | USB ainda sem sessao de dados");
}

static void DrawSystem(HDC dc, RECT client)
{
    RECT r;
    MEMORYSTATUS ms;
    OSVERSIONINFO vi;
    WCHAR line[160];
    WCHAR path[MAX_PATH];
    DWORD len;

    ZeroMemory(&ms, sizeof(ms));
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatus(&ms);

    ZeroMemory(&vi, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);
    GetVersionExW(&vi);

    path[0] = 0;
    len = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (!len) lstrcpyW(path, L"(caminho indisponivel)");

    SetTextColor(dc, RGB(236, 244, 248));
    SetRect(&r, 16, 78, client.right - 16, 103);
    DrawTextLeft(dc, r, L"SISTEMA");

    SetTextColor(dc, RGB(176, 194, 206));
    wsprintfW(line, L"Tela: %dx%d   Windows CE: %u.%u",
              GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
              vi.dwMajorVersion, vi.dwMinorVersion);
    SetRect(&r, 16, 108, client.right - 16, 132);
    DrawTextLeft(dc, r, line);

    wsprintfW(line, L"RAM livre: %lu KB   RAM total: %lu KB",
              (unsigned long)(ms.dwAvailPhys / 1024),
              (unsigned long)(ms.dwTotalPhys / 1024));
    SetRect(&r, 16, 134, client.right - 16, 158);
    DrawTextLeft(dc, r, line);

    SetRect(&r, 16, 160, client.right - 16, 188);
    DrawTextLeft(dc, r, path);

    Button(dc, g_back, L"VOLTAR", TRUE);
}

static void DrawMedia(HDC dc, RECT client)
{
    RECT r;

    SetTextColor(dc, RGB(236, 244, 248));
    SetRect(&r, 16, 78, client.right - 16, 103);
    DrawTextLeft(dc, r, L"MIDIA");

    SetTextColor(dc, RGB(176, 194, 206));
    SetRect(&r, 16, 88, client.right - 16, 115);
    DrawTextCentered(dc, r, L"Controles preparados para a Tardis");

    Button(dc, g_action1, L"ANTERIOR", FALSE);
    Button(dc, g_action2, L"PLAY / PAUSA", FALSE);
    Button(dc, g_action3, L"PROXIMO", FALSE);

    SetTextColor(dc, RGB(232, 168, 70));
    SetRect(&r, 16, 170, client.right - 16, 205);
    DrawTextCentered(dc, r, L"Aguardando conexao USB com a Tardis");

    Button(dc, g_back, L"VOLTAR", TRUE);
}

static void DrawNetwork(HDC dc, RECT client)
{
    RECT r;

    SetTextColor(dc, RGB(236, 244, 248));
    SetRect(&r, 16, 78, client.right - 16, 103);
    DrawTextLeft(dc, r, L"REDE");

    SetTextColor(dc, RGB(176, 194, 206));
    SetRect(&r, 16, 112, client.right - 16, 138);
    DrawTextLeft(dc, r, L"USB: disponivel");
    SetRect(&r, 16, 140, client.right - 16, 166);
    DrawTextLeft(dc, r, L"ActiveSync/IP: nao conectado");
    SetRect(&r, 16, 168, client.right - 16, 194);
    DrawTextLeft(dc, r, L"Servidor Tardis: offline");

    Button(dc, g_back, L"VOLTAR", TRUE);
}

static void DrawControls(HDC dc, RECT client)
{
    RECT r;

    SetTextColor(dc, RGB(236, 244, 248));
    SetRect(&r, 16, 78, client.right - 16, 103);
    DrawTextLeft(dc, r, L"CONTROLES");

    Button(dc, g_action1, L"TELA", TRUE);
    Button(dc, g_action2, L"ATUALIZAR", TRUE);
    Button(dc, g_action3, L"SAIR", TRUE);

    SetTextColor(dc, RGB(176, 194, 206));
    SetRect(&r, 16, 171, client.right - 16, 200);
    DrawTextCentered(dc, r, g_message[0] ? g_message : L"Comandos remotos serao habilitados via Tardis");

    Button(dc, g_back, L"VOLTAR", TRUE);
}

static void DrawScreen(HDC dc, RECT client)
{
    HFONT oldFont;

    oldFont = (HFONT)SelectObject(dc, GetStockObject(SYSTEM_FONT));
    DrawHeader(dc, client);

    switch (g_page) {
    case PAGE_SYSTEM:   DrawSystem(dc, client); break;
    case PAGE_MEDIA:    DrawMedia(dc, client); break;
    case PAGE_NETWORK:  DrawNetwork(dc, client); break;
    case PAGE_CONTROLS: DrawControls(dc, client); break;
    default:            DrawHome(dc, client); break;
    }

    SelectObject(dc, oldFont);
}

static void GoPage(int page)
{
    g_page = page;
    g_message[0] = 0;
    InvalidateRect(g_hwnd, NULL, TRUE);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_SIZE:
        SetRects(LOWORD(lp), HIWORD(lp));
        return 0;

    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_LBUTTONUP:
        {
            int x = LOWORD(lp);
            int y = HIWORD(lp);

            if (g_page == PAGE_HOME) {
                if (PtIn(&g_tile_system, x, y)) GoPage(PAGE_SYSTEM);
                else if (PtIn(&g_tile_media, x, y)) GoPage(PAGE_MEDIA);
                else if (PtIn(&g_tile_network, x, y)) GoPage(PAGE_NETWORK);
                else if (PtIn(&g_tile_controls, x, y)) GoPage(PAGE_CONTROLS);
            } else {
                if (PtIn(&g_back, x, y)) {
                    GoPage(PAGE_HOME);
                } else if (g_page == PAGE_CONTROLS) {
                    if (PtIn(&g_action1, x, y)) {
                        lstrcpyW(g_message, L"Tela ativa");
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (PtIn(&g_action2, x, y)) {
                        lstrcpyW(g_message, L"Interface atualizada");
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (PtIn(&g_action3, x, y)) {
                        DestroyWindow(hwnd);
                    }
                }
            }
        }
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT client;
            HDC dc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &client);
            DrawScreen(dc, client);
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmd, int show)
{
    WNDCLASSW wc;
    MSG msg;
    RECT client;

    (void)prev;
    (void)cmd;
    (void)show;

    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"TardisMainWindow";

    if (!RegisterClassW(&wc)) return 1;

    g_hwnd = CreateWindowExW(0, wc.lpszClassName, L"TARDIS",
                             WS_POPUP, 0, 0, 480, 272,
                             NULL, NULL, inst, NULL);
    if (!g_hwnd) return 2;

    GetClientRect(g_hwnd, &client);
    SetRects(client.right, client.bottom);

    ShowWindow(g_hwnd, SW_SHOWMAXIMIZED);
    SetForegroundWindow(g_hwnd);
    UpdateWindow(g_hwnd);
    SetTimer(g_hwnd, 1, 1000, NULL);

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
