#include <windows.h>

#include "message.h"
#include "matrix.h"

TCHAR szMessages[MAXMESSAGES][MAXMSGLEN];

extern TCHAR szAppName[];
int nNumMessages;
extern int Density;
extern int MessageSpeed;
extern int MatrixSpeed;
extern int FontSize;
extern TCHAR szFontName[];
extern BOOL FontBold;
extern BOOL EnablePreviews;
extern BOOL RandomizeMessages;

void LoadSettings()
{
	HKEY hkey;
	LONG value;
	ULONG len;
	TCHAR hugechar[4096];
	TCHAR *hptr = hugechar;
	TCHAR empty[] = _T("");

	RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Catch22\\Matrix Screen Saver"), 0,
		empty, 0, KEY_READ, NULL, &hkey, NULL);

	len = sizeof value;
	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("MessageSpeed"), 0, 0, (BYTE *)&value, &len))
	{
		if(value >= MSGSPEED_MIN && value <= MSGSPEED_MAX)
			MessageSpeed = value;
	}

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("MatrixSpeed"),  0, 0, (BYTE *)&value, &len))
	{
		if(value >= SPEED_MIN && value <= SPEED_MAX)
			MatrixSpeed  = value;
	}
	
	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("Density"),      0, 0, (BYTE *)&value, &len))
	{
		if(value >= DENSITY_MIN && value <= DENSITY_MAX)
			Density      = value;
	}

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("FontSize"),      0, 0, (BYTE *)&value, &len))
	{
		if(value >= FONT_MIN && value <= FONT_MAX)
			FontSize	 = value;
	}

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("FontBold"),      0, 0, (BYTE *)&value, &len))
		FontBold = (value == 0 ? FALSE : TRUE);

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("Preview"),      0, 0, (BYTE *)&value, &len))
		EnablePreviews = (value == 0 ? FALSE : TRUE);

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("Randomize"),      0, 0, (BYTE *)&value, &len))
		RandomizeMessages = (value == 0 ? FALSE : TRUE);

	len = sizeof(hugechar);

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("FontName"),  0, 0, (BYTE *)&hugechar, &len))
	{
		lstrcpy(szFontName, hugechar);
	}

	len = sizeof(hugechar);

	if(ERROR_SUCCESS == RegQueryValueEx(hkey, _T("Messages"), 0, 0, (BYTE*)hugechar, &len))
	{
		while(*hptr && isascii(*hptr))
		{
			lstrcpy(szMessages[nNumMessages], hptr);
			++nNumMessages;
			hptr += lstrlen(hptr) + 1;
		}
	}

	RegCloseKey(hkey);
}

void SaveSettings()
{
	HKEY hkey;
	TCHAR hugechar[4096] = _T("");
	TCHAR empty[] = _T("");
	TCHAR *msgptr = hugechar;
	int totallen = 0;
	LONG value;

	RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Catch22\\Matrix Screen Saver"), 0,
		empty, 0, KEY_WRITE, NULL, &hkey, NULL);

	value = MessageSpeed;
	RegSetValueEx(hkey, _T("MessageSpeed"), 0, REG_DWORD, (BYTE *)&value, sizeof(value));

	value = MatrixSpeed;
	RegSetValueEx(hkey, _T("MatrixSpeed"), 0, REG_DWORD, (BYTE *)&value, sizeof(value));

	value = Density;
	RegSetValueEx(hkey, _T("Density"), 0, REG_DWORD, (BYTE *)&value, sizeof(value));

	value = FontSize;
	RegSetValueEx(hkey, _T("FontSize"), 0, REG_DWORD, (BYTE *)&value, sizeof(value));

	value = EnablePreviews;
	RegSetValueEx(hkey, _T("Previews"), 0, REG_DWORD, (BYTE *)&value, sizeof(value));

	value = RandomizeMessages;
	RegSetValueEx(hkey, _T("Randomize"), 0, REG_DWORD, (BYTE *)&value, sizeof(value));

	value = FontBold;
	RegSetValueEx(hkey, _T("FontBold"), 0, REG_DWORD, (BYTE*)&value, sizeof(value));

	RegSetValueEx(hkey, _T("FontName"), 0, REG_SZ, (BYTE *)szFontName, lstrlen(szFontName)*sizeof(TCHAR));

	for(int i = 0; i < nNumMessages; i++)
	{
		int len = lstrlen(szMessages[i]);
		lstrcat(msgptr, szMessages[i]);
		totallen += len + 1;
		msgptr += len + 1;
	}

	*msgptr = '\0';
	totallen++;

	RegSetValueEx(hkey, _T("Messages"), 0, REG_MULTI_SZ, (BYTE*)hugechar, totallen*sizeof(TCHAR));
	RegCloseKey(hkey);
}