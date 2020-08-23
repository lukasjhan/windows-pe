#include "stdafx.h"
#include "Resource.h"
#include "PEAnals.h"
#include "PEView.h"

PCWSTR PEAnals::GSZ_RES_TYPES[] =
{
	L"NULL",
	L"CURSOR",
	L"BITMAP",
	L"ICON",
	L"MENU",
	L"DIALOG",
	L"STRING",
	L"FONTDIR",
	L"FONT",
	L"ACCELERATOR",
	L"RCDATA",
	L"MESSAGETABLE",
	L"GROUP_CURSOR",
	L"EMPTY13",
	L"GROUP_ICON",
	L"EMPTY15",
	L"VERSION",
	L"DLGINCLUDE",
	L"EMPTY18",
	L"PLUGPLAY",
	L"VXD",
	L"ANICURSOR",
	L"ANIICON",
	L"HTML",
	L"MANIFEST"
};

bool PEAnals::ParseDirEntryResource(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	CString sz; USES_CONVERSION;

	PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index,
		dwOffset, L"Resource", L"IMAGE_RESOURCE_DIRECTORY");
	AppendStructMembers(pn);

	DWORD dwResStart = dwOffset;
	PIMAGE_RESOURCE_DIRECTORY prd = (PIMAGE_RESOURCE_DIRECTORY)(m_pImgView + dwOffset);
	dwOffset += sizeof(IMAGE_RESOURCE_DIRECTORY);
	ParseResDirEntry(dwResStart, prd, dwOffset, pn, 0, L"");

	return false;
}

DWORD PEAnals::ParseResDirEntry(DWORD dwResStart, PIMAGE_RESOURCE_DIRECTORY prd, 
				DWORD dwOffset, PPE_NODE pn, int nDepth, PCTSTR pszResName)
{
	PPE_ENUM peLangs = NULL;
	if (nDepth == 2)
		peLangs = SCHEMA.EnumMap()->find(L"LANGUAGE_ID")->second;

	for (WORD i = 0; i < prd->NumberOfIdEntries + prd->NumberOfNamedEntries; i++)
	{
		CString szName, szRes;
		PPE_NODE pne = InsertStructNode(pn->Node, pn->Index,
			dwOffset, L"", L"IMAGE_RESOURCE_DIRECTORY_ENTRY");
		AppendStructMembers(pne);

		PIMAGE_RESOURCE_DIRECTORY_ENTRY prde =
			(PIMAGE_RESOURCE_DIRECTORY_ENTRY)(m_pImgView + dwOffset);
		if (prde->NameIsString)
		{
			DWORD dwStrOff = dwResStart + prde->NameOffset;
			PIMAGE_RESOURCE_DIR_STRING_U prds = (PIMAGE_RESOURCE_DIR_STRING_U)(m_pImgView + dwStrOff);
			PWSTR pszName = szName.GetBufferSetLength(prds->Length);
			memcpy(pszName, (PBYTE)prds->NameString, prds->Length * sizeof(WCHAR));
			szRes = szName;
		}
		else
		{
			WORD wId = (WORD)prde->Id;
			if (nDepth == 0)
			{
				if (wId <= (WORD)RT_MANIFEST && nDepth == 0)
					szName = szRes = GSZ_RES_TYPES[wId];
				else
					szName.Format(L"RT:%d", wId);
			}
			else if (nDepth == 2)
			{
				szName = szRes = peLangs->Find(wId);
				if (szName.IsEmpty())
					szName.Format(L"LangID: 0x%04X", wId);
			}
			else
				szName.Format(L"ResID: %d", wId);
			if (szRes.IsEmpty())
				szRes.Format(L"%04X", wId);
		}
		PPE_NODE pnd = FindNode(pne->Node, L"NameOrId");
		UpdateNodeText(pnd->Node, szName, COL_IDX_INFO);

		CString szDtn = pszResName;
		if (!szDtn.IsEmpty())
			szDtn.Append(L":");
		szDtn.Append(szRes);
		pnd = FindNode(pne->Node, L"OffsetToData");
		if (prde->DataIsDirectory)
		{
			DWORD dwDirOff = dwResStart + prde->OffsetToDirectory;
			PPE_NODE pnSub = InsertStructNode(pnd->Node, pnd->Index,
				dwDirOff, L"Resource", L"IMAGE_RESOURCE_DIRECTORY");
			AppendStructMembers(pnSub);
			PIMAGE_RESOURCE_DIRECTORY prd2 = (PIMAGE_RESOURCE_DIRECTORY)(m_pImgView + dwDirOff);
			dwDirOff += sizeof(IMAGE_RESOURCE_DIRECTORY);

			ParseResDirEntry(dwResStart, prd2, dwDirOff, pnSub, nDepth + 1, szDtn);
		}
		else
		{
			DWORD dwEntOff = dwResStart + prde->OffsetToData;
			PPE_NODE pnSub = InsertStructNode(pnd->Node, pnd->Index,
				dwEntOff, L"Data", L"IMAGE_RESOURCE_DATA_ENTRY");
			AppendStructMembers(pnSub);
			PIMAGE_RESOURCE_DATA_ENTRY prre = (PIMAGE_RESOURCE_DATA_ENTRY)(m_pImgView + dwEntOff);
			dwEntOff += sizeof(IMAGE_RESOURCE_DATA_ENTRY);

			PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(m_pImgView, prre->OffsetToData);
			if (psh != NULL)
			{
				PPE_NODE pnd2 = FindNode(pnSub->Node, L"OffsetToData");
				DWORD dwResOff = RVA_TO_OFFSET(psh, prre->OffsetToData);
				PPE_NODE pnr = InsertFieldNode(pnd2->Node, pnd2->Index, dwResOff, szDtn, PE_TYPE::UInt8, prre->Size);
				pnr->Kind = IMAGE_DIRECTORY_ENTRY_RESOURCE;
				int nPos = szDtn.Find(L':');
				if (nPos > 0)
				{
					szDtn = szDtn.Left(nPos);
					WORD wRcType = 0;
					for (; wRcType < sizeof(GSZ_RES_TYPES) / sizeof(PCWSTR); wRcType++)
					{
						if (szDtn == GSZ_RES_TYPES[wRcType])
							break;
					}
					if (wRcType > 0 && wRcType < sizeof(GSZ_RES_TYPES) / sizeof(PCWSTR))
						pnr->SubT = (BYTE)wRcType;
				}
			}
		}

		CString sz;
		sz.Format(L"[%d:%d]%s", nDepth + 1, i, szName);
		UpdateNodeText(pne->Node, sz, COL_IDX_NAME);

		dwOffset += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
	}
	return 0;
}

#include "ResrcDlgs\ResViewDlg.h"
#include "ResrcDlgs\ResViewBitmap.h"
#include "ResrcDlgs\ResViewStrTbl.h"
#include "ResrcDlgs\ResViewAccelTbl.h"
#include "ResrcDlgs\ResViewMenu.h"
#include "ResrcDlgs\ResViewVerInfo.h"

void PEAnals::ViewResDataEntry(PPE_NODE ppn)
{
	switch (ppn->SubT)
	{
		case (BYTE)RT_CURSOR      :
		case (BYTE)RT_BITMAP:
		case (BYTE)RT_ICON:
		{
			CResViewBitmap dlg(m_pImgView + ppn->Offset, (DWORD)ppn->SubT);
			dlg.DoModal();
		}
		break;
		case (BYTE)RT_DIALOG:
		{
			CResViewDlg dlg(m_pImgView + ppn->Offset);
			dlg.DoModal();
		}
		break;
		case (BYTE)RT_MENU:
		{
			CResViewMenu dlg(m_pImgView + ppn->Offset);
			dlg.DoModal();
		}
		break;
		case (BYTE)RT_ACCELERATOR :
		case (BYTE)RT_GROUP_CURSOR:
		case (BYTE)RT_GROUP_ICON	:
		{
			CResViewAccelTbl dlg(ppn->SubT, m_pImgView + ppn->Offset);
			dlg.DoModal();
		}
		break;
		case (BYTE)RT_VERSION:
		{
			CResViewVerInfo dlg(m_pImgView + ppn->Offset);
			dlg.DoModal();
		}
		break;
		case (BYTE)RT_STRING:
		case (BYTE)RT_MESSAGETABLE:
		{
			HTREEITEM hItr = ppn->Node;
			for (int i = 0; i < 7; i++)
			{
				HTREEITEM hUp = m_pView->GetParentItem(hItr);
				if (hUp == NULL)
					break;
				hItr = hUp;
			}
			PPE_NODE ppnId = (PPE_NODE)m_pView->GetItemData(hItr);
			PIMAGE_RESOURCE_DIRECTORY_ENTRY prde = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(m_pImgView + ppnId->Offset);
			CResViewStrTbl dlg(m_pImgView + ppn->Offset, prde->Id, ppn->Size);
			dlg.DoModal();
		}
		break;
		case (BYTE)RT_MANIFEST:
		case (BYTE)RT_HTML:
		break;
		default						:
			//case (LPARAM)RT_FONTDIR	:
			//case (LPARAM)RT_FONT		:
			//case (LPARAM)RT_RCDATA	:
			//case (LPARAM)RT_ANICURSOR	:
			//case (LPARAM)RT_ANIICON	:
		break;
	}
}
