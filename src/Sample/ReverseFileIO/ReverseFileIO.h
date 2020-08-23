#ifndef REVERSE_FILEIO_API
#define REVERSE_FILEIO_API extern "C" __declspec(dllimport)
#endif

REVERSE_FILEIO_API BOOL WINAPI SetHookMsgProc(BOOL, DWORD);
