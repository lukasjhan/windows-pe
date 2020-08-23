#pragma once

class CPEChild : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CPEChild)

public:
	CPEChild();
	virtual ~CPEChild();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

};
