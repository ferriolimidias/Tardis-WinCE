/* serial_probe.exe - abre e fecha COM2..COM9 sem transmitir dados. */
#include <windows.h>
#pragma comment(lib, "coredll.lib")

#define PORT_COUNT 8

static BYTE g_template[4096];
static BOOL g_opened[PORT_COUNT];

static BYTE *PutWord(BYTE *p, WORD value) { *(WORD *)p = value; return p + 2; }
static BYTE *PutDword(BYTE *p, DWORD value) { *(DWORD *)p = value; return p + 4; }
static BYTE *PutString(BYTE *p, LPCWSTR text) { while (*text) p = PutWord(p, (WORD)*text++); return PutWord(p, 0); }
static BYTE *AlignDword(BYTE *p) { DWORD n = (DWORD)(p - g_template); return g_template + ((n + 3u) & ~3u); }

static BYTE *PutStatic(BYTE *p, short x, short y, short cx, short cy,
                       LPCWSTR text)
{
    p = AlignDword(p);
    p = PutDword(p, WS_CHILD | WS_VISIBLE | SS_LEFT);
    p = PutDword(p, 0);
    p = PutWord(p, (WORD)x); p = PutWord(p, (WORD)y);
    p = PutWord(p, (WORD)cx); p = PutWord(p, (WORD)cy);
    p = PutWord(p, 0);
    p = PutWord(p, 0xffff); p = PutWord(p, 0x0082);
    p = PutString(p, text);
    return PutWord(p, 0);
}

static BYTE *BeginTemplate(void)
{
    BYTE *p = g_template;
    p = PutDword(p, WS_POPUP | WS_VISIBLE);
    p = PutDword(p, 0);
    p = PutWord(p, 9);
    p = PutWord(p, 0); p = PutWord(p, 0);
    p = PutWord(p, 316); p = PutWord(p, 184);
    p = PutWord(p, 0); p = PutWord(p, 0);
    return PutString(p, L"SERIAL PROBE");
}

static LPCWSTR PortText(int index)
{
    if (g_opened[index]) {
        switch (index) {
        case 0: return L"COM2   ABRIU";
        case 1: return L"COM3   ABRIU";
        case 2: return L"COM4   ABRIU";
        case 3: return L"COM5   ABRIU";
        case 4: return L"COM6   ABRIU";
        case 5: return L"COM7   ABRIU";
        case 6: return L"COM8   ABRIU";
        default: return L"COM9   ABRIU";
        }
    }
    switch (index) {
    case 0: return L"COM2   FALHOU";
    case 1: return L"COM3   FALHOU";
    case 2: return L"COM4   FALHOU";
    case 3: return L"COM5   FALHOU";
    case 4: return L"COM6   FALHOU";
    case 5: return L"COM7   FALHOU";
    case 6: return L"COM8   FALHOU";
    default: return L"COM9   FALHOU";
    }
}

static BYTE *BuildTemplate(void)
{
    BYTE *p = BeginTemplate();
    int i;
    p = PutStatic(p, 24, 12, 260, 12, L"PORTAS TESTADAS");
    for (i = 0; i < PORT_COUNT; ++i) {
        p = PutStatic(p, 24, (short)(30 + i * 16), 260, 12, PortText(i));
    }
    return p;
}

static void ProbePorts(void)
{
    static LPCWSTR names[PORT_COUNT] = {
        L"COM2:", L"COM3:", L"COM4:", L"COM5:",
        L"COM6:", L"COM7:", L"COM8:", L"COM9:"
    };
    int i;
    for (i = 0; i < PORT_COUNT; ++i) {
        HANDLE handle = CreateFileW(names[i],
                                    GENERIC_READ | GENERIC_WRITE,
                                    0, NULL, OPEN_EXISTING, 0, NULL);
        if (handle != INVALID_HANDLE_VALUE) {
            g_opened[i] = TRUE;
            CloseHandle(handle);
        } else {
            g_opened[i] = FALSE;
        }
    }
}

static BOOL CALLBACK ProbeDialogProc(HWND dialog, UINT message,
                                     WPARAM wParam, LPARAM lParam)
{
    (void)dialog; (void)wParam; (void)lParam;
    if (message == WM_INITDIALOG) return TRUE;
    return FALSE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous,
                  LPWSTR command_line, int show)
{
    INT_PTR result;
    (void)previous; (void)command_line; (void)show;
    ProbePorts();
    BuildTemplate();
    result = DialogBoxIndirectParamW(instance,
                (LPCDLGTEMPLATE)g_template,
                NULL, ProbeDialogProc, 0);
    return (result == -1) ? 1 : 0;
}
