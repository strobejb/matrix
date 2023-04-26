#include <windows.h>

typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
typedef VOID (WINAPI *PWDCHANGEPASSWORD) (LPCSTR lpcRegkeyname,HWND hwnd,UINT uiReserved1,UINT uiReserved2);

BOOL VerifyPassword(HWND hwnd)
{ 
#ifdef _WIN32_WINNT
	// Under NT, we return TRUE immediately. This lets the saver quit,
	// and the system manages passwords. Under '95, we call VerifyScreenSavePwd.
	// This checks the appropriate registry key and, if necessary,
	// pops up a verify dialog
	return TRUE;
#else
	HINSTANCE hpwdcpl;
	VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
	BOOL bres;
	
	if(GetVersion() < 0x80000000)
		return TRUE;

	hpwdcpl = LoadLibrary(L"PASSWORD.CPL");

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

	return bres;
#endif
}


BOOL ChangePassword(HWND hwnd)
{ 
#ifdef _WIN32_WINNT
	return TRUE;
#else
	// This only ever gets called under '95, when started with the /a option.
	HINSTANCE hmpr = LoadLibrary(L"MPR.DLL");
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
	return TRUE;
#endif
}
