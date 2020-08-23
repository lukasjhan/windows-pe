#include "stdafx.h"
#include "PEDAsm.h"

#define SSE_CMP_MAX_RANGE	8
#define AVX_VCMP_MAX_RANGE	32

static BIT_MODE G_ADSIZE_TBL[] = { BIT_MODE_32, BIT_MODE_16, BIT_MODE_32 };
static BIT_MODE G_OPSIZE_TBL[] = { BIT_MODE_32, BIT_MODE_16, BIT_MODE_16 };


void PEDisAsm::GetEffOpAddrSize(BIT_MODE bm, PDEC_STATE pds)
{
	// 16 -> 32, 32 -> 16, 64 -> 32
	if (pds->_prefix & OPAF_PRE_AD_SIZE)
		pds->_sizeAd = G_ADSIZE_TBL[bm];
	else
		pds->_sizeAd = bm;

	// 16 -> 32, 32 -> 16, 64 -> 16
	if (pds->_prefix & OPAF_PRE_OP_SIZE)
		pds->_sizeOp = G_OPSIZE_TBL[bm];
	else
	{
		if (bm == BIT_MODE_64)
		{
			if (((pds->_opFlags & (OPAF_64BITS | OPAF_PRE_REX)) == OPAF_64BITS) ||
				((pds->_prefix & (OPAF_PRE_REX | OPAF_PRE_VEX)) && (pds->_vrex.bits & VREX_BIT_W)))
				pds->_sizeOp = BIT_MODE_64;
			else
				pds->_sizeOp = BIT_MODE_32;
		}
		else
			pds->_sizeOp = bm;
	}
}

int PEDisAsm::DecodePrefixes(PDEC_STATE pds)
{
	bool bChecked = false;
	BYTE op = pds->_code[pds->_pos];
	do
	{
		if (CHECK_PF_REX(op)) 
		{
			if (pds->_bits != BIT_MODE_64)
				return pds->_pos;

			pds->_prefix |= OPAF_PRE_REX;
			pds->_vrex.bits = (USHORT)(op & 0xf);
		}
		else
		{
			OPBYTE_INFO oi = G_1BCODE_TBL[op];
			if (GET_OP_CAT(oi) != OP_CATEGORY::OC_PREFIX)
				break;

			USHORT uOpIdx = (USHORT)GET_OP_IDX(oi);
			pds->_prefix |= (1 << uOpIdx);
		}
		op = pds->_code[++pds->_pos];
	}
	while (pds->_pos < pds->_size);

	if (op == PFX_CODE::PF_VEX2B || op == PFX_CODE::PF_VEX3B)
	{
		const UINT uIllegal = (OPAF_PRE_OP_SIZE | OPAF_PRE_LOCK | OPAF_PRE_REP | OPAF_PRE_REPNZ | OPAF_PRE_REX);
		if ((pds->_prefix & uIllegal) != 0)
			throw E_INVALIDARG;

		pds->_prefix |= OPAF_PRE_VEX;
		if (op == PFX_CODE::PF_VEX2B)
		{
			CHECK_BOUND(pds, 2);
			if (pds->_bits == BIT_MODE_64 || pds->_code[pds->_pos + 1] >= MODRM_PIVOT_VAL)
			{
				BYTE v1 = pds->_code[pds->_pos + 1];
				if (~v1 & 0x80 && pds->_bits == BIT_MODE_64)
					pds->_vrex.bits |= VREX_BIT_R;
				if (v1 & 0x04)
					pds->_vrex.bits |= VREX_BIT_L;
				pds->_vrex.vv = (~v1 >> 3) & 0x07;
				pds->_vrex.pp = (v1 & 0x03);
				pds->_vrex.mm = 1;
				pds->_pos += 2;
			}
		}
		else
		{
			CHECK_BOUND(pds, 3);
			if (pds->_bits == BIT_MODE_64 || pds->_code[pds->_pos + 1] >= MODRM_PIVOT_VAL)
			{
				pds->_vrex.vex3 = 1;

				BYTE v1 = pds->_code[pds->_pos + 1];
				pds->_vrex.bits = ((~v1 >> 5) & 0x07);
				BYTE mm = (v1 & 0x1F);
				if (mm == VEX_VOP_UD || mm > 3)
					throw E_INVALIDARG;
				pds->_vrex.mm = mm;

				BYTE v2 = pds->_code[pds->_pos + 2];
				if (v2 & 0x80)
					pds->_vrex.bits |= VREX_BIT_W;
				if (v2 & 0x04)
					pds->_vrex.bits |= VREX_BIT_L;
				pds->_vrex.vv = ((~v2 >> 3) & 0x0F);
				pds->_vrex.pp = (v2 & 0x03);
				if (pds->_bits != BIT_MODE_64)
					pds->_vrex.bits &= (~REX_BITS_MASKS & 0x0F);
				pds->_pos += 3;
			}
		}
	}

	return pds->_pos;
}

void PEDisAsm::UpdateInstInfo(PDEC_STATE pds, POP_INFO poi, BYTE modrm, PDECODED_INS pdi)
{
	USHORT uCmpVal = 0;
	if (poi->_flags & OPAF_PSEUDO_OPCODE)
	{
		CHECK_BOUND(pds, 1);
		uCmpVal = pds->_code[pds->_pos++];
		if (poi->_flags & OPAF_PRE_VEX)
		{
			if (uCmpVal >= AVX_VCMP_MAX_RANGE)
				throw E_INVALIDARG;
		}
		else
		{
			if (uCmpVal >= SSE_CMP_MAX_RANGE)
				throw E_INVALIDARG;
		}
	}

	if ((poi->_flags & OPAF_PRE_LOCK) && (pds->_prefix & OPAF_PRE_LOCK))
		pdi->_flags |= FLAG_LOCK;
	else if ((poi->_flags & OPAF_PRE_REPNZ) && (pds->_prefix & OPAF_PRE_REPNZ))
		pdi->_flags |= FLAG_REPNZ;
	else if ((poi->_flags & OPAF_PRE_REP) && (pds->_prefix & OPAF_PRE_REP))
		pdi->_flags |= FLAG_REP;

	if ((poi->_flags & (OPAF_PRE_AD_SIZE | OPAF_USE_EXMNEMONIC)) ==
		(OPAF_PRE_AD_SIZE | OPAF_USE_EXMNEMONIC))
	{
		if (pds->_sizeAd == BIT_MODE_16)
			pdi->_opCode = poi->_opId;
		else if (pds->_sizeAd == BIT_MODE_32)
			pdi->_opCode = ((POP_INFOEX)poi)->_opId2;
		else
			pdi->_opCode = ((POP_INFOEX)poi)->_opId3;
	}
	else if ((poi->_flags & (OPAF_PRE_AD_SIZE | OPAF_NATIVE)) ==
		(OPAF_PRE_AD_SIZE | OPAF_NATIVE))
		pdi->_opCode = poi->_opId;
	else if (pds->_sizeOp == BIT_MODE_16)
	{
		FLAG_SET_OPSIZE(pdi, BIT_MODE_16);
		pdi->_opCode = poi->_opId;
	}
	else if (pds->_sizeOp == BIT_MODE_32)
	{
		FLAG_SET_OPSIZE(pdi, BIT_MODE_32);
		if (poi->_flags & OPAF_USE_EXMNEMONIC)
		{
			if (poi->_flags & OPAF_MNEMONIC_MODRM_BASED)
			{
				if (modrm >= MODRM_PIVOT_VAL)
					pdi->_opCode = poi->_opId;
				else
					pdi->_opCode = ((POP_INFOEX)poi)->_opId2;
			}
			else
				pdi->_opCode = ((POP_INFOEX)poi)->_opId2;
		}
		else
			pdi->_opCode = poi->_opId;
	}
	else
	{
		FLAG_SET_OPSIZE(pdi, BIT_MODE_64);
		if (poi->_flags & (OPAF_USE_EXMNEMONIC | OPAF_USE_EXMNEMONIC2))
		{
			if ((poi->_flags & OPAF_MNEMONIC_MODRM_BASED) && (modrm >= MODRM_PIVOT_VAL))
				throw E_INVALIDARG;

			if ((poi->_flags & OPAF_USE_EXMNEMONIC2) && (pds->_vrex.bits & VREX_BIT_W))
				pdi->_opCode = ((POP_INFOEX)poi)->_opId3;
			else
				pdi->_opCode = ((POP_INFOEX)poi)->_opId2;
		}
		else
			pdi->_opCode = poi->_opId;
	}

	if ((poi->_flags & OPAF_PRE_VEX) &&
		(((((POP_INFOEX)poi)->_flagEx & OPAF_MNEMONIC_VEXW_BASED) &&
		(pds->_vrex.bits & VREX_BIT_W)) ||
		((((POP_INFOEX)poi)->_flagEx & OPAF_MNEMONIC_VEXL_BASED) &&
		(pds->_vrex.bits & VREX_BIT_L))))
		pdi->_opCode = ((POP_INFOEX)poi)->_opId2;

	if (poi->_flags & OPAF_PSEUDO_OPCODE)
		pdi->_opCode = (OP_IID)(poi->_opId + uCmpVal);

	FLAG_SET_ADDRSIZE(pdi, pds->_sizeAd);
	if (poi->_flags & OPAF_DST_WR)
		pdi->_flags |= FLAG_DST_WR;
	pdi->_meta = poi->_class;
	if (pdi->_seg == 0)
		pdi->_seg = REGI_NONE;
}

int PEDisAsm::DecodeInstruction(PDEC_STATE pds, int nCTblIdx, PDECODED_INS pdi)
{
	OPBYTE_INFO* pOpTbl = G_CODE_TBLS[nCTblIdx];
	BYTE		 op = pds->_code[pds->_pos];
	OPBYTE_INFO	 oi = pOpTbl[op];
	OP_CATEGORY	 ct = GET_OP_CAT(oi);
	USHORT		 uOpIdx = GET_OP_IDX(oi);

	while (true)
	{
		if (ct <= OP_CATEGORY::OC_INFOEX)
		{
			if (ct == OP_CATEGORY::OC_NONE)
				throw E_INVALIDARG;
			if (ct == OP_CATEGORY::OC_INFO && pds->_prefix & OPAF_PRE_VEX)
				throw E_INVALIDARG;

			POP_INFO poi = (ct == OC_INFO) ? &OP_INFOS[uOpIdx] : &OP_INFOEXS[uOpIdx];
			if (nCTblIdx == CTBL_IDX_1BCODE)
			{
				if (poi->_opId == OP_IID::OI_ARPL)
				{
					if (pds->_bits == BIT_MODE_64)
						poi = &OP_INFOSPS[2];		// MOVSXD
				}
				else if (poi->_opId == OP_IID::OI_XCHG && op == OPCODE_NOP)
				{
					if (pds->_prefix & OPAF_PRE_REP)
						poi = &OP_INFOSPS[1];		// PAUSE
					else
					{
						if (pds->_bits != BIT_MODE_64 || (pds->_vrex.bits & VREX_BIT_B) == 0)
							poi = &OP_INFOSPS[0];	// NOP
					}
				}
				else if (poi->_opId == OP_IID::OI_LEA)
					pds->_prefix &= ~OPAF_PRE_SEGOVRD_MASK;
			}
			if ((pds->_bits == BIT_MODE_64) && (poi->_flags & OPAF_INVALID_64BITS))
				throw E_NOT_VALID_STATE;
			if ((pds->_bits != BIT_MODE_64) && (poi->_flags & OPAF_64BITS_FETCH))
				throw E_NOT_VALID_STATE;

			BYTE modrm = 0;
			if (poi->_flags & OPAF_MODRM_REQUIRED)
			{
				if (~poi->_flags & OPAF_MODRM_INCLUDED)
				{
					CHECK_BOUND(pds, 1);
					pds->_pos++;
				}
				modrm = pds->_code[pds->_pos];

				if ((poi->_flags & OPAF_FORCE_REG0) && GET_REG_MODRM(modrm) != 0)
					throw E_NOT_VALID_STATE;
				if ((poi->_flags & OPAF_MODRR_REQUIRED) && (modrm < MODRM_PIVOT_VAL))
					throw E_NOT_VALID_STATE;
			}
			CHECK_BOUND(pds, 1); pds->_pos++;

			pds->_opFlags = poi->_flags;
			GetEffOpAddrSize(pds->_bits, pds);
			OPRND_TYPE oprs[] = { poi->_or1, poi->_or2, OT_NONE, OT_NONE };
			if (ct == OC_INFOEX)
			{
				oprs[2] = POP_INFOEX(poi)->_or3;
				oprs[3] = POP_INFOEX(poi)->_or4;
			}

			for (int i = 0; i < MAX_OPRNDS_COUNT; i++)
			{
				OPRND_TYPE ot = oprs[i];
				if (ot == OT_NONE)
					break;

				int otCat = OPR_TYPE_CATS[ot];
				if (otCat == OTCAT_OP_REG)
					modrm = (USHORT)pds->_code[pds->_pos - 1];
				else if (otCat == OTCAT_REG_FROM_IMM)
				{
					CHECK_BOUND(pds, 1);
					modrm = pds->_code[pds->_pos++];
				}
				else if (otCat == OTCAT_READ_VAL && op == OPCODE_ENTER)
					modrm = 1;
				G_PFN_OPRANDS[otCat](ot, i, pdi, pds, modrm);
			}
			UpdateInstInfo(pds, poi, modrm, pdi);
			break;
		}

		if (ct == OP_CATEGORY::OC_WITHPF)
		{
			int nNext = 0;
			if (pds->_prefix & OPAF_PRE_VEX)
				nNext = 4 + pds->_vrex.pp;
			else
			{
				if (pds->_prefix & OPAF_PRE_OP_SIZE)
					nNext = 1;
				else if (pds->_prefix & OPAF_PRE_REP)
					nNext = 2;
				else if (pds->_prefix & OPAF_PRE_REPNZ)
					nNext = 3;
			}
			uOpIdx += nNext;
		}
		else
		{
			CHECK_BOUND(pds, 1);
			op = pds->_code[++pds->_pos];
			switch (ct)
			{
				case OP_CATEGORY::OC_VEX_C0:
				{
					if (op < MODRM_PIVOT_VAL)
						uOpIdx++;
				}
				break;
				case OP_CATEGORY::OC_GRP_VEX:
					if (~pds->_prefix & OPAF_PRE_VEX)
						throw E_INVALIDARG;
				case OP_CATEGORY::OC_GROUP:
				case OP_CATEGORY::OC_GRP_C0:
					uOpIdx += GET_REG_MODRM(op);
					if (ct == OP_CATEGORY::OC_GRP_C0 && op >= MODRM_PIVOT_VAL)
						uOpIdx += 8;
				break;
				case OP_CATEGORY::OC_FULL:
					uOpIdx += (op < MODRM_PIVOT_VAL) ? 
						GET_REG_MODRM(op) : (op - MODRM_PIVOT_VAL + 8);
				break;
				case OP_CATEGORY::OC_EXT:
				{
					nCTblIdx = uOpIdx;
					uOpIdx = op;
				}
				break;
			}
		}
		oi = G_CODE_TBLS[nCTblIdx][uOpIdx];
		ct = GET_OP_CAT(oi);
		uOpIdx = GET_OP_IDX(oi);
	}
	return pds->_pos;
}

int PEDisAsm::DisAssemble(PDECODED_INS pdis, int& ndiCnt, const BYTE* pCode, DWORD dwOffset, int nSize, bool bIs32)
{
	int nCodeSize = nSize;
	int nCodeIdx  = (int)dwOffset;
	BIT_MODE bm	  = (!bIs32) ? BIT_MODE_64 : BIT_MODE_32;

	int i = 0;
	for (; i < ndiCnt && nCodeSize > 0; i++)
	{
		PDECODED_INS pdi = &pdis[i];
		memset(pdi, 0, sizeof(DECODED_INS));

		int nSize = (nCodeSize > MAX_INST_LENGTH) ? MAX_INST_LENGTH : nCodeSize;
		int nReadBytes = 0;
		try
		{
			DEC_STATE ds(bm, (PBYTE)pCode + nCodeIdx, nSize);
			pdi->_offset = nCodeIdx;
			DecodePrefixes(&ds);

			int nTblIdx = (ds._prefix & OPAF_PRE_VEX) ? ds._vrex.mm : CTBL_IDX_1BCODE;
			nReadBytes = DecodeInstruction(&ds, nTblIdx, pdi);
			pdi->_count = nReadBytes;
		}
		catch (HRESULT/* hr*/)
		{
			DWORD dwPreIdx = pdi->_offset;
			memset(pdi, 0, sizeof(DECODED_INS));
			pdi->_offset = dwPreIdx;
			pdi->_opCode = OI_UNKNOWN;
			pdi->_flags = FLAG_NOT_DECODABLE;
			pdi->_count = 1;
			nReadBytes = 1;
		}
		nCodeIdx += nReadBytes , nCodeSize -= nReadBytes;
	}
	ndiCnt = i;
	return (nCodeIdx - dwOffset);
}
