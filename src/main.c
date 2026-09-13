/* Tardis.exe - interface CE baseada em bitmap, sem estilos transparentes. */
#include <windows.h>
#pragma comment(lib, "coredll.lib")

#define IDB_SKIN       200
#define PAGE_DASHBOARD 0
#define PAGE_WIFI      1
#define PAGE_NETWORK   2
#define PAGE_CONTROLS  3

static BYTE g_template[4096];
static int g_page = PAGE_DASHBOARD;
static BOOL g_quit = FALSE;
static HBRUSH g_dark_brush = NULL;

static BYTE *PutWord(BYTE *p, WORD v) { *(WORD *)p = v; return p + 2; }
static BYTE *PutDword(BYTE *p, DWORD v) { *(DWORD *)p = v; return p + 4; }
static BYTE *PutString(BYTE *p, LPCWSTR s) { while (*s) p = PutWord(p, (WORD)*s++); return PutWord(p, 0); }
static BYTE *AlignDword(BYTE *p) { DWORD n = (DWORD)(p - g_template); return g_template + ((n + 3u) & ~3u); }

static BYTE *PutStatic(BYTE *p, DWORD style, short x, short y, short cx,
                       short cy, LPCWSTR text)
{
    p = AlignDword(p);
    p = PutDword(p, style);
    p = PutDword(p, 0);
    p = PutWord(p, (WORD)x); p = PutWord(p, (WORD)y);
    p = PutWord(p, (WORD)cx); p = PutWord(p, (WORD)cy);
    p = PutWord(p, 0);
    p = PutWord(p, 0xffff); p = PutWord(p, 0x0082);
    p = PutString(p, text);
    return PutWord(p, 0);
}

static BYTE *PutBitmap(BYTE *p)
{
    DWORD style = WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_LEFT;
    p = AlignDword(p);
    p = PutDword(p, style); p = PutDword(p, 0);
    p = PutWord(p, 0); p = PutWord(p, 0);
    p = PutWord(p, 316); p = PutWord(p, 184);
    p = PutWord(p, 0); p = PutWord(p, 0xffff); p = PutWord(p, 0x0082);
    p = PutWord(p, 0xffff); p = PutWord(p, IDB_SKIN);
    return PutWord(p, 0);
}

static BYTE *BeginTemplate(LPCWSTR title, WORD controls)
{
    BYTE *p = g_template;
    p = PutDword(p, WS_POPUP | WS_VISIBLE); p = PutDword(p, 0);
    p = PutWord(p, controls); p = PutWord(p, 0);
    p = PutWord(p, 0); p = PutWord(p, 0);
    p = PutWord(p, 316); p = PutWord(p, 184);
    p = PutWord(p, 0); p = PutWord(p, 0);
    return PutString(p, title);
}

/* These are state values only; all fixed labels and decoration live in skin.bmp. */
static BYTE *Dynamic(BYTE *p, short x, short y, short cx, short cy, LPCWSTR text)
{
    return PutStatic(p, WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, cx, cy, text);
}
static BYTE *DynamicCenter(BYTE *p, short x, short y, short cx, short cy, LPCWSTR text)
{
    return PutStatic(p, WS_CHILD | WS_VISIBLE | SS_CENTER, x, y, cx, cy, text);
}

/* The bitmap is appended after the dynamic windows.  On the CE dialog manager
   this puts the opaque skin at the bottom of the child z-order. */
static BYTE *BuildDashboard(void)
{
    BYTE *p = BeginTemplate(L"TARDIS", 7);
    p = DynamicCenter(p, 252, 7, 58, 10, L"OFFLINE");
    p = Dynamic(p, 18, 31, 94, 8, L"OFFLINE");
    p = Dynamic(p, 138, 31, 70, 8, L"-- ms");
    p = Dynamic(p, 226, 31, 78, 8, L"-- %");
    p = DynamicCenter(p, 18, 70, 286, 9, L"-- -- -- -- -- -- -- --");
    p = Dynamic(p, 18, 108, 286, 8, L"--      --      --      --");
    return PutBitmap(p);
}

static BYTE *BuildWifi(void)
{
    BYTE *p = BeginTemplate(L"WIFI", 4);
    p = Dynamic(p, 24, 28, 260, 10, L"SSID: --");
    p = Dynamic(p, 24, 43, 260, 10, L"SENHA: ********");
    p = DynamicCenter(p, 24, 72, 260, 10, L"QR CODE");
    return PutBitmap(p);
}

static BYTE *BuildNetwork(void)
{
    BYTE *p = BeginTemplate(L"REDE", 8);
    p = Dynamic(p, 22, 27, 140, 8, L"--");
    p = Dynamic(p, 164, 27, 140, 8, L"--");
    p = Dynamic(p, 22, 42, 140, 8, L"--");
    p = Dynamic(p, 164, 42, 140, 8, L"--");
    p = Dynamic(p, 22, 57, 140, 8, L"--");
    p = Dynamic(p, 164, 57, 140, 8, L"-- ms");
    p = Dynamic(p, 22, 72, 140, 8, L"-- %");
    return PutBitmap(p);
}

static BYTE *BuildControls(void)
{
    BYTE *p = BeginTemplate(L"CONTROLES", 2);
    p = DynamicCenter(p, 24, 122, 268, 10, L"OFFLINE");
    return PutBitmap(p);
}

static BYTE *BuildTemplate(void)
{
    if (g_page == PAGE_WIFI) return BuildWifi();
    if (g_page == PAGE_NETWORK) return BuildNetwork();
    if (g_page == PAGE_CONTROLS) return BuildControls();
    return BuildDashboard();
}

static void OfflineMessage(void)
{
    MessageBoxW(NULL, L"Tardis offline", L"TARDIS", MB_OK);
}

static void LeaveDialog(void)
{
    PostQuitMessage(0);
}

static void HandleTouch(int x, int y)
{
    if (g_page == PAGE_DASHBOARD && y >= 228) {
        if (x >= 8 && x < 160) g_page = PAGE_WIFI;
        else if (x >= 160 && x < 320) g_page = PAGE_NETWORK;
        else if (x >= 320) g_page = PAGE_CONTROLS;
        LeaveDialog();
        return;
    }
    if (g_page == PAGE_WIFI && y >= 228) {
        if (x < 180) g_page = PAGE_DASHBOARD;
        LeaveDialog();
        return;
    }
    if (g_page == PAGE_NETWORK && y >= 228) {
        if (x < 300) OfflineMessage();
        else g_page = PAGE_DASHBOARD;
        return;
    }
    if (g_page == PAGE_CONTROLS) {
        if (y >= 228 && x < 180) { g_page = PAGE_DASHBOARD; LeaveDialog(); return; }
        if (y >= 228 && x >= 300) { g_quit = TRUE; LeaveDialog(); return; }
        if (y >= 44 && y < 216) OfflineMessage();
    }
}

static BOOL CALLBACK TardisDialogProc(HWND dialog, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
    (void)dialog; (void)wParam;
    if (message == WM_INITDIALOG) {
        g_dark_brush = CreateSolidBrush(RGB(10, 20, 34));
        return TRUE;
    }
    if (message == WM_CTLCOLORDLG || message == WM_CTLCOLORSTATIC) {
        SetTextColor((HDC)wParam, RGB(225, 242, 255));
        return (INT_PTR)g_dark_brush;
    }
    if (message == WM_LBUTTONDOWN) {
        HandleTouch((short)LOWORD(lParam), (short)HIWORD(lParam));
        return TRUE;
    }
    if (message == WM_DESTROY) {
        if (g_dark_brush != NULL) { DeleteObject(g_dark_brush); g_dark_brush = NULL; }
        return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous,
                  LPWSTR command_line, int show)
{
    INT_PTR result;
    (void)previous; (void)command_line; (void)show;
    while (!g_quit) {
        BuildTemplate();
        result = DialogBoxIndirectParamW(instance,
                    (LPCDLGTEMPLATE)g_template, NULL, TardisDialogProc, 0);
        if (result == -1) {
            MessageBoxW(NULL, L"TARDIS: erro ao criar janela", L"Windows CE", MB_OK);
            return 1;
        }
    }
    return 0;
}
