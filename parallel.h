// parallel.h: interface for the CParPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARALLEL_H__E896C809_C0DA_4158_823F_FA4851EB0527__INCLUDED_)
#define AFX_PARALLEL_H__E896C809_C0DA_4158_823F_FA4851EB0527__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <conio.h>

#define POLL_S7_FOR_BUSY
#ifdef POLL_S7_FOR_BUSY
#define iBUSY_VAL		0x80
#define oSTROBE_VAL		0x10
#else
#define iBUSY_VAL		0x40
#define oSTROBE_VAL		0x8
#endif

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
	void SetBaseAddr( WORD base );
    WORD GetBaseAddr() const;
	CParPort::EPorType GetPorType() const;
	void WriteDataPort( WORD value ) const;
    BYTE ReadStatusPort() const;
    static void DataPortWrite( WORD base, WORD value );
	static BYTE StatusPortRead( WORD base );
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
	LONG GetParallelControllerKey( CString& rKey );
	LONG GetLptPortInTheRegistry( int myPort );
	void Setup();
	BOOL PortIsPresent() const;
    WORD GetBaseAddr() const;
	void EnterIdleCondition() const;
    BOOL DetectTheGuest() const;
	static BYTE MakeByteFromNibbles( BYTE bLow, BYTE bHigh );
private:
	CParPort m_cParaport;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};


// Writes a byte to a parallel port's data port.
inline void CParPort::DataPortWrite( WORD base, WORD value )
	{ _outp( base, value ); }
// Reads a parallel port's status port. And inverts bit 7 of the byte read
// for that the status-port hardware reinverts these bits.
inline BYTE CParPort::StatusPortRead( WORD base )
	{ return _inp( base + 1 ) ^ 0x80; }

inline WORD CParPort::GetBaseAddr() const
	{ ASSERT_VALID( this ); return m_wBaseAddress; }
inline CParPort::EPorType CParPort::GetPorType() const
	{ ASSERT_VALID( this ); return (EPorType) m_nPortType; }
inline void CParPort::WriteDataPort( WORD value ) const
    { ASSERT_VALID( this ); DataPortWrite( m_wBaseAddress, value ); }
inline BYTE CParPort::ReadStatusPort() const
	{ ASSERT_VALID( this ); return StatusPortRead( m_wBaseAddress ); }

inline CNibbleModeProto::CNibbleModeProto()
	{ }
inline BOOL CNibbleModeProto::PortIsPresent() const
	{ ASSERT_VALID( this ); return (m_cParaport.GetPorType() != CParPort::ptNONE); }
inline WORD CNibbleModeProto::GetBaseAddr() const
	{ ASSERT_VALID( this ); return m_cParaport.GetBaseAddr(); }

#endif // !defined(AFX_PARALLEL_H__E896C809_C0DA_4158_823F_FA4851EB0527__INCLUDED_)
