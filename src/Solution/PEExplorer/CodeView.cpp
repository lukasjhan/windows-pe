#include "stdafx.h"
#include "PEExplorer.h"
#include "PEAnals.h"
#include "CodeView.h"
#include "afxdialogex.h"


// CCodeView 대화 상자입니다.
IMPLEMENT_DYNAMIC(CCodeView, CDialogEx)

CCodeView::CCodeView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent /*=NULL*/)
	: CDialogEx(CCodeView::IDD, pParent)
{
	m_pPN = pNode;
	m_pPE = pPE;
}

CCodeView::~CCodeView()
{
}

void CCodeView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LV_DATA, m_lv);
}

void CCodeView::GetLocalVars(IDiaSymbol* pIFunc, LOCV_MAP& mapLocs)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (pIFunc->findChildren(SymTagData, NULL, nsNone, &pIEnumSyms) == S_OK)
	{
		ULONG uCelt = 0;
		CComPtr<IDiaSymbol> pISubSym;
		while (SUCCEEDED(pIEnumSyms->Next(1, &pISubSym, &uCelt)) && (uCelt == 1))
		{
			DataKind dk;
			CComBSTR bszName;
			LONG lRegOff;
			DWORD dwRegId;

			if ((pISubSym->get_dataKind((PDWORD)&dk) == S_OK &&
				(dk == DataKind::DataIsLocal || dk == DataKind::DataIsParam)) &&
				(pISubSym->get_name(&bszName) == S_OK) &&
				(pISubSym->get_offset(&lRegOff) == S_OK) &&
				(pISubSym->get_registerId(&dwRegId) == S_OK))
			{
				CString sz;
				if (dwRegId == CV_ALLREG_VFRAME)
					dwRegId = CV_REG_EBP;
				sz = PEPdb::ToStr_C7REG((USHORT)dwRegId, m_pPE->DIA_PDB.MACHINE_TYPE);
				sz.MakeUpper();
				sz.AppendFormat(L"%d", lRegOff);

				CString szt;
				CComPtr<IDiaSymbol> pIType;
				if (pISubSym->get_type(&pIType) == S_OK)
				{
					szt = PEPdb::ToStr_Type(pIType);
					pIType = 0;
				}
				
				CString szi;
				szi.Format(L"%s{%s:%s}", bszName, PEPdb::GSZ_DATA_KINDS[dk], szt);
				mapLocs.insert(std::make_pair(sz, szi));
			}
			pISubSym = 0;
		}
		pIEnumSyms = 0;
	}
}

void CCodeView::ScanLocalVars(IDiaSymbol* pIFunc, LOCV_MAP& mapLocs)
{
	GetLocalVars(pIFunc, mapLocs);
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (pIFunc->findChildren(SymTagBlock, NULL, nsNone, &pIEnumSyms) == S_OK)
	{
		ULONG uCelt = 0;
		CComPtr<IDiaSymbol> pISubSym;
		while (SUCCEEDED(pIEnumSyms->Next(1, &pISubSym, &uCelt)) && (uCelt == 1))
		{
			ScanLocalVars(pISubSym, mapLocs);
			pISubSym = 0;
		}
		pIEnumSyms = 0;
	}
}

void CCodeView::BuildCodeList(PIMAGE_SECTION_HEADER psh, IDiaSymbol* pIFunc, LOCV_MAP& mapLocs)
{
	CString sz; USES_CONVERSION;
	int nItemIdx = 0;
	DWORD dwCodeOff = m_pPN->Offset;
	int nCodeLen = (int)m_pPN->Size;
	while (nCodeLen > 0)
	{
		DECODED_INS di; int ndiCnt = 1;
		PEDisAsm::DisAssemble(&di, ndiCnt, m_pPE->GetImgView(), 
			dwCodeOff, psh->Misc.VirtualSize, m_pPE->Is32Bit());
		dwCodeOff += di._count;
		nCodeLen -= di._count;

		DWORD dwRVA = OFFSET_TO_RVA(psh, di._offset);
		sz.Format(L"0x%08X:0x%08X", di._offset, dwRVA);
		int nCurIdx = m_lv.InsertItem(nItemIdx, sz);
		m_lv.SetItemText(nCurIdx, 1, PE_SCHEMA::Bin2Str
			(m_pPE->GetImgView() + di._offset, PE_TYPE::UInt8, di._count));

		DECODED dec;
		PEDisAsm::DecodedFormat(&di, &dec);
		m_lv.SetItemText(nCurIdx, 2, A2T(dec.Mnemonic));
		if (dec.Operands != NULL)
			m_lv.SetItemText(nCurIdx, 3, A2CT(dec.Operands));
		nItemIdx++;

		sz.Empty();
		for (int j = 0; j < MAX_OPRNDS_COUNT; j++)
		{
			if (di._oprs[j]._type == O_NONE)
				break;

			if ((di._oprs[j]._type == O_SMEM) &&
				(di._oprs[j]._regid == REG_IID::R_RSP ||
				di._oprs[j]._regid == REG_IID::R_ESP ||
				di._oprs[j]._regid == REG_IID::R_EBP))
			{
				if (pIFunc == NULL)
					continue;

				LONG v = (long)di._dispV;
				CString szv;
				szv.Format(L"%s%d", A2CT(PEDisAsm::G_REGISTERS[di._oprs[j]._regid]), v);
				LOCV_MAP::iterator it = mapLocs.find(szv);
				if (it != mapLocs.end())
				{
					if (!sz.IsEmpty())
						sz.Append(L", ");
					sz.Append(it->second);
				}
				continue;
			}

			INT64 ll = 0;
			if (di._oprs[j]._type == O_PC)
			{
				ll = di._imm.sqword;
			}
			else if ((di._oprs[j]._type == O_DISP &&
				(di._seg != REG_IID::R_FS && di._seg != REG_IID::R_GS)) ||
				(di._oprs[j]._type == O_SMEM && di._oprs[j]._regid == REG_IID::R_RIP))
			{
				ll = di._dispV;
			}
			if (ll == 0)
				continue;

			if (!sz.IsEmpty())
				sz.Append(L", ");

			DWORD dwRVA = 0;
			if (di._oprs[j]._type == O_DISP)
			{
				INT64 nImgBase = PEPlus::GetImageBase(m_pPE->GetImgView());
				dwRVA = (DWORD)(ll - nImgBase);
			}
			else
			{
				dwRVA = OFFSET_TO_RVA(psh, dwCodeOff);
				dwRVA += (int)ll;
			}
			if (!m_pPE->DIA_PDB.Loaded())
			{
				PIMAGE_SECTION_HEADER psh2 = PEPlus::FindSectHdr(m_pPE->GetImgView(), dwRVA);
				if (psh2 != NULL)
					sz.AppendFormat(L"0x%08X:[%s]0x%08X", dwRVA,
					PEPlus::GetSectionName(psh2), RVA_TO_OFFSET(psh2, dwRVA));
				else
					sz.AppendFormat(L"RVA:0x%08X", dwRVA);
				m_lv.SetItemText(nCurIdx, 4, sz);
				continue;
			}

			CComPtr<IDiaSymbol> pISym;
			LONG lDisp = 0;
			if (m_pPE->DIA_PDB.SESSION->findSymbolByRVAEx(dwRVA, SymTagNull, &pISym, &lDisp) != S_OK)
				continue;

			DIA_SYMTAG tag = SymTagNull;
			pISym->get_symTag((PDWORD)&tag);

			CComBSTR bszName;
			pISym->get_name(&bszName);
			if (tag == DIA_SYMTAG::SymTagData)
			{
				sz = bszName;
				DataKind dk;
				if (pISym->get_dataKind((PDWORD)&dk) == S_OK)
				{
					CString szt;
					CComPtr<IDiaSymbol> pIType;
					if (pISym->get_type(&pIType) == S_OK)
					{
						szt = PEPdb::ToStr_Type(pIType);
						pIType = 0;
					}
					if (lDisp > 0)
						sz.AppendFormat(L"+0x%X{0x%X:%s:%s}", lDisp, dwRVA, PEPdb::GSZ_DATA_KINDS[dk], szt);
					else
						sz.AppendFormat(L"{0x%X:%s:%s}", dwRVA, PEPdb::GSZ_DATA_KINDS[dk], szt);
				}
			}
			else
			{
				if (pIFunc != NULL && tag == DIA_SYMTAG::SymTagBlock)
					pIFunc->get_name(&bszName);
				sz.Append(bszName);
				if (lDisp > 0)
					sz.AppendFormat(L"+0x%X", lDisp);
				sz.AppendFormat(L"{0x%X:%s}", dwRVA, PEPdb::GSZ_SYM_TAGS[tag]);
			}
		}
		m_lv.SetItemText(nCurIdx, 4, sz);
	}
}


BEGIN_MESSAGE_MAP(CCodeView, CDialogEx)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LV_DATA, &CCodeView::OnLvnGetdispinfoLvData)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LV_DATA, &CCodeView::OnLvnKeydownLvData)
END_MESSAGE_MAP()

// CCodeView 메시지 처리기입니다.
BOOL CCodeView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rc;
	GetClientRect(rc);
	CRect rcView;
	m_lv.GetWindowRect(rcView);
	ScreenToClient(rcView);
	m_siMargin.cx = rc.Width() - rcView.Width();
	m_siMargin.cy = rc.Height() - rcView.Height();

	//m_fnt.CreateFont(15, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
	//	0, 0, 0, DEFAULT_PITCH | FF_DONTCARE, _T("Courier New")); //_T("Lucida Console"));
	//m_lv.SetFont(&m_fnt);

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	m_lv.SetExtendedStyle(m_lv.GetExtendedStyle() | dwExStyle);
	m_lv.InsertColumn(0, _T("오프셋:RVA"), LVCFMT_LEFT, 160);
	m_lv.InsertColumn(1, _T("코드"), LVCFMT_LEFT, 250);
	m_lv.InsertColumn(2, _T("니모닉"), LVCFMT_LEFT, 100);
	m_lv.InsertColumn(3, _T("오퍼랜드"), LVCFMT_LEFT, 250);
	m_lv.InsertColumn(4, _T("상세"), LVCFMT_LEFT, 320);

	LOCV_MAP mapLocs;
	PIMAGE_SECTION_HEADER psh = &(m_pPE->GetSectHdrs()[m_pPN->Index]);

	CString sz;
	CComPtr<IDiaSymbol> pIFunc;
	if (m_pPN->Tag != 0)
	{
		pIFunc = *((CComPtr<IDiaSymbol>*)m_pPN->Tag);
		ScanLocalVars(pIFunc, mapLocs);

		DIA_SYMTAG tag;
		pIFunc->get_symTag((PDWORD)&tag);
		if (tag == SymTagFunction)
		{
			CComBSTR bszName;
			pIFunc->get_name(&bszName);
			sz.Format(L"코드 보기: %s", PEPdb::ToStr_Function(pIFunc));
		}
		else
		{
			CComPtr<IDiaSymbol> pIUp;
			pIFunc->get_lexicalParent(&pIUp);
			CComBSTR bszName;
			pIUp->get_name(&bszName);
			sz.Format(L"코드 보기: Block: [0x%08X]%s", m_pPN->Offset, bszName);
		}
	}
	else
		sz.Format(L"코드 보기: 『%s』", PEPlus::GetSectionName(psh));
	SetWindowText(sz);

	BuildCodeList(psh, pIFunc, mapLocs);
	if (pIFunc != 0)
		pIFunc = 0;

	return TRUE;
}

void CCodeView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_lv.GetSafeHwnd())
	{
		m_lv.SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, cy - m_siMargin.cy, SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CCodeView::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CCodeView::OnLvnGetdispinfoLvData(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}

void CCodeView::OnLvnKeydownLvData(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN plvkd = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	if (GetAsyncKeyState(plvkd->wVKey) < 0)
	{
		if(plvkd->wVKey == 'a' || plvkd->wVKey == 'A')
		{
			int nCnt = m_lv.GetItemCount();
			for (int i = 0; i < nCnt; i++)
			{
				m_lv.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
		else if (plvkd->wVKey == 'c' || plvkd->wVKey == 'C')
		{
			//int arSpaces[COL_IDX_MAX - 2] = { 10, 4, }
			CString sz;
			POSITION pos = m_lv.GetFirstSelectedItemPosition();
			while (pos)
			{
				int nSelIdx = m_lv.GetNextSelectedItem(pos);
				for (int j = 0; j < COL_IDX_MAX; j++)
				{
					CString szi = m_lv.GetItemText(nSelIdx, j);
					if (szi.IsEmpty())
						continue;

					if (j == 0)
					{
						sz.Append(szi);
						continue;
					}
					
					if (j == 1)
						sz.AppendFormat(L"\t%-23s", szi);
					else if (j == 2)
						sz.AppendFormat(L"\t%-7s", szi);
					else
					{
						if (j == 4)
							sz.AppendFormat(L"\t; %s", szi);
						else
							sz.AppendFormat(L"\t%s", szi);
					}
				}
				sz.Append(L"\xd\xa");
			}
			sz = sz.Trim();
			if (!sz.IsEmpty())
			{
				int nSize = (sz.GetLength() + 1) * sizeof(TCHAR);
				HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
				PBYTE pData = (PBYTE)::GlobalLock(hData);
				memcpy(pData, (PBYTE)(PCTSTR)sz, nSize);
				::GlobalUnlock(hData);

				COleDataSource* pDS = new COleDataSource;
				pDS->CacheGlobalData(CF_UNICODETEXT, hData);
				pDS->SetClipboard();
			}
		}
	}
	*pResult = 0;
}
