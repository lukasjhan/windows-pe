#include "dia2.h"

void PrintHelpOptions();
bool ParseArg(int, PWSTR[], IDiaSession*, IDiaSymbol*);

void DumpGlobalScope(IDiaSymbol*);
void DumpAllPdbInfo(IDiaSession*, IDiaSymbol*);
bool DumpAllMods(IDiaSymbol*);
bool DumpAllPublics(IDiaSymbol*);
bool DumpCompiland(IDiaSymbol*, PCWSTR);
bool DumpAllSymbols(IDiaSymbol*);
bool DumpAllGlobals(IDiaSymbol*);
bool DumpAllTypes(IDiaSymbol*);
bool DumpAllUDTs(IDiaSymbol*);
bool DumpAllEnums(IDiaSymbol*);
bool DumpAllTypedefs(IDiaSymbol*);
bool DumpAllOEMs(IDiaSymbol*);
bool DumpAllFiles(IDiaSession*, IDiaSymbol*);
bool DumpAllLines(IDiaSession*, IDiaSymbol*);
bool DumpAllLines(IDiaSession*, DWORD, DWORD);
bool DumpAllSecContribs(IDiaSession*);
bool DumpAllDebugStreams(IDiaSession*);
bool DumpAllInjectedSources(IDiaSession*);
bool DumpInjectedSource(IDiaSession*, PCWSTR);
bool DumpAllSourceFiles(IDiaSession*, IDiaSymbol*);
bool DumpAllFPO(IDiaSession*);
bool DumpFPO(IDiaSession*, DWORD);
bool DumpFPO(IDiaSession*, IDiaSymbol*, PCWSTR);
bool DumpSymbolWithRVA(IDiaSession*, DWORD, PCWSTR);
bool DumpSymbolsWithRegEx(IDiaSymbol* , PCWSTR, PCWSTR);
bool DumpSymbolWithChildren(IDiaSymbol*, PCWSTR);
bool DumpLines(IDiaSession*, DWORD);
bool DumpLines(IDiaSession*, IDiaSymbol* , PCWSTR);
bool DumpType(IDiaSymbol*, PCWSTR);
bool DumpLinesForSourceFile(IDiaSession*, PCWSTR, DWORD);
bool DumpPublicSymbolsSorted(IDiaSession*, DWORD, DWORD, bool);
bool DumpLabel(IDiaSession*, DWORD);
bool DumpAnnotations(IDiaSession*, DWORD);
bool DumpMapToSrc(IDiaSession*, DWORD);
bool DumpMapFromSrc(IDiaSession*, DWORD);

HRESULT GetTable(IDiaSession*, REFIID, PVOID*);

///////////////////////////////////////////////////////////////////
// Functions defined in regs.cpp
PCWSTR SzNameC7Reg(USHORT, DWORD);
