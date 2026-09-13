/* Tardis.exe - recuperacao visual: BMP desenhado diretamente no HDC do dialogo. */
#include <windows.h>
#pragma comment(lib, "coredll.lib")

#define IDB_SKIN 200

static BYTE g_dialog_template[4096];
static HINSTANCE g_instance = NULL;
static HBITMAP g_skin_bitmap = NULL;
static HDC g_skin_dc = NULL;
static int g_last_touch_x = 0;
static int g_last_touch_y = 0;

static BYTE *PutWord(BYTE *p, WORD value)
{
    *(WORD *)p = value;
    return p + sizeof(WORD);
}

static BYTE *PutDword(BYTE *p, DWORD value)
{
    *(DWORD *)p = value;
    return p + sizeof(DWORD);
}

static BYTE *PutString(BYTE *p, LPCWSTR text)
{
    while (*text) p = PutWord(p, (WORD)*text++);
    return PutWord(p, 0);
}

static BYTE *BeginTemplate(void)
{
    BYTE *p = g_dialog_template;
    p = PutDword(p, WS_POPUP | WS_VISIBLE);
    p = PutDword(p, 0);
    p = PutWord(p, 0);             /* control_count: nenhum filho */
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    p = PutWord(p, 316);           /* 480x272 DLU neste firmware */
    p = PutWord(p, 184);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    return PutString(p, L"TARDIS");
}

static void ShowSkinError(LPCWSTR text)
{
    MessageBoxW(NULL, text, L"TARDIS", MB_OK);
}

static BOOL CALLBACK TardisDialogProc(HWND dialog, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    (void)dialog;
    if (message == WM_INITDIALOG) return TRUE;

    if (message == WM_ERASEBKGND) {
        hdc = (HDC)wParam;
        if (g_skin_bitmap == NULL) {
            g_skin_bitmap = LoadBitmapW(g_instance, MAKEINTRESOURCE(IDB_SKIN));
            if (g_skin_bitmap == NULL) {
                ShowSkinError(L"Falha ao carregar skin");
                return TRUE;
            }
        }
        if (g_skin_dc == NULL) {
            g_skin_dc = CreateCompatibleDC(hdc);
            if (g_skin_dc == NULL) {
                ShowSkinError(L"Falha ao criar DC da skin");
                return TRUE;
            }
            SelectObject(g_skin_dc, g_skin_bitmap);
        }
        StretchBlt(hdc, 0, 0, 480, 272,
                   g_skin_dc, 0, 0, 480, 272, SRCCOPY);
        return TRUE;
    }

    if (message == WM_LBUTTONDOWN) {
        g_last_touch_x = (short)LOWORD(lParam);
        g_last_touch_y = (short)HIWORD(lParam);
        return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous,
                  LPWSTR command_line, int show)
{
    INT_PTR result;
    (void)previous;
    (void)command_line;
    (void)show;
    g_instance = instance;
    BeginTemplate();
    result = DialogBoxIndirectParamW(instance,
                (LPCDLGTEMPLATE)g_dialog_template,
                NULL, TardisDialogProc, 0);
    if (result == -1) {
        MessageBoxW(NULL, L"TARDIS: erro ao criar janela", L"Windows CE", MB_OK);
        return 1;
    }
    return 0;
}
