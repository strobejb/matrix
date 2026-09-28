#include <windows.h>
#include "palette.h"

extern COLORREF MatrixColor;

HBITMAP hDDB;

BYTE ScaleColor(BYTE color, BYTE brightness)
{
	return (BYTE)((int)color * brightness / 255);
}

BYTE Max3(BYTE a, BYTE b, BYTE c)
{
	return max(a, max(b, c));
}

void CopyPaletteEntry(PALETTEENTRY *entry, RGBQUAD *rgb)
{
	BYTE brightness = Max3(rgb->rgbRed, rgb->rgbGreen, rgb->rgbBlue);

	if(MatrixColor == MATRIX_DEFAULT_COLOR)
	{
		entry->peBlue  = rgb->rgbBlue;
		entry->peGreen = rgb->rgbGreen;
		entry->peRed   = rgb->rgbRed;
	}
	else
	{
		entry->peBlue  = ScaleColor(GetBValue(MatrixColor), brightness);
		entry->peGreen = ScaleColor(GetGValue(MatrixColor), brightness);
		entry->peRed   = ScaleColor(GetRValue(MatrixColor), brightness);
	}

	entry->peFlags = 0;
}

COLORREF HueToColor(int hue)
{
	int r = 0, g = 0, b = 0;
	int x;

	hue %= 360;
	if(hue < 0) hue += 360;

	x = 255 * (60 - abs(hue % 120 - 60)) / 60;

	if(hue < 60)
	{
		r = 255; g = x;
	}
	else if(hue < 120)
	{
		r = x; g = 255;
	}
	else if(hue < 180)
	{
		g = 255; b = x;
	}
	else if(hue < 240)
	{
		g = x; b = 255;
	}
	else if(hue < 300)
	{
		r = x; b = 255;
	}
	else
	{
		r = 255; b = x;
	}

	return RGB(r, g, b);
}

int ColorToHue(COLORREF color)
{
	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);
	int maxc = max(r, max(g, b));
	int minc = min(r, min(g, b));
	int delta = maxc - minc;
	int hue;

	if(delta == 0)
		return 120;

	if(maxc == r)
		hue = 60 * (g - b) / delta;
	else if(maxc == g)
		hue = 120 + 60 * (b - r) / delta;
	else
		hue = 240 + 60 * (r - g) / delta;

	if(hue < 0) hue += 360;
	return hue;
}

HPALETTE ReadPalette(HINSTANCE hInstance, const TCHAR *bmpfile)
{
	HPALETTE hPalette;
	LOGPALETTE *lp;

	HANDLE hFile = INVALID_HANDLE_VALUE, hMap = INVALID_HANDLE_VALUE;

	HRSRC hrsrc = 0;
	HANDLE hResource = 0;
	
	BYTE *pmem;
	BITMAPFILEHEADER *bfh;
	
	BITMAPINFO *bi;
	BITMAPINFOHEADER *bih;
	
	
	if(hInstance == 0)
	{
		hFile = CreateFile(bmpfile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);

		if(hFile == INVALID_HANDLE_VALUE) return 0;

		hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);

		if(hMap == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return 0;
		}

		pmem = (BYTE *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		
		bfh = (BITMAPFILEHEADER *)pmem;
		bih = (BITMAPINFOHEADER *)(pmem + sizeof BITMAPFILEHEADER);
	}
	else
	{
		hrsrc = FindResource(hInstance, bmpfile, RT_BITMAP);
		hResource = LoadResource(hInstance, hrsrc);
		pmem = (BYTE *)LockResource(hResource);
	
		//there is no bitmap file header in a resource
		bih = (BITMAPINFOHEADER *)pmem;
	}
	
	if(!bih) return 0;

	unsigned numcols = bih->biClrUsed;
	if(!numcols) numcols = 1 << bih->biBitCount;
	//now read the bitmap
	lp = (LOGPALETTE *)HeapAlloc(GetProcessHeap(), 0, (sizeof LOGPALETTE) + (sizeof PALETTEENTRY) * numcols);

	lp->palNumEntries = (WORD)numcols;
	lp->palVersion = 0x300;
	
	bi = (BITMAPINFO *)bih;
	if(!bi) return 0;
	for(unsigned i = 0; i < numcols; i++)
	{
		CopyPaletteEntry(&lp->palPalEntry[i], &bi->bmiColors[i]);
	}

	hPalette = CreatePalette(lp);

	if(hInstance == 0)
	{
		UnmapViewOfFile(pmem);
		CloseHandle(hMap);
		CloseHandle(hFile);
	}
	else
	{
		DeleteObject(hResource);
	}
	
	return hPalette;
}

HPALETTE UseNicePalette(HDC hdc, HPALETTE hPalette)
{
	HPALETTE hp;
	hp = SelectPalette(hdc, hPalette, FALSE);
	RealizePalette(hdc);
	return hp;
}



HPALETTE ReadBMPPalette(HINSTANCE hInstance, HDC hdc, const TCHAR *bmpfile)
{
	HPALETTE hPalette;
	LOGPALETTE *lp;

	HANDLE hFile = INVALID_HANDLE_VALUE, hMap = INVALID_HANDLE_VALUE;

	HRSRC hrsrc;
	HANDLE hResource = NULL ;
	
	BYTE *pmem;
	BITMAPFILEHEADER *bfh;
	
	BITMAPINFO *bi;
	BITMAPINFO *biTint;
	BITMAPINFOHEADER *bih;
	
	
	if(hInstance == 0)
	{
		hFile = CreateFile(bmpfile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);

		if(hFile == 0) return 0;

		hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);

		if(hMap == 0)
		{
			CloseHandle(hFile);
			return 0;
		}

		pmem = (BYTE *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		
		bfh = (BITMAPFILEHEADER *)pmem;
		bih = (BITMAPINFOHEADER *)(pmem + sizeof BITMAPFILEHEADER);
	}
	else
	{
		hrsrc = FindResource(hInstance, bmpfile, RT_BITMAP);
		hResource = LoadResource(hInstance, hrsrc);
		pmem = (BYTE *)LockResource(hResource);
	
		//there is no bitmap file header in a resource
		bih = (BITMAPINFOHEADER *)pmem;
	}
	
	if(!bih) return 0;

	unsigned numcols = bih->biClrUsed;
	if(!numcols) numcols = 1 << bih->biBitCount;
	//now read the bitmap
	lp = (LOGPALETTE *)HeapAlloc(GetProcessHeap(), 0, (sizeof LOGPALETTE) + (sizeof PALETTEENTRY) * numcols);

	lp->palNumEntries = (WORD)numcols;
	lp->palVersion = 0x300;
	
	bi = (BITMAPINFO *)bih;
	if(!bi) return 0;

	biTint = (BITMAPINFO *)HeapAlloc(GetProcessHeap(), 0, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * numcols));
	if(!biTint) return 0;

	CopyMemory(&biTint->bmiHeader, &bi->bmiHeader, sizeof(BITMAPINFOHEADER));

	for(unsigned i = 0; i < numcols; i++)
	{
		CopyPaletteEntry(&lp->palPalEntry[i], &bi->bmiColors[i]);
		biTint->bmiColors[i].rgbBlue  = lp->palPalEntry[i].peBlue;
		biTint->bmiColors[i].rgbGreen = lp->palPalEntry[i].peGreen;
		biTint->bmiColors[i].rgbRed   = lp->palPalEntry[i].peRed;
		biTint->bmiColors[i].rgbReserved = 0;
	}

	hPalette = CreatePalette(lp);
	UseNicePalette(hdc, hPalette);

	/* now the bitmap!!! */
	void * pDIBBits;

	if(bi->bmiHeader.biBitCount > 8 )
		pDIBBits = (void *)((WORD *)(bi->bmiColors + bi->bmiHeader.biClrUsed) + 
			((bi->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
	else
		pDIBBits = (void *)(bi->bmiColors + numcols);

	
	
	hDDB = CreateDIBitmap(hdc,			// handle to device context
			(BITMAPINFOHEADER *)&biTint->bmiHeader,	// pointer to bitmap info header
			(LONG)CBM_INIT,				// initialization flag
			pDIBBits,					// pointer to initialization data 
			(BITMAPINFO *)biTint,		// pointer to bitmap info
			DIB_RGB_COLORS);			// color-data usage 

	HeapFree(GetProcessHeap(), 0, biTint);
	
	if(hInstance == 0)
	{
		UnmapViewOfFile(pmem);
		CloseHandle(hMap);
		CloseHandle(hFile);
	}
	else
	{
		DeleteObject(hResource);
	}
	
	return hPalette;
}
