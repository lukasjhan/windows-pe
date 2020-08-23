#pragma once

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif
#include "targetver.h"
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS
#include <afxwin.h>				// MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>				// MFC 확장입니다.
#include <afxcview.h>
#include <afxdisp.h>			// MFC 자동화 클래스입니다.
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>			// Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>				// Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxcontrolbars.h>		// MFC의 리본 및 컨트롤 막대 지원
#include <afxhtml.h>			// MFC HTML 뷰 지원

#include <atlbase.h>
#include <atlcom.h>
//#include <atlstr.h>

#include <vector>
#include <map>
#include <set>
#include <list>

#include <Dia2.h>
#import  <msxml3.dll> 
typedef MSXML2::IXMLDOMDocument2Ptr				XmlDocument;
typedef MSXML2::IXMLDOMElementPtr				XmlElement;
typedef MSXML2::IXMLDOMAttributePtr				XmlAttribute;
typedef MSXML2::IXMLDOMNodePtr					XmlNode;
typedef MSXML2::IXMLDOMNodeListPtr				XmlNodeList;
typedef MSXML2::IXMLDOMNamedNodeMapPtr			XmlNodeMap;
typedef MSXML2::IXMLDOMProcessingInstructionPtr	XmlProcInstruct;
typedef MSXML2::IXMLDOMParseErrorPtr			XmlParseError;
typedef MSXML2::IXMLDOMCDATASectionPtr			XmlCDataSection;

#include "..\PEDAsm\PEDAsm.h"
#include "..\PEPlus\PEPlus.h"
#include "..\PEPdb\PEPdb.h"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


