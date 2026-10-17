#include <windows.h>
#include "matrix.h"

typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
typedef VOID (WINAPI *PWDCHANGEPASSWORD) (LPCSTR lpcRegkeyname,HWND hwnd,UINT uiReserved1,UINT uiReserved2);

BOOL VerifyPassword(HWND hwnd)
{
	BOOL bres = TRUE;
#ifndef _WIN64
	// Under NT, we return TRUE immediately. This lets the saver quit,
	// and the system manages passwords. Under '95, we call VerifyScreenSavePwd.
	// This checks the appropriate registry key and, if necessary,
	// pops up a verify dialog
	if (IsNT())
		return TRUE;

	HINSTANCE hpwdcpl;
	VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
	
	hpwdcpl = LoadLibrary(_T("PASSWORD.CPL"));

	if(hpwdcpl == NULL) 
	{
		return FALSE;
	}
	
	VerifyScreenSavePwd = (VERIFYSCREENSAVEPWD)GetProcAddress(hpwdcpl, "VerifyScreenSavePwd");
	if(VerifyScreenSavePwd==NULL)
	{
		FreeLibrary(hpwdcpl);
		return FALSE;
	}

	bres = VerifyScreenSavePwd(hwnd); 
	FreeLibrary(hpwdcpl);
#endif
	return bres;
}


BOOL ChangePassword(HWND hwnd)
{
#ifndef _WIN64
	if (IsNT())
		return TRUE;

	// This only ever gets called under '95, when started with the /a option.
	HINSTANCE hmpr = LoadLibrary(_T("MPR.DLL"));
	PWDCHANGEPASSWORD PwdChangePassword;

	if(hmpr == NULL) 
		return FALSE;

	PwdChangePassword = (PWDCHANGEPASSWORD)GetProcAddress(hmpr, "PwdChangePasswordA");

	if(PwdChangePassword == NULL)
	{
		FreeLibrary(hmpr);
		return FALSE;
	}

	PwdChangePassword("SCRSAVE", hwnd, 0, 0); 
	FreeLibrary(hmpr);
#endif
	return TRUE;
}
