#include "Windows.h"
#include "tchar.h"

#ifdef __cplusplus
#	define SHARE_SEC_API extern "C" __declspec(dllexport)
#else
#	define SHARE_SEC_API __declspec(dllexport)
#endif
#include "ShareSecDll.h"

#define MAX_MSG_LEN	511

#pragma section("YHDSHARE", read, write, shared)	
__declspec(allocate("YHDSHARE"))  
	CHAR SHARED_BUFF[MAX_MSG_LEN + 1] = "YHD Shared Section";


int WINAPI Share_ReadBuff(PSTR pszMsg, int nLen)
{
	if (nLen > MAX_MSG_LEN)
		nLen = MAX_MSG_LEN;
	memcpy(pszMsg, SHARED_BUFF, nLen);
	pszMsg[nLen] = 0;
	return nLen;
}

int WINAPI Share_WriteBuff(PCSTR pszMsg, int nLen)
{
	if (nLen > MAX_MSG_LEN)
		nLen = MAX_MSG_LEN;
	memcpy(SHARED_BUFF, pszMsg, nLen);
	SHARED_BUFF[nLen] = 0;
	return nLen;
}
