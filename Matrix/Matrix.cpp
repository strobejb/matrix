#include <windows.h>
#include "resource.h"
#include "palette.h"
#include "message.h"
#include "matrix.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

Message message;

TCHAR szAppName[] = _T("Matrix Screensaver");

HINSTANCE hInst;

RECT ScreenSize;

HPALETTE hPalette;
HDC hdcSymbols;
HBITMAP hSymbolBitmap;

//state for matrix
int dispx, dispy;
int maxrows, maxcols;
int numrows, numcols;
int xChar, yChar;

HFONT hfont;

int MessageSpeed = 150;			//
int Density      = 32;			//5-50
int MatrixSpeed  = 5;			//1-10
int FontSize	 = 12;			//8-30
BOOL FontBold	 = TRUE;
BOOL RandomizeMessages = FALSE;
TCHAR szFontName[512] = _T("MS Sans Serif");

void LoadSettings(void);

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

int Configure(HWND hwndParent);
BOOL ChangePassword(HWND hwnd);
BOOL VerifyPassword(HWND hwnd);


bool fScreenSaving = false;

HDC		hdcMessage;
HBITMAP hBitmapMsg;

int jjrand(void) 
{ 
	static unsigned short reg = (unsigned short)(GetTickCount() & 0xffff);
	unsigned short mask = 0xb400;

	if(reg & 1)
		reg = (reg >> 1) ^ mask;
	else 
		reg = (reg >> 1);

	return reg;
	//return rand();
}


Matrix *matrix;

inline int INTENSITY(int n)
{
	return (n < 0 ? -1 : n/32);
}

void Matrix::ScrollDown(HDC hdc)
{
	//set a state - blanks or data, and an associated run length
	//each time we are called, draw the "state", and decrement
	//the runlength. When runlength gets to zero, swap states and
	//reset the run-length to another value.

	//for each column, iterate down
	//find the start of a "run" (the divide between blank and data)
	//delete a character
	//find the end of a "run" (divide between data and blank)
	//add a jjrandom character
	//
	//Any digits are in the range 0-n
	//blanks are represented as any negative number.
	//
	if(started == false)
	{
		if(--initcount <= 0) started = true;
		return;
	}
	
	//reset the update list
	for(int i = 0; i < numrows; i++)
		update[i] = false;

	int oldchar = state ? 127 : -1;

	for(int i = 0; i < numrows; i++)
	{
		//work out intensity of the "pixels" we will compare
		//iterate down, look for a change in INTENSITY 
		int oldins = INTENSITY(oldchar);
		int runins = INTENSITY(run[i]);

		//tail-end of a run (top-most part)
		if(runins > oldins && runins >= 0)
		{
			run[i] -= 32;			//darken the colour (this will get < 0 at some point)
			update[i] = true;

			if(runins == 3)	i++;	//if we have just darkened a "normal" character,
									//skip on so we don't darken the whole run
		}
		//head of the run (bottom-most part). Add a random character, maximum brightness
		else if(oldins >= 0 && runins < 0)
		{
			run[i] = jjrand() % 26 + 96;	//there are 4 brightness levels, 32 characters in each
			update[i] = true;
			i++;
		}

		oldchar = run[i];
	}

	//do we need to change state from blanks to runs??
	if(--statecount <= 0)
	{
		state = state ^ 1;		//swap state
		
		if(state == 0)			//if blanks
		{
			statecount = jjrand() % (DENSITY_MAX+1-Density) + (DENSITY_MIN*2);
		}
		else					//if normals
		{
			statecount = jjrand() % (3*Density/2) + DENSITY_MIN;
		}
		
	}

	//make the blip run down. There are two blips, 2 characters height each.
	//blip1 is at blippos, blip2 is at blippos+8
	if(blippos >= 0 && blippos < runlen)
	{
		update[blippos] = true;
		update[blippos+1] = true;
		update[blippos+8] = true;
		update[blippos+9] = true;
	}

	blippos += 2;

	//if the blip gets to the end, start it again (for a random length, 
	//so that the blips in each run never get synchronized and fall at
	//the same time).
	if(blippos >= bliplen)
	{
		bliplen = numrows + jjrand() % 50;
		blippos = 0;
	}

	if(blippos >= 0 && blippos < runlen)
	{
		update[blippos] = true;
		update[blippos+1] = true;
		update[blippos+8] = true;
		update[blippos+9] = true;
	}
}

void DecodeMatrix(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	int x;

	UseNicePalette(hdc, hPalette);
	SelectObject(hdc, hfont);
	SetBkColor(hdc, 0);
	
	for(x = 0; x < numcols; x++)
	{
		matrix[x].jjrandomise();
		matrix[x].ScrollDown(hdc);

		for(int y = 0; y < numrows; y++)
		{
			if(matrix[x].update[y] == false)
				continue;

			if(matrix[x].run[y] < 0)
			{
				//erase a character position if this is a blank
				RECT rect;
				SetRect(&rect,  x * xChar, y * yChar, (x + 1) * xChar, (y + 1) * yChar);
				ExtTextOut(hdc, x * xChar, y * yChar, ETO_OPAQUE, &rect, _T(""), 0, 0);
			}
			else
			{
				if(matrix[x].blippos == y  || matrix[x].blippos+1 == y
					|| matrix[x].blippos+8 == y || matrix[x].blippos +9 == y)
				{
					//a blip uses maximum brightness
					TCHAR c = matrix[x].run[y] & 31;
					BitBlt(hdc, x*xChar, y*yChar, 14, 14, hdcSymbols, c*14,14*4, SRCCOPY);
				}
				else
				{
					//c is in range 0-127  (0-31, 32-63, 64-95, 96-127)
					TCHAR c = matrix[x].run[y];
					int sx = c & 31;
					int sy = c / 32;
					
					BitBlt(hdc, x*xChar, y*yChar, 14, 14, hdcSymbols, sx*14,sy*14, SRCCOPY);
				}
			}
		}
	}

	DoMessages(hdc);

	ReleaseDC(hwnd, hdc);
}

void Matrix::jjrandomise()
{
	//go through each run, and jjrandomly change a few characters
	int p = 0;
	for(int i = 1; i < 20; i++)
	{
		//find a run
		while(run[p] < 96 && p < numrows) p++;

		if(p >= numrows) break;

		run[p] = jjrand() % 26 + 96;
		update[p] = true;

		p += jjrand() % 10;
	}
}

void InitMatrix(HWND hwnd)
{
	//make a matrix!
	matrix = new Matrix[maxcols];// { maxrows };
	
	for(int i = 0; i < maxcols; i++)
		matrix[i].Init(maxrows);

	//Start the matrix
	//Matrix speed 1-10. We need 10x this for the timer
	SetTimer(hwnd, 0xDeadBeef, MatrixSpeed * 10, 0);
}

int Normal(int iCmdShow)
{
	HWND	hwnd;
	MSG	msg;
	WNDCLASSEX	wndclass;
	DWORD exStyle, style;
	HCURSOR hcurs;

	if(iCmdShow == SW_MAXIMIZE)
	{
		exStyle = WS_EX_TOPMOST;
		style   = WS_POPUP | WS_VISIBLE;
		hcurs   = LoadCursor(hInst, MAKEINTRESOURCE(IDC_BLANKCURSOR));
	}
	else
	{
		exStyle = WS_EX_CLIENTEDGE;
		style   = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
		hcurs   = LoadCursor(NULL, IDC_ARROW);
	}
	//Window class for the main application parent window
	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= 0;//CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInst;
	wndclass.hIcon			= LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor		= hcurs;
	wndclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);// NULL;
	wndclass.lpszMenuName	= 0;
	wndclass.lpszClassName	= szAppName;
	wndclass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	InitMessage();

	hwnd = CreateWindowEx(exStyle,
		szAppName,				// window class name
		szAppName,				// window caption
		style,	// window style
		CW_USEDEFAULT,			// initial x position
		CW_USEDEFAULT,			// initial y position
		CW_USEDEFAULT,			// initial x size
		CW_USEDEFAULT,			// initial y size
		NULL,					// parent window handle
		NULL,					// use window class menu
		hInst,					// program instance handle
		NULL);					// creation parameters

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeInitMessage();

	return (int)msg.wParam;
}

int ScreenSave(void)
{
	UINT nPreviousState;
	fScreenSaving = true;
	
	SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, TRUE, &nPreviousState, 0);
	
	Normal(SW_MAXIMIZE);

	SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, FALSE, &nPreviousState, 0);
	

	return 0;
}


BOOL GetCommandLineOption(PSTR szCmdLine, int *chOption, HWND *hwndParent)
{
	int ch = *szCmdLine++;

	if(ch == '-' || ch == '/')
		ch = *szCmdLine++;

	if(ch >= 'A' && ch <= 'Z')
		ch += 'a' - 'A';		//convert to lower case

	*chOption = ch;
	ch = *szCmdLine++;
	
	if(ch == ':') 
		ch = *szCmdLine++;

	while(ch == ' ' || ch == '\t')
		ch = *szCmdLine++;

	if(isdigit(ch))
	{
		unsigned int i = atoi(szCmdLine);
		*hwndParent = (HWND)(UINT_PTR)i;
	}
	else
		*hwndParent = NULL;

	return TRUE;
}

//
//
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	int		chOption;
	HWND	hwndParent;

	hInst = hInstance;			//Make the instance globally available
	TCHAR* s = GetCommandLine();

	//Make sure that only 1 instance runs at a time - 
	//Win98 seems to want us to restart every 5 seconds!!
	if(FindWindowEx(NULL, NULL, szAppName, szAppName))
	{
		return 0;
	}

	//see how big the screen is
	SetRect(&ScreenSize, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	xChar = 14;
	yChar = 14;
	maxcols = ScreenSize.right / xChar;
	maxrows = ScreenSize.bottom / yChar + 1;
	
	LoadSettings();

	GetCommandLineOption(szCmdLine, &chOption, &hwndParent);

	switch(chOption)
	{
	case 's':	return ScreenSave();				//screen save
	case 'p':	return 0;							//preview in small window
	case 'a':	return ChangePassword(hwndParent);	//ask for password
	case 'c':	return Configure(hwndParent);		//configuration dialog
	default:	return Normal(iCmdShow);
	}

	return 0;	
}

//-----------------------------------------------------------------------------
int main(void) {}

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	static DWORD frames = 0;
	static int skip=0;
	static HANDLE holddc;
	static HPALETTE holdpal;

	//WHY???
	static bool fHere = false;
    static POINT    ptLast;
    POINT           ptCursor, ptCheck;

	static LARGE_INTEGER freq;
	LARGE_INTEGER pc1, pc2;

	static DWORD median;
	static int fpscount;

	switch(iMsg)
	{
	case WM_CREATE:

		hdc = GetDC(hwnd);

		holdpal = UseNicePalette(hdc, hPalette);
		hdcSymbols = CreateCompatibleDC(hdc);

		//load bitmap as a DDB, with palette!
		hPalette = ReadBMPPalette(hInst, hdc, MAKEINTRESOURCE(IDB_BITMAP1));
		extern HBITMAP hDDB;
		hSymbolBitmap = hDDB;
		
		holddc = (HANDLE)SelectObject(hdcSymbols, hSymbolBitmap);
		
		ReleaseDC(hwnd, hdc);

		InitMatrix(hwnd);
		i = QueryPerformanceFrequency(&freq);
		
		if(fScreenSaving)
			SetCursor(NULL);
		
		return 0;

	case WM_SIZE:

		numcols = (short)LOWORD(lParam) / xChar + 1;
		numrows = (short)HIWORD(lParam) / yChar + 1;

		if(numrows <= 0 || numrows >= maxrows) numrows = maxrows - 1;
		if(numcols <= 0 || numcols >= maxcols) numcols = maxcols - 1;

		for(i = numcols; i < maxcols; i++)
		{
			matrix[i].started = false;
			matrix[i].initcount = jjrand() % 20;
			matrix[i].blippos = jjrand() % numrows;

			for(j = 0; j < numrows; j++)
				matrix[i].run[j] = -1;

		}
		
		return 0;

	
	case WM_TIMER:
		
		if(!fScreenSaving)
			QueryPerformanceCounter(&pc1);
		
		DecodeMatrix(hwnd);
		
		if(!fScreenSaving)
		{
			QueryPerformanceCounter(&pc2);
			
			TCHAR buf[32];
			
			//build up an average
			median += DWORD(DWORD(freq.QuadPart) / DWORD(pc2.QuadPart - pc1.QuadPart));
			
			if(++fpscount == 16)
			{
				wsprintf(buf, _T("%s - %u FPS"), szAppName, median / 16);
				SetWindowText(hwnd, buf);
				median = 0;
				fpscount = 0;
			}
		}
		
		skip++;
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, 0xdeadbeef);

		SelectObject(hdcSymbols, holddc);
		SelectPalette(hdcSymbols, holdpal, FALSE);
		DeleteDC	(hdcSymbols);
		DeleteObject(hSymbolBitmap);
		DeleteObject(hPalette);
		
		PostQuitMessage(0);
		return 0;

	case WM_CLOSE:
		
		if(fScreenSaving && VerifyPassword(hwnd) || !fScreenSaving)
			DestroyWindow(hwnd);
		return 0;

    case WM_ACTIVATEAPP:
	case WM_ACTIVATE:
		if(wParam != FALSE) break;

    case WM_MOUSEMOVE:
        
		if(!fScreenSaving) return 0;

		if(!fHere) 
		{
            GetCursorPos(&ptLast);
            fHere = true;
        } 
		else 
		{
            GetCursorPos(&ptCheck);
            if(ptCursor.x = ptCheck.x - ptLast.x) 
			{
                if(ptCursor.x < 0) ptCursor.x *= -1;
            }
            
			if(ptCursor.y = ptCheck.y - ptLast.y) 
			{
                if(ptCursor.y < 0) ptCursor.y *= -1;
            }
            
			if((ptCursor.x + ptCursor.y) > 3) 
			{
                PostMessage(hwnd, WM_CLOSE, 0, 0l);
            }
        }

        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:

		if(!fScreenSaving) return 0;

		GetCursorPos(&ptCursor);
        ptCursor.x ++;
        ptCursor.y ++;
        SetCursorPos(ptCursor.x, ptCursor.y);
        GetCursorPos(&ptCheck);
        
		if(ptCheck.x != ptCursor.x && ptCheck.y != ptCursor.y)
			ptCursor.x -= 2;
        
		ptCursor.y -= 2;
        SetCursorPos(ptCursor.x,ptCursor.y);

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        PostMessage(hwnd, WM_CLOSE, 0, 0l);
        break;
	}

	return DefWindowProc (hwnd, iMsg, wParam, lParam);
}

