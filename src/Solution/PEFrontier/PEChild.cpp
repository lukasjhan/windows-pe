#include "stdafx.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEChild.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CPEChild, CMDIChildWndEx)
BEGIN_MESSAGE_MAP(CPEChild, CMDIChildWndEx)
END_MESSAGE_MAP()


CPEChild::CPEChild()
{
}

CPEChild::~CPEChild()
{
}

#ifdef _DEBUG
void CPEChild::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CPEChild::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

void CPEChild::OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt)
{
	CMDIFrameWndEx* pParent = (CMDIFrameWndEx*)GetParentFrame();
	if (bActive || pParent->GetMDITabGroups().GetCount() > 0)
		CMDIChildWndEx::OnUpdateFrameMenu(bActive, pActiveWnd, hMenuAlt);
}

void CPEChild::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CMDIChildWndEx::OnUpdateFrameTitle(bAddToTitle);
}
