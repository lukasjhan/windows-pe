#pragma once

class CViewTree : public CTreeCtrl
{
public:
	CViewTree();
	virtual ~CViewTree();

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CViewList
class CViewList : public CListCtrl
{
public:
	CViewList();
	virtual ~CViewList();

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

};
