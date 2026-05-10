// CE compatibility shims for desktop Win32 APIs that don't exist on Windows CE.
//
// The Win2000 calc relies on:
//   - HtmlHelp / WinHelp / ShellAbout      -> stubbed (no help on CE)
//   - GetProfileInt / GetProfileString /
//     WriteProfileString                   -> redirected to HKCU\Software\CECalc
//   - RegisterClassEx                      -> wrapped over RegisterClass
//   - CharNextA                            -> single-byte step over ANSI clipboard text

#ifndef CE_COMPAT_H
#define CE_COMPAT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// htmlhelp.h does not exist on CE.
#define HH_DISPLAY_TOPIC 0
#define HH_HELP_CONTEXT  0xF
BOOL WINAPI HtmlHelpW(HWND hwnd, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData);
#define HtmlHelp HtmlHelpW

// WinHelp is in the W2K headers but not in the CE coredll.
#undef WinHelp
BOOL WINAPI CeStubWinHelpW(HWND hwnd, LPCWSTR file, UINT cmd, DWORD data);
#define WinHelp CeStubWinHelpW

// ShellAbout is declared in shellapi.h but not exported on CE.
#undef ShellAbout
int WINAPI CeShellAboutW(HWND hwnd, LPCWSTR app, LPCWSTR other, HICON icon);
#define ShellAbout CeShellAboutW

// WIN.INI didn't survive the trip to Windows CE; redirect to the registry.
#undef GetProfileIntW
#undef GetProfileStringW
#undef WriteProfileStringW
#undef GetProfileInt
#undef GetProfileString
#undef WriteProfileString

UINT  WINAPI CeProfileGetInt    (LPCWSTR section, LPCWSTR key, INT defaultVal);
DWORD WINAPI CeProfileGetString (LPCWSTR section, LPCWSTR key, LPCWSTR defaultVal,
                                 LPWSTR out, DWORD cchOut);
BOOL  WINAPI CeProfileWriteString(LPCWSTR section, LPCWSTR key, LPCWSTR value);

#define GetProfileInt        CeProfileGetInt
#define GetProfileString     CeProfileGetString
#define WriteProfileString   CeProfileWriteString
#define GetProfileIntW       CeProfileGetInt
#define GetProfileStringW    CeProfileGetString
#define WriteProfileStringW  CeProfileWriteString

// RegisterClassEx is not available on CE; wrap it onto plain RegisterClass.
typedef struct tagWNDCLASSEX_CE {
    UINT      cbSize;
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra;
    int       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCWSTR   lpszMenuName;
    LPCWSTR   lpszClassName;
    HICON     hIconSm;
} WNDCLASSEX_CE, *LPWNDCLASSEX_CE;

ATOM WINAPI CeRegisterClassEx(const WNDCLASSEX_CE *wcex);

#undef WNDCLASSEX
#undef WNDCLASSEXW
#undef PWNDCLASSEX
#undef PWNDCLASSEXW
#undef LPWNDCLASSEX
#undef LPWNDCLASSEXW
#undef RegisterClassEx
#undef RegisterClassExW
#define WNDCLASSEX     WNDCLASSEX_CE
#define WNDCLASSEXW    WNDCLASSEX_CE
#define LPWNDCLASSEX   LPWNDCLASSEX_CE
#define LPWNDCLASSEXW  LPWNDCLASSEX_CE
#define RegisterClassEx  CeRegisterClassEx
#define RegisterClassExW CeRegisterClassEx

// CharNextA does not exist on CE; clipboard ANSI parsing steps single bytes.
#undef CharNextA
LPSTR WINAPI CeCharNextA(LPCSTR p);
#define CharNextA CeCharNextA

// GlobalCompact is declared but not exported. It's a no-op on modern Win32
// anyway — return 0 so callers fall through their failure paths cleanly.
#undef GlobalCompact
SIZE_T WINAPI CeGlobalCompact(DWORD minFree);
#define GlobalCompact CeGlobalCompact

// CE has no GetMenu / SetMenu in coredll: shim via GetProp/SetProp so that
// the calc's "fetch the menu I just attached" pattern keeps working.
HMENU WINAPI CeGetMenu(HWND hwnd);
BOOL  WINAPI CeSetMenu(HWND hwnd, HMENU menu);
#define GetMenu CeGetMenu
#define SetMenu CeSetMenu

// W2K-isms used by sciproc/scimenu that CE headers do not provide.
#ifndef GetWindowID
#define GetWindowID(hwnd) GetDlgCtrlID(hwnd)
#endif

#ifndef GET_WM_COMMAND_MPS
#define GET_WM_COMMAND_MPS(id, hwnd, code) \
    (WPARAM)MAKELONG((id), (code)), (LPARAM)(hwnd)
#endif
#ifndef GET_WM_COMMAND_ID
#define GET_WM_COMMAND_ID(wp, lp)   LOWORD(wp)
#endif
#ifndef GET_WM_COMMAND_CMD
#define GET_WM_COMMAND_CMD(wp, lp)  HIWORD(wp)
#endif

// HELP_* values referenced by stubbed WinHelp calls.
#ifndef HELP_QUIT
#define HELP_QUIT          0x0002
#define HELP_CONTEXTPOPUP  0x0008
#define HELP_CONTEXTMENU   0x000A
#define HELP_WM_HELP       0x000C
#endif

#ifdef __cplusplus
}
#endif

#endif // CE_COMPAT_H
