#ifndef _PALETTE_INCLUDED
#define _PALETTE_INCLUDED

#include <windows.h>

#define MATRIX_DEFAULT_COLOR RGB(120,255,119)

#ifdef __cplusplus
extern "C" {
#endif

COLORREF HueToColor(int hue);
int ColorToHue(COLORREF color);
HPALETTE ReadPalette(HINSTANCE hInstance, const TCHAR *bmpfile);
HPALETTE UseNicePalette(HDC hdc, HPALETTE hPalette);
HPALETTE ReadBMPPalette(HINSTANCE hInstance, HDC hdc, const TCHAR *bmpfile);

#ifdef __cplusplus
}
#endif



#endif
