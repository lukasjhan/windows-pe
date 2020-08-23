#pragma once
#include "PEDAsm.Defs.h"
#include <map>
#include <list>


enum OPR_TYPECAT
{
	O_NONE, O_REG, O_IMM, O_DISP, O_SMEM, O_MEM, O_PC, O_PTR
};

struct OP_OPRND
{
	UINT8 _type : 4;
	UINT8 _size : 4;
	UINT8 _regid;
};
typedef OP_OPRND* POP_OPRND;

#define META_GET_ISC(meta) (((meta) >> 3) & 0x1f)
#define META_SET_ISC(di, isc) (((di)->_meta) |= ((isc) << 3))

#define VREX_BIT_B	1
#define VREX_BIT_X	2
#define VREX_BIT_R	4
#define VREX_BIT_W	8
#define REX_BITS_MASKS	(VREX_BIT_B | VREX_BIT_X | VREX_BIT_R | VREX_BIT_W)
#define VREX_BIT_L	16

typedef USHORT PFX_FLAGS;

#define VEX_PP_NO	0	
#define VEX_PP_66	1
#define VEX_PP_F3	2
#define VEX_PP_F2	3

#define VEX_VOP_UD		0
#define VEX_VOP_0F		1	
#define VEX_VOP_0F38	2
#define VEX_VOP_0F3A	3


/* Instruction could not be disassembled. */
#define FLAG_NOT_DECODABLE	((UINT16)-1)
/* The instruction locks memory access. */
#define FLAG_LOCK			(1 << 0)
/* The instruction is prefixed with a REPNZ. */
#define FLAG_REPNZ			(1 << 1)
/* The instruction is prefixed with a REP, this can be a REPZ, it depends on the specific instruction. */
#define FLAG_REP			(1 << 2)
/* Indicates there is a hint taken for Jcc instructions only. */
#define FLAG_HINT_TAKEN		(1 << 3)
/* Indicates there is a hint non-taken for Jcc instructions only. */
#define FLAG_HINT_NOT_TAKEN	(1 << 4)
/* The Imm value is signed extended (E.G in 64 bit decoding mode, a 32 bit imm is usually sign extended into 64 bit imm). */
#define FLAG_IMM_SIGNED		(1 << 5)
/* The destination operand is writable. */
#define FLAG_DST_WR			(1 << 6)
/* The instruction uses RIP-relative indirection. */
#define FLAG_RIP_RELATIVE	(1 << 7)

/* The instruction is privileged and can only be used from Ring0. */
#define FLAG_PRIVILEGED_INSTRUCTION (1 << 15)



union OPR_VALUE
{
	/* Used by O_IMM: */
	INT8	sbyte;
	UINT8	byte;
	INT16	sword;
	UINT16	word;
	INT32	sdword;
	UINT32	dword;
	INT64	sqword; /* All immediates are SIGN-EXTENDED to 64 bits! */
	UINT64	qword;

	/* Used by O_PTR: */
	struct	{ UINT16 seg; UINT32 off; } ptr;
	/* Used by O_IMM1 (i1) and O_IMM2 (i2). ENTER instruction only. */
	struct	{ UINT32 i1, i2; } ex;
};
//
//struct OPR_ADREF
//{
//	INT64		_dispV;
//	UINT8		_dispS  : 2;
//	UINT8		_scale  : 2;
//	UINT8		_segDef : 1;
//	UINT8		_segId  : 3;
//	UINT8		_index;
//};

struct DECODED_INS
{
	DWORD		_offset;
	UINT8		_count;
	UINT8		_meta;
	USHORT		_flags;

	OP_IID		_opCode;
	OP_OPRND	_oprs[MAX_OPRNDS_COUNT];

	/* Used by ops[n].type == O_IMM/O_IMM1&O_IMM2/O_PTR/O_PC. Its size is ops[n].size. */
	OPR_VALUE	_imm;
	/* Used by ops[n].type == O_SMEM/O_MEM/O_DISP. Its size is dispSize. */
	INT64		_dispV;
	UINT8		_dispS:4;
	UINT8		_scale:4;
	UINT8		_index;
	/* General flags of instruction, holds prefixes and more, if FLAG_NOT_DECODABLE, instruction is invalid. */
	/* ID of _opCode in the global _opCode table. Use for mnemonic look up. */
	/* Up to four operands per instruction, ignored if ops[n].type == O_NONE. */
	/* Size of the whole instruction in bytes. */
	/* Segment information of memory indirection, default segment, or overriden one, can be -1. Use SEGMENT macros. */
	UINT8		_seg;
	/* Used by ops[n].type == O_MEM. Base global register index (might be REGI_NONE), scale size (2/4/8), ignored for 0 or 1. */
	/* Meta defines the instruction set class, and the flow control flags. Use META macros. */
};
typedef DECODED_INS* PDECODED_INS;

struct DECODED
{
	DWORD	Offset;
	INT		Count;
	PSTR	Mnemonic;
	PSTR	Operands;

	DECODED()
	{
		Offset = 0, Count = 0;
		Mnemonic = Operands = NULL;
	}
	~DECODED()
	{
		if (Operands != NULL)
			delete[] Operands;
		if (Mnemonic != NULL)
			delete[] Mnemonic;
	}
};
typedef DECODED* PDECODED;

#define CHECK_BOUND(pds, inc)	\
	if(pds->_pos + (inc) > pds->_size) throw E_OUTOFMEMORY;

#define OP_INFO_SIZE	1251
#define OP_INFOEX_SIZE	418

#define CTBL_IDX_1BCODE	0
#define CTBL_IDX_2BCODE	1
#define CTBL_IDX_3B_38	2
#define CTBL_IDX_3B_3A	3
#define CTBL_IDX_3DNOW	4
#define CTBL_MAX_SIZE	5

#define CTBL_LEN_1BCODE		1024
#define CTBL_LEN_2BCODE		1432
#define CTBL_LEN_3B_38		1304
#define CTBL_LEN_3B_3A		 576
#define CTBL_LEN_3DNOW		 256

class PEDisAsm
{
	struct DEC_STATE
	{
		PBYTE		_code;
		int			_pos;
		int			_size;
		BIT_MODE	_bits;


		PFX_FLAGS	_prefix;
		struct
		{
			USHORT	bits : 5;
			USHORT	vex3 : 1;
			USHORT	res	 : 2;
			USHORT	mm	 : 2;
			USHORT	vv	 : 4;
			USHORT	pp	 : 2;
		} _vrex;

		BIT_MODE _sizeAd, _sizeOp;
		UINT	 _opFlags;

		DEC_STATE()
		{
			_code = NULL;
			_pos = _size = 0;
			_bits = BIT_MODE_64;

			_prefix			 = 0;
			*PUSHORT(&_vrex) = 0;

			_sizeAd = _sizeOp = BIT_MODE_32;
			_opFlags = 0;
		}
		DEC_STATE(BIT_MODE bits, PBYTE pCurCode, int nSize) : DEC_STATE()
		{
			_code = pCurCode;
			_pos = 0, _size = nSize;
			_bits = bits;
		}
	};
	typedef DEC_STATE* PDEC_STATE;

	static OP_INFO		OP_INFOS[OP_INFO_SIZE];
	static OP_INFO		OP_INFOSPS[5];
	static OP_INFOEX	OP_INFOEXS[OP_INFOEX_SIZE];

	static OPBYTE_INFO G_1BCODE_TBL[CTBL_LEN_1BCODE];
	static OPBYTE_INFO G_2BCODE_TBL[CTBL_LEN_2BCODE];
	static OPBYTE_INFO G_3B_38_TBL[CTBL_LEN_3B_38];
	static OPBYTE_INFO G_3B_3A_TBL[CTBL_LEN_3B_3A];
	static OPBYTE_INFO G_3DNOW_TBL[CTBL_LEN_3DNOW];
	static OPBYTE_INFO* G_CODE_TBLS[CTBL_MAX_SIZE];

	static BYTE OPR_TYPE_CATS[OT_MAX_TYPE];

private:
	static UINT _fastcall OperandsFix8bitRexBase(UINT reg);
	static void PrefixesUseSegment(UINT defaultSeg, PFX_FLAGS pef, BIT_MODE bm, PDECODED_INS pdi);
	

	static void FormatSignedDisp(PSTR pszOpers, PDECODED_INS pdi, UINT64 addrMask);
	static void FormatSize(PSTR pszOpers, PDECODED_INS pdi, int opNum);

	static void GetEffOpAddrSize(BIT_MODE bm, PDEC_STATE pds);

	static int GetUIntValue(PDEC_STATE pds, PVOID result, int nReadBytes);
	static int GetSIntValue(PDEC_STATE pds, PINT64 result, int nReadBytes);

	static void GetOprnd_Register(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	static void GetOprnd_RegFromOp(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	static void GetOprnd_ReadValue(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	static void GetOprnd_RegFromModRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	static void GetOprnd_RegFromImm(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	static void GetOprnd_MemOnly(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	static void GetOprnd_FromRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm);
	typedef void (*PFN_GET_OPRND)(OPRND_TYPE, int, PDECODED_INS, PDEC_STATE, BYTE);
	static PFN_GET_OPRND G_PFN_OPRANDS[OTCAT_OPR_FUNC_CNT];

	static void GetOprnd_MemFromRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE mod, BYTE rm);
	static void GetOprnd_RegFromRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE rm);

	static void CheckModRM(BYTE mod, BYTE rm, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, bool bVSib = false);
	static void CheckModSIB(BYTE mod, BYTE sib, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds);
	static void CheckModVSIB(BYTE sib, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds);

	static int DecodePrefixes(PDEC_STATE pds);
	static int DecodeInstruction(PDEC_STATE pds, int nCTblIdx, PDECODED_INS pdi);
	static void UpdateInstInfo(PDEC_STATE pds, POP_INFO poi, BYTE modrm, PDECODED_INS pdi);

public:
	static PCSTR	G_PREFIXES[];
	static PCSTR	G_MNEMONICS[];
	static PCSTR	G_REGISTERS[];
	static UINT16	OP_SIZETOINT[9];
	static PCSTR	OP_SIZETOSTR[9];

	static int DisAssemble(PDECODED_INS pdis, int& ndiCnt, 
		const BYTE* pCode, DWORD dwOffset, int nSize, bool bIs32 = false);
	static void DecodedFormat(PDECODED_INS pdi, PDECODED pd, bool bIs32 = false);

};
