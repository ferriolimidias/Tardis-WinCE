/*
 * Tardis.exe - skin bitmap 480x272 para Windows CE OEM.
 *
 * O recurso BMP fornece o fundo, cards e botoes. Textos de estado sao
 * controles STATIC sobre a imagem; as areas de toque sao STATIC SS_NOTIFY
 * sem aparencia nativa. Nenhuma API de desenho nao comprovada e importada.
 */
#include <windows.h>

#pragma comment(lib, "coredll.lib")

#define IDB_SKIN         200
#define PAGE_DASHBOARD   0
#define PAGE_WIFI        1
#define PAGE_NETWORK     2
#define PAGE_CONTROLS    3

#define IDC_WIFI         100
#define IDC_NETWORK      101
#define IDC_CONTROLS     102
#define IDC_BACK         103
#define IDC_EXIT         104
#define IDC_SHOW_PASS    105
#define IDC_QR           106
#define IDC_REFRESH      107
#define IDC_POWER_PC     108
#define IDC_REFRESH_NET  109
#define IDC_RESTART      110
#define IDC_SHUTDOWN     111

typedef struct {
    BOOL tardis_online;
    BOOL internet_online;
    int ping_ms;
    int packet_loss_percent;
    BOOL router_online;
    BOOL pc_online;
    BOOL tv_online;
    BOOL ps3_online;
    int latency[30];
    int latency_count;
} TARDIS_STATUS;

static BYTE g_dialog_template[4096];
static int g_page = PAGE_DASHBOARD;
static BOOL g_quit = FALSE;
static TARDIS_STATUS g_status = {0};
static HBRUSH g_dark_brush = NULL;

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

static BYTE *PutBitmapItem(BYTE *p, short x, short y, short cx, short cy)
{
    DWORD style = WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_LEFT;
    p = AlignDword(p);
    p = PutDword(p, style);
    p = PutDword(p, 0);
    p = PutWord(p, (WORD)x);
    p = PutWord(p, (WORD)y);
    p = PutWord(p, (WORD)cx);
    p = PutWord(p, (WORD)cy);
    p = PutWord(p, 0);
    p = PutWord(p, 0xffff);
    p = PutWord(p, 0x0082);
    p = PutWord(p, 0xffff);
    p = PutWord(p, IDB_SKIN);
    return PutWord(p, 0);
}

static BYTE *BeginTemplate(LPCWSTR caption, WORD control_count)
{
    BYTE *p = g_dialog_template;
    p = PutDword(p, WS_POPUP | WS_VISIBLE);
    p = PutDword(p, 0);
    p = PutWord(p, control_count);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    /* 316x184 DLU ocupa aproximadamente 480x272 neste firmware. */
    p = PutWord(p, 316);
    p = PutWord(p, 184);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    p = PutString(p, caption);
    return p;
}

static BYTE *AddLabel(BYTE *p, short x, short y, short cx, short cy,
                      LPCWSTR text)
{
    DWORD style = WS_CHILD | WS_VISIBLE | SS_LEFT | SS_TRANSPARENT;
    return PutItem(p, style, x, y, cx, cy, 0, 0x0082, text);
}

static BYTE *AddCentered(BYTE *p, short x, short y, short cx, short cy,
                         LPCWSTR text)
{
    DWORD style = WS_CHILD | WS_VISIBLE | SS_CENTER | SS_TRANSPARENT;
    return PutItem(p, style, x, y, cx, cy, 0, 0x0082, text);
}

static BYTE *AddHit(BYTE *p, short x, short y, short cx, short cy, WORD id)
{
    DWORD style = WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_TRANSPARENT;
    return PutItem(p, style, x, y, cx, cy, id, 0x0082, L"");
}

static BYTE *BuildDashboard(void)
{
    BYTE *p = BeginTemplate(L"TARDIS", 18);
    (void)g_status;
    p = PutBitmapItem(p, 0, 0, 316, 184);
    p = AddLabel(p, 16, 7, 170, 10, L"TARDIS");
    p = AddLabel(p, 174, 7, 66, 10, L"NETWORK CONTROL");
    p = AddCentered(p, 246, 7, 58, 10, L"OFFLINE");
    p = AddLabel(p, 18, 22, 100, 8, L"INTERNET");
    p = AddLabel(p, 18, 31, 94, 8, L"OFFLINE");
    p = AddLabel(p, 138, 31, 70, 8, L"PING: -- ms");
    p = AddLabel(p, 226, 31, 78, 8, L"LOSS: -- %");
    p = AddLabel(p, 18, 57, 100, 8, L"LATENCY");
    p = AddCentered(p, 18, 70, 286, 9,
                    L"-- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
    p = AddLabel(p, 18, 96, 110, 8, L"DEVICES");
    p = AddLabel(p, 18, 108, 286, 8,
                 L"ROUTER --   PC --   TV --   PS3 --");
    p = AddCentered(p, 8, 143, 94, 12, L"WIFI");
    p = AddCentered(p, 111, 143, 94, 12, L"REDE");
    p = AddCentered(p, 214, 143, 94, 12, L"CONTROLES");
    p = AddHit(p, 8, 136, 94, 28, IDC_WIFI);
    p = AddHit(p, 111, 136, 94, 28, IDC_NETWORK);
    p = AddHit(p, 214, 136, 94, 28, IDC_CONTROLS);
    return p;
}

static BYTE *BuildWifi(void)
{
    BYTE *p = BeginTemplate(L"WIFI", 8);
    p = PutBitmapItem(p, 0, 0, 316, 184);
    p = AddLabel(p, 18, 7, 270, 10, L"< WIFI");
    p = AddLabel(p, 24, 28, 260, 10, L"SSID: --");
    p = AddLabel(p, 24, 43, 260, 10, L"SENHA: ********");
    p = AddCentered(p, 24, 72, 260, 10, L"QR CODE");
    p = AddCentered(p, 24, 87, 260, 10, L"aguardando configuracao");
    p = AddCentered(p, 8, 143, 94, 12, L"VOLTAR");
    p = AddHit(p, 8, 136, 94, 28, IDC_BACK);
    return p;
}

static BYTE *BuildNetwork(void)
{
    BYTE *p = BeginTemplate(L"REDE", 13);
    p = PutBitmapItem(p, 0, 0, 316, 184);
    p = AddLabel(p, 18, 7, 270, 10, L"< REDE");
    p = AddLabel(p, 22, 27, 140, 8, L"Internet   --");
    p = AddLabel(p, 164, 27, 140, 8, L"Router     --");
    p = AddLabel(p, 22, 42, 140, 8, L"PC         --");
    p = AddLabel(p, 164, 42, 140, 8, L"TV         --");
    p = AddLabel(p, 22, 57, 140, 8, L"PS3        --");
    p = AddLabel(p, 164, 57, 140, 8, L"Ping       -- ms");
    p = AddLabel(p, 22, 72, 140, 8, L"Perda      -- %");
    p = AddCentered(p, 8, 143, 94, 12, L"ATUALIZAR");
    p = AddCentered(p, 214, 143, 94, 12, L"VOLTAR");
    p = AddHit(p, 8, 136, 145, 28, IDC_REFRESH);
    p = AddHit(p, 163, 136, 145, 28, IDC_BACK);
    return p;
}

static BYTE *BuildControls(void)
{
    BYTE *p = BeginTemplate(L"CONTROLES", 12);
    p = PutBitmapItem(p, 0, 0, 316, 184);
    p = AddLabel(p, 18, 7, 270, 10, L"< CONTROLES");
    p = AddCentered(p, 24, 29, 268, 10, L"LIGAR PC");
    p = AddCentered(p, 24, 52, 268, 10, L"ATUALIZAR REDE");
    p = AddCentered(p, 24, 75, 268, 10, L"REINICIAR TARDIS");
    p = AddCentered(p, 24, 98, 268, 10, L"DESLIGAR TARDIS");
    p = AddCentered(p, 8, 143, 94, 12, L"VOLTAR");
    p = AddHit(p, 8, 24, 300, 18, IDC_POWER_PC);
    p = AddHit(p, 8, 47, 300, 18, IDC_REFRESH_NET);
    p = AddHit(p, 8, 70, 300, 18, IDC_RESTART);
    p = AddHit(p, 8, 93, 300, 18, IDC_SHUTDOWN);
    p = AddHit(p, 8, 136, 94, 28, IDC_BACK);
    return p;
}

static BYTE *BuildDialogTemplate(void)
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

static BOOL CALLBACK TardisDialogProc(HWND dialog, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
    WORD id;
    (void)dialog;
    (void)lParam;
    if (message == WM_INITDIALOG) {
        g_dark_brush = CreateSolidBrush(RGB(10, 20, 34));
        return TRUE;
    }
    if (message == WM_CTLCOLORDLG || message == WM_CTLCOLORSTATIC) {
        SetTextColor((HDC)wParam, RGB(225, 242, 255));
        return (INT_PTR)g_dark_brush;
    }
    if (message == WM_DESTROY) {
        if (g_dark_brush != NULL) {
            DeleteObject(g_dark_brush);
            g_dark_brush = NULL;
        }
        return TRUE;
    }
    if (message != WM_COMMAND) return FALSE;
    id = LOWORD(wParam);
    if (id == IDC_EXIT) {
        g_quit = TRUE;
        PostQuitMessage(0);
        return TRUE;
    }
    if (id == IDC_WIFI) {
        g_page = PAGE_WIFI;
        PostQuitMessage(0);
        return TRUE;
    }
    if (id == IDC_NETWORK) {
        g_page = PAGE_NETWORK;
        PostQuitMessage(0);
        return TRUE;
    }
    if (id == IDC_CONTROLS) {
        g_page = PAGE_CONTROLS;
        PostQuitMessage(0);
        return TRUE;
    }
    if (id == IDC_BACK) {
        g_page = PAGE_DASHBOARD;
        PostQuitMessage(0);
        return TRUE;
    }
    if (id == IDC_SHOW_PASS || id == IDC_QR || id == IDC_REFRESH ||
        id == IDC_POWER_PC || id == IDC_REFRESH_NET || id == IDC_RESTART ||
        id == IDC_SHUTDOWN) {
        OfflineMessage();
        return TRUE;
    }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous,
                  LPWSTR command_line, int show)
{
    INT_PTR result;
    (void)previous;
    (void)command_line;
    (void)show;
    g_status.latency_count = 0;
    while (!g_quit) {
        BuildDialogTemplate();
        result = DialogBoxIndirectParamW(instance,
                                         (LPCDLGTEMPLATE)g_dialog_template,
                                         NULL, TardisDialogProc, 0);
        if (result == -1) {
            MessageBoxW(NULL, L"TARDIS: erro ao criar janela",
                        L"Windows CE", MB_OK);
            return 1;
        }
    }
    return 0;
}
