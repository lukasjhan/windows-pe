#include "..\stdafx.h"
#include "..\PEExplorer.h"
#include "ResViewBitmap.h"
#include "afxdialogex.h"


// CResViewBitmap 대화 상자입니다.
IMPLEMENT_DYNAMIC(CResViewBitmap, CDialogEx)

CResViewBitmap::CResViewBitmap(PBYTE pImgStart, UINT uType, CWnd* pParent /*=NULL*/)
	: CDialogEx(CResViewBitmap::IDD, pParent)
{
	m_pImgStart		= pImgStart;
	m_uType			= uType;
	m_bIsCompressed = false;
}

CResViewBitmap::~CResViewBitmap()
{
}

void CResViewBitmap::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResViewBitmap, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CResViewBitmap 메시지 처리기입니다.
BOOL CResViewBitmap::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString szTitle;
	if (m_uType == (UINT)RT_BITMAP)
		szTitle = L" 비트맵 리소스 정보";
	else if (m_uType == (UINT)RT_ICON)
		szTitle = L" 아이콘 리소스 정보";
	else
		szTitle = L" 커서 리소스 정보";
	SetWindowText(szTitle);

	CRect rc;
	GetClientRect(rc);
	CRect rcView;
	GetDlgItem(IDC_STC_SHOWBMP)->GetWindowRect(rcView);
	ScreenToClient(rcView);
	m_siMargin.cx = rc.Width() - rcView.Width();
	m_siMargin.cy = rc.Height() - rcView.Height();

	CListCtrl* pLV = (CListCtrl*)GetDlgItem(IDC_LV_BMPINFO);
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	pLV->SetExtendedStyle(pLV->GetExtendedStyle() | dwExStyle);
	pLV->InsertColumn(0, L"필드", LVCFMT_LEFT, 80);
	pLV->InsertColumn(1, L"값", LVCFMT_LEFT, 110);
	ParseBitmap(pLV);

	return TRUE;
}

void CResViewBitmap::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	DrawPalette(&dc);
	if (!m_bIsCompressed)
		DrawBitmap(&dc);
}

void CResViewBitmap::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (GetDlgItem(IDC_STC_SHOWBMP)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STC_SHOWBMP)->SetWindowPos(NULL, 0, 0, 
			cx - m_siMargin.cx, cy - m_siMargin.cy, SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CResViewBitmap::ParseBitmap(CListCtrl* pLV)
{
	CString sz;
	int nItem = 0;
	PBYTE pIter = m_pImgStart;

	if(m_uType == (UINT)RT_CURSOR)
	{
		WORD wHspX = *((PWORD)pIter); pIter += sizeof(WORD);
		WORD wHspY = *((PWORD)pIter); pIter += sizeof(WORD);
		pLV->InsertItem(nItem, L"핫스팟");
		sz.Format(L"(%d, %d)", wHspX, wHspY);
		pLV->SetItemText(nItem, 1, sz); nItem++;
	}

	PBITMAPINFOHEADER pbmi = (PBITMAPINFOHEADER)pIter;
	pIter += sizeof(BITMAPINFOHEADER);

	pLV->InsertItem(nItem, L"헤더 크기");
	sz.Format(L"%d (0x%08X)", pbmi->biSize, pbmi->biSize);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"너비x높이");
	sz.Format(L"%dx%d", pbmi->biWidth, pbmi->biHeight);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"플레인 수");
	sz.Format(L"%d", pbmi->biPlanes);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"컬러 비트수");
	sz.Format(L"%d", pbmi->biBitCount);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"이미지 크기");
	sz.Format(L"%d (0x%08X)", pbmi->biSizeImage, pbmi->biSizeImage);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"압축");
	sz.Format(L"%d", pbmi->biCompression);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"X, Y P/M");
	sz.Format(L"%d, %d", pbmi->biXPelsPerMeter, pbmi->biYPelsPerMeter);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"사용된 컬러");
	sz.Format(L"%d", pbmi->biClrUsed);
	pLV->SetItemText(nItem, 1, sz); nItem++;
	pLV->InsertItem(nItem, L"주요 컬러");
	sz.Format(L"%d", pbmi->biClrImportant);
	pLV->SetItemText(nItem, 1, sz); nItem++;

	m_bIsCompressed = (pbmi->biCompression != 0);
	if(m_bIsCompressed)
		AfxMessageBox(L"이미지가 압축되어 있습니다. 이미지 출력이 되지 않습니다.");
}

LONG CResViewBitmap::GetStorageWidth(LONG lWidth, WORD wBitCnt)
{
	LONG lStorW = 0;
	if (wBitCnt <= 8)
		lStorW = lWidth / (8 / wBitCnt);
	else
		lStorW = lWidth * (wBitCnt / 8);
	return ROUND_UP(lStorW, 4);
}

COLORREF CResViewBitmap::GetPixelColor(LONG x, LONG y, WORD wBitCnt, 
	LONG lStorageWidth, LONG lHeight, PBYTE pRawBits, LPRGBQUAD pRgbPal)
{
	COLORREF rgb = 0;
	if (wBitCnt <= 8)
	{
		LONG lBasePos, lBitOff, lClrIdx;
		switch (wBitCnt)
		{
			case 1:
				lBasePos = (lHeight - y - 1) * lStorageWidth + (x >> 3);
				lBitOff = 7 - (x & 7);
				lClrIdx = (pRawBits[lBasePos] & (1 << lBitOff)) >> lBitOff;
			break;
			case 4:
				lBasePos = (lHeight - y - 1) * lStorageWidth + (x >> 1);
				lBitOff = (1 - (x & 1)) << 2;
				lClrIdx = (pRawBits[lBasePos] & (0x0F << lBitOff)) >> lBitOff;
			break;
			case 8:
				lBasePos = (lHeight - y - 1) * lStorageWidth + x;
				lClrIdx = pRawBits[lBasePos];
			break;
		}
		rgb = RGB(pRgbPal[lClrIdx].rgbRed,
			pRgbPal[lClrIdx].rgbGreen, pRgbPal[lClrIdx].rgbBlue);
	}
	else
	{
		LONG lBasePos = (lHeight - y - 1) * lStorageWidth + (x * (wBitCnt >> 3));
		if (wBitCnt == 16)
		{
			WORD wClr = *((PWORD)(pRawBits + lBasePos));
			rgb = RGB((((wClr & 0x7C00) >> 10) << 3),
				(((wClr & 0x03E0) >> 5) << 3), ((wClr & 0x001F) << 3));
		}
		else
		{
			rgb = RGB(pRawBits[lBasePos + 2],
				pRawBits[lBasePos + 1], pRawBits[lBasePos]);
		}

	}
	return rgb;
}

void CResViewBitmap::DrawBitmap(CDC* pDC)
{
	PBYTE pIter = m_pImgStart;

	if (m_uType == (UINT)RT_CURSOR)
		pIter += sizeof(DWORD);
	LPBITMAPINFOHEADER pbmi = (LPBITMAPINFOHEADER)pIter;
	pIter += sizeof(BITMAPINFOHEADER);

	INT nClrCnt = (pbmi->biBitCount > 0 && 
		pbmi->biBitCount <= 8) ? (1 << pbmi->biBitCount) : 0;
	LPRGBQUAD pRgbPal = (LPRGBQUAD)pIter;
	pIter += sizeof(RGBQUAD) * nClrCnt;

	CRect rc;
	GetDlgItem(IDC_STC_SHOWBMP)->GetWindowRect(rc);
	ScreenToClient(rc);

	CRgn rgnClip;
	rgnClip.CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
	pDC->SelectClipRgn(&rgnClip);

	INT nSX = rc.left + 1;
	INT nSY = rc.top  + 1;

	PBYTE pRawBits = pIter;
	LONG lStorageWidth = GetStorageWidth(pbmi->biWidth, pbmi->biBitCount);
	LONG lHeight = (m_uType == (UINT)RT_BITMAP) ? 
		pbmi->biHeight : (pbmi->biHeight >> 1);
	for(LONG y=0; y < lHeight; y++)
	{
		for(LONG x=0; x<pbmi->biWidth; x++)
		{
			COLORREF clrPixel = GetPixelColor(x, y, pbmi->biBitCount, 
				lStorageWidth, lHeight, pRawBits, pRgbPal);
			pDC->SetPixel(nSX + x, nSY + y, clrPixel);
		}
	}
	if (m_uType == (UINT)RT_BITMAP)
		return;

	pRawBits += (lStorageWidth * lHeight);
	nSX += (pbmi->biWidth + 5);

	lStorageWidth = GetStorageWidth(pbmi->biWidth, 1);	
	RGBQUAD rqMono[2] = { 0, 0, 0, 0, 255, 255, 255, 255 };
	for(LONG y = 0; y < lHeight; y++)
	{
		for(LONG x = 0; x < pbmi->biWidth; x++)
		{
			COLORREF clrPixel = GetPixelColor
				(x, y, 1, lStorageWidth, lHeight, pRawBits, rqMono);
			pDC->SetPixel(nSX + x, nSY + y, clrPixel);
		}
	}
}

void CResViewBitmap::DrawPalette(CDC* pDC)
{
	PBYTE pIter = m_pImgStart;

	if (m_uType == (UINT)RT_CURSOR)
		pIter += sizeof(DWORD);
	LPBITMAPINFOHEADER pbmi = (LPBITMAPINFOHEADER)pIter;
	pIter += sizeof(BITMAPINFOHEADER);

	if(!pbmi->biBitCount || pbmi->biBitCount > 8)
		return;

	INT nClrCnt = (1 << pbmi->biBitCount);
	LPRGBQUAD pRgbPal = (LPRGBQUAD)pIter;
	pIter += sizeof(RGBQUAD) * nClrCnt;

	CRect rc;
	GetDlgItem(IDC_LV_BMPINFO)->GetWindowRect(rc);
	ScreenToClient(rc);

	CRect rcClr;
	bool bIsEnd = false;

	rcClr.top = rc.top;
	for(int j=0; j<8; j++)
	{
		rcClr.bottom = rcClr.top + 10;
		rcClr.left = rc.right + 10;

		for(int i=0; i<32; i++)
		{
			int nClrIdx = j * 32 + i;
			if(nClrIdx >= nClrCnt)
			{
				bIsEnd = true;
				break;
			}

			rcClr.right = rcClr.left + 10;

			CBrush br(RGB(pRgbPal[nClrIdx].rgbRed,
				pRgbPal[nClrIdx].rgbGreen, pRgbPal[nClrIdx].rgbBlue));
			CBrush* pOldBr = pDC->SelectObject(&br);
			pDC->Rectangle(rcClr);
			pDC->SelectObject(pOldBr);

			rcClr.left = rcClr.right-1;
		}
		if(bIsEnd)
			break;
		rcClr.top = rcClr.bottom - 1;
	}
}
