#include "stdafx.h"
#include "PEDAsm.h"


BYTE PEDisAsm::OPR_TYPE_CATS[OT_MAX_TYPE] = 
{
	OTCAT_NONE,			// OT_NONE,
	OTCAT_OP_REG,		// OT_FREG_SI
	OTCAT_OP_REG,		// OT_OP_R8
	OTCAT_OP_REG,		// OT_OP_R_ALL
	OTCAT_DIR_REG,		// OT_ACC8
	OTCAT_DIR_REG,		// OT_ACC16
	OTCAT_DIR_REG,		// OT_ACC_ALL_NO64
	OTCAT_DIR_REG,		// OT_ACC_ALL
	OTCAT_DIR_REG,		// OT_REG_CL
	OTCAT_DIR_REG,		// OT_REG_DX
	OTCAT_DIR_REG,		// OT_FREG_SI0
	OTCAT_DIR_REG,		// OT_REG_ESI
	OTCAT_DIR_REG,		// OT_REG_EDI
	OTCAT_DIR_REG,		// OT_SEG_PF
	OTCAT_DIR_REG,		// OT_REGI_EBXAL
	OTCAT_READ_VAL,		// OT_IMM8
	OTCAT_READ_VAL,		// OT_IMM_ALL 
	OTCAT_READ_VAL,		// OT_IMM32 
	OTCAT_READ_VAL,		// OT_IMM16
	OTCAT_READ_VAL,		// OT_SE_IMM8 
	OTCAT_READ_VAL,		// OT_PTR16_ALL
	OTCAT_READ_VAL,		// OT_REL_8
	OTCAT_READ_VAL,		// OT_REL_16_32
	OTCAT_READ_VAL,		// OT_MOFFS8
	OTCAT_READ_VAL,		// OT_MOFFS_ALL
	OTCAT_READ_VAL,		// OT_CONST1
	OTCAT_REG_MODRM,	// OT_REG8
	OTCAT_REG_MODRM,	// OT_REG16
	OTCAT_REG_MODRM,	// OT_REG_ALL
	OTCAT_REG_MODRM,	// OT_REG32
	OTCAT_REG_MODRM,	// OT_REG32_64
	OTCAT_REG_MODRM,	// OT_REG32_64_RM
	OTCAT_REG_MODRM,	// OT_REG_CR 
	OTCAT_REG_MODRM,	// OT_REG_DR
	OTCAT_REG_MODRM,	// OT_REG_SEG
	OTCAT_REG_MODRM,	// OT_MMX
	OTCAT_REG_MODRM,	// OT_MMX_RM
	OTCAT_REG_MODRM,	// OT_XMM
	OTCAT_REG_MODRM,	// OT_XMM_RM
	OTCAT_REG_MODRM,	// OT_VXMM
	OTCAT_REG_MODRM,	// OT_YXMM
	OTCAT_REG_MODRM,	// OT_YMM
	OTCAT_REG_MODRM,	// OT_VYMM
	OTCAT_REG_MODRM,	// OT_VYXMM
	OTCAT_REG_MODRM,	// OT_WREG32_64
	OTCAT_REG_FROM_IMM,	// OT_XMM_IMM
	OTCAT_REG_FROM_IMM,	// OT_YXMM_IMM
	OTCAT_MEM_REF,		// OT_MEM64_128
	OTCAT_MEM_REF,		// OT_MEM32
	OTCAT_MEM_REF,		// OT_MEM32_64
	OTCAT_MEM_REF,		// OT_MEM64 
	OTCAT_MEM_REF,		// OT_MEM128
	OTCAT_MEM_REF,		// OT_FPUM16
	OTCAT_MEM_REF,		// OT_FPUM32
	OTCAT_MEM_REF,		// OT_FPUM64
	OTCAT_MEM_REF,		// OT_FPUM80
	OTCAT_MEM_REF,		// OT_LMEM128_256
	OTCAT_MEM_REF,		// OT_VSIB_M32_64
	OTCAT_MEM_REF,		// OT_MEM
	OTCAT_MEM_REF,		// OT_MEM16_ALL
	OTCAT_MEM_REF,		// OT_M16_A_3264
	OTCAT_MEM_REF,		// OT_M1632_A_1632
	OTCAT_RM_FROM_RM,	// OT_R_ALL_M16
	OTCAT_RM_FROM_RM,	// OT_RM_ALL
	OTCAT_RM_FROM_RM,	// OT_R32_64_M8
	OTCAT_RM_FROM_RM,	// OT_R32_64_M16
	OTCAT_RM_FROM_RM,	// OT_RM32_64
	OTCAT_RM_FROM_RM,	// OT_RM16_32
	OTCAT_RM_FROM_RM,	// OT_RM16
	OTCAT_RM_FROM_RM,	// OT_RM8
	OTCAT_RM_FROM_RM,	// OT_MMX32
	OTCAT_RM_FROM_RM,	// OT_MMX64
	OTCAT_RM_FROM_RM,	// OT_XMM16
	OTCAT_RM_FROM_RM,	// OT_XMM32
	OTCAT_RM_FROM_RM,	// OT_XMM64
	OTCAT_RM_FROM_RM,	// OT_XMM128
	OTCAT_RM_FROM_RM,	// OT_RM32
	OTCAT_RM_FROM_RM,	// OT_R32_M8
	OTCAT_RM_FROM_RM,	// OT_R32_M16
	OTCAT_RM_FROM_RM,	// OT_YMM256
	OTCAT_RM_FROM_RM,	// OT_YXMM64_256
	OTCAT_RM_FROM_RM,	// OT_YXMM128_256
	OTCAT_RM_FROM_RM,	// OT_WXMM32_64
	OTCAT_RM_FROM_RM,	// OT_LXMM64_128
	OTCAT_RM_FROM_RM,	// OT_WRM32_64
	OTCAT_RM_FROM_RM,	// OT_REG32_64_M8
	OTCAT_RM_FROM_RM,	// OT_REG32_64_M16
};

PEDisAsm::PFN_GET_OPRND PEDisAsm::G_PFN_OPRANDS[OTCAT_OPR_FUNC_CNT] =
{
	NULL,					// OTCAT_NONE
	GetOprnd_RegFromOp,		// OTCAT_OP_REG
	GetOprnd_Register,		// OTCAT_DIR_REG
	GetOprnd_ReadValue,		// OTCAT_READ_VAL
	GetOprnd_RegFromModRM,	// OTCAT_REG_MODRM
	GetOprnd_RegFromImm,	// OTCAT_REG_FROM_IMM
	GetOprnd_MemOnly,		// OTCAT_MEM_REF
	GetOprnd_FromRM			// OTCAT_RM_FROM_RM
};

int PEDisAsm::GetUIntValue(PDEC_STATE pds, PVOID result, int nReadBytes)
{
	CHECK_BOUND(pds, nReadBytes);
	switch (nReadBytes)
	{
		case 1: *PUINT8 (result) = *PUINT8 (pds->_code + pds->_pos); break;
		case 2: *PUINT16(result) = *PUINT16(pds->_code + pds->_pos); break;
		case 4: *PUINT32(result) = *PUINT32(pds->_code + pds->_pos); break;
		case 8: *PUINT64(result) = *PUINT64(pds->_code + pds->_pos); break;
	}
	return nReadBytes;
}

int PEDisAsm::GetSIntValue(PDEC_STATE pds, PINT64 result, int nReadBytes)
{
	CHECK_BOUND(pds, nReadBytes);
	switch (nReadBytes)
	{
		case 1: *result = *PINT8 (pds->_code + pds->_pos); break;
		case 2: *result = *PINT16(pds->_code + pds->_pos); break;
		case 4: *result = *PINT32(pds->_code + pds->_pos); break;
		case 8: *result = *PINT64(pds->_code + pds->_pos); break;
	}
	return nReadBytes;
}

UINT PEDisAsm::OperandsFix8bitRexBase(UINT reg)
{
	if ((reg >= 4) && (reg < 8)) return reg + REGS8_REX_BASE - 4;
	return reg + REGS8_BASE;
}

void PEDisAsm::PrefixesUseSegment(UINT defSeg, PFX_FLAGS pef, BIT_MODE bm, PDECODED_INS pdi)
{
	UINT flags = 0;
	if (bm == BIT_MODE_64)
		flags = pef & OPAF_PRE_SEGOVRD_MASK64;
	else
		flags = pef & OPAF_PRE_SEGOVRD_MASK;

	if ((flags == 0) || (flags == defSeg))
	{
		flags = defSeg;
		pdi->_seg |= SEGR_DEFAULT;
	}
	else if (flags != defSeg)
	{
		/* Use it only if it's non-default segment. */
	}

	/* ASSERT: R_XX must be below 128. */
	switch (flags)
	{
		case OPAF_PRE_ES: pdi->_seg |= R_ES; break;
		case OPAF_PRE_CS: pdi->_seg |= R_CS; break;
		case OPAF_PRE_SS: pdi->_seg |= R_SS; break;
		case OPAF_PRE_DS: pdi->_seg |= R_DS; break;
		case OPAF_PRE_FS: pdi->_seg |= R_FS; break;
		case OPAF_PRE_GS: pdi->_seg |= R_GS; break;
	}

	/* If it's one of the CS,SS,DS,ES and the mode is 64 bits, set segment it to none, since it's ignored. */
	if ((bm == BIT_MODE_64) && (pef & OPAF_PRE_SEGOVRD_MASK32))
		pdi->_seg = REGI_NONE;
}


void PEDisAsm::GetOprnd_Register(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND	pop = &pdi->_oprs[nOpOrd];
	UINT8		seg = REGI_NONE;

	pop->_type = O_REG;
	switch (type)
	{
		case OT_ACC8:
			pop->_size = OP_SIZE::OPR_SZ8, pop->_regid = R_AL;
		break;
		case OT_ACC16:
			pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = R_AX;
		break;
		case OT_ACC_ALL_NO64: 
			pds->_prefix = (pds->_prefix & ~VREX_BIT_W);
		case OT_ACC_ALL:
			if (pds->_sizeOp == BIT_MODE_16)
				pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = R_AX;
			else if (pds->_sizeOp == BIT_MODE_32)
				pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = R_EAX;
			else
				pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = R_RAX;
		break;

		case OT_REG_CL:
			pop->_size = OP_SIZE::OPR_SZ8, pop->_regid = R_CL;
		break;
		case OT_REG_DX:
			pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = R_DX;
		break;
		case OT_FREG_SI0:
			pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = R_ST0;
		break;

		case OT_REG_ESI:
			pop->_type = O_SMEM;
			if (pds->_opFlags & OPAF_16BITS)
			{
				if (pds->_sizeOp == BIT_MODE_16)
					pop->_size = OP_SIZE::OPR_SZ16;
				else if ((pds->_sizeOp == BIT_MODE_64) && (pds->_opFlags & OPAF_64BITS))
					pop->_size = OP_SIZE::OPR_SZ64;
				else 
					pop->_size = OP_SIZE::OPR_SZ32;
			}
			else
				pop->_size = OP_SIZE::OPR_SZ8;
			PrefixesUseSegment(OPAF_PRE_DS, pds->_prefix, pds->_bits, pdi);

			if (pds->_sizeAd == BIT_MODE_16)
				pop->_regid = R_SI;
			else if (pds->_sizeAd == BIT_MODE_32)
				pop->_regid = R_ESI;
			else
				pop->_regid = R_RSI;
		break;
		case OT_REG_EDI:
			pop->_type = O_SMEM;
			if (pds->_opFlags & OPAF_16BITS)
			{
				if (pds->_sizeOp == BIT_MODE_16)
					pop->_size = OP_SIZE::OPR_SZ16;
				else if ((pds->_sizeOp == BIT_MODE_64) && (pds->_opFlags & OPAF_64BITS))
					pop->_size = OP_SIZE::OPR_SZ64;
				else
					pop->_size = OP_SIZE::OPR_SZ32;
			}
			else
				pop->_size = OP_SIZE::OPR_SZ8;

			if ((nOpOrd == 0) && (pds->_bits != BIT_MODE_64))
				pdi->_seg = R_ES | SEGR_DEFAULT; /* No ES in 64 bits mode. */

			if (pds->_sizeAd == BIT_MODE_16)
				pop->_regid = R_DI;
			else if (pds->_sizeAd == BIT_MODE_32)
				pop->_regid = R_EDI;
			else 
				pop->_regid = R_RDI;
		break;

		case OT_SEG_PF:
			pop->_size = OP_SIZE::OPR_SZ16;
			switch (pds->_opFlags & OPAF_PRE_SEGOVRD_MASK)
			{
				case OPAF_PRE_ES: pop->_regid = R_ES; break;
				case OPAF_PRE_CS: pop->_regid = R_CS; break;
				case OPAF_PRE_SS: pop->_regid = R_SS; break;
				case OPAF_PRE_DS: pop->_regid = R_DS; break;
				case OPAF_PRE_FS: pop->_regid = R_FS; break;
				case OPAF_PRE_GS: pop->_regid = R_GS; break;
			}
		break;

		case OT_REGI_EBXAL:
			PrefixesUseSegment(OPAF_PRE_DS, pds->_prefix, pds->_bits, pdi);

			pop->_type = O_MEM, pop->_size = OP_SIZE::OPR_SZ8, pop->_regid = R_AL;
			if (pds->_sizeAd == BIT_MODE_16)
				pdi->_index = R_BX;
			else if (pds->_sizeAd == BIT_MODE_32)
				pdi->_index = R_EBX;
			else
				pdi->_index = R_RBX;
		break;

		default:
			throw E_INVALIDARG;
	}
}

void PEDisAsm::GetOprnd_RegFromOp(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];
	BYTE reg = GET_RM_MODRM(prm);

	pop->_type = O_REG;
	if (type == OT_FREG_SI)
	{
		pop->_size = OP_SIZE::OPR_SZ32;
		pop->_regid = FPUREGS_BASE + reg;// GET_RM_MODRM((BYTE)prm);
	}
	else
	{
		/*
		* Special treatment for Instructions-Block:
		* INC/DEC (only 16/32 bits) /PUSH/POP/XCHG instructions, which get their REG from their own binary code.

		* Notice these instructions are 1 or 2 byte long,
		* code points after the byte which represents the instruction itself,
		* thus, even if the instructions are 2 bytes long it will read its last byte which contains the REG info.
		*/
		if (type != OT_OP_R8 && type != OT_OP_R_ALL)
			throw E_INVALIDARG;

		if (type == OT_OP_R8)
		{
			pop->_size = OP_SIZE::OPR_SZ8;
			if (pds->_vrex.bits & VREX_BIT_B)
				pop->_regid = (UINT8)OperandsFix8bitRexBase(reg + EX_GPR_BASE);
			else if (pds->_opFlags & OPAF_PRE_REX)
				pop->_regid = (UINT8)OperandsFix8bitRexBase(reg);
			else
				pop->_regid = (UINT8)(REGS8_BASE + reg);
		}
		else
		{
			switch (pds->_sizeOp)
			{
				case BIT_MODE_16:
					if (pds->_vrex.bits & VREX_BIT_B)
						reg += EX_GPR_BASE;
					pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = REGS16_BASE + reg;
				break;
				case BIT_MODE_32:
					if (pds->_vrex.bits & VREX_BIT_B)
						reg += EX_GPR_BASE;
					pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + reg;
				break;
				case BIT_MODE_64:
					/*
					* Automatically promoted instruction can drop REX prefix if not required.
					* PUSH/POP defaults to 64 bits. --> OPAF_64BITS
					* MOV imm64 / BSWAP requires REX.W to be 64 bits --> OPAF_64BITS | OPAF_PRE_REX
					*/
					if ((pds->_opFlags & OPAF_64BITS) && ((pds->_opFlags & OPAF_PRE_REX) == 0))
					{
						if (pds->_vrex.bits & VREX_BIT_B)
							reg += EX_GPR_BASE;
					}
					else
						reg += (pds->_vrex.bits & VREX_BIT_B) ? EX_GPR_BASE : 0;
					pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = REGS64_BASE + reg;
				break;
			}
		}
	}
}

void PEDisAsm::GetOprnd_ReadValue(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND	pop = &pdi->_oprs[nOpOrd];
	PVOID		pVal = NULL;
	bool		bHandled = true;

	pop->_type = O_IMM;
	switch (type)
	{
		case OT_IMM8	: 
			pop->_size = OP_SIZE::OPR_SZ8;
			if (prm != 0)
			{
				if (nOpOrd != 1)
					throw E_INVALIDARG;
				pVal = &pdi->_imm.ex.i2;
			}
			else
				pVal = &pdi->_imm.byte;
		break;
		case OT_IMM_ALL: 
			pop->_size = OP_SIZE(pds->_sizeOp + 2);
			if (pop->_size == OP_SIZE::OPR_SZ16)
			{
				pVal = &pdi->_imm.word;
				break;
			}
			else if ((pop->_size == OP_SIZE::OPR_SZ64) &&
				((pds->_opFlags & (OPAF_64BITS | OPAF_PRE_REX)) == (OPAF_64BITS | OPAF_PRE_REX)))
			{
				pVal = &pdi->_imm.qword;
				break;
			}
		case OT_IMM32: 
			pdi->_flags |= FLAG_IMM_SIGNED;
			pop->_size = OP_SIZE::OPR_SZ32;
			pVal = &pdi->_imm.dword;
		break;
		case OT_IMM16	: 
			pop->_size = OP_SIZE::OPR_SZ16;
			if (prm != 0)
			{
				if (nOpOrd != 0)
					throw E_INVALIDARG;
				pVal = &pdi->_imm.ex.i1;
			}
			else
				pVal = &pdi->_imm.word;
		break;
		case OT_SE_IMM8	: 
			pop->_size = (pds->_opFlags & OPAF_PRE_OP_SIZE) ? OP_SIZE(pds->_sizeOp + 2) : OP_SIZE::OPR_SZ8;
			pdi->_flags |= FLAG_IMM_SIGNED;
			pVal = &pdi->_imm.sqword;
		break;
		default: 
			bHandled = false;
		break;
	}
	if (bHandled)
	{
		if (type == OT_SE_IMM8)
			pds->_pos += GetSIntValue(pds, (PINT64)pVal, sizeof(INT8));
		else
			pds->_pos += GetUIntValue(pds, pVal, (1 << (pop->_size - 1)));
		return;
	}

	switch (type)
	{
		case OT_PTR16_ALL:
			if (pds->_bits == BIT_MODE_64)
				throw E_INVALIDARG;
			pop->_type = O_PTR;
			if (pds->_sizeOp == BIT_MODE_16)
			{
				CHECK_BOUND(pds, sizeof(INT16) * 2);
				pop->_size = OP_SIZE::OPR_SZ16;
				pdi->_imm.ptr.off = *PUINT16(pds->_code + pds->_pos);
				pdi->_imm.ptr.seg = *PUINT16(pds->_code + pds->_pos + sizeof(INT16));
				pds->_pos += sizeof(INT16) * 2;
			}
			else
			{ 
				CHECK_BOUND(pds, (sizeof(INT32) + sizeof(INT16)));
				pop->_size = OP_SIZE::OPR_SZ32;
				pdi->_imm.ptr.off = *PUINT32(pds->_code + pds->_pos);
				pdi->_imm.ptr.seg = *PUINT16(pds->_code + pds->_pos + sizeof(INT32));
				pds->_pos += (sizeof(INT32) + sizeof(INT16));
			}
		break;

		case OT_REL_8:
		case OT_REL_16_32:
			pop->_type = O_PC;
			if (type == OT_REL_8)
				pop->_size = OP_SIZE::OPR_SZ8;
			else
				pop->_size = (pds->_sizeOp == BIT_MODE_16) ? OP_SIZE::OPR_SZ16 : OP_SIZE::OPR_SZ32;
			pds->_pos += GetSIntValue(pds, &pdi->_imm.sqword, (int)(1 << (pop->_size - 1)));
		break;

		case OT_MOFFS8:
		case OT_MOFFS_ALL:
			pop->_type = O_DISP;
			if (type == OT_MOFFS8)
				pop->_size = OP_SIZE::OPR_SZ8;
			else
				pop->_size = OP_SIZE(pds->_sizeOp + OPR_SZ16);
			PrefixesUseSegment(OPAF_PRE_DS, pds->_prefix, pds->_bits, pdi);
			pdi->_dispS = OP_SIZE(pds->_sizeAd + OPR_SZ16);
			pds->_pos += GetUIntValue(pds, &pdi->_dispV, (int)(1 << (pdi->_dispS - 1)));
		break;

		case OT_CONST1:
			pop->_type = O_IMM, pop->_size = OP_SIZE::OPR_SZ8;
			pdi->_imm.byte = 1;
		break;

		default: 
			throw E_INVALIDARG;
	}
}

void PEDisAsm::GetOprnd_RegFromModRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];
	BYTE mod = GET_MOD_MODRM(prm);
	BYTE reg = GET_REG_MODRM(prm);
	BYTE rm  = GET_RM_MODRM (prm);
	BYTE vexV = pds->_vrex.vv;

	pop->_type = O_REG;
	switch (type)
	{
		case OT_REG8:
			pop->_size = OP_SIZE::OPR_SZ8;
			if (pds->_prefix & (OPAF_PRE_REX | OPAF_PRE_VEX))
			{
				/*
				* If REX prefix is valid then we will have to use low bytes.
				* This is a PASSIVE behavior changer of REX prefix, it affects operands even if its value is 0x40 !
				*/
				pop->_regid = (UINT8)OperandsFix8bitRexBase
					(reg + ((pds->_vrex.bits & VREX_BIT_R) ? EX_GPR_BASE : 0));
			}
			else
				pop->_regid = (UINT8)(REGS8_BASE + reg);
		break;
		case OT_REG16:
			pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = REGS16_BASE + reg;
		break;
		case OT_REG_ALL:
			switch (pds->_sizeOp)
			{
				case BIT_MODE_16:
					if (pds->_vrex.bits & VREX_BIT_R)
						reg += EX_GPR_BASE;
					pop->_type = O_REG, pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = REGS16_BASE + reg;
				break;
				case BIT_MODE_32:
					if (pds->_vrex.bits & VREX_BIT_R)
						reg += EX_GPR_BASE;
					pop->_type = O_REG, pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + reg;
				break;
				case BIT_MODE_64: /* rex must be presented. */
					pop->_type = O_REG, pop->_size = OP_SIZE::OPR_SZ64;
					pop->_regid = REGS64_BASE + reg + ((pds->_vrex.bits & VREX_BIT_R) ? EX_GPR_BASE : 0);
				break;
			}
		break;
		case OT_REG32:
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + reg;
		break;
		case OT_REG32_64: /* Handle CVT's, MOVxX and MOVNTI instructions which could be extended to 64 bits registers with REX. */
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;

			/* Is it a promoted instruction? (only OPAF_64BITS is set and REX isn't required.) */
			if ((pds->_bits == BIT_MODE_64) && ((pds->_opFlags & (OPAF_64BITS | OPAF_PRE_REX)) == OPAF_64BITS))
			{
				pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = REGS64_BASE + reg;
				break;
			}
			/* Give a chance to REX.W. Because if it was a promoted instruction we don't care about REX.W anyways. */
			if (pds->_vrex.bits & VREX_BIT_W)
				pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = REGS64_BASE + reg;
			else
				pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + reg;
		break;

		/* Force decoding mode. Used for MOV CR(n)/DR(n) which defaults to 64 bits operand size in 64 bits. */
		case OT_REG32_64_RM:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			if (pds->_bits == BIT_MODE_64)
				pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = REGS64_BASE + rm;
			else
				pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + rm;
		break;
		case OT_REG_CR: 
		case OT_REG_DR:
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;
			pop->_size = (pds->_bits == BIT_MODE_64) ? OP_SIZE::OPR_SZ64 : OP_SIZE::OPR_SZ32;
			pop->_regid = (UINT8)(DREGS_BASE + reg);
		break;

		case OT_REG_SEG: /* Works with REG16 only! */ //MOV RAX, DS
			if (reg <= SEG_REGS_MAX - 1)
				pop->_size = OP_SIZE::OPR_SZ16, pop->_regid = SREGS_BASE + reg;
			else
				throw E_INVALIDARG;
		break;

		case OT_MMX: /* MMX register */
			pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = MMXREGS_BASE + reg;
		break;
		case OT_MMX_RM: /* MMX register, this time from the RM field */
			pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = MMXREGS_BASE + rm;
		break;

		case OT_XMM: /* SSE register */
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + reg;
			break;
		case OT_XMM_RM: /* SSE register, this time from the RM field */
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + rm;
		break;

		case OT_VXMM:
			pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + vexV;
			break;
		case OT_YXMM:
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;
			if (pds->_vrex.bits & VREX_BIT_L)
				pop->_size = OP_SIZE::OPR_SZ256, pop->_regid = AVXREGS_BASE + reg;
			else
				pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + reg;
		break;
		case OT_YMM:
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ256, pop->_regid = AVXREGS_BASE + reg;
		break;
		case OT_VYMM:
			pop->_size = OP_SIZE::OPR_SZ256, pop->_regid = AVXREGS_BASE + vexV;
		break;
		case OT_VYXMM:
			if (pds->_vrex.bits & VREX_BIT_L)
				pop->_size = OP_SIZE::OPR_SZ256, pop->_regid = AVXREGS_BASE + vexV;
			else
				pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + vexV;
		break;
		case OT_WREG32_64:
			if (pds->_vrex.bits & VREX_BIT_R)
				reg += EX_GPR_BASE;
			if (pds->_prefix & OPAF_PRE_VEX)
			{
				if (pds->_vrex.bits & VREX_BIT_W)
					pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = REGS64_BASE + vexV;
				else
					pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + vexV;
			}
			else
			{
				if (pds->_vrex.bits & VREX_BIT_W)
					pop->_size = OP_SIZE::OPR_SZ64, pop->_regid = REGS64_BASE + reg;
				else
					pop->_size = OP_SIZE::OPR_SZ32, pop->_regid = REGS32_BASE + reg;
			}
		break;

		default: throw E_INVALIDARG;
	}
}

void PEDisAsm::GetOprnd_RegFromImm(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND	pop = &pdi->_oprs[nOpOrd];
	USHORT		reg = 0;

	pop->_type = O_REG;
	switch (type)
	{
		case OT_XMM_IMM:
			if (pds->_bits == BIT_MODE_32) 
				reg = (prm >> 4) & 0x7;
			else
				reg = (prm >> 4) & 0xf;
			pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + reg;
		break;
		case OT_YXMM_IMM:
			if (pds->_bits == BIT_MODE_32)
				reg = (prm >> 4) & 0x7;
			else
				reg = (prm >> 4) & 0xf;
			if (pds->_vrex.bits & VREX_BIT_L)
				pop->_size = OP_SIZE::OPR_SZ256, pop->_regid = AVXREGS_BASE + reg;
			else
				pop->_size = OP_SIZE::OPR_SZ128, pop->_regid = SSEREGS_BASE + reg;
		break;
		default: 
			throw E_INVALIDARG;
	}
}

void PEDisAsm::GetOprnd_MemOnly(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];
	BYTE mod = GET_MOD_MODRM(prm);
	BYTE reg = GET_REG_MODRM(prm);
	BYTE rm  = GET_RM_MODRM (prm);

	if (mod == 0x3)
		throw E_INVALIDARG;
	/* -- Memory Indirection Operands (that cannot be a general purpose register) -- */
	pop->_size = OPR_SZ0;
	switch (type)
	{
		case OT_MEM64_128: /* Used only by CMPXCHG8/16B. */
			/* Make a specific check when the type is OT_MEM64_128 since the lockable CMPXCHG8B uses this one... */
			//if (lockableInstruction && (ps->decodedPrefixes & OPAF_PRE_LOCK))
			//	*lockableInstruction = TRUE;
			pop->_size = (pds->_sizeOp == BIT_MODE_64) ? OPR_SZ128 : OPR_SZ64;
		break;

		case OT_MEM32: pop->_size = OPR_SZ32; break;
		case OT_MEM32_64:
			/* Used by MOVNTI. Default size is 32bits, 64bits with REX. */
			pop->_size = (pds->_sizeOp == BIT_MODE_64) ? OPR_SZ64 : OPR_SZ32;
		break;

		case OT_MEM64 : pop->_size = OPR_SZ64; break;
		case OT_MEM128: pop->_size = OPR_SZ128; break;
		case OT_FPUM16: pop->_size = OPR_SZ16; break;
		case OT_FPUM32: pop->_size = OPR_SZ32; break;
		case OT_FPUM64: pop->_size = OPR_SZ64; break;
		case OT_FPUM80: pop->_size = OPR_SZ80; break;

		case OT_LMEM128_256:
			pop->_size = (pds->_vrex.bits & VREX_BIT_L) ? OPR_SZ256 : OPR_SZ128;
		break;

		case OT_VSIB_M32_64:
			pop->_size = OPR_SZ32;
		break;

		case OT_MEM: /* Size is unknown, but still handled. */
		break;

		case OT_MEM16_ALL: /* The size indicates about the second item of the pair. */
			switch (pds->_sizeOp)
			{
				case BIT_MODE_16: pop->_size = OPR_SZ16; break;
				case BIT_MODE_32: pop->_size = OPR_SZ32; break;
				case BIT_MODE_64: pop->_size = OPR_SZ64; break;
			}
		break;

		case OT_M16_A_3264: /* The size indicates about the second item of the pair. */
			//pop->_size = (pds->_bits == BIT_MODE_64) ? OPR_SZ64 : OPR_SZ32;
		break;
		case OT_M1632_A_1632:
			pop->_size = (pds->_sizeAd == BIT_MODE_16) ? OPR_SZ32 : OPR_SZ64;
		break;

		default: 
			throw E_INVALIDARG;
	}

	pop->_type = O_DISP;
	CheckModRM(mod, rm, nOpOrd, pdi, pds, (type == OT_VSIB_M32_64));
	if (pdi->_dispS > OPR_SZ0)
		pds->_pos += GetSIntValue(pds, (INT64*)&pdi->_dispV, (int)(1 << (pdi->_dispS - 1)));
}

void PEDisAsm::GetOprnd_FromRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE prm)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];
	BYTE mod = GET_MOD_MODRM(prm);
	BYTE rm  = GET_RM_MODRM (prm);

	if (mod == 3)
		return GetOprnd_RegFromRM(type, nOpOrd, pdi, pds, rm);
	else
		return GetOprnd_MemFromRM(type, nOpOrd, pdi, pds, mod, rm);
}

void PEDisAsm::GetOprnd_RegFromRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE rm)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];

	switch (type)
	{
		case OT_R_ALL_M16:
		case OT_RM_ALL:
			switch (pds->_sizeOp)
			{
				case BIT_MODE_16:
					if (pds->_vrex.bits & VREX_BIT_B)
						rm += EX_GPR_BASE;
					pop->_size = OP_SIZE::OPR_SZ16;
					rm += REGS16_BASE;
				break;
				case BIT_MODE_32:
					if (pds->_vrex.bits & VREX_BIT_B)
						rm += EX_GPR_BASE;
					pop->_size = OP_SIZE::OPR_SZ32;
					rm += REGS32_BASE;
				break;
				case BIT_MODE_64:
					/* A fix for SMSW RAX which use the REX prefix. */
					/* CALL NEAR/PUSH/POP defaults to 64 bits. --> OPAF_64BITS, REX isn't required, thus ignored anyways. */
					/* Include REX if used for REX.B. */
					if (pds->_vrex.bits & VREX_BIT_B)
						rm += EX_GPR_BASE;
					pop->_size = OP_SIZE::OPR_SZ64;
					rm += REGS64_BASE;
				break;
			}
		break;
		case OT_R32_64_M8:
			/* FALL THROUGH, decode 32 or 64 bits register. */
		case OT_R32_64_M16:
			/* FALL THROUGH, decode 32 or 64 bits register. */
		case OT_RM32_64: /* Take care specifically in MOVNTI/MOVD/CVT's instructions, making it _REG64 with REX or if they are promoted. */
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			/* Is it a promoted instruction? (only OPAF_64BITS is set and REX isn't required.) */
			if ((pds->_bits == BIT_MODE_64) && ((pds->_opFlags & (OPAF_64BITS | OPAF_PRE_REX)) == OPAF_64BITS))
			{
				pop->_size = OP_SIZE::OPR_SZ64;
				rm += REGS64_BASE;
				break;
			}
			/* Give a chance to REX.W. Because if it was a promoted instruction we don't care about REX.W anyways. */
			if (pds->_vrex.bits & VREX_BIT_W)
			{
				pop->_size = OP_SIZE::OPR_SZ64;
				rm += REGS64_BASE;
			}
			else
			{
				pop->_size = OP_SIZE::OPR_SZ32;
				rm += REGS32_BASE;
			}
		break;
		case OT_RM16_32: /* Used only with MOVZXD instruction to support 16 bits operand. */
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			/* Is it 16 bits operand size? */
			if (pds->_prefix & OPAF_PRE_OP_SIZE)
			{
				pop->_size = OP_SIZE::OPR_SZ16;
				rm += REGS16_BASE;
			}
			else
			{
				pop->_size = OP_SIZE::OPR_SZ32;
				rm += REGS32_BASE;
			}
		break;
		case OT_RM16:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			rm += REGS16_BASE;
		break;
		case OT_RM8:
			if (pds->_prefix & OPAF_PRE_REX)
				rm = OperandsFix8bitRexBase(rm + ((pds->_vrex.bits & VREX_BIT_B) ? EX_GPR_BASE : 0));
			else
				rm += REGS8_BASE;
		break;
		case OT_MMX32:
		case OT_MMX64:
			/* MMX doesn't support extended registers. */
			pop->_size = OP_SIZE::OPR_SZ64;
			rm += MMXREGS_BASE;
		break;

		case OT_XMM16:
		case OT_XMM32:
		case OT_XMM64:
		case OT_XMM128:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ128;
			rm += SSEREGS_BASE;
		break;

		case OT_RM32:
		case OT_R32_M8:
		case OT_R32_M16:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ32;
			rm += REGS32_BASE;
		break;

		case OT_YMM256:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			rm += AVXREGS_BASE;
		break;
		case OT_YXMM64_256:
		case OT_YXMM128_256:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			if (pds->_vrex.bits & VREX_BIT_L)
			{
				pop->_size = OP_SIZE::OPR_SZ256;
				rm += AVXREGS_BASE;
			}
			else
			{
				pop->_size = OP_SIZE::OPR_SZ128;
				rm += SSEREGS_BASE;
			}
		break;
		case OT_WXMM32_64:
		case OT_LXMM64_128:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			pop->_size = OP_SIZE::OPR_SZ128;
			rm += SSEREGS_BASE;
		break;

		case OT_WRM32_64:
		case OT_REG32_64_M8:
		case OT_REG32_64_M16:
			if (pds->_vrex.bits & VREX_BIT_B)
				rm += EX_GPR_BASE;
			if (pds->_vrex.bits & VREX_BIT_W)
			{
				pop->_size = OP_SIZE::OPR_SZ64;
				rm += REGS64_BASE;
			}
			else
			{
				pop->_size = OP_SIZE::OPR_SZ32;
				rm += REGS32_BASE;
			}
		break;
		default: throw E_INVALIDARG;
	}
	pop->_regid = (UINT8)rm;
	pop->_type = O_REG;
}

void PEDisAsm::GetOprnd_MemFromRM(OPRND_TYPE type, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, BYTE mod, BYTE rm)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];

	/* -- Memory Indirection Operands (that can be a register) -- */
	pop->_size = OPR_SZ0;
	switch (type)
	{
		case OT_RM_ALL:
			/* PUSH/JMP/CALL are automatically promoted to 64 bits! */
			if (pds->_sizeOp == BIT_MODE_32)
			{
				pop->_size = OPR_SZ32;
				break;
			}
			else if (pds->_sizeOp == BIT_MODE_64)
			{
				/* Mark usage of REX only if it was required. */
				pop->_size = OPR_SZ64;
				break;
			}
			/* FALL THROUGH BECAUSE dt==Decoded16Bits @-<----*/
		case OT_RM16:
			/* If we got here not from OT_RM16, then the prefix was used. */
			pop->_size = OPR_SZ16;
		break;
		case OT_RM32_64:
			/* The default size is 32, which can be 64 with a REX only. */
			if (pds->_sizeOp == BIT_MODE_64)
			{
				pop->_size = OPR_SZ64;
				/* Mark REX prefix as used if non-promoted instruction. */
			}
			else
				pop->_size = OPR_SZ32;
		break;
		case OT_RM16_32:
			/* Ignore REX, it's either 32 or 16 bits RM. */
			if (pds->_prefix & OPAF_PRE_OP_SIZE)
				pop->_size = OPR_SZ16;
			else
				pop->_size = OPR_SZ32;
		break;
		case OT_WXMM32_64:
		case OT_WRM32_64:
			if (pds->_vrex.bits & VREX_BIT_W) pop->_size = OPR_SZ64;
			else pop->_size = OPR_SZ32;
		break;
		case OT_YXMM64_256:
			if (pds->_vrex.bits & VREX_BIT_L) pop->_size = OPR_SZ256;
			else pop->_size = OPR_SZ64;
		break;
		case OT_YXMM128_256:
			if (pds->_vrex.bits & VREX_BIT_L) pop->_size = OPR_SZ256;
			else pop->_size = OPR_SZ128;
		break;
		case OT_LXMM64_128:
			if (pds->_vrex.bits & VREX_BIT_L) pop->_size = OPR_SZ128;
			else pop->_size = OPR_SZ64;
		break;
		case OT_R_ALL_M16:
			pop->_size = OPR_SZ16;
		break;

		case OT_RM8: case OT_R32_M8: case OT_R32_64_M8: case OT_REG32_64_M8:
			pop->_size = OPR_SZ8;
		break;

		case OT_XMM16: case OT_R32_M16: case OT_R32_64_M16: case OT_REG32_64_M16:
			pop->_size = OPR_SZ16;
		break;

		case OT_RM32: case OT_MMX32: case OT_XMM32:
			pop->_size = OPR_SZ32;
		break;

		case OT_MMX64: case OT_XMM64:
			pop->_size = OPR_SZ64;
		break;

		case OT_XMM128:
			pop->_size = OPR_SZ128;
		break;
		case OT_YMM256:
			pop->_size = OPR_SZ256;
		break;
		default:
			throw E_INVALIDARG;
	}

	pop->_type = O_DISP;
	CheckModRM(mod, rm, nOpOrd, pdi, pds);
	if (pdi->_dispS > OPR_SZ0)
		pds->_pos += GetSIntValue(pds, (INT64*)&pdi->_dispV, (int)(1 << (pdi->_dispS - 1)));
}




#define SIB_IDX_BASE_ONLY	4
#define SIB_BASE_REQ_DISP	5
#define RM_REQ_SIB			4

void PEDisAsm::CheckModRM(BYTE mod, BYTE rm, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds, bool bVSib)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];

	if (pds->_sizeAd == BIT_MODE_16)
	{
		if ((mod == 0) && (rm == 6))
		{
			pop->_type = O_DISP;
			pdi->_dispS = OP_SIZE::OPR_SZ16;
		}
		else
		{
			static UINT8 MODS[] = { R_BX, R_BX, R_BP, R_BP, R_SI, R_DI, R_BP, R_BX };
			static UINT8 MODS2[] = { R_SI, R_DI, R_SI, R_DI };
			if (rm < 4)
			{
				pop->_type = O_MEM;
				pop->_regid = MODS2[rm];
				pdi->_index = MODS[rm];
			}
			else
			{
				pop->_type = O_SMEM;
				pop->_regid = MODS[rm];
			}

			if (mod == 1)
				pdi->_dispS = OP_SIZE::OPR_SZ8;
			else if (mod == 2)
				pdi->_dispS = OP_SIZE::OPR_SZ16;
		}
		if ((rm == 2) || (rm == 3) || ((rm == 6) && (mod != 0)))
			PrefixesUseSegment(OPAF_PRE_SS, pds->_prefix, pds->_bits, pdi);
		else
			PrefixesUseSegment(OPAF_PRE_DS, pds->_prefix, pds->_bits, pdi);
	}
	else
	{
		if (mod == 0 && rm == 5)
		{
			pdi->_dispS = OP_SIZE::OPR_SZ32;
			if (pds->_bits == BIT_MODE_64)
			{
				pop->_type = O_SMEM;
				pop->_regid = R_RIP;
			}
			else
				pop->_type = O_DISP;
		}
		else
		{
			BYTE sib = 0;
			if (rm == RM_REQ_SIB)
			{
				CHECK_BOUND(pds, 1);
				sib = pds->_code[pds->_pos++];
				if (bVSib)
					CheckModVSIB(sib, nOpOrd, pdi, pds);
				else
					CheckModSIB(mod, sib, nOpOrd, pdi, pds);
			}
			else
			{
				pop->_type = O_SMEM;
				if (pds->_vrex.bits & VREX_BIT_B)
					rm += EX_GPR_BASE;
				if (pds->_sizeAd == BIT_MODE_64)
					pop->_regid = (UINT8)(REGS64_BASE + rm);
				else
					pop->_regid = (UINT8)(REGS32_BASE + rm);
			}
			if (mod == 1)
				pdi->_dispS = OP_SIZE::OPR_SZ8;
			else if (mod == 2)
				pdi->_dispS = OP_SIZE::OPR_SZ32;
		}
		if ((pop->_regid == R_EBP) || (pop->_regid == R_ESP))
			PrefixesUseSegment(OPAF_PRE_SS, pds->_prefix, pds->_bits, pdi);
		else
			PrefixesUseSegment(OPAF_PRE_DS, pds->_prefix, pds->_bits, pdi);
	}
}

void PEDisAsm::CheckModSIB(BYTE mod, BYTE sib, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];
	BYTE	scale = GET_SCALE_SIB(sib);
	BYTE	index = GET_INDEX_SIB(sib);
	BYTE	base  = GET_BASE_SIB (sib);

	if (pds->_vrex.bits & VREX_BIT_X)
		index += EX_GPR_BASE;

	if (index == SIB_IDX_BASE_ONLY)
		pop->_type = O_SMEM;
	else
		pop->_type = O_MEM;

	bool bReqDisp = (base == SIB_BASE_REQ_DISP);
	base += ((pds->_sizeAd == BIT_MODE_64) ? REGS64_BASE : REGS32_BASE);
	if (pds->_vrex.bits & VREX_BIT_B)
		base += EX_GPR_BASE;
	if (bReqDisp)
	{
		if (mod >= 3) throw E_INVALIDARG;

		if (mod == 0)
		{
			pdi->_dispS = OPR_SZ32;
			if (index == SIB_IDX_BASE_ONLY)
				pop->_type = O_DISP;
			else
				base = REGI_NONE;
		}
		//else
		//	pdi->_dispS = (mod == 1) ? OPR_SZ8 : OPR_SZ32;
	}

	pop->_regid = base;
	pdi->_index = index;
	pdi->_scale = scale;
}

void PEDisAsm::CheckModVSIB(BYTE sib, int nOpOrd, PDECODED_INS pdi, PDEC_STATE pds)
{
	POP_OPRND pop = &pdi->_oprs[nOpOrd];
	BYTE	scale = GET_SCALE_SIB(sib);
	BYTE	index = GET_INDEX_SIB(sib);
	BYTE	base = GET_BASE_SIB(sib);

	if (pds->_vrex.bits & VREX_BIT_L)
		index += AVXREGS_BASE;
	else
		index += SSEREGS_BASE;
	if (pds->_vrex.bits & VREX_BIT_X)
		index += EX_GPR_BASE;

	base += ((pds->_sizeAd == BIT_MODE_64) ? REGS64_BASE : REGS32_BASE);
	if (pds->_vrex.bits & VREX_BIT_B)
		base += EX_GPR_BASE;

	pop->_type = O_MEM;
	pop->_regid = base;
	pdi->_index = index;
	pdi->_scale = scale;
}
