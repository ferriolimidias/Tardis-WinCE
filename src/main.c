/*
 * Tardis.exe - interface nativa para GPS Aquarius / Windows CE OEM.
 *
 * Estratégia de compatibilidade:
 * - usa somente APIs já comprovadas neste aparelho;
 * - interface construída com template de diálogo nativo;
 * - sem GDI customizado, timers, fontes externas, .NET, MFC ou ATL.
 */
#include <windows.h>

#pragma comment(lib, "coredll.lib")

#define IDC_SYSTEM   100
#define IDC_MEDIA    101
#define IDC_NETWORK  102
#define IDC_CONTROLS 103
#define IDC_EXIT     105

static BYTE g_dialog_template[4096];

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

    /*
     * O primeiro protótipo (120x34 DLU) abriu corretamente, porém pequeno.
     * Para este firmware/Tahoma 8-10, cerca de 316x184 DLU ocupa praticamente
     * toda a área física de 480x272 pixels.
     */
    p = PutDword(p, WS_POPUP | WS_VISIBLE | DS_SETFONT);
    p = PutDword(p, 0);
    p = PutWord(p, 10);              /* quantidade de controles */
    p = PutWord(p, 0);               /* x */
    p = PutWord(p, 0);               /* y */
    p = PutWord(p, 316);             /* largura aproximada 480 px */
    p = PutWord(p, 184);             /* altura aproximada 272 px */
    p = PutWord(p, 0);               /* sem menu */
    p = PutWord(p, 0);               /* classe padrão */
    p = PutString(p, L"TARDIS");
    p = PutWord(p, 10);
    p = PutString(p, L"Tahoma");

    /* Cabeçalho */
    p = PutItem(p, child | SS_CENTER, 12, 8, 292, 16,
                0, 0x0082, L"TARDIS");
    p = PutItem(p, child | SS_CENTER, 12, 27, 292, 12,
                0, 0x0082, L"STATUS: OFFLINE");

    /* Painel principal 2x2 */
    p = PutItem(p, button, 12, 48, 138, 34,
                IDC_SYSTEM, 0x0080, L"SISTEMA");
    p = PutItem(p, button, 166, 48, 138, 34,
                IDC_MEDIA, 0x0080, L"MIDIA");
    p = PutItem(p, button, 12, 92, 138, 34,
                IDC_NETWORK, 0x0080, L"REDE");
    p = PutItem(p, button, 166, 92, 138, 34,
                IDC_CONTROLS, 0x0080, L"CONTROLES");

    /* Rodapé */
    p = PutItem(p, child | SS_CENTER, 12, 134, 292, 10,
                0, 0x0082, L"Terminal local Windows CE | Tardis");
    p = PutItem(p, child | SS_CENTER, 12, 146, 292, 8,
                0, 0x0082, L"USB pronto - aguardando link de dados");
    p = PutItem(p, child | SS_CENTER, 12, 160, 180, 14,
                0, 0x0082, L"TARDIS v1");
    p = PutItem(p, button, 220, 156, 84, 22,
                IDC_EXIT, 0x0080, L"SAIR");

    return p;
}

static BOOL CALLBACK TardisDialogProc(HWND dialog, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
    WORD id;
    (void)dialog;
    (void)lParam;

    switch (message) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        id = LOWORD(wParam);

        if (id == IDC_SYSTEM) {
            MessageBoxW(dialog,
                        L"Windows CE operacional\nInterface Tardis ativa\nTela alvo: 480x272",
                        L"TARDIS - SISTEMA", MB_OK);
            return TRUE;
        }

        if (id == IDC_MEDIA) {
            MessageBoxW(dialog,
                        L"Controles de midia preparados.\nAguardando conexao com a Tardis.",
                        L"TARDIS - MIDIA", MB_OK);
            return TRUE;
        }

        if (id == IDC_NETWORK) {
            MessageBoxW(dialog,
                        L"USB: disponivel\nTardis: offline\nLink de dados: aguardando configuracao",
                        L"TARDIS - REDE", MB_OK);
            return TRUE;
        }

        if (id == IDC_CONTROLS) {
            MessageBoxW(dialog,
                        L"Touch ativo.\nOs comandos remotos serao ligados a Tardis pelo USB.",
                        L"TARDIS - CONTROLES", MB_OK);
            return TRUE;
        }

        if (id == IDC_EXIT) {
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
                                     NULL,
                                     TardisDialogProc,
                                     0);

    if (result == -1) {
        MessageBoxW(NULL,
                    L"TARDIS: erro ao criar interface",
                    L"Windows CE",
                    MB_OK);
        return 1;
    }

    return 0;
}
