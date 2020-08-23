#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PdbParser.h"
#endif
#include "PdbDoc.h"
#include <propkey.h>

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

DWORD CPdbDoc::MACHINE_TYPE;

// CPdbDoc
IMPLEMENT_DYNCREATE(CPdbDoc, CDocument)
BEGIN_MESSAGE_MAP(CPdbDoc, CDocument)
END_MESSAGE_MAP()


// CPdbDoc 생성/소멸
CPdbDoc::CPdbDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.
}

CPdbDoc::~CPdbDoc()
{
	//for (SYM_ID_MAP::iterator it = SYMBOLS.begin(); it != SYMBOLS.end(); it++)
	//	delete it->second;
	DIA_SOURCE  = 0;
	DIA_SESSION = 0;
	DIA_PESCOPE = 0;
}


#ifdef SHARED_HANDLERS
// 축소판 그림을 지원합니다.
void CPdbDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 문서의 데이터를 그리려면 이 코드를 수정하십시오.
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 검색 처리기를 지원합니다.
void CPdbDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 문서의 데이터에서 검색 콘텐츠를 설정합니다.
	// 콘텐츠 부분은 ";"로 구분되어야 합니다.

	// 예: strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CPdbDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CPdbDoc 진단
#ifdef _DEBUG
void CPdbDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPdbDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CPdbDoc 명령
BOOL CPdbDoc::OnNewDocument()
{
	return TRUE;
}

BOOL CPdbDoc::OnSaveDocument(PCTSTR lpszPathName)
{
	return FALSE;
}

BOOL CPdbDoc::OnOpenDocument(PCTSTR pszPathName)
{
	DIA_SESSION = 0;
	DIA_PESCOPE = 0;
	DIA_SOURCE  = 0;

	try
	{
		HRESULT hr = CoCreateInstance
		(
			__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiaDataSource), (PVOID*)&DIA_SOURCE
		);
		if (FAILED(hr))
			throw hr;
		hr = DIA_SOURCE->loadDataFromPdb(pszPathName);
		if (FAILED(hr))
			throw hr;
		hr = DIA_SOURCE->openSession(&DIA_SESSION);
		if (FAILED(hr))
			throw hr;
		hr = DIA_SESSION->get_globalScope(&DIA_PESCOPE);
		if (hr != S_OK)
			throw hr;

		DWORD dwMachType = 0;
		if (DIA_PESCOPE->get_machineType(&dwMachType) == S_OK)
		{
			switch (dwMachType)
			{
			case IMAGE_FILE_MACHINE_I386 : MACHINE_TYPE = CV_CFL_80386; break;
			case IMAGE_FILE_MACHINE_IA64 : MACHINE_TYPE = CV_CFL_IA64 ; break;
			case IMAGE_FILE_MACHINE_AMD64: MACHINE_TYPE = CV_CFL_AMD64; break;
			}
		}

		return TRUE;
	}
	catch (HRESULT e)
	{
		CString szMsg;
		szMsg.Format(L"Open pdb file \"%s\" failed, Code = 0x%08X", e);
		AfxMessageBox(szMsg);
		return FALSE;
	}
}
