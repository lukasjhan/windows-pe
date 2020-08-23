#include "stdafx.h"
#include "PEDAsm.h"


UINT16 PEDisAsm::OP_SIZETOINT[9] =
{
	0, 8, 16, 32, 64, 128, 256, 48, 80
};
PCSTR PEDisAsm::OP_SIZETOSTR[9] = 
{
	"", 
	"BYTE", "WORD", "DWORD", 
	"QWORD",	// or "MMWORD"
	"OWORD",	// or "XMMWORD" 
	"YMMWORD", 
	"FWORD",
	"TBYTE",
};
// 48BIT : FWORD

#pragma warning(push)
#pragma warning(disable : 4996)

void PEDisAsm::FormatSize(PSTR pszOpers, PDECODED_INS pdi, int opNum)
{
	/* We only have to output the size explicitly if it's not clear from the operands.
	 * For example:
	 * mov al, [0x1234] -> The size is 8, we know it from the AL register operand.
	 * mov [0x1234], 0x11 -> Now we don't know the size. Pam pam pam
	 * If given operand number is higher than 2, then output the size anyways. */
	bool bReqSizing = ((opNum > 1) || 
		((pdi->_oprs[0]._type != O_REG) && (pdi->_oprs[1]._type != O_REG)));

	/* Still not sure? Try some special instructions. */
	if (!bReqSizing)
	{
		/* INS/OUTS are exception, because DX is a port specifier and not a real src/dst register.
		 * A few exceptions that always requires sizing:
		 * MOVZX, MOVSX, MOVSXD.
		 * ROL, ROR, RCL, RCR, SHL, SHR, SAL, SAR.
		 * SHLD, SHRD. */
		switch (pdi->_opCode)
		{
			case OI_INS:
			case OI_OUTS:
			case OI_MOVZX:
			case OI_MOVSX:
			case OI_MOVSXD:
			case OI_ROL:
			case OI_ROR:
			case OI_RCL:
			case OI_RCR:
			case OI_SHL:
			case OI_SHR:
			case OI_SAL:
			case OI_SAR:
			case OI_SHLD:
			case OI_SHRD: bReqSizing = true; break;
			default: /* Instruction doesn't require sizing. */ break;
		}
	}
	bReqSizing = true;
	if (bReqSizing)
	{
		strcat(pszOpers, PEDisAsm::OP_SIZETOSTR[pdi->_oprs[opNum]._size]);
		strcat(pszOpers, " ");
	}
}

void PEDisAsm::DecodedFormat(PDECODED_INS pdi, PDECODED pd, bool bIs32)
{
	char	szMNemo[64] = { 0, };
	char	szOpers[256] = { 0, };
	UINT64	llAddrMask = (bIs32) ? (UINT64)-1 : 0xffffffff;

	pd->Count  = pdi->_count;
	pd->Offset = pdi->_offset;
	if (pdi->_flags == FLAG_NOT_DECODABLE)
	{
		sprintf(szMNemo + strlen(szMNemo), "DB %02xh", pdi->_imm.byte);
		goto $LABEL_ENDPROC;	/* Skip to next instruction. */
	}

	if (pdi->_flags & OPAF_PRE_LOCK)
		strcpy(szMNemo, "LOCK ");
	else if (pdi->_flags & OPAF_PRE_REP)
	{
		/* REP prefix for CMPS and SCAS is really a REPZ. */
		if ((pdi->_opCode == OI_CMPS) || (pdi->_opCode == OI_SCAS))
			strcpy(szMNemo, "REPZ ");
		else
			strcpy(szMNemo, "REP ");
	}
	else if (pdi->_flags & OPAF_PRE_REPNZ)
		strcpy(szMNemo, "REPNZ ");
	if (pdi->_opCode == OI_UNKNOWN)
		strcat(szMNemo, "DB");
	else
		strcat(szMNemo, G_MNEMONICS[pdi->_opCode]);

	if ((META_GET_ISC(pdi->_meta) == ISC_INTEGER) &&
		((pdi->_opCode == OI_MOVS) || (pdi->_opCode == OI_CMPS) ||
		 (pdi->_opCode == OI_STOS) || (pdi->_opCode == OI_LODS) ||
		 (pdi->_opCode == OI_SCAS)))
	{
		/*
		* No operands are needed if the address size is the default one,
		* and no btSegment is overridden, so add the suffix letter,
		* to indicate size of operation and continue to next instruction.
		*/
		//if ((FLAG_GET_ADDRSIZE(pdi->_flags) == m_bits) && (SEGR_IS_DEFAULT(pdi->_seg)))
		if (SEGR_IS_DEFAULT(pdi->_seg))
		{
			switch (pdi->_oprs[0]._size)
			{
				case OP_SIZE::OPR_SZ8 : strcat(szMNemo, "B"); break;
				case OP_SIZE::OPR_SZ16: strcat(szMNemo, "W"); break;
				case OP_SIZE::OPR_SZ32: strcat(szMNemo, "D"); break;
				case OP_SIZE::OPR_SZ64: strcat(szMNemo, "Q"); break;
			}
			goto $LABEL_ENDPROC;
		}
	}

	for (UINT i = 0; (i < MAX_OPRNDS_COUNT) && (pdi->_oprs[i]._type != O_NONE); i++)
	{
		if (i > 0)
			strcat(szOpers, ", ");

		switch (pdi->_oprs[i]._type)
		{
			case O_REG:
				strcat(szOpers, G_REGISTERS[pdi->_oprs[i]._regid]);
			break;

			case O_IMM:
				if (pdi->_opCode == OI_ENTER)
				{
					sprintf(szOpers + strlen(szOpers), "%xh", 
						(i == 0) ? pdi->_imm.ex.i1 : pdi->_imm.ex.i2);
					break;
				}
				if ((pdi->_flags & FLAG_IMM_SIGNED) && (pdi->_oprs[i]._size == OP_SIZE::OPR_SZ8))
				{
					if (pdi->_imm.sbyte < 0)
					{
						sprintf(szOpers + strlen(szOpers), "-%xh", -pdi->_imm.sbyte);
						break;
					}
				}
				if (pdi->_oprs[i]._size == OP_SIZE::OPR_SZ64)
					sprintf(szOpers + strlen(szOpers), "%I64xh", pdi->_imm.qword);
				else
					sprintf(szOpers + strlen(szOpers), "%xh", pdi->_imm.dword);
			break;

			case O_DISP:
			case O_SMEM:
			case O_MEM:
			{
				bool	bIsDefault = true;
				UINT8	btSegment = SEGR_DEFAULT;

				FormatSize(szOpers, pdi, i);
				if (pdi->_oprs[i]._size != OP_SIZE::OPR_SZ0)
					strcat(szOpers, "PTR ");

				btSegment = SEGR_GET(pdi->_seg);
				bIsDefault = SEGR_IS_DEFAULT(pdi->_seg);
				if (pdi->_oprs[i]._type == O_SMEM)
				{
					switch (pdi->_opCode)
					{
						case OI_MOVS:
							bIsDefault = false;
							if (i == 0)
								btSegment = R_ES;
						break;
						case OI_CMPS:
							bIsDefault = false;
							if (i == 1)
								btSegment = R_ES;
						break;
						case OI_INS:
						case OI_LODS:
						case OI_STOS:
						case OI_SCAS: bIsDefault = false; break;
					}
				}
				if (!bIsDefault && (btSegment != REGI_NONE))
				{
					strcat(szOpers, G_REGISTERS[SEGR_GET(pdi->_seg)]);
					strcat(szOpers, ":");
				}
				strcat(szOpers, "[");
				if (pdi->_oprs[i]._type != O_DISP && pdi->_oprs[i]._regid != REGI_NONE)
					strcat(szOpers, G_REGISTERS[pdi->_oprs[i]._regid]);
				if (pdi->_oprs[i]._type == O_MEM)
				{
					if (pdi->_index != REGI_NONE)
					{
						if (pdi->_oprs[i]._regid != REGI_NONE)
							strcat(szOpers, "+");
						strcat(szOpers, G_REGISTERS[pdi->_index]);
						if (pdi->_scale > 0)
						{
							strcat(szOpers, "*");
							if (pdi->_scale == 1) strcat(szOpers, "2");
							else if (pdi->_scale == 2) strcat(szOpers, "4");
							else /* if (pdi->_scale == 3) */ strcat(szOpers, "8");
						}
					}
				}
				if (pdi->_dispS > OP_SIZE::OPR_SZ0)
				{
					UINT64	llAddrMask = (bIs32) ? (UINT64)-1 : 0xffffffff;
					INT64	llTempDisp64 = 0LL;

					if ((INT64)pdi->_dispV < 0)
					{
						llTempDisp64 = -(INT64)pdi->_dispV;
						strcat(szOpers, "-");
					}
					else
					{
						llTempDisp64 = pdi->_dispV;
						if (pdi->_oprs[i]._type != O_DISP)
							strcat(szOpers, "+");
					}
					llTempDisp64 &= llAddrMask;
					sprintf(szOpers + strlen(szOpers), "%I64xh", llTempDisp64);
				}
				strcat(szOpers, "]");
			}
			break;

			case O_PC:
				if (pdi->_imm.sqword < 0)
					sprintf(szOpers + strlen(szOpers), "-%xh", -pdi->_imm.sqword);
				else
					sprintf(szOpers + strlen(szOpers), "%xh", pdi->_imm.sqword);
			break;

			case O_PTR:
				sprintf(szOpers + strlen(szOpers), 
					"%04xh:%08h", pdi->_imm.ptr.seg, pdi->_imm.ptr.off);
			break;
		}
	}

$LABEL_ENDPROC:
	if (pd->Mnemonic != NULL)
		delete[] pd->Mnemonic;
	if (szMNemo[0] != 0)
	{
		pd->Mnemonic = new char[strlen(szMNemo) + 1];
		strcpy(pd->Mnemonic, szMNemo);
	}
	else
		pd->Mnemonic = NULL;

	if (pd->Operands != NULL)
		delete[] pd->Operands;
	if (szOpers[0] != 0)
	{
		pd->Operands = new char[strlen(szOpers) + 1];
		strcpy(pd->Operands, szOpers);
	}
	else
		pd->Operands = NULL;
}
#pragma warning(pop)
