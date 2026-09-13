/*
 * Tardis.exe - interface de compatibilidade para o Windows CE OEM.
 *
 * O GPS executa hello_tardis.exe, cujo import table foi observado em campo.
 * O navegador original importa COREDLL por ordinais. Para evitar que uma API
 * ausente impeça o carregamento antes de WinMain, esta versão usa somente:
 *
 *   - MessageBoxW e o CRT já presentes no hello_tardis.exe;
 *   - DialogBoxIndirectParamW e PostQuitMessage, presentes no MobileNavigator
 *     pelos ordinais COREDLL 260 e 803.
 *
 * A interface é composta por controles nativos de diálogo. Isso elimina
 * RegisterClass, GDI, timers, fontes e consultas de sistema não comprovadas.
 */
#include <windows.h>

#pragma comment(lib, "coredll.lib")

#define IDC_SYSTEM   100
#define IDC_MEDIA    101
#define IDC_NETWORK  102
#define IDC_CONTROLS 103
#define IDC_BACK     104
#define IDC_EXIT     105

static BYTE g_dialog_template[2048];

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
    while (*text) {
        p = PutWord(p, (WORD)*text++);
    }
    return PutWord(p, 0);
}

static BYTE *AlignDword(BYTE *p)
{
    DWORD v = (DWORD)(p - g_dialog_template);
    v = (v + 3u) & ~3u;
    return g_dialog_template + v;
}

static BYTE *PutItem(BYTE *p, DWORD style, short x, short y,
                     short cx, short cy, WORD id, WORD class_atom,
                     LPCWSTR text)
{
    p = AlignDword(p);
    p = PutDword(p, style);
    p = PutDword(p, 0);
    p = PutWord(p, (WORD)x);
    p = PutWord(p, (WORD)y);
    p = PutWord(p, (WORD)cx);
    p = PutWord(p, (WORD)cy);
    p = PutWord(p, id);
    p = PutWord(p, 0xffff);
    p = PutWord(p, class_atom);
    p = PutString(p, text);
    return PutWord(p, 0);
}

static BYTE *BuildDialogTemplate(void)
{
    BYTE *p = g_dialog_template;
    DWORD child = WS_CHILD | WS_VISIBLE;
    DWORD button = child | WS_TABSTOP | BS_PUSHBUTTON;

    p = PutDword(p, WS_POPUP | WS_VISIBLE | DS_SETFONT);
    p = PutDword(p, 0);
    p = PutWord(p, 12);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    p = PutWord(p, 120);
    p = PutWord(p, 34);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    p = PutString(p, L"TARDIS");
    p = PutWord(p, 8);
    p = PutString(p, L"Tahoma");

    p = PutItem(p, child | SS_CENTER, 4, 1, 112, 4, 0, 0x0082, L"TARDIS");
    p = PutItem(p, child | SS_CENTER, 4, 5, 112, 3, 0, 0x0082, L"STATUS: OFFLINE");
    p = PutItem(p, button, 4, 9, 52, 6, IDC_SYSTEM, 0x0080, L"SISTEMA");
    p = PutItem(p, button, 62, 9, 52, 6, IDC_MEDIA, 0x0080, L"MIDIA");
    p = PutItem(p, button, 4, 17, 52, 6, IDC_NETWORK, 0x0080, L"REDE");
    p = PutItem(p, button, 62, 17, 52, 6, IDC_CONTROLS, 0x0080, L"CONTROLES");
    p = PutItem(p, child | SS_CENTER, 4, 24, 52, 3, 0, 0x0082, L"Windows CE");
    p = PutItem(p, child | SS_CENTER, 62, 24, 52, 3, 0, 0x0082, L"Controles visuais");
    p = PutItem(p, child | SS_CENTER, 4, 27, 52, 2, 0, 0x0082, L"USB");
    p = PutItem(p, child | SS_CENTER, 62, 27, 52, 2, 0, 0x0082, L"TARDIS OFFLINE");
    p = PutItem(p, button, 4, 30, 52, 4, IDC_BACK, 0x0080, L"VOLTAR");
    p = PutItem(p, button, 62, 30, 52, 4, IDC_EXIT, 0x0080, L"SAIR");
    return p;
}

static BOOL CALLBACK TardisDialogProc(HWND dialog, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
    (void)dialog;
    (void)lParam;
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_EXIT) {
            PostQuitMessage(0);
            return TRUE;
        }
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

    BuildDialogTemplate();
    result = DialogBoxIndirectParamW(instance,
                                     (LPCDLGTEMPLATE)g_dialog_template,
                                     NULL, TardisDialogProc, 0);
    if (result == -1) {
        MessageBoxW(NULL, L"TARDIS: erro ao criar janela",
                    L"Windows CE", MB_OK);
        return 1;
    }
    return 0;
}
