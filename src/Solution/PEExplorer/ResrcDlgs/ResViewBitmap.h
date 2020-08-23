#pragma once


// CResViewBitmap ��ȭ �����Դϴ�.
class CResViewBitmap : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewBitmap)

	LPBYTE	m_pImgStart;
	UINT	m_uType;
	bool	m_bIsCompressed;
	CSize	m_siMargin;

	void ParseBitmap(CListCtrl*);
	void DrawBitmap(CDC* pDC);
	void DrawPalette(CDC* pDC);
	LONG GetStorageWidth(LONG, WORD);
	COLORREF GetPixelColor(LONG, LONG, 
		WORD, LONG, LONG, PBYTE, LPRGBQUAD);

public:
	CResViewBitmap(PBYTE, UINT, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CResViewBitmap();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_RES_BITMAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
