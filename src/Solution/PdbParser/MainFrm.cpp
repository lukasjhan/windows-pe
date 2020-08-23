#include "stdafx.h"
#include "PdbParser.h"

#include "MainFrm.h"
#include "PdbDoc.h"
#include "PdbTreeView.h"
#include "PdbListView.h"
#include "PdbPropView.h"
#include "PdbTextView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMySplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CSplitterWndEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMySplitterWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bShowSplitter)
		CSplitterWnd::OnMouseMove(nFlags, point);
	else
		CWnd::OnMouseMove(nFlags, point);
}

void CMySplitterWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bShowSplitter)
		CSplitterWnd::OnLButtonDown(nFlags, point);
	else
		CWnd::OnLButtonDown(nFlags, point);
}


// CMainFrm
IMPLEMENT_DYNCREATE(CMainFrm, CFrameWnd)
BEGIN_MESSAGE_MAP(CMainFrm, CFrameWnd)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, &CMainFrm::OnUpdateViewStyles)
	ON_COMMAND_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, &CMainFrm::OnViewStyle)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 상태 줄 표시기
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrm 생성/소멸

CMainFrm::CMainFrm()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CMainFrm::~CMainFrm()
{
}

int CMainFrm::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	return 0;
}

BOOL CMainFrm::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	// 분할 창을 만듭니다.
	if (!m_spltMain.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_spltNested.CreateStatic(&m_spltMain, 2, 1,
		WS_CHILD | WS_VISIBLE | WS_BORDER, m_spltMain.IdFromRowCol(0, 1)))
		return FALSE;

	if (!m_spltLeft.CreateStatic(&m_spltNested, 1, 2,
		WS_CHILD | WS_VISIBLE | WS_BORDER, m_spltNested.IdFromRowCol(1, 0)))
		return FALSE;

	BOOL bIsOK = m_spltLeft.CreateView(0, 0, RUNTIME_CLASS(CPdbPropView), CSize(500, 500), pContext);
	if (!bIsOK)
	{
		m_spltMain.DestroyWindow();
		m_spltNested.DestroyWindow();
		return FALSE;
	}
	bIsOK = m_spltLeft.CreateView(0, 1, RUNTIME_CLASS(CPdbTextView), CSize(0, 0), pContext);
	if (!bIsOK)
	{
		m_spltMain.DestroyWindow();
		m_spltNested.DestroyWindow();
		return FALSE;
	}

	bIsOK = m_spltNested.CreateView(0, 0, RUNTIME_CLASS(CPdbListView), CSize(100, 500), pContext);
	if (!bIsOK)
	{
		m_spltMain.DestroyWindow();
		m_spltNested.DestroyWindow();
		return FALSE;
	}

	bIsOK = m_spltMain.CreateView(0, 0, RUNTIME_CLASS(CPdbTreeView), CSize(250, 100), pContext);
	if (!bIsOK)
	{
		m_spltMain.DestroyWindow();
		m_spltNested.DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrm::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

// CMainFrm 진단
#ifdef _DEBUG
void CMainFrm::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrm::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrm 메시지 처리기

CPdbListView* CMainFrm::GetRightPane()
{
	CWnd* pWnd = m_spltMain.GetPane(0, 1);
	CPdbListView* pView = DYNAMIC_DOWNCAST(CPdbListView, pWnd);
	return pView;
}

void CMainFrm::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
	if (!pCmdUI)
		return;

	// TODO: 이 코드를 사용자 지정하거나 확장하여 보기 메뉴에서의 선택을 처리합니다.
	CPdbListView* pView = GetRightPane();

	// 오른쪽 창이 만들어지지 않았거나 뷰가 아닌 경우
	// 해당 범위에 있는 명령을 비활성화합니다.
	if (pView == NULL)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

		// 명령이 ID_VIEW_LINEUP이면
		// LVS_ICON 또는 LVS_SMALLICON 모드인 경우에만 이 명령을 활성화합니다.

		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			// 그렇지 않으면 점을 사용하여 뷰의 스타일을 반영합니다.
			pCmdUI->Enable();
			BOOL bChecked = FALSE;

			switch (pCmdUI->m_nID)
			{
			case ID_VIEW_DETAILS:
				bChecked = (dwStyle == LVS_REPORT);
				break;

			case ID_VIEW_SMALLICON:
				bChecked = (dwStyle == LVS_SMALLICON);
				break;

			case ID_VIEW_LARGEICON:
				bChecked = (dwStyle == LVS_ICON);
				break;

			case ID_VIEW_LIST:
				bChecked = (dwStyle == LVS_LIST);
				break;

			default:
				bChecked = FALSE;
				break;
			}
			pCmdUI->SetRadio(bChecked ? 1 : 0);
		}
	}
}

void CMainFrm::OnViewStyle(UINT nCommandID)
{
	// TODO: 이 코드를 사용자 지정하거나 확장하여 보기 메뉴에서의 선택을 처리합니다.
	CPdbListView* pView = GetRightPane();

	if (pView != NULL)
	{
		DWORD dwStyle = -1;

		switch (nCommandID)
		{
		case ID_VIEW_LINEUP:
			{
				// list 컨트롤을 모눈에 맞춥니다.
				CListCtrl& refListCtrl = pView->GetListCtrl();
				refListCtrl.Arrange(LVA_SNAPTOGRID);
			}
			break;

		// 다른 명령에서 list 컨트롤의 스타일을 변경합니다.
		case ID_VIEW_DETAILS:
			dwStyle = LVS_REPORT;
			break;

		case ID_VIEW_SMALLICON:
			dwStyle = LVS_SMALLICON;
			break;

		case ID_VIEW_LARGEICON:
			dwStyle = LVS_ICON;
			break;

		case ID_VIEW_LIST:
			dwStyle = LVS_LIST;
			break;
		}

		// 스타일을 변경합니다. 창이 자동으로 다시 그려집니다.
		if (dwStyle != -1)
			pView->ModifyStyle(LVS_TYPEMASK, dwStyle);
	}
}

