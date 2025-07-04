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
		IoControlError
	};
	CPortTalk();
	virtual ~CPortTalk();
	int OpenPortTalk();
	void ClosePortTalk();
	int EnableIOPM(WORD wOffset);
protected:
	void StartPortTalkDriver();
	HANDLE m_hPortTalk;
	DWORD m_dwProcessId;
};

#endif // !defined(AFX_PORTTALK_H__FA21F7FE_D3F8_4D8A_B5C8_C6B3C5BDBF85__INCLUDED_)
