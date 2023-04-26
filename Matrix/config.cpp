#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "resource.h"
#include "message.h"
#include "matrix.h"

extern int numrows, numcols;
extern int maxrows, maxcols;

extern Message message;
extern HINSTANCE hInst;

extern RECT ScreenSize;
extern int Density;			//matrix configuration
extern int MessageSpeed;
extern int MatrixSpeed;
extern int FontSize;
extern BOOL RandomizeMessages;
extern BOOL FontBold;
extern TCHAR szFontName[];

HDC hdcPrev;
HBITMAP hbmPrev;

BOOL EnablePreviews = TRUE;

void SaveSettings();

int CALLBACK fontproc(ENUMLOGFONT *lpelfe, NEWTEXTMETRIC *lpntme, int FontType, LPARAM lParam)
{
	HWND hwnd = (HWND)lParam;
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfLogFont.lfFaceName);
	return 1;
}

void AddFonts(HWND hwnd)
{
	LOGFONT lf;
	lf.lfCharSet = ANSI_CHARSET;//DEFAULT_CHARSET;
	lf.lfPitchAndFamily = 0;
	lstrcpy(lf.lfFaceName, _T(""));

	HDC hdc = GetDC(0);
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)fontproc, (LONG_PTR)hwnd, 0);

	ReleaseDC(0, hdc);
}

#pragma comment(lib, "version.lib")

//
//	Get the specified file-version information string from a file
//	
//	szItem	- version item string, e.g:
//		"FileDescription", "FileVersion", "InternalName", 
//		"ProductName", "ProductVersion", etc  (see MSDN for others)
//
TCHAR* GetVersionString(const TCHAR* szFileName, const TCHAR* szValue, TCHAR* szBuffer, ULONG nLength)
{
	UINT   len;
	PVOID  ver;
	DWORD* codepage;
	TCHAR  fmt[0x40];
	PVOID  ptr = 0;
	BOOL   result = FALSE;

	szBuffer[0] = '\0';

	len = GetFileVersionInfoSize(szFileName, 0);

	if (len == 0 || (ver = malloc(len)) == 0)
		return NULL;

	if (GetFileVersionInfo(szFileName, 0, len, ver))
	{
		if (VerQueryValue(ver, TEXT("\\VarFileInfo\\Translation"),  (LPVOID*) & codepage, &len))
		{
			wsprintf(fmt, TEXT("\\StringFileInfo\\%04x%04x\\%s"), (*codepage) & 0xFFFF,
				(*codepage) >> 16, szValue);

			if (VerQueryValue(ver, fmt, &ptr, &len))
			{
				lstrcpyn(szBuffer, (TCHAR*)ptr, min(nLength, len));
				result = TRUE;
			}
		}
	}

	free(ver);
	return result ? szBuffer : NULL;
}

INT_PTR CALLBACK configdlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR buf[256];
	HDC hdc;
	HWND hwndCombo, hwndCtrl;
	int index, items, val;
	RECT rect;

	TCHAR szCurExe[MAX_PATH];
	TCHAR szVersion[40];

	switch(uMsg)
	{
	case WM_INITDIALOG:

		InitMessage();
		
		numcols = maxcols;
		numrows = maxrows;

		//Add any saved messages to the combo box
		for(index = 0; index < nNumMessages; index++)
			SendDlgItemMessage(hwnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)szMessages[index]);

		//select the first message, and preview it
		SendDlgItemMessage(hwnd, IDC_COMBO1, CB_SETCURSEL, 0, 0);
		PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PREV, BN_CLICKED), (LPARAM)GetDlgItem(hwnd,IDC_PREV));
		
		SendDlgItemMessage(hwnd, IDC_SLIDER1, TBM_SETRANGE, 0, MAKELONG(SPEED_MIN, SPEED_MAX));
		SendDlgItemMessage(hwnd, IDC_SLIDER2, TBM_SETRANGE, 0, MAKELONG(DENSITY_MIN, DENSITY_MAX));
		SendDlgItemMessage(hwnd, IDC_SLIDER3, TBM_SETRANGE, 0, MAKELONG(MSGSPEED_MIN, MSGSPEED_MAX));
		SendDlgItemMessage(hwnd, IDC_SLIDER4, TBM_SETRANGE, 0, MAKELONG(FONT_MIN, FONT_MAX));

		//SendDlgItemMessage(hwnd, IDC_SLIDER1, TBM_SETTICFREQ, 5, 0);
		SendDlgItemMessage(hwnd, IDC_SLIDER2, TBM_SETTICFREQ, 5, 0);
		SendDlgItemMessage(hwnd, IDC_SLIDER3, TBM_SETTICFREQ, 50, 0);
		SendDlgItemMessage(hwnd, IDC_SLIDER4, TBM_SETTICFREQ, 2, 0);
		
		SendDlgItemMessage(hwnd, IDC_SLIDER1, TBM_SETPOS, TRUE, MatrixSpeed);
		SendDlgItemMessage(hwnd, IDC_SLIDER2, TBM_SETPOS, TRUE, Density);
		SendDlgItemMessage(hwnd, IDC_SLIDER3, TBM_SETPOS, TRUE, MessageSpeed);
		SendDlgItemMessage(hwnd, IDC_SLIDER4, TBM_SETPOS, TRUE, FontSize);

		GetModuleFileName(0, szCurExe, MAX_PATH);
		GetVersionString(szCurExe, TEXT("FileVersion"), szVersion, 40);
		SetDlgItemText(hwnd, IDC_VERSION, szVersion);


		CheckDlgButton(hwnd, IDC_ENABLEPREV, EnablePreviews);
		CheckDlgButton(hwnd, IDC_RANDOM, RandomizeMessages);
		CheckDlgButton(hwnd, IDC_BOLD, FontBold);

		AddFonts(GetDlgItem(hwnd, IDC_COMBO2));

		index = (int)SendDlgItemMessage(hwnd, IDC_COMBO2, CB_FINDSTRING, 0, (LPARAM)szFontName);
		SendDlgItemMessage(hwnd, IDC_COMBO2, CB_SETCURSEL, index, 0);
		return 0;

	case WM_DESTROY:
		DeInitMessage();
		return 0;

	case WM_CTLCOLORSTATIC:
		if((HWND)lParam == GetDlgItem(hwnd, IDC_PREVIEW))
		{
			GetClientRect(GetDlgItem(hwnd, IDC_PREVIEW), &rect);
			BitBlt((HDC)wParam, (rect.right-maxcols)/2, (rect.bottom-maxrows)/2, maxcols, maxrows, hdcPrev, 0, 0, SRCCOPY);
			return (INT_PTR)GetStockObject(NULL_BRUSH);
		}	
		else
		{
			break;
		}

	case WM_HSCROLL:
		/*if((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER3))
		{
			MatrixSpeed = SendDlgItemMessage(hwnd, IDC_SLIDER1, TBM_GETPOS, 0, 0);	
			MessageSpeed = SendDlgItemMessage(hwnd, IDC_SLIDER3, TBM_GETPOS, 0, 0);	

			char ach[80];
			wsprintf(ach, "Message Speed Display (%ds)", MatrixSpeed * (MSGSPEED_MAX-MessageSpeed));
			SetWindowText(GetDlgItem(hwnd, IDC_MSGSPEEDGRP), ach);
		}*/

		if((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER4))
		{
			if(EnablePreviews)
				PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PREV, BN_CLICKED), (LPARAM)GetDlgItem(hwnd,IDC_PREV));
		}

		return 0;
	case WM_COMMAND:

		switch(HIWORD(wParam))
		{
		case CBN_EDITCHANGE:

			if(!EnablePreviews)
				return 0;

		case CBN_SELCHANGE:
			//fall through to Preview:
			index = (int)SendDlgItemMessage(hwnd, IDC_COMBO2, CB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hwnd, IDC_COMBO2, CB_GETLBTEXT, index, (LPARAM)szFontName);
			
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PREV, BN_CLICKED), (LPARAM)GetDlgItem(hwnd,IDC_PREV));
			return 0;
		}

		switch(LOWORD(wParam))
		{
		case IDC_RANDOM:
			RandomizeMessages = IsDlgButtonChecked(hwnd, IDC_RANDOM);
			break;

		case IDC_ENABLEPREV:
			EnablePreviews = IsDlgButtonChecked(hwnd, IDC_ENABLEPREV);
			EnableWindow(GetDlgItem(hwnd, IDC_PREV), !EnablePreviews);

			break;

		case IDC_BOLD:
			FontBold = IsDlgButtonChecked(hwnd, IDC_BOLD);
			if(EnablePreviews)
				PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PREV, BN_CLICKED), (LPARAM)GetDlgItem(hwnd,IDC_PREV));
			break;

		case IDOK:
			
			hwndCtrl = GetDlgItem(hwnd, IDC_COMBO1);

			items = (int)min(MAXMESSAGES, SendMessage(hwndCtrl, CB_GETCOUNT, 0, 0));

			for(index = 0; index < items; index++)
			{
				SendMessage(hwndCtrl, CB_GETLBTEXT, index, (LPARAM)szMessages[index]);
			}

			nNumMessages = items;
			
			//matrix speed
			val = (int)SendDlgItemMessage(hwnd, IDC_SLIDER1, TBM_GETPOS, 0, 0);
			if(val >= SPEED_MIN && val <= SPEED_MAX)
				MatrixSpeed = val;

			//density
			val = (int)SendDlgItemMessage(hwnd, IDC_SLIDER2, TBM_GETPOS, 0, 0);
			if(val >= DENSITY_MIN && val <= DENSITY_MAX)
				Density = val;

			//message speed
			val = (int)SendDlgItemMessage(hwnd, IDC_SLIDER3, TBM_GETPOS, 0, 0);
			if(val >= MSGSPEED_MIN && val <= MSGSPEED_MAX)
				MessageSpeed = val;

			//font size
			val = (int)SendDlgItemMessage(hwnd, IDC_SLIDER4, TBM_GETPOS, 0, 0);
			if(val >= FONT_MIN && val <= FONT_MAX)
				FontSize = val;

			SaveSettings();
			EndDialog(hwnd, 0);
			return 0;

		case IDC_PREV:
			numrows = maxrows;
			numcols = maxcols;
			
			val = (int)SendDlgItemMessage(hwnd, IDC_SLIDER4, TBM_GETPOS,0, 0);
			
			GetWindowText(GetDlgItem(hwnd, IDC_COMBO1), buf, 256);
			message.ClearMessage();
			message.SetMessage(buf, val);
			
			hdc = GetDC(GetDlgItem(hwnd, IDC_PREVIEW));

			message.Preview(hdcPrev);

			GetClientRect(GetDlgItem(hwnd, IDC_PREVIEW), &rect);
			BitBlt(hdc, (rect.right-maxcols)/2, (rect.bottom-maxrows)/2, maxcols, maxrows, hdcPrev, 0, 0, SRCCOPY);

			ReleaseDC(GetDlgItem(hwnd, IDC_PREVIEW), hdc);
			return 0;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			return TRUE;

		case IDC_ADD:
			hwndCombo = GetDlgItem(hwnd, IDC_COMBO1);
			GetWindowText(hwndCombo, buf, 256);
			SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)buf);
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PREV, BN_CLICKED), (LPARAM)GetDlgItem(hwnd,IDC_PREV));

			return 0;

		case IDC_REMOVE:
			hwndCombo = GetDlgItem(hwnd, IDC_COMBO1);
			GetWindowText(hwndCombo, buf, 256);

			index = (int)SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
			SendMessage(hwndCombo, CB_DELETESTRING, index, 0);

			SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PREV, BN_CLICKED), (LPARAM)GetDlgItem(hwnd,IDC_PREV));
			return 0;
		}
		return 0;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return 0;
	}
	return 0;
}


int Configure(HWND hwndParent)
{
	INITCOMMONCONTROLSEX icc;
	HANDLE hold;

	icc.dwICC = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;
	icc.dwSize = sizeof icc;

	hdcPrev = CreateCompatibleDC(NULL);
	hbmPrev = CreateCompatibleBitmap(hdcPrev, maxcols, maxrows);
	hold    = SelectObject(hdcPrev, hbmPrev);

	InitCommonControlsEx(&icc);

	if(hwndParent == NULL)
		hwndParent = GetForegroundWindow();

	DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIG), hwndParent, configdlgproc);
	
	SelectObject(hdcPrev, hold);
	DeleteObject(hbmPrev);
	DeleteDC	(hdcPrev);

	return 0;
}
