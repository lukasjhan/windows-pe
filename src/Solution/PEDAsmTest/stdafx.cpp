#include "stdafx.h"

#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x64/Debug/PEDAsm.lib")
#	else
#		pragma comment(lib, "../../3.lib/x64/Release/PEDAsm.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x86/Debug/PEDAsm.lib")
#	else
#		pragma comment(lib, "../../3.lib/x86/Release/PEDAsm.lib")
#	endif
#endif
