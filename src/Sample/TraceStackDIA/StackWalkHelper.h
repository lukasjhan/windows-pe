#pragma once
#define USE_VISUALSTUDIO_DIA_SDK14
#include "Dia2.h"

class YhdStkWlkHlp : public IDiaStackWalkHelper 
{
private:
	long m_refCount;

	CONTEXT m_ctx;

public:
	YhdStkWlkHlp() { m_refCount = 0;  }

public:
	// IUnknown
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		InterlockedIncrement(&m_refCount);
		return m_refCount;
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		InterlockedDecrement(&m_refCount);
		if (m_refCount == 0)
		{
			delete this;
			return 0;
		}
		return m_refCount;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		if (!ppvObject)
		{
			return E_INVALIDARG;
		}

		if (riid == __uuidof(IDiaLoadCallback))
		{
			*ppvObject = (IDiaLoadCallback*)this;
			AddRef();
			return S_OK;
		}
		else if (riid == __uuidof(IUnknown))
		{
			*ppvObject = (IUnknown*)this;
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

public:

	STDMETHOD(get_registerValue)(DWORD index, ULONGLONG *pRetVal)
	{
		if (!pRetVal)
		{
			return E_INVALIDARG;
		}
		//_assert(m_fInitialized);

		switch (index)
		{
		case CV_REG_EAX:
		case CV_REG_ECX:
		case CV_REG_EDX:
		case CV_REG_EBX:
		case CV_REG_ESP:
		case CV_REG_EBP:
		case CV_REG_EIP:
		case CV_REG_ESI:
		case CV_REG_EDI:
		case CV_REG_EFLAGS:
		case CV_REG_CS:
		case CV_REG_FS:
		case CV_REG_ES:
		case CV_REG_DS:

			*pRetVal = m_rgRegisters[index];
			return NOERROR;
		default:
			assert(!"Unknown register being asked for.");
			return E_FAIL;
		}
	}

	STDMETHOD(put_registerValue)(DWORD index, ULONGLONG NewVal)
	{
		//ASSERT(m_fInitialized);
		switch (index)
		{
		case CV_REG_EAX:
		case CV_REG_ECX:
		case CV_REG_EDX:
		case CV_REG_EBX:
		case CV_REG_ESP:
		case CV_REG_EBP:
		case CV_REG_EIP:
		case CV_REG_ESI:
		case CV_REG_EDI:
		case CV_REG_EFLAGS:
		case CV_REG_CS:
		case CV_REG_FS:
		case CV_REG_ES:
		case CV_REG_DS:
			m_rgRegisters[index] = NewVal;
			return NOERROR;
		default:
			//assert(!"Unknown register being set");
			return E_FAIL;
		}
	}

	STDMETHOD(readMemory)(MemoryTypeEnum type, ULONGLONG va, DWORD cbData,
		DWORD* pcbData, BYTE* pbData)
	{
		bool fContinue = true;
		BYTE* pCurrByte = pbData;
		ULONGLONG currVa = va;
		DWORD totalRead = 0;

		while (totalRead < cbData)
		{
			DWORD cActual;
			if (!::ReadProcessMemory(m_hProcess, (LPVOID)currVa, pCurrByte, 1, &cActual))
			{
				// We've read as far as we can.
				fContinue = false;
				break;
			}
			totalRead += cActual;
			currVa++;
			pCurrByte++;
		}

		if (totalRead == 0)
		{
			// If we failed to read anything, return an error;
			return E_FAIL;
		}

		*pcbData = totalRead + 1;

		return S_OK;
	}

    STDMETHOD(searchForReturnAddress)(IDiaFrameData* frame, ULONGLONG* returnAddress)
	{
		return E_NOTIMPL;
	}
    
    STDMETHOD(searchForReturnAddressStart)( 
        IDiaFrameData *frame,
        ULONGLONG startAddress,
		ULONGLONG *returnAddress) {
		return E_NOTIMPL;
	}
    
    STDMETHOD(frameForVA)(ULONGLONG va, IDiaFrameData** ppFrame)
	{
		CComPtr<IDiaEnumFrameData> pDiaEnumFrameData;
		HRESULT hr = m_pSymbolEngine->GetEnumFrameData(&pDiaEnumFrameData);
		if (SUCCEEDED(hr))
			hr = pDiaEnumFrameData->frameByVA(va, ppFrame);
		return hr;
	}
    
    STDMETHOD(symbolForVA)(ULONGLONG va, IDiaSymbol** ppSymbol)
	{
		return m_pSymbolEngine->SymbolForVA(va, ppSymbol);
	}

    STDMETHOD(pdataForVA)(ULONGLONG va, DWORD cbData, DWORD* pcbData, BYTE* pbData)
	{
		return E_NOTIMPL;
	}
    
    STDMETHOD(imageForVA)(ULONGLONG vaContext, ULONGLONG* pvaImageStart) 
	{
		bool fFound = false;
		for (int i = 0; i < m_cModuleInfos; i++)
		{
			if ((vaContext >= m_pModuleInfos[i].BaseAddress) &&
				(vaContext < m_pModuleInfos[i].BaseAddress + m_pModuleInfos[i].Size))
			{
				*pvaImageStart = m_pModuleInfos[i].BaseAddress;
				fFound = true;
				break;
			}
		}
		return (!fFound) ? S_FALSE : S_OK;
	}

	STDMETHOD(addressForVA)(ULONGLONG va, DWORD* pISect, DWORD* pOffset)
	{
		return S_FALSE;
	}

#ifdef USE_VISUALSTUDIO_DIA_SDK14
	STDMETHOD(numberOfFunctionFragmentsForVA)(ULONGLONG vaFunc, DWORD cbFunc, DWORD* pNumFragments)
	{
		return S_FALSE;
	}

	STDMETHOD(functionFragmentsForVA)(ULONGLONG vaFunc, DWORD cbFunc, 
		DWORD cFragments, ULONGLONG* pVaFragment, DWORD* pLenFragment)
	{
		return S_FALSE;
	}
#endif
};
