// PortTalk.h: interface for the CPortTalk class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORTTALK_H__FA21F7FE_D3F8_4D8A_B5C8_C6B3C5BDBF85__INCLUDED_)
#define AFX_PORTTALK_H__FA21F7FE_D3F8_4D8A_B5C8_C6B3C5BDBF85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPortTalk : public CObject  
{
public:
	enum {
		Success = 0,
		InvalidHandleValue,
		IoControlError,
		InvalidHandleLPT1
	};
	CPortTalk();
	virtual ~CPortTalk();
	int OpenPortTalk();
	void ClosePortTalk();
	int EnableIOPM(WORD wOffset);
	void ParAllocPort();
	void ParFreePort();
protected:
	void StartPortTalkDriver() const;
	BOOL InstallPortTalkDriver() const;
	DWORD m_dwProcessId;
	UINT m_nAllocPortCounter;
	HANDLE m_hPortTalk;
	HANDLE m_hLPT1;
};

#endif // !defined(AFX_PORTTALK_H__FA21F7FE_D3F8_4D8A_B5C8_C6B3C5BDBF85__INCLUDED_)
