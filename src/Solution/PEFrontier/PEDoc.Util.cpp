#include "stdafx.h"
#include "PEApp.h"
#include "PEDoc.h"
#include "PEFrame.h"
#include <TlHelp32.h>

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


PBYTE CPEDoc::AllocBuff(bool bFromPE, HANDLE hRef, PBYTE pRefAddr, DWORD dwReadSize)
{
	PBYTE pBuff = NULL;
	if (bFromPE)
	{
		pBuff = (PBYTE)MapViewOfFile(hRef, FILE_MAP_READ, 0, (DWORD)pRefAddr, dwReadSize);
		if (pBuff == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		pBuff = (PBYTE)VirtualAlloc(NULL, dwReadSize, MEM_COMMIT, PAGE_READWRITE);
		if (pBuff == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());
		ReadProcessMemory(hRef, pRefAddr, pBuff, dwReadSize, NULL);
	}
	return pBuff;
}

void CPEDoc::FreeBuff(bool bFromPE, PBYTE pBuffAddr)
{
	if (bFromPE)
		UnmapViewOfFile(pBuffAddr);
	else
		VirtualFree(pBuffAddr, 0, MEM_RELEASE);
}

CString CPEDoc::CreateNewProject(PCTSTR pszPrjName, PCTSTR pszPrjPath, bool bIsProc, 
								 PCTSTR pszExePath, PCTSTR pszExeArgs)
{
	CString szXml = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\xd\xa";
	szXml.AppendFormat(L"<Project name=\"%s\" type=\"%s\" target=\"%s\"",
		pszPrjName, (bIsProc) ? L"process" : L"pefile", pszExePath);
	if (pszExeArgs != NULL && wcslen(pszExeArgs) > 0)
		szXml.AppendFormat(L" args=\"%s\"", pszExeArgs);
	szXml.Replace(L"&", L"&amp;");
	szXml.Append(L" />\xd\xa");

	XmlDocument pIDoc(__uuidof(MSXML2::DOMDocument));
	VARIANT_BOOL bIsOK = pIDoc->loadXML((PCWSTR)szXml);
	if (bIsOK == VARIANT_FALSE)
	{
		XmlParseError pIErr = pIDoc->GetparseError();
		throw (HRESULT)pIErr->errorCode;
	}
	CString szPrjPath = pszPrjPath;
	szPrjPath.AppendFormat(L"\\%s.dbgprj", pszPrjName);
	pIDoc->save((PCWSTR)szPrjPath);
	return szPrjPath;
}

void CPEDoc::SaveDebugProject(PCTSTR pszPrjPath)
{
	XmlDocument pIDoc(__uuidof(MSXML2::DOMDocument));
	XmlElement pIRoot = pIDoc->createElement(L"Project");
	variant_t vt = m_szPrjName;
	pIRoot->setAttribute(L"name", vt);
	vt = (m_bIsProc) ? L"process" : L"pefile";
	pIRoot->setAttribute(L"type", vt);
	vt = m_dp.ModulePath + L"\\" + m_dp.ModuleName;
	pIRoot->setAttribute(L"target", vt);
	if (!m_szExeArgs.IsEmpty())
	{
		vt = m_szExeArgs;
		pIRoot->setAttribute(L"args", vt);
	}
	if (m_bBrkEntPos)
	{
		vt = L"true";
		pIRoot->setAttribute(L"breakEntryPoint", vt);
	}
	if (m_bBrkRtlInit)
	{
		vt = L"true";
		pIRoot->setAttribute(L"breakRtlStartup", vt);
	}
	pIDoc->appendChild(pIRoot);

	CString sz;
	XmlElement pIList = NULL;
	if (m_mapBrks.size() > 0)
	{
		pIList = pIDoc->createElement(L"BreakPointList");
		for (DBG_BREAK_MAP::iterator it = m_mapBrks.begin(); it != m_mapBrks.end(); it++)
		{
			PDBG_BRKPNT pdb = it->second;
			if (pdb->IsTemp)
				continue;
			XmlElement pIItem = pIDoc->createElement(L"Item");

			sz.Format(L"0x%08X", pdb->HaltRVA);
			vt = sz;
			pIItem->setAttribute(L"HaltRva", vt);

			if (!pdb->Active)
			{
				vt = L"false";
				pIItem->setAttribute(L"Active", vt);
			}

			if (!pdb->Label.IsEmpty())
			{
				vt = pdb->Label;
				pIItem->setAttribute(L"Label", vt);
			}

			if (!pdb->HaltIf.IsEmpty())
			{
				vt = pdb->HaltIf;
				pIItem->setAttribute(L"HaltIf", vt);
			}
			vt = pdb->Module->ModulePath + L"\\" + pdb->Module->ModuleName;
			pIItem->setAttribute(L"Module", vt);

			pIList->appendChild(pIItem);
		}
		pIRoot->appendChild(pIList);
	}
	if (m_mapNlBrks.size() > 0)
	{
		if (pIList == NULL)
			pIList = pIDoc->createElement(L"BreakPointList");
		for (DBG_NLBRK_MAP::iterator it = m_mapNlBrks.begin(); it != m_mapNlBrks.end(); it++)
		{
			PDBG_MODBRK_MAP pmbm = it->second;
			for (DBG_MODBRK_MAP::iterator im = pmbm->begin(); im != pmbm->end(); im++)
			{
				PDBG_BRKPNT pdb = im->second;
				XmlElement pIItem = pIDoc->createElement(L"Item");

				sz.Format(L"0x%08X", pdb->HaltRVA);
				vt = sz;
				pIItem->setAttribute(L"HaltRva", vt);

				if (!pdb->Active)
				{
					vt = L"false";
					pIItem->setAttribute(L"Active", vt);
				}
				if (!pdb->Label.IsEmpty())
				{
					vt = pdb->Label;
					pIItem->setAttribute(L"Label", vt);
				}
				if (!pdb->HaltIf.IsEmpty())
				{
					vt = pdb->HaltIf;
					pIItem->setAttribute(L"HaltIf", vt);
				}
				vt = pdb->ModPath;
				pIItem->setAttribute(L"Module", vt);

				pIList->appendChild(pIItem);
			}
		}
		pIRoot->appendChild(pIList);
	}
	pIList = 0;

	if (m_mapMarks.size() > 0)
	{
		pIList = pIDoc->createElement(L"BookMarkList");
		for (DBG_BKMARK_MAP::iterator it = m_mapMarks.begin(); it != m_mapMarks.end(); it++)
		{
			PDBG_BKMARK pbm = it->second;
			XmlElement pIItem = pIDoc->createElement(L"Item");

			//MarkRVA
			sz.Format(L"0x%08X", pbm->MarkRVA);
			vt = sz;
			pIItem->setAttribute(L"MarkRVA", vt);
			//Label
			if (!pbm->Label.IsEmpty())
			{
				vt = pbm->Label;
				pIItem->setAttribute(L"Label", vt);
			}
			// Description
			if (!pbm->Desc.IsEmpty())
			{
				XmlCDataSection pICData = pIDoc->createCDATASection((PCWSTR)pbm->Desc);
				pIItem->appendChild(pICData);
			}
			//Module
			vt = pbm->Module->ModulePath + L"\\" + pbm->Module->ModuleName;
			pIItem->setAttribute(L"Module", vt);

			pIList->appendChild(pIItem);
		}
		pIRoot->appendChild(pIList);
	}
	if (m_mapNlBms.size() > 0)
	{
		if (pIList == NULL)
			pIList = pIDoc->createElement(L"BookMarkList");
		for (DBG_NLBM_MAP::iterator it = m_mapNlBms.begin(); it != m_mapNlBms.end(); it++)
		{
			PDBG_MODBM_MAP pmbm = it->second;
			for (DBG_MODBM_MAP::iterator im = pmbm->begin(); im != pmbm->end(); im++)
			{
				PDBG_BKMARK pbm = im->second;
				XmlElement pIItem = pIDoc->createElement(L"Item");

				//MarkRVA
				sz.Format(L"0x%08X", pbm->MarkRVA);
				vt = sz;
				pIItem->setAttribute(L"MarkRVA", vt);
				//Label
				if (!pbm->Label.IsEmpty())
				{
					vt = pbm->Label;
					pIItem->setAttribute(L"Label", vt);
				}
				// Description
				if (!pbm->Desc.IsEmpty())
				{
					XmlCDataSection pICData = pIDoc->createCDATASection((PCWSTR)pbm->Desc);
					pIItem->appendChild(pICData);
				}
				//Module
				vt = pbm->ModPath;
				pIItem->setAttribute(L"Module", vt);

				pIList->appendChild(pIItem);
			}
		}
		pIRoot->appendChild(pIList);
	}

	HRESULT hr = pIDoc->save(pszPrjPath);
	if (FAILED(hr))
		throw hr;
}

void CPEDoc::LoadDebugProject(PCTSTR pszPrjPath)
{
	XmlDocument pIDoc(__uuidof(MSXML2::DOMDocument));
	VARIANT_BOOL bIsOK = pIDoc->load(pszPrjPath);
	if (bIsOK == VARIANT_FALSE)
	{
		XmlParseError pIErr = pIDoc->GetparseError();
		throw (HRESULT)pIErr->errorCode;
	}
	XmlElement pIRoot = pIDoc->documentElement;
	variant_t vt = pIRoot->getAttribute(L"name");
	if (vt.vt != VT_BSTR)
		throw _T("Parent \"name\" not found.");
	m_szPrjName = vt.bstrVal;

	vt = pIRoot->getAttribute(L"type");
	if (vt.vt == VT_BSTR)
		m_bIsProc = (_tcsicmp(vt.bstrVal, L"process") == 0);
	vt = pIRoot->getAttribute(L"target");
	if (vt.vt != VT_BSTR)
		throw _T("Parent \"target\" not found.");
	CString szFullName = vt.bstrVal;
	int nPos = szFullName.ReverseFind(L'\\');
	if (nPos > 0)
	{
		m_dp.ModuleName = szFullName.Mid(nPos + 1);
		m_dp.ModulePath = szFullName.Left(nPos);
	}
	else
		m_dp.ModuleName = szFullName;
	vt = pIRoot->getAttribute(L"args");
	if (vt.vt == VT_BSTR)
		m_szExeArgs = vt.bstrVal;

	vt = pIRoot->getAttribute(L"breakEntryPoint");
	if (vt.vt == VT_BSTR)
		m_bBrkEntPos = (_wcsicmp(vt.bstrVal, L"true") == 0);
	else
		m_bBrkEntPos = false;
	vt = pIRoot->getAttribute(L"breakRtlStartup");
	if (vt.vt == VT_BSTR)
		m_bBrkEntPos = (_wcsicmp(vt.bstrVal, L"true") == 0);
	else
		m_bBrkEntPos = false;

	XmlNodeList pIList = pIRoot->selectNodes(L"BreakPointList/Item");
	while (XmlElement pIItem = pIList->nextNode())
	{
		vt = pIItem->getAttribute(L"HaltRva");
		if (vt.vt != VT_BSTR)
			throw _T("Parent \"HaltRva\" not found.");
		DWORD dwHaltRva = 0;
		if (wcsncmp(vt.bstrVal, L"0x", 2) == 0)
		{
			PTSTR pszEndPos = NULL;
			dwHaltRva = (DWORD)_tcstoul(vt.bstrVal, &pszEndPos, 16);
		}
		else
			dwHaltRva = (DWORD)_ttoi(vt.bstrVal);

		//Active
		bool bActive = true;
		vt = pIItem->getAttribute(L"Active");
		if (vt.vt != VT_BSTR)
			bActive = (_wcsicmp(vt.bstrVal, L"true") == 0);

		vt = pIItem->getAttribute(L"Module");
		if (vt.vt == VT_BSTR)
			throw _T("Parent \"Module\" not found.");
		CString szModPath = vt.bstrVal;

		CString szLabel;
		vt = pIItem->getAttribute(L"Label");
		if (vt.vt == VT_BSTR)
			szLabel = vt.bstrVal;

		PDBG_MODBRK_MAP pmbm = NULL;
		DBG_NLBRK_MAP::iterator it = m_mapNlBrks.find(szModPath);
		if (it == m_mapNlBrks.end())
		{
			pmbm = new DBG_MODBRK_MAP();
			m_mapNlBrks.insert(std::make_pair(szModPath, pmbm));
		}
		else
			pmbm = it->second;

		PDBG_BRKPNT pdb = new DBG_BRKPNT(dwHaltRva, szModPath, 
			szModPath.GetLength(), szLabel.GetLength() == 0? NULL : szLabel);
		pdb->Active = bActive;
		pmbm->insert(std::make_pair(dwHaltRva, pdb));
	}

	pIList = pIRoot->selectNodes(L"BookMarkList/Item");
	while (XmlElement pIItem = pIList->nextNode())
	{
		vt = pIItem->getAttribute(L"MarkRVA");
		if (vt.vt != VT_BSTR)
			throw _T("Parent \"MarkRVA\" not found.");
		DWORD dwMarkRva = 0;
		if (wcsncmp(vt.bstrVal, L"0x", 2) == 0)
		{
			PTSTR pszEndPos = NULL;
			dwMarkRva = (DWORD)_tcstoul(vt.bstrVal, &pszEndPos, 16);
		}
		else
			dwMarkRva = (DWORD)_ttoi(vt.bstrVal);

		vt = pIItem->getAttribute(L"Module");
		if (vt.vt != VT_BSTR)
			throw _T("Parent \"Module\" not found.");
		CString szModPath = vt.bstrVal;

		CString szLabel;
		vt = pIItem->getAttribute(L"Label");
		if (vt.vt == VT_BSTR)
			szLabel = vt.bstrVal;

		CString szDesc;
		XmlCDataSection pICData = (XmlCDataSection)pIItem->GetfirstChild();
		if (pICData != NULL)
		{
			vt.bstrVal = pICData->Gettext();
			szDesc = vt.bstrVal;
		}

		PDBG_MODBM_MAP pmbm = NULL;
		DBG_NLBM_MAP::iterator it = m_mapNlBms.find(szModPath);
		if (it == m_mapNlBms.end())
		{
			pmbm = new DBG_MODBM_MAP();
			m_mapNlBms.insert(std::make_pair(szModPath, pmbm));
		}
		else
			pmbm = it->second;

		PDBG_BKMARK pdb = new DBG_BKMARK(dwMarkRva, szModPath,
			szModPath.GetLength(), szLabel.GetLength() == 0 ? NULL : szLabel);
		if (!szDesc.IsEmpty())
			pdb->Desc = szDesc;
		pmbm->insert(std::make_pair(dwMarkRva, pdb));
	}
}

void CPEDoc::SetBreakPoint(int nCodeIdx)
{
	PostThreadMessage(m_dwDbgThId, TM_BREAK_EVENT, 1, nCodeIdx);
	WaitForSingleObject(m_hevAck, INFINITE);
}


PDBG_MODULE CPEDoc::FindModule(PBYTE pAddr)
{
	PDBG_MODULE pdm = NULL;
	for (DBG_MODULE_MAP::iterator it = m_mapMods.begin(); it != m_mapMods.end(); it++)
	{
		PBYTE uBegin = it->first;
		if (pAddr >= uBegin && pAddr < uBegin + it->second->ImageSize)
		{
			pdm = it->second;
			break;
		}
	}
	return pdm;
}

PDBG_DASM CPEDoc::FindDAsm(PDBG_MODULE pdm, DWORD dwCodeRva)
{
	PDBG_CBASE pdc = FindCodes(pdm, dwCodeRva);
	if (pdc == NULL || pdc->CodeType == CT_NONE)
		return NULL;

	if (pdc->CodeType == CT_THUNK)
		return ((PDBG_THUNK)pdc)->AsmCode;

	PDBG_DUMMY pdd = (PDBG_DUMMY)pdc;
	if (pdd->AsmCodes == NULL)
	{

	}
	return FindDAsm(pdd, dwCodeRva);
}

PDBG_DASM CPEDoc::FindDAsm(PDBG_DUMMY pdd, DWORD dwCodeRva)
{
	int low  = 0;
	int high = pdd->CodeCount - 1;

	while (low <= high)
	{
		int i = (low + high) >> 1;
		PDBG_DASM pda = pdd->AsmCodes[i];
		if (dwCodeRva >= pda->Code._offset && dwCodeRva < pda->Code._offset + pda->Code._count)
			return pda;

		if (dwCodeRva < pda->Code._offset)
			high = i - 1;
		else
			low  = i + 1;
	}
	return NULL;
}

PDBG_CBASE CPEDoc::FindCodes(PDBG_MODULE pdm, DWORD dwCodeRva)
{
	int low = 0;
	int high = pdm->CBaseCnt - 1;

	while (low <= high)
	{
		int i = (low + high) >> 1;
		PDBG_CBASE pdc = pdm->CBaseList[i];
		if (dwCodeRva >= pdc->BeginRVA && dwCodeRva < pdc->BeginRVA + pdc->CodeSize)
			return pdc;

		if (dwCodeRva < pdc->BeginRVA)
			high = i - 1;
		else
			low = i + 1;
	}
	return NULL;
}


void CPEDoc::SuspendDebuggeeProcess()
{
	for (DBG_THREAD_MAP::iterator it = m_mapThrs.begin(); it != m_mapThrs.end(); it++)
	{
#ifdef _X86_
		SuspendThread(it->second->Thread);
#else
		if (m_dp.Is32Bit)
			Wow64SuspendThread(it->second->Thread);
		else
			SuspendThread(it->second->Thread);
#endif
	}
	m_dp.Active = FALSE;
}

void CPEDoc::ResumeDebuggeeProcess()
{
	for (DBG_THREAD_MAP::iterator it = m_mapThrs.begin(); it != m_mapThrs.end(); it++)
		ResumeThread(it->second->Thread);
	m_dp.Active = TRUE;
}

HRESULT CPEDoc::GetProcHeapList(DBG_PROCESS* pdp, DBG_HEAP_MAP& mapHeap)
{
	HANDLE hsnHeap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, pdp->ProcessId);
	if (hsnHeap == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());

	HEAPLIST32 hl;
	hl.dwSize = sizeof(HEAPLIST32);
	if (Heap32ListFirst(hsnHeap, &hl))
	{
		do
		{
			PDBG_HEAP pdh = new DBG_HEAP();
			pdh->HeapID = hl.th32HeapID;
			pdh->Flags = hl.dwFlags;
			mapHeap.insert(std::make_pair(hl.th32HeapID, pdh));
			hl.dwSize = sizeof(HEAPLIST32);
		} while (Heap32ListNext(hsnHeap, &hl));
	}
	CloseHandle(hsnHeap);
	return S_OK;
}

void CPEDoc::UpdateThreadContext(HANDLE hThread, bool bIs32Bit, bool bInTrap, bool bIpBack)
{
	BOOL bIsOK = TRUE;
#ifdef _X86_
	CONTEXT ct;
	ct.ContextFlags = CONTEXT_CONTROL;
	bIsOK = GetThreadContext(hThread, &ct);
	if (bIpBack)
	{
		ct.Eip--;
		ct.EFlags |= TF_BIT;
	}
	if (bInTrap)
		ct.EFlags |= TF_BIT;
	SetThreadContext(hThread, &ct);
#else
	if (bIs32Bit)
	{
		WOW64_CONTEXT ct;
		ct.ContextFlags = CONTEXT_CONTROL;
		bIsOK = Wow64GetThreadContext(hThread, &ct);
		if (bIpBack)
		{
			ct.Eip--;
			ct.EFlags |= TF_BIT;
		}
		if (bInTrap)
			ct.EFlags |= TF_BIT;
		Wow64SetThreadContext(hThread, &ct);
	}
	else
	{
		CONTEXT ct;
		ct.ContextFlags = CONTEXT_CONTROL;
		bIsOK = GetThreadContext(hThread, &ct);
		if (bIpBack)
		{
			ct.Rip--;
			ct.EFlags |= TF_BIT;
		}
		if (bInTrap)
			ct.EFlags |= TF_BIT;
		SetThreadContext(hThread, &ct);
	}
#endif
}

BYTE CPEDoc::UpdateBreakPoint(int nOpt, PBYTE pBrkAddr, BYTE btCode)
{
	BYTE opCode = 0;
	if (nOpt == DBG_BP_CREATE)
	{
		ReadProcessMemory(m_dp.Process, pBrkAddr, &opCode, 1, NULL);
	}
	WriteProcessMemory(m_dp.Process, pBrkAddr, &btCode, 1, NULL);
	FlushInstructionCache(m_dp.Process, pBrkAddr, 1);
	return opCode;
}

HRESULT CPEDoc::WalkProcess(DBG_HEAP_MAP& heaps, DBG_VMOBJ_MAP& vmos)
{
	GetProcHeapList(&m_dp, heaps);

	DWORD dwPageSize = PAGE_SIZE;
	PVOID pMaxAppVA = (PVOID)WOW64_VA_MAX;
	if (!m_dp.Is32Bit)	//TODO:
	{
		SYSTEM_INFO	si;
		GetSystemInfo(&si);
		dwPageSize = si.dwPageSize;
		pMaxAppVA  = si.lpMaximumApplicationAddress;
	}

	PBYTE pMemVa = NULL;
	while (pMemVa < pMaxAppVA)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQueryEx(m_dp.Process, pMemVa, &mbi, sizeof(mbi));

		PDBG_VMOBJ pdv = new DBG_VMOBJ();
		pdv->Mbi = mbi; pdv->IsNew = FALSE;
		vmos.insert(std::make_pair(mbi.BaseAddress, pdv));

		pMemVa = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
	}

	for (DBG_HEAP_MAP::iterator it = heaps.begin(); it != heaps.end(); it++)
	{
		PDBG_HEAP pdh = it->second;
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQueryEx(m_dp.Process, (PVOID)pdh->HeapID, &mbi, sizeof(mbi));
		if (mbi.AllocationBase == 0)
			continue;

		DBG_VMOBJ_MAP::iterator it2 = vmos.find(mbi.BaseAddress);
		if (it2 != vmos.end())
		{
			for (; it2 != vmos.end(); it2++)
			{
				PDBG_VMOBJ pvm = it2->second;
				if (pvm->Mbi.AllocationBase != mbi.AllocationBase)
					break;

				it2->second->ObjType = PAGE_TYPE::PT_HEAP;
				if (pdh->Flags & HF32_DEFAULT)
					pvm->Section = L"디폴트 힙";
			}
		}
	}

	PBYTE pRsp = NULL;
	for (DBG_THREAD_MAP::iterator it = m_mapThrs.begin(); it != m_mapThrs.end(); it++)
	{
		PDBG_THREAD pdt = it->second;
#ifdef _X86_
		CONTEXT cc;
		cc.ContextFlags = CONTEXT_CONTROL;
		if (!GetThreadContext(pdt->Thread, &cc))
			continue;
		pRsp = (PBYTE)cc.Esp;
#else /* _AMD64 */
		if (m_dp.Is32Bit)
		{
			WOW64_CONTEXT cc;
			cc.ContextFlags = CONTEXT_CONTROL;
			if (!Wow64GetThreadContext(pdt->Thread, &cc))
				continue;
			pRsp = (PBYTE)cc.Esp;
		}
		else
		{
			CONTEXT cc;
			cc.ContextFlags = CONTEXT_CONTROL;
			if (!GetThreadContext(pdt->Thread, &cc))
				continue;
			pRsp = (PBYTE)cc.Rsp;
		}
#endif

		MEMORY_BASIC_INFORMATION mbi;
		VirtualQueryEx(m_dp.Process, (PVOID)pRsp, &mbi, sizeof(mbi));
		if (mbi.AllocationBase == 0)
			continue;

		DBG_VMOBJ_MAP::iterator it2 = vmos.find(mbi.AllocationBase);
		if (it2 != vmos.end())
		{
			for (; it2 != vmos.end(); it2++)
			{
				PDBG_VMOBJ pvm = it2->second;
				if (pvm->Mbi.AllocationBase != mbi.AllocationBase)
					break;

				it2->second->ObjType = PAGE_TYPE::PT_STACK;
				it2->second->Module.Format(L"스레드(%d)", pdt->ThreadId);
				if (pvm->Mbi.Protect & PAGE_GUARD)
					it2->second->Section = L"가드스택";
			}
		}
	}

	for (DBG_MODULE_MAP::iterator it = m_mapMods.begin(); it != m_mapMods.end(); it++)
	{
		PDBG_MODULE pdm = it->second;
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQueryEx(m_dp.Process, (PVOID)pdm->ImageBase, &mbi, sizeof(mbi));
		if (mbi.AllocationBase == 0)
			continue;

		BYTE arBuff[PAGE_SIZE];
		ReadProcessMemory(m_dp.Process, pdm->ImageBase, arBuff, PAGE_SIZE, NULL);

		DBG_VMOBJ_MAP::iterator it2 = vmos.find(mbi.BaseAddress);
		if (it2 != vmos.end())
		{
			PDBG_VMOBJ pvm = it2->second;
			pvm->ObjType = (pdm->IsExe) ? PAGE_TYPE::PT_EXE : PAGE_TYPE::PT_DLL;
			pvm->Module = pdm->ModuleName;
			pvm->Section = L"PE 헤더";

			DWORD dwModSize = (DWORD)pvm->Mbi.RegionSize;
			it2++;
			for (; it2 != vmos.end(); it2++)
			{
				pvm = it2->second;
				DWORD dwOffset = (DWORD)((SIZE_T)pvm->Mbi.BaseAddress - (SIZE_T)pdm->ImageBase);
				if (dwOffset == 0)
					continue;

				pvm->ObjType = (pdm->IsExe) ? PAGE_TYPE::PT_EXE : PAGE_TYPE::PT_DLL;
				pvm->Module = pdm->ModuleName;
				PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(arBuff, dwOffset);
				if (psh != NULL)
					pvm->Section = PEPlus::GetSectionName(psh);
				dwModSize += (DWORD)pvm->Mbi.RegionSize;
				if (dwModSize >= pdm->ImageSize)
					break;
			}
		}
	}
	return S_OK;
}

PBYTE CPEDoc::GetStackBase(HANDLE hThread, HANDLE hProcess, bool bWowProc)
{
	PBYTE pRsp = NULL;
#ifdef _X86_
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(hThread, &ctx);
	pRsp = (PBYTE)ctx.Esp;
#else
	if (bWowProc)
	{
		WOW64_CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_CONTROL;
		Wow64GetThreadContext(hThread, &ctx);
		pRsp = (PBYTE)ctx.Esp;
	}
	else
	{
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_CONTROL;
		GetThreadContext(hThread, &ctx);
		pRsp = (PBYTE)ctx.Rsp;
	}
#endif

	MEMORY_BASIC_INFORMATION mbi;
	PBYTE pStkAddr = pRsp;
	VirtualQueryEx(m_dp.Process, pStkAddr, &mbi, sizeof(mbi));
	PBYTE pStkMax = (PBYTE)mbi.AllocationBase;
	for (;;)
	{
		pStkAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
		VirtualQueryEx(m_dp.Process, pStkAddr, &mbi, sizeof(mbi));
		if (mbi.AllocationBase == 0 || pStkMax != mbi.AllocationBase)
			break;
	}
	return pStkAddr;
}

PDBG_THREAD CPEDoc::LoadThreadInfo(DWORD dwThreadId, PDBG_PROCESS pdp, CREATE_THREAD_DEBUG_INFO& di)
{
	PDBG_THREAD pdt = new DBG_THREAD(pdp);
	pdt->StartAddr	= (PBYTE)di.lpStartAddress;
	pdt->Priority	= GetThreadPriority(di.hThread);
	pdt->Thread		= di.hThread;
	pdt->ThreadId	= dwThreadId;
	pdt->StackBase  = GetStackBase(di.hThread, pdp->Process, pdp->Is32Bit);

	PDBG_MODULE pdm = FindModule(pdt->StartAddr);
	if (pdm != NULL && pdm->DiaRef != NULL)
	{
		DWORD dwRVA = (DWORD)(pdt->StartAddr - pdm->ImageBase);
		CComPtr<IDiaSymbol> pISym;
		if (pdm->DiaRef->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISym) == S_OK)
		{
			CComBSTR bszName;
			if(pISym->get_name(&bszName) == S_OK)
				pdt->ThreadName.Format(L"%s!%s", pdm->ModuleName, bszName);
			pISym = 0;
		}
	}
	if (pdt->ThreadName.IsEmpty())
		pdt->ThreadName.Format(L"Thread_#%d", pdt->ThreadId);
	return pdt;
}

void CPEDoc::LoadProcessInfo(DWORD dwThreadId, CREATE_PROCESS_DEBUG_INFO& di)
{
	PDBG_THREAD pth = new DBG_THREAD(&m_dp);
	pth->StartAddr	= (PBYTE)di.lpStartAddress;
	pth->Priority	= GetThreadPriority(di.hThread);
	pth->Thread		= di.hThread;
	pth->IsMain		= 1;
	pth->ThreadId	= dwThreadId;
	pth->StackBase = GetStackBase(di.hThread, m_dp.Process, m_dp.Is32Bit);
	m_mapThrs.insert(std::make_pair(dwThreadId, pth));

	m_dp.ImageFile	= di.hFile;
	m_dp.ImageBase	= (PBYTE)di.lpBaseOfImage;
	m_dp.EntryPoint = (DWORD)(pth->StartAddr - m_dp.ImageBase);
	m_dp.MainThread = pth;
	m_mapMods.insert(std::make_pair(m_dp.ImageBase, &m_dp));

	CString szImgPath = m_dp.ModulePath + L"\\" + m_dp.ModuleName;
	MoveBreakPointFileToLoad(szImgPath, &m_dp);
	MoveBookMarkFileToLoad(szImgPath, &m_dp);

	m_dp.Header = GetPEHeader(m_dp.Process, &m_dp);
	m_dp.DiaRef = LoadPdbRef(m_dp.Header, &m_dp);
	if (m_dp.DiaRef != NULL)
	{
		CComPtr<IDiaSymbol> pISym;
		if (m_dp.DiaRef->SESSION->findSymbolByRVA(m_dp.EntryPoint, SymTagNull, &pISym) == S_OK)
		{
			CComBSTR bszName;
			pISym->get_name(&bszName);
			pth->ThreadName.Format(L"%s!%s", m_dp.ModuleName, bszName);
			pISym = 0;
		}
	}
	if (pth->ThreadName.IsEmpty())
		pth->ThreadName.Format(L"Main_Thread_#%d", pth->ThreadId);
	BuildFunctoin(m_dp.Process, &m_dp);
}

PDBG_MODULE CPEDoc::LoadModuleInfo(PDBG_PROCESS pdp, LOAD_DLL_DEBUG_INFO& di)
{
	PDBG_MODULE pdm = new DBG_MODULE(pdp);
	pdm->ImageFile	= di.hFile;
	pdm->ImageBase	= (PBYTE)di.lpBaseOfDll;
	pdm->Unicode	= di.fUnicode;
	pdm->IsExe		= 0;

	CString szFullName = GetImageName(pdp->Process, di.lpImageName, di.hFile, di.fUnicode);
	int nPos = szFullName.ReverseFind(L'\\');
	if (nPos > 0)
	{
		pdm->ModuleName = szFullName.Mid(nPos + 1);
		pdm->ModulePath = szFullName.Left(nPos);
	}
	else
		pdm->ModuleName = szFullName;

	MoveBreakPointFileToLoad(szFullName, pdm);
	MoveBookMarkFileToLoad(szFullName, pdm);

	pdm->Header = GetPEHeader(pdp->Process, pdm);

	HANDLE hMapFile = CreateFileMapping(di.hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapFile != NULL)
	{
		PBYTE pMapBase = (PBYTE)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
		if (pMapBase != NULL)
		{
			pdm->DiaRef = LoadPdbRef(pMapBase);
			if (m_bBrkRtlInit && pdm->ModuleName.CompareNoCase(L"ntdll.dll") == 0)
			{
				BOOL bIsWow64 = FALSE;
				IsWow64Process(m_dp.Process, &bIsWow64);
				if (bIsWow64)
				{
					if(pdm->Is32Bit)
						m_pfnUsrThrSt = GetRtlThreadStartFromNTDll(pMapBase, pdm->ImageBase);
				}
				else
					m_pfnUsrThrSt = GetRtlThreadStartFromNTDll(pMapBase, pdm->ImageBase);
			}
			UnmapViewOfFile(pMapBase);
		}
		CloseHandle(hMapFile);
	}
	return pdm;
}

PEPdb* CPEDoc::LoadPdbRef(PBYTE pImgBase, DBG_PROCESS* pdp)
{
	String	szPdbPath;
	DWORD	dwTimeStamp = 0;
	PBYTE	pDbgBuff = NULL;
	BYTE	arBuff[1024];
	PIMAGE_PDB_INFO ppi = NULL;

	if (pdp == NULL)
		ppi = PEPlus::GetPdbInfo(pImgBase, &dwTimeStamp);
	else
	{
		PIMAGE_DATA_DIRECTORY pdd = PEPlus::GetDataDir(pImgBase, IMAGE_DIRECTORY_ENTRY_DEBUG);
		if (pdd == NULL)
			return NULL;

		PBYTE pDbgBuff = new BYTE[pdd->Size];
		SIZE_T dwReadBytes = 0;
		ReadProcessMemory(pdp->Process, (PBYTE)pdp->ImageBase +
			pdd->VirtualAddress, pDbgBuff, pdd->Size, &dwReadBytes);

		PIMAGE_DEBUG_DIRECTORY pdbgs = PIMAGE_DEBUG_DIRECTORY(pDbgBuff);
		int nItemCnt = pdd->Size / sizeof(IMAGE_DEBUG_DIRECTORY);
		int i = 0;
		for (; i < nItemCnt; i++)
		{
			PIMAGE_DEBUG_DIRECTORY pdbg = &pdbgs[i];
			if (pdbg->Type == IMAGE_DEBUG_TYPE_CODEVIEW)
			{
				ReadProcessMemory(pdp->Process, (PBYTE)pdp->ImageBase +
					pdbg->AddressOfRawData, arBuff, sizeof(arBuff), &dwReadBytes);
				dwTimeStamp = pdbg->TimeDateStamp;
				ppi = PIMAGE_PDB_INFO(arBuff);
				break;
			}
		}
	}
	if (ppi == NULL)
		return NULL;

	PEPdb* pdia = NULL;
	try
	{
		pdia = new PEPdb(ppi, dwTimeStamp, ((CPEApp*)AfxGetApp())->GetSymbolPath());
	}
	catch (HRESULT)
	{
	}
	if (pDbgBuff)
		delete[] pDbgBuff;
	return pdia;
}

void CPEDoc::GetCodeSections(HANDLE hProcess, PDBG_MODULE pdm)
{
	std::vector<BYTE> css;
	PIMAGE_FILE_HEADER pfh = PEPlus::GetFileHdr(pdm->Header);
	PIMAGE_SECTION_HEADER pshs = PEPlus::GetSectHdrs(pdm->Header);
	for (int i = 0; i < pfh->NumberOfSections; i++)
	{
		if (pshs[i].Characteristics & IMAGE_SCN_CNT_CODE)
			css.push_back((BYTE)i);
	}
	pdm->CSecCnt = (DWORD)css.size();
	if (pdm->CSecCnt == 0)
		throw HRESULT_FROM_WIN32(ERROR_INVALID_BLOCK);

	pdm->CodeSecs = new DBG_CODESET[pdm->CSecCnt];
	for (int i = 0; i < (int)pdm->CSecCnt; i++)
	{
		PIMAGE_SECTION_HEADER psh = &pshs[css.at(i)];
		pdm->CodeSecs[i].CodeHdr = psh;
		pdm->CodeSecs[i].CodeBase = (PBYTE)
			VirtualAlloc(NULL, psh->Misc.VirtualSize, MEM_COMMIT, PAGE_READWRITE);
		ReadProcessMemory(hProcess, pdm->ImageBase + psh->VirtualAddress,
			pdm->CodeSecs[i].CodeBase, psh->Misc.VirtualSize, NULL);
	}
}

CString CPEDoc::GetImageName(HANDLE hProc, PVOID pImgName, HANDLE hFile, BOOL bUniCode)
{
	CString szImgName;
	WCHAR szNameBuff[MAX_PATH] = { 0, };

	if (pImgName != NULL)
	{
		PBYTE pAddr = NULL;
		ReadProcessMemory(hProc, pImgName, &pAddr, sizeof(PBYTE), NULL);
		if (pAddr != NULL)
		{
			int nSize = MAX_PATH * sizeof(WCHAR);
			ReadProcessMemory(hProc, pAddr, szNameBuff, nSize, NULL);
			if (bUniCode)
				return szNameBuff;
			USES_CONVERSION;
			return A2CW((PCSTR)szNameBuff);
		}
	}

	DWORD dwLen = GetFinalPathNameByHandle(hFile, szNameBuff, MAX_PATH, VOLUME_NAME_DOS);
	if (dwLen > 0)
	{
		PWSTR pPos = wcschr(szNameBuff, L'?');
		if (pPos != NULL)
		{
			if (*(pPos + 1) == L'\\')
				pPos += 2;
		}
		return pPos;
	}
	return PEPlus::RetrieveModuleName(hFile);
}

void CPEDoc::MoveBreakPointFileToLoad(PCWSTR pszImgPath, PDBG_MODULE pdm)
{
	DBG_NLBRK_MAP::iterator it = m_mapNlBrks.find(pszImgPath);
	if (it != m_mapNlBrks.end())
	{
		for (DBG_MODBRK_MAP::iterator ib = it->second->begin(); ib != it->second->end(); ib++)
		{
			PDBG_BRKPNT pdb = ib->second;
			delete[] pdb->ModPath;
			pdb->Module = pdm;
			pdb->Loaded = 1;

			PBYTE pTrgAddr = pdm->ImageBase + pdb->HaltRVA;
			pdb->OrgCode = UpdateBreakPoint(DBG_BP_CREATE, pTrgAddr, OP_INT3);
			m_mapBrks.insert(std::make_pair(pTrgAddr, pdb));
		}
		delete it->second;
		m_mapNlBrks.erase(it);
	}
}

void CPEDoc::MoveBookMarkFileToLoad(PCWSTR pszImgPath, PDBG_MODULE pdm)
{
	DBG_NLBM_MAP::iterator it = m_mapNlBms.find(pszImgPath);
	if (it != m_mapNlBms.end())
	{
		PDBG_MODBM_MAP pmbm = it->second;
		for (DBG_MODBM_MAP::iterator ib = pmbm->begin(); ib != pmbm->end(); ib++)
		{
			PDBG_BKMARK pdb = ib->second;
			delete[] pdb->ModPath;
			pdb->Module = pdm;
			pdb->Loaded = true;
			m_mapMarks.insert(std::make_pair(pdb->MarkAddr(), pdb));
		}
		delete pmbm;
		m_mapNlBms.erase(it);
	}
}

PBYTE CPEDoc::GetRtlThreadStartFromNTDll(PBYTE pMapBase, PBYTE pImgBase)
{
	PIMAGE_DATA_DIRECTORY pdd = PEPlus::GetDataDir(pMapBase, IMAGE_DIRECTORY_ENTRY_EXPORT);
	if (pdd == NULL)
		return NULL;
	PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(pMapBase, pdd->VirtualAddress);
	if (psh == NULL)
		return NULL;

	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	PIMAGE_EXPORT_DIRECTORY ped = (PIMAGE_EXPORT_DIRECTORY)(pMapBase + dwOffset);

	dwOffset = RVA_TO_OFFSET(psh, ped->AddressOfFunctions);
	PDWORD pFuncTbl = (PDWORD)(pMapBase + dwOffset);

	dwOffset = RVA_TO_OFFSET(psh, ped->AddressOfNameOrdinals);
	PWORD pOrdnTbl = (PWORD)(pMapBase + dwOffset);

	dwOffset = RVA_TO_OFFSET(psh, ped->AddressOfNames);
	PDWORD pFuncNameTbl = (PDWORD)(pMapBase + dwOffset);

	DWORD dwWantRVA = 0;
	for (DWORD i = 0; i < ped->NumberOfNames; i++)
	{
		dwOffset = RVA_TO_OFFSET(psh, pFuncNameTbl[i]);
		PCSTR pFuncName = (PCSTR)(pMapBase + dwOffset);
		if (strcmp("RtlUserThreadStart", pFuncName) == 0)
		{
			WORD wOrdinal = pOrdnTbl[i];
			return pImgBase + pFuncTbl[wOrdinal];
		}
	}
	return NULL;
}