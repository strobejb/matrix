#include <windows.h>
#include "palette.h"

HBITMAP hDDB;

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
		lp->palPalEntry[i].peBlue  = bi->bmiColors[i].rgbBlue;
		lp->palPalEntry[i].peGreen = bi->bmiColors[i].rgbGreen;
		lp->palPalEntry[i].peRed   = bi->bmiColors[i].rgbRed;
		lp->palPalEntry[i].peFlags = 0;
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
	for(unsigned i = 0; i < numcols; i++)
	{
		lp->palPalEntry[i].peBlue  = bi->bmiColors[i].rgbBlue;
		lp->palPalEntry[i].peGreen = bi->bmiColors[i].rgbGreen;
		lp->palPalEntry[i].peRed   = bi->bmiColors[i].rgbRed;
		lp->palPalEntry[i].peFlags = 0;
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
			(BITMAPINFOHEADER *)bih,	// pointer to bitmap info header 
			(LONG)CBM_INIT,				// initialization flag
			pDIBBits,					// pointer to initialization data 
			(BITMAPINFO *)bi,			// pointer to bitmap info
			DIB_RGB_COLORS);			// color-data usage 
	
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
