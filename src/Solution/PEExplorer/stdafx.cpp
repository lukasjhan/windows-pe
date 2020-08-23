#include "stdafx.h"

#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x64/Debug/PEDAsm.lib")
#		pragma comment(lib, "../../3.lib/x64/Debug/PEPlus.lib")
#		pragma comment(lib, "../../3.lib/x64/Debug/PEPdb.lib")
#	else
#		pragma comment(lib, "../../3.lib/x64/Release/PEDAsm.lib")
#		pragma comment(lib, "../../3.lib/x64/Release/PEPlus.lib")
#		pragma comment(lib, "../../3.lib/x64/Release/PEPdb.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x86/Debug/PEDAsm.lib")
#		pragma comment(lib, "../../3.lib/x86/Debug/PEPlus.lib")
#		pragma comment(lib, "../../3.lib/x86/Debug/PEPdb.lib")
#	else
#		pragma comment(lib, "../../3.lib/x86/Release/PEDAsm.lib")
#		pragma comment(lib, "../../3.lib/x86/Release/PEPlus.lib")
#		pragma comment(lib, "../../3.lib/x86/Release/PEPdb.lib")
#	endif
#endif
