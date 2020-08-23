#pragma once


struct MENU_ITEM
{
	WORD	_wAttr;
	WORD	_wID;
	WCHAR	_szName[MAX_PATH];

	MENU_ITEM()
	{
		_wAttr	= 0;
		_wID	= 0;
		memset(_szName, 0x00, sizeof(_szName));
	}
};
typedef MENU_ITEM* PMENU_ITEM;

struct CREATE_PRM
{
	HMENU		_hMenu;
	CTreeCtrl*	_pTree;
};
typedef CREATE_PRM* PCREATE_PRM;

// CResViewMenu ��ȭ �����Դϴ�.
class CResViewMenu : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewMenu)

	LPBYTE	m_pImgStart;

	void ShowParsedMenu(bool);
	HMENU MakeMenuItem(LPBYTE&, bool=true);
	CTreeCtrl* Tv() { return (CTreeCtrl*)GetDlgItem(IDC_TV_MENU);}

	void ParseMenuItems();
	void MakeMenuTree(LPBYTE& pIter, HTREEITEM);

public:
	CResViewMenu(LPBYTE, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CResViewMenu();

	static PMENU_ITEM FindItem(CTreeCtrl*, HTREEITEM, WORD);

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_RES_MENU };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTvnGetdispinfoTvMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnShowMain();
	afx_msg void OnBnClickedBtnShowPopup();
	afx_msg void OnTvnSelchangedTvMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDeleteitemTvMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
};
