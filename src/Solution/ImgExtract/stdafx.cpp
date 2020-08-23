#include "stdafx.h"

#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x64/Debug/PEPlus.lib")
#	else
#		pragma comment(lib, "../../3.lib/x64/Release/PEPlus.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x86/Debug/PEPlus.lib")
#	else
#		pragma comment(lib, "../../3.lib/x86/Release/PEPlus.lib")
#	endif
#endif
