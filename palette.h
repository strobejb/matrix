#ifndef _PALETTE_INCLUDED
#define _PALETTE_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

HPALETTE ReadPalette(HINSTANCE hInstance, const char *bmpfile);
HPALETTE UseNicePalette(HDC hdc, HPALETTE hPalette);
HPALETTE ReadBMPPalette(HINSTANCE hInstance, HDC hdc, const char *bmpfile);

#ifdef __cplusplus
}
#endif



#endif