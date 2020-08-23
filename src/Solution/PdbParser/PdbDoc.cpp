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


// CPdbDoc ����/�Ҹ�
CPdbDoc::CPdbDoc()
{
	// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.
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
// ����� �׸��� �����մϴ�.
void CPdbDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// ������ �����͸� �׸����� �� �ڵ带 �����Ͻʽÿ�.
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

// �˻� ó���⸦ �����մϴ�.
void CPdbDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ������ �����Ϳ��� �˻� �������� �����մϴ�.
	// ������ �κ��� ";"�� ���еǾ�� �մϴ�.

	// ��: strSearchContent = _T("point;rectangle;circle;ole object;");
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

// CPdbDoc ����
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

// CPdbDoc ���
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
