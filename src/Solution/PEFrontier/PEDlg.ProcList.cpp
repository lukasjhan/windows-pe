#include "stdafx.h"
#include "PEApp.h"
#include "PEDlg.ProcList.h"
#include "afxdialogex.h"
#include "..\PEPlus\PEPlus.h"
#include <TlHelp32.h>


// CProcList ��ȭ �����Դϴ�.
IMPLEMENT_DYNAMIC(CProcList, CDialogEx)

CProcList::CProcList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProcList::IDD, pParent)
{
	m_dwProcID = 0;
}

CProcList::~CProcList()
{
}

void CProcList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProcList, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &CProcList::OnBnClickedBtnRefresh)
	ON_NOTIFY(NM_DBLCLK, IDC_LV_PROCESS, &CProcList::OnNMDblclkLvProcess)
END_MESSAGE_MAP()

BOOL CProcList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: 
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	LV()->SetExtendedStyle(LV()->GetExtendedStyle() | dwExStyle);

	LV()->InsertColumn(0, _T("���μ��� �̸�"), LVCFMT_LEFT, 160);
	LV()->InsertColumn(1, _T("���μ��� ID"), LVCFMT_RIGHT, 80);

	OnBnClickedBtnRefresh();
	return TRUE;
}

// CProcList �޽��� ó�����Դϴ�.
void CProcList::OnBnClickedBtnRefresh()
{
	LV()->DeleteAllItems();

	PROCESSENTRY32 pe;
	HANDLE hSnapshot = NULL;
	try
	{
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (!hSnapshot)
			throw HRESULT_FROM_WIN32(GetLastError());

		pe.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe))
			throw HRESULT_FROM_WIN32(GetLastError());
		int nIdx = 0;
		do
		{
			LV()->InsertItem(nIdx, pe.szExeFile);
			CString szId; szId.Format(L"%d", pe.th32ProcessID);
			LV()->SetItemText(nIdx, 1, szId);
			LV()->SetItemData(nIdx, (ULONG_PTR)pe.th32ProcessID);
			nIdx++;
		} while (Process32Next(hSnapshot, &pe));
	}
	catch (HRESULT hr)
	{
		AfxMessageBox(PEPlus::GetErrMsg(hr));
	}
	if (hSnapshot != NULL)
		CloseHandle(hSnapshot);
}

void CProcList::OnOK()
{
	// TODO: 
	POSITION pos = LV()->GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(L"���õ� ���μ����� �����ϴ�.");
		LV()->SetFocus();
		return;
	}
	UpdateData();
	int nIdx = LV()->GetNextSelectedItem(pos);
	m_dwProcID = (DWORD)LV()->GetItemData(nIdx);
	CDialogEx::OnOK();
}

void CProcList::OnNMDblclkLvProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	POINT  spt;
	GetCursorPos(&spt);
	ScreenToClient(&spt);
	CPoint cpt;
	cpt.x = spt.x, cpt.y = spt.y;

	int nIndex = LV()->HitTest(spt);
	if (nIndex < 0)
		return;

	DWORD dwProcID = (DWORD)LV()->GetItemData(nIndex);
	if (dwProcID == 0)
		return;
	m_dwProcID = dwProcID;
	CDialogEx::OnOK();
	*pResult = 0;
}
