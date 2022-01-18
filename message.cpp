#include <windows.h>
#include "message.h"
#include "matrix.h"

static HDC hdcMessage;
static HBITMAP hBitmapMsg;
static HANDLE hdcold;

extern int numrows, numcols;
extern int jjrand();
extern int xChar, yChar;
extern HDC hdcSymbols;
extern int MessageSpeed;
extern BOOL RandomizeMessages;
extern BOOL FontBold;
extern int FontSize;
extern Message message;
extern char szFontName[];
//
//	A class which handles matrix messages appearing
//

Message::Message()
{
	HideMessage();
}

int Message::rand()
{ 
	static unsigned short reg = (unsigned short)(GetTickCount() & 0xffff);
	unsigned short mask = 0xb400;

	if(reg & 1)
		reg = (reg >> 1) ^ mask;
	else 
		reg = (reg >> 1);

	return reg;
}

void InitMessage(void)
{
	HDC hdc = GetDC(0);
	hdcMessage = CreateCompatibleDC(hdc);
	hBitmapMsg = CreateCompatibleBitmap(hdc, MSGWIDTH, MSGHEIGHT*3);
	hdcold = SelectObject(hdcMessage, hBitmapMsg);

	ReleaseDC(0, hdc);
}

void DeInitMessage(void)
{
	SelectObject(hdcMessage, hdcold);
	DeleteObject(hBitmapMsg);
	DeleteDC	(hdcMessage);
}

void Message::HideMessage()
{
	for(int x = 0; x < MSGWIDTH; x++)
		for(int y = 0; y < MSGHEIGHT; y++)
			visible[x][y] = false;
}

void Message::ClearMessage()
{
	for(int x = 0; x < MSGWIDTH; x++)
		for(int y = 0; y < MSGHEIGHT; y++)
			bitmap[x][y] = false;
}

void Message::SetMessage(char *newmsg, int PointSize)
{
	RECT rect;
	HFONT hfont, holdfont;
	int height;
	HDC hdc;

	hdc = GetDC(0);
	int lfHeight = -MulDiv(PointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	ClearMessage();

	lstrcpy(curmsg, newmsg);

	hfont = (HFONT)CreateFont(lfHeight, 0, 0, 0, 
		FontBold ? FW_BOLD: FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, szFontName);

	SetRect(&rect, 0, 0, numcols,MSGHEIGHT*3);
	holdfont = (HFONT)SelectObject(hdcMessage, hfont);
	
	FillRect(hdcMessage, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	height = DrawText(hdcMessage, newmsg, lstrlen(newmsg), &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

	int curline = 0;
	bool lastempty = true;
	int start = -1;

	for(int y = 0; y < height; y++)
	{
		bool empty = true;

		for(int x = 0; x < numcols; x++)
		{
			if(GetPixel(hdcMessage, x, y) < RGB(96,96,96))
			{
				if(start == -1) { curline = y; start = y; }
				bitmap[x][curline] = true;
				empty = false;
			}
			else
				bitmap[x][curline] = false;

		}

		if(!empty || empty && !lastempty) curline++;
		lastempty = empty;
	}

	SelectObject(hdcMessage, holdfont);
	DeleteObject(hfont);

	ReleaseDC(0, hdc);
}



void Message::ShowMessage(HDC hdc)
{
	for(int x = 0; x < numcols; x++)
	{
		for(int y = 0; y < numrows; y++)
		{
			int c = jjrand() % 26;
			
			if(bitmap[x][y] == true && visible[x][y])
			{
				BitBlt(hdc, x*xChar, y*yChar, 14, 14, hdcSymbols, c*14,14*4, SRCCOPY);
			}
		}
	}
}

void Message::Reveal(int amt)
{
	for(int k = 0; k < amt; k++)
	{
		int pos = rand() & 0xffff;
		visible[pos / 256][pos % 256] = true;
	}
}

void Message::Preview(HDC hdc)
{
	for(int x = 0; x < numcols; x++)
	{
		for(int y = 0; y < numrows; y++)
		{
			COLORREF col;
			if(bitmap[x][y] == true)
			{
				col = RGB(128,255,128);
			}
			else 
			{
				col = 0;
			}
			SetPixelV(hdc, x,y, col);
		}
	}
}

//
//	Called for each iteration of the display
//
void DoMessages(HDC hdc)
{
	static int nCurrentMessage = -1;

	//convert from 50-500 (fast-slow) to slow(50) - fast(500)
	//
	int RealSpeed = (MSGSPEED_MAX-MSGSPEED_MIN) - (MessageSpeed-MSGSPEED_MIN) + MSGSPEED_MIN;

	if(nNumMessages > 0)
	{
		//start off showing nothing
		static int burncounter = RealSpeed / 2;
		
		if(burncounter++ == RealSpeed / 2)
		{
			message.HideMessage();
		}
		
		if(burncounter == RealSpeed)
		{
			//reset the message counter, and display a new message!!
			if(RandomizeMessages)
				nCurrentMessage = jjrand() % nNumMessages;
			else
				if(++nCurrentMessage >= nNumMessages) nCurrentMessage = 0;
				
			message.SetMessage(szMessages[nCurrentMessage], FontSize);
			burncounter = 0;
		}
		
		if(burncounter < RealSpeed / 2)
		{
//			float a,b;
//			b = MessageSpeed - MSGSPEED_MIN;
//			b = 1+b/MSGSPEED_MAX;
//			a = float(MessageSpeed) * b * 3;
			int w = (MessageSpeed - MSGSPEED_MIN);
			w = (1 << 16) + ((w<<16) / MSGSPEED_MAX);
			w = (w * 3 * MessageSpeed) >> 16;

			message.Reveal(w + 100);
		}
		
		message.ShowMessage(hdc);
	}

//	SelectObject(hdc, GetStockObject(WHITE_PEN));
//	Rectangle(hdc, 0, 0, maxcols, maxrows);
//	BitBlt(hdc, 0, 0, maxcols, maxrows, hdcMessage, 0, 0, SRCCOPY);
}