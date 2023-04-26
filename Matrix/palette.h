#ifndef _PALETTE_INCLUDED
#define _PALETTE_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

HPALETTE ReadPalette(HINSTANCE hInstance, const TCHAR *bmpfile);
HPALETTE UseNicePalette(HDC hdc, HPALETTE hPalette);
HPALETTE ReadBMPPalette(HINSTANCE hInstance, HDC hdc, const TCHAR *bmpfile);

#ifdef __cplusplus
}
#endif



#endif