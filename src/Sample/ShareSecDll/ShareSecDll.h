#ifndef __SHARE_SEC_H__
#define __SHARE_SEC_H__

#ifndef SHARE_SEC_API
#	ifdef __cplusplus
#		define SHARE_SEC_API extern "C" __declspec(dllimport)
#	else
#		define SHARE_SEC_API __declspec(dllimport)
#	endif
#endif

SHARE_SEC_API int WINAPI Share_ReadBuff(PSTR pszMsg, int nLen);
SHARE_SEC_API int WINAPI Share_WriteBuff(PCSTR pszMsg, int nLen);

#endif	//__SHARE_SEC_H__