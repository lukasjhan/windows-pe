extern PCWSTR  const rgRegX86[];
extern PCWSTR  const rgRegAMD64[];
extern PCWSTR  const rgRegMips[];
extern PCWSTR  const rgReg68k[];
extern PCWSTR  const rgRegAlpha[];
extern PCWSTR  const rgRegPpc[];
extern PCWSTR  const rgRegSh[];
extern PCWSTR  const rgRegArm[];

typedef struct MapIa64Reg
{
    CV_HREG_e  iCvReg;
    PCWSTR wszRegName;
}MapIa64Reg;
extern const MapIa64Reg mpIa64regSz[];
int cmpIa64regSz(LPCVOID, LPCVOID);

PCWSTR SzNameC7Reg(USHORT, DWORD);
