#include <windows.h>


#include "bitmap.h"




HBITMAP LoadBitmap2(HDC hdc,HINSTANCE hInstance, TCHAR *bmpfile)
{
	HANDLE hFile = INVALID_HANDLE_VALUE, hMap = 0;

	HRSRC hrsrc;
	HANDLE hResource = NULL;
	
	BYTE *pmem;
	BITMAPFILEHEADER *bfh;
	
	BITMAPINFO *bi;
	BITMAPINFOHEADER *bih;
	
	if(hInstance == 0)
	{
		hFile = CreateFile(bmpfile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);

		if(hFile == INVALID_HANDLE_VALUE) return 0;

		hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);

		if(hMap == NULL)
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
	bi = (BITMAPINFO *)bih;

//	HDC hdc = GetDC(0);
	//HBITMAP j = CreateDIBitmap(hdc, bih, CBM_INIT, pmem+(bfh->bfOffBits), bi, DIB_RGB_COLORS);
	
	HBITMAP j = CreateCompatibleBitmap(hdc, bih->biWidth, bih->biHeight);
	SetDIBits(hdc, j, 0, bih->biHeight, bi->bmiColors, bi, DIB_RGB_COLORS);

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

//	ReleaseDC(0, hdc);
	return j;
}




HBITMAP DIBtoDDB(HDC hdc,void *hDIB)
{
	BITMAPINFOHEADER *bih;
	HBITMAP		     hBitmap;
	BITMAPINFO		*bi;
	HPALETTE		hPalette;
	void *pDIBits;
	
	if(hDIB == 0) return 0;

	bih = (BITMAPINFOHEADER *)hDIB;

	int nColors = bih->biClrUsed ? bih->biClrUsed : 1 << bih->biBitCount;

	bi = (BITMAPINFO *)hDIB;

	if(nColors <= 256)// && GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
	{
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *lp = (LOGPALETTE *)new BYTE[nSize];

		lp->palVersion = 0x300;
		lp->palNumEntries = nColors;

		for(int i = 0; i < nColors; i++)
		{
			lp->palPalEntry[i].peRed   = bi->bmiColors[i].rgbRed;
			lp->palPalEntry[i].peGreen = bi->bmiColors[i].rgbGreen;
			lp->palPalEntry[i].peBlue  = bi->bmiColors[i].rgbBlue;
			lp->palPalEntry[i].peFlags = 0;
		}
	
		hPalette = CreatePalette(lp);
		delete[] lp;

		SelectPalette(hdc, hPalette, FALSE);
		RealizePalette(hdc);
	}

	if(bi->bmiHeader.biBitCount > 8)
		pDIBits = bi->bmiColors + bi->bmiHeader.biClrUsed + 
		(bi->bmiHeader.biCompression == BI_BITFIELDS ? 3 : 0);
	else
		pDIBits = bi->bmiColors + nColors;

	pDIBits = bi->bmiColors + nColors;

	hBitmap = CreateDIBitmap(hdc, bih, CBM_INIT, pDIBits, bi, DIB_RGB_COLORS);
		

	return hBitmap;
}

HBITMAP LoadBitmap3(HDC hdc, HINSTANCE hInstance, TCHAR *bmpfile)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMap = 0;

	HRSRC hrsrc;
	HANDLE hResource = 0;
	
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
	bi = (BITMAPINFO *)bih;

	//Load the bitmap here
	HBITMAP hBitmap = DIBtoDDB(hdc, bih);
		
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

//	ReleaseDC(0, hdc);
	return hBitmap;
}
