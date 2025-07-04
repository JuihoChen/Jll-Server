// parallel.h: interface for the CParPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARALLEL_H__E896C809_C0DA_4158_823F_FA4851EB0527__INCLUDED_)
#define AFX_PARALLEL_H__E896C809_C0DA_4158_823F_FA4851EB0527__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CParPort : public CObject  
{
public:
	enum EPorType {
		ptNONE = 0x0000,
		ptECP  = 0x0001,
		ptEPP  = 0x0002,
		ptPS2  = 0x0004,
		ptSPP  = 0x0008,
		ptICTL = 0x0100		// Control supports input for 8-bit transfer
	};

	CParPort();
	virtual ~CParPort();
	void SetBaseAddr( WORD base );
    WORD GetBaseAddr() const;
	CParPort::EPorType GetPorType() const;
protected:
	BOOL TestPort();
private:
	int m_nPortType;
	WORD m_wBaseAddress;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};


class CNibbleModeProto : public CObject  
{
public:
	CNibbleModeProto();
	virtual ~CNibbleModeProto();
	BOOL PortIsPresent() const;
    WORD GetBaseAddr() const;
	LONG GetParallelControllerKey( CString& rKey );
	LONG GetLptPortInTheRegistry( int myPort );
private:
	CParPort m_cParaport;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

inline WORD CParPort::GetBaseAddr() const
	{ ASSERT_VALID( this ); return m_wBaseAddress; }
inline CParPort::EPorType CParPort::GetPorType() const
	{ ASSERT_VALID( this ); return (EPorType) m_nPortType; }

inline BOOL CNibbleModeProto::PortIsPresent() const
	{ ASSERT_VALID( this ); return (m_cParaport.GetPorType() != CParPort::ptNONE); }
inline WORD CNibbleModeProto::GetBaseAddr() const
	{ ASSERT_VALID( this ); return m_cParaport.GetBaseAddr(); }

#endif // !defined(AFX_PARALLEL_H__E896C809_C0DA_4158_823F_FA4851EB0527__INCLUDED_)
