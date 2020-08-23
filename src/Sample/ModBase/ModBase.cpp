#include "stdafx.h"
#include "Windows.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

void main()
{
	HMODULE hModBase = GetModuleHandle(NULL);
	printf("GetModuleHandle = %p, __ImageBase=%p\n", hModBase, &__ImageBase);

	PBYTE pImgBase = (PBYTE)hModBase;
	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)pImgBase;
	if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
	{
		printf("Invalid dos signature: 0x%04X\n", pdh->e_magic);
		return;
	}

	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + pdh->e_lfanew);
	if (pnh->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("Invalid NT signature: 0x%08X\n", pnh->Signature);
		return;
	}
	printf("NT Signature : 0x%08X\n", pnh->Signature);

	printf("File Header :\n");
	PIMAGE_FILE_HEADER pfh = (PIMAGE_FILE_HEADER)&pnh->FileHeader;
	printf("\tMachine              : 0x%04X\n", pfh->Machine);
	printf("\tNumberOfSections     : 0x%04X\n", pfh->NumberOfSections);
	printf("\tTimeDateStamp        : 0x%08X\n", pfh->TimeDateStamp);
	printf("\tPointerToSymbolTable : 0x%08X\n", pfh->PointerToSymbolTable);
	printf("\tNumberOfSymbols      : 0x%08X\n", pfh->NumberOfSymbols);
	printf("\tSizeOfOptionalHeader : 0x%04X\n", pfh->SizeOfOptionalHeader);
	printf("\tCharacteristics      : 0x%04X\n", pfh->Characteristics);
}
