// Minimal implementations of the desktop Win32 APIs that the Win2000 calc
// expects but Windows CE does not export. See ce_compat.h for the surface
// area. The intent is to compile and run, not to faithfully reproduce help
// or about-box behavior — Windows CE has no HTML Help engine of its own.

#include <windows.h>
#include <commctrl.h>
#include "ce_compat.h"

static const WCHAR kCeProfileRegPath[] = L"Software\\CECalc";

static HKEY OpenSectionKey(LPCWSTR section, BOOL writable)
{
    WCHAR path[256];
    HKEY hKey = NULL;
    DWORD disp = 0;

    wcscpy(path, kCeProfileRegPath);
    if (section && *section) {
        wcscat(path, L"\\");
        wcscat(path, section);
    }

    if (writable) {
        if (RegCreateKeyExW(HKEY_CURRENT_USER, path, 0, NULL, 0, 0, NULL,
                            &hKey, &disp) != ERROR_SUCCESS)
            return NULL;
    } else {
        if (RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, 0, &hKey)
                != ERROR_SUCCESS)
            return NULL;
    }
    return hKey;
}

extern "C" UINT WINAPI CeProfileGetInt(LPCWSTR section, LPCWSTR key,
                                       INT defaultVal)
{
    HKEY hKey = OpenSectionKey(section, FALSE);
    if (!hKey)
        return (UINT)defaultVal;

    WCHAR buf[32];
    DWORD cb = sizeof(buf);
    DWORD type = 0;
    UINT result = (UINT)defaultVal;

    if (RegQueryValueExW(hKey, key, NULL, &type, (LPBYTE)buf, &cb)
            == ERROR_SUCCESS) {
        if (type == REG_DWORD && cb >= sizeof(DWORD)) {
            result = *(const DWORD*)buf;
        } else if (type == REG_SZ) {
            INT v = 0;
            const WCHAR *p = buf;
            while (*p == L' ' || *p == L'\t') p++;
            int sign = 1;
            if (*p == L'-') { sign = -1; p++; }
            while (*p >= L'0' && *p <= L'9') {
                v = v * 10 + (*p - L'0');
                p++;
            }
            result = (UINT)(sign * v);
        }
    }
    RegCloseKey(hKey);
    return result;
}

extern "C" DWORD WINAPI CeProfileGetString(LPCWSTR section, LPCWSTR key,
                                           LPCWSTR defaultVal,
                                           LPWSTR out, DWORD cchOut)
{
    HKEY hKey = OpenSectionKey(section, FALSE);
    if (hKey) {
        DWORD type = 0;
        DWORD cbOut = cchOut * sizeof(WCHAR);
        if (RegQueryValueExW(hKey, key, NULL, &type,
                             (LPBYTE)out, &cbOut) == ERROR_SUCCESS &&
                (type == REG_SZ || type == REG_EXPAND_SZ)) {
            RegCloseKey(hKey);
            DWORD cch = cbOut / sizeof(WCHAR);
            if (cch > 0 && out[cch - 1] == 0) cch--;
            return cch;
        }
        RegCloseKey(hKey);
    }
    if (!defaultVal) defaultVal = L"";
    DWORD i = 0;
    while (i + 1 < cchOut && defaultVal[i]) {
        out[i] = defaultVal[i];
        i++;
    }
    if (cchOut > 0) out[i] = 0;
    return i;
}

extern "C" BOOL WINAPI CeProfileWriteString(LPCWSTR section, LPCWSTR key,
                                            LPCWSTR value)
{
    HKEY hKey = OpenSectionKey(section, TRUE);
    if (!hKey) return FALSE;

    LONG r;
    if (value == NULL) {
        r = RegDeleteValueW(hKey, key);
    } else {
        DWORD cb = (DWORD)((wcslen(value) + 1) * sizeof(WCHAR));
        r = RegSetValueExW(hKey, key, 0, REG_SZ, (const BYTE*)value, cb);
    }
    RegCloseKey(hKey);
    return r == ERROR_SUCCESS;
}

extern "C" BOOL WINAPI HtmlHelpW(HWND hwnd, LPCWSTR file, UINT cmd,
                                 DWORD_PTR data)
{
    return FALSE;
}

extern "C" BOOL WINAPI CeStubWinHelpW(HWND hwnd, LPCWSTR file, UINT cmd,
                                      DWORD data)
{
    return FALSE;
}

extern "C" int WINAPI CeShellAboutW(HWND hwnd, LPCWSTR app, LPCWSTR other,
                                    HICON icon)
{
    WCHAR title[128] = L"About ";
    WCHAR body[256];
    if (app) wcsncat(title, app, 110);
    body[0] = 0;
    if (app) {
        wcscat(body, app);
        wcscat(body, L"\r\n");
    }
    wcscat(body, L"Windows CE port of the Win2000 Scientific Calculator.");
    MessageBoxW(hwnd, body, title, MB_OK | MB_ICONINFORMATION);
    return 1;
}

extern "C" ATOM WINAPI CeRegisterClassEx(const WNDCLASSEX_CE *wcex)
{
    if (!wcex) return 0;
    WNDCLASSW wc;
    wc.style         = wcex->style;
    wc.lpfnWndProc   = wcex->lpfnWndProc;
    wc.cbClsExtra    = wcex->cbClsExtra;
    wc.cbWndExtra    = wcex->cbWndExtra;
    wc.hInstance     = wcex->hInstance;
    wc.hIcon         = wcex->hIcon;
    wc.hCursor       = wcex->hCursor;
    wc.hbrBackground = wcex->hbrBackground;
    wc.lpszMenuName  = wcex->lpszMenuName;
    wc.lpszClassName = wcex->lpszClassName;
    return RegisterClassW(&wc);
}

static const WCHAR kMenuProp[] = L"CECalc.Menu";
static const WCHAR kMenuBarProp[] = L"CECalc.MenuBar";
static const WCHAR kMenuBarHeightProp[] = L"CECalc.MenuBarHeight";

static void OffsetDialogChildren(HWND hwnd, HWND hwndSkip, int dy)
{
    HWND hwndChild;

    if (!dy)
        return;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild;
         hwndChild = GetWindow(hwndChild, GW_HWNDNEXT)) {
        RECT rc;

        if (hwndChild == hwndSkip)
            continue;

        if (GetWindowRect(hwndChild, &rc)) {
            MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
            SetWindowPos(hwndChild, NULL, rc.left, rc.top + dy, 0, 0,
                         SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }
}

extern "C" HMENU WINAPI CeGetMenu(HWND hwnd)
{
    return (HMENU)GetProp(hwnd, kMenuProp);
}

extern "C" int WINAPI CeGetMenuBarHeight(HWND hwnd)
{
    return (int)(LONG_PTR)GetProp(hwnd, kMenuBarHeightProp);
}

extern "C" BOOL WINAPI CeSetMenu(HWND hwnd, HMENU menu)
{
    HWND hwndMenuBar = (HWND)GetProp(hwnd, kMenuBarProp);

    if (hwndMenuBar) {
        DestroyWindow(hwndMenuBar);
        RemoveProp(hwnd, kMenuBarProp);
        RemoveProp(hwnd, kMenuBarHeightProp);
    }

    if (menu == NULL)
        RemoveProp(hwnd, kMenuProp);
    else
        SetProp(hwnd, kMenuProp, (HANDLE)menu);

    return TRUE;
}

extern "C" BOOL WINAPI CeSetMenuResource(HWND hwnd, HINSTANCE hinst,
                                         UINT menuId, HMENU menu)
{
    RECT rc;
    int cy = 0;
    int cyOld = CeGetMenuBarHeight(hwnd);
    HWND hwndMenuBar = (HWND)GetProp(hwnd, kMenuBarProp);

    if (hwndMenuBar) {
        DestroyWindow(hwndMenuBar);
        RemoveProp(hwnd, kMenuBarProp);
        RemoveProp(hwnd, kMenuBarHeightProp);
    }

    if (menu == NULL || menuId == 0) {
        RemoveProp(hwnd, kMenuProp);
        OffsetDialogChildren(hwnd, NULL, -cyOld);
        if (cyOld && GetWindowRect(hwnd, &rc)) {
            SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left,
                         rc.bottom - rc.top - cyOld,
                         SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
        }
        return TRUE;
    }

    SetProp(hwnd, kMenuProp, (HANDLE)menu);

    hwndMenuBar = CommandBar_Create(hinst, hwnd, 1);
    if (!hwndMenuBar)
        return FALSE;

    if (!CommandBar_InsertMenubar(hwndMenuBar, hinst, (WORD)menuId, 0)) {
        DestroyWindow(hwndMenuBar);
        return FALSE;
    }

    CommandBar_Show(hwndMenuBar, TRUE);
    CommandBar_DrawMenuBar(hwndMenuBar, 0);
    ShowWindow(hwndMenuBar, SW_SHOW);
    UpdateWindow(hwndMenuBar);

    cy = CommandBar_Height(hwndMenuBar);
    if (cy <= 0 && GetWindowRect(hwndMenuBar, &rc))
        cy = rc.bottom - rc.top;

    HMENU liveMenu = CommandBar_GetMenu(hwndMenuBar, 0);
    SetProp(hwnd, kMenuProp, (HANDLE)(liveMenu ? liveMenu : menu));
    SetProp(hwnd, kMenuBarProp, (HANDLE)hwndMenuBar);
    SetProp(hwnd, kMenuBarHeightProp, (HANDLE)(LONG_PTR)cy);

    if (cy != cyOld && GetWindowRect(hwnd, &rc)) {
        OffsetDialogChildren(hwnd, hwndMenuBar, cy - cyOld);
        SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left,
                     rc.bottom - rc.top + cy - cyOld,
                     SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    return TRUE;
}

extern "C" LPSTR WINAPI CeCharNextA(LPCSTR p)
{
    // Calc only pastes from CF_TEXT, which is single-byte ASCII for this
    // app's purposes; advancing by one byte is sufficient.
    if (!p || !*p) return (LPSTR)p;
    return (LPSTR)(p + 1);
}

extern "C" SIZE_T WINAPI CeGlobalCompact(DWORD)
{
    return 0;
}
