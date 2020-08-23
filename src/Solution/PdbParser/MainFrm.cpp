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
	ID_SEPARATOR,           // ���� �� ǥ�ñ�
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrm ����/�Ҹ�

CMainFrm::CMainFrm()
{
	// TODO: ���⿡ ��� �ʱ�ȭ �ڵ带 �߰��մϴ�.
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
		TRACE0("���� ǥ������ ������ ���߽��ϴ�.\n");
		return -1;      // ������ ���߽��ϴ�.
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	return 0;
}

BOOL CMainFrm::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	// ���� â�� ����ϴ�.
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

// CMainFrm ����
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


// CMainFrm �޽��� ó����

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

	// TODO: �� �ڵ带 ����� �����ϰų� Ȯ���Ͽ� ���� �޴������� ������ ó���մϴ�.
	CPdbListView* pView = GetRightPane();

	// ������ â�� ��������� �ʾҰų� �䰡 �ƴ� ���
	// �ش� ������ �ִ� ����� ��Ȱ��ȭ�մϴ�.
	if (pView == NULL)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

		// ����� ID_VIEW_LINEUP�̸�
		// LVS_ICON �Ǵ� LVS_SMALLICON ����� ��쿡�� �� ����� Ȱ��ȭ�մϴ�.

		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			// �׷��� ������ ���� ����Ͽ� ���� ��Ÿ���� �ݿ��մϴ�.
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
	// TODO: �� �ڵ带 ����� �����ϰų� Ȯ���Ͽ� ���� �޴������� ������ ó���մϴ�.
	CPdbListView* pView = GetRightPane();

	if (pView != NULL)
	{
		DWORD dwStyle = -1;

		switch (nCommandID)
		{
		case ID_VIEW_LINEUP:
			{
				// list ��Ʈ���� �𴫿� ����ϴ�.
				CListCtrl& refListCtrl = pView->GetListCtrl();
				refListCtrl.Arrange(LVA_SNAPTOGRID);
			}
			break;

		// �ٸ� ��ɿ��� list ��Ʈ���� ��Ÿ���� �����մϴ�.
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

		// ��Ÿ���� �����մϴ�. â�� �ڵ����� �ٽ� �׷����ϴ�.
		if (dwStyle != -1)
			pView->ModifyStyle(LVS_TYPEMASK, dwStyle);
	}
}

