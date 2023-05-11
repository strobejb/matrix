// define this file to overcome the "Cannot open include file "afxres.h"" error
// when building with VS2022. The alternative is to download and install the
// v142 build tools (C++ MFC) via Visual Studio Installer, but this
// seems like a simpler workaround 
#pragma once
#include <windows.h>
#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif // !IDC_STATIC

