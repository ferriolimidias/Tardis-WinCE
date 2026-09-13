/*
 * Tardis.exe - painel de rede para o Windows CE OEM.
 *
 * A imagem do GPS foi validada com um conjunto pequeno de imports. Esta
 * implementação usa somente controles nativos de diálogo e troca páginas
 * reconstruindo o template. Não há GDI, timers, rede ou dados simulados.
 */
#include <windows.h>

#pragma comment(lib, "coredll.lib")

#define PAGE_DASHBOARD 0
#define PAGE_WIFI      1
#define PAGE_NETWORK   2
#define PAGE_CONTROLS  3

#define IDC_WIFI        100
#define IDC_NETWORK     101
#define IDC_CONTROLS    102
#define IDC_BACK        103
#define IDC_EXIT        104
#define IDC_SHOW_PASS   105
#define IDC_QR          106
#define IDC_REFRESH     107
#define IDC_POWER_PC    108
#define IDC_REFRESH_NET 109
#define IDC_RESTART     110
#define IDC_SHUTDOWN    111

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

static BYTE *BeginTemplate(LPCWSTR caption)
{
    BYTE *p = g_dialog_template;
    p = PutDword(p, WS_POPUP | WS_VISIBLE | DS_SETFONT);
    p = PutDword(p, 0);
    p = PutWord(p, 8);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    /* 316x184 DLU ocupa a área útil aproximada de 480x272 neste firmware. */
    p = PutWord(p, 316);
    p = PutWord(p, 184);
    p = PutWord(p, 0);
    p = PutWord(p, 0);
    p = PutString(p, caption);
    p = PutWord(p, 8);
    p = PutString(p, L"Tahoma");
    return p;
}

static DWORD ChildStyle(void)
{
    return WS_CHILD | WS_VISIBLE;
}

static DWORD ButtonStyle(void)
{
    return WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
}

static BYTE *AddLabel(BYTE *p, short x, short y, short cx, short cy,
                      LPCWSTR text)
{
    return PutItem(p, ChildStyle() | SS_LEFT, x, y, cx, cy, 0,
                   0x0082, text);
}

static BYTE *AddCentered(BYTE *p, short x, short y, short cx, short cy,
                         LPCWSTR text)
{
    return PutItem(p, ChildStyle() | SS_CENTER, x, y, cx, cy, 0,
                   0x0082, text);
}

static BYTE *AddButton(BYTE *p, short x, short y, short cx, short cy,
                       WORD id, LPCWSTR text)
{
    return PutItem(p, ButtonStyle(), x, y, cx, cy, id, 0x0080, text);
}

static BYTE *AddGroup(BYTE *p, short x, short y, short cx, short cy,
                      LPCWSTR text)
{
    return PutItem(p, ChildStyle() | BS_GROUPBOX, x, y, cx, cy, 0,
                   0x0080, text);
}

static BYTE *BuildDashboard(void)
{
    BYTE *p = BeginTemplate(L"TARDIS");
    (void)g_status;
    p = AddLabel(p, 10, 5, 220, 10, L"TARDIS  |  PAINEL DE REDE");
    p = AddCentered(p, 244, 5, 62, 10, L"[ OFFLINE ]");
    p = AddGroup(p, 8, 18, 300, 32, L"INTERNET");
    p = AddLabel(p, 16, 29, 284, 8, L"OFFLINE       Ping: -- ms       Perda: -- %");
    p = AddGroup(p, 8, 54, 300, 35, L"LATENCIA");
    p = AddLabel(p, 16, 67, 284, 9, L"-- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
    p = AddGroup(p, 8, 93, 300, 31, L"DISPOSITIVOS");
    p = AddLabel(p, 16, 106, 284, 8, L"Roteador --   PC --   TV --   PS3 --");
    p = AddButton(p, 8, 136, 94, 24, IDC_WIFI, L"WIFI");
    p = AddButton(p, 111, 136, 94, 24, IDC_NETWORK, L"REDE");
    p = AddButton(p, 214, 136, 94, 24, IDC_CONTROLS, L"CONTROLES");
    return p;
}

static BYTE *BuildWifi(void)
{
    BYTE *p = BeginTemplate(L"WIFI");
    p = AddLabel(p, 8, 4, 288, 8, L"< WIFI");
    p = AddGroup(p, 8, 18, 300, 42, L"REDE");
    p = AddLabel(p, 16, 30, 284, 8, L"SSID: --");
    p = AddLabel(p, 16, 44, 284, 8, L"Senha: ********");
    p = AddButton(p, 8, 68, 145, 24, IDC_SHOW_PASS, L"MOSTRAR SENHA");
    p = AddButton(p, 163, 68, 145, 24, IDC_QR, L"QR CODE");
    p = AddGroup(p, 8, 98, 300, 30, L"QR CODE");
    p = AddCentered(p, 16, 110, 284, 8, L"aguardando configuracao");
    p = AddButton(p, 8, 140, 94, 24, IDC_BACK, L"VOLTAR");
    return p;
}

static BYTE *BuildNetwork(void)
{
    BYTE *p = BeginTemplate(L"REDE");
    p = AddLabel(p, 8, 4, 288, 8, L"< REDE");
    p = AddGroup(p, 8, 18, 300, 82, L"ESTADO DA REDE");
    p = AddLabel(p, 16, 30, 140, 8, L"Internet   --");
    p = AddLabel(p, 164, 30, 136, 8, L"Roteador   --");
    p = AddLabel(p, 16, 44, 140, 8, L"PC         --");
    p = AddLabel(p, 164, 44, 136, 8, L"TV         --");
    p = AddLabel(p, 16, 58, 140, 8, L"PS3        --");
    p = AddLabel(p, 164, 58, 136, 8, L"Ping       -- ms");
    p = AddLabel(p, 16, 76, 284, 8, L"Perda      -- %");
    p = AddButton(p, 8, 110, 145, 24, IDC_REFRESH, L"ATUALIZAR");
    p = AddButton(p, 163, 110, 145, 24, IDC_BACK, L"VOLTAR");
    return p;
}

static BYTE *BuildControls(void)
{
    BYTE *p = BeginTemplate(L"CONTROLES");
    p = AddLabel(p, 8, 4, 288, 8, L"< CONTROLES");
    p = AddGroup(p, 8, 18, 300, 112, L"ACOES");
    p = AddButton(p, 16, 31, 284, 18, IDC_POWER_PC, L"LIGAR PC");
    p = AddButton(p, 16, 53, 284, 18, IDC_REFRESH_NET, L"ATUALIZAR REDE");
    p = AddButton(p, 16, 75, 284, 18, IDC_RESTART, L"REINICIAR TARDIS");
    p = AddButton(p, 16, 97, 284, 18, IDC_SHUTDOWN, L"DESLIGAR TARDIS");
    p = AddButton(p, 8, 140, 94, 24, IDC_BACK, L"VOLTAR");
    p = AddButton(p, 214, 140, 94, 24, IDC_EXIT, L"SAIR");
    return p;
}

static BYTE *BuildDialogTemplate(void)
{
    if (g_page == PAGE_WIFI) {
        return BuildWifi();
    }
    if (g_page == PAGE_NETWORK) {
        return BuildNetwork();
    }
    if (g_page == PAGE_CONTROLS) {
        return BuildControls();
    }
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
    if (message != WM_COMMAND) {
        return message == WM_INITDIALOG ? TRUE : FALSE;
    }
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
