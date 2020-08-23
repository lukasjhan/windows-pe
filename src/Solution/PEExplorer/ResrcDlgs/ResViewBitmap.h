#pragma once


// CResViewBitmap 대화 상자입니다.
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
	CResViewBitmap(PBYTE, UINT, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CResViewBitmap();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_RES_BITMAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
