#ifndef _BITMAP_INCLUDED
#define _BITMAP_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

HBITMAP LoadBitmap2(HDC hdc,HINSTANCE hInstance, char *bmpfile);
HBITMAP LoadBitmap3(HDC hdc,HINSTANCE hInstance, char *bmpfile);

#ifdef __cplusplus
}
#endif

#endif