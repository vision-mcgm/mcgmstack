///////////////
//
//
// StopWatch.cpp
//
// StopWatch Class declaration
//
// Jason Dale 22/10/2000
////////////////////////////////////////////
////////////////////////////////////////////


//#include "stdafx.h"
#include "CStopWatch.h"

//////////////////////////
//Construction
CStopWatch::CStopWatch(void)
{
	m_bInit=false;
	m_nFreq=0;
	m_nTimerStart=0;
	m_nTimerRead=0;
	m_nTime=0;
	m_fTime=0.0f;
	Reset();
}

CStopWatch::~CStopWatch(void)
{
}

//////////////////////////
//Reset Must be called first
bool CStopWatch::Reset(void)
{
	m_nTimerStart=(_int64)0;
	m_nTimerRead=(_int64)0;
	LARGE_INTEGER liFreq;
	if(QueryPerformanceFrequency(&liFreq)==0){
		return false;
	}
	m_nFreq=(_int64)liFreq.QuadPart;
	//TRACE("clock speed: %d\n", m_nFreq);
	m_bInit=true;
	return true;
}

//Counter resolution.  Current PIV timer is small enough to easily fit in a long
long CStopWatch::Resolution(void)
{
	long lfreq= (long)(m_nFreq);
	//TRACE2("Performance Freq=%d		Resolution=%f ms\n",lfreq,1000.0/lfreq);
	return lfreq;
}

//////////////////////////
void CStopWatch::Start(void){
	if (m_bInit!=true){
		Reset();
		if (m_bInit!=true){return;}
	}
	LARGE_INTEGER liStart;
	QueryPerformanceCounter(&liStart);
	m_nTimerStart=(_int64)liStart.QuadPart;
}


//////////////////////////
float CStopWatch::Read(void){
	if (m_bInit!=true){return (0.0f);}
	LARGE_INTEGER liRead;
	QueryPerformanceCounter(&liRead);
	m_nTimerRead=(_int64)liRead.QuadPart;

	m_nTime=(m_nTimerRead-m_nTimerStart);
	m_fTime=(float)((1000.0*m_nTime)/m_nFreq);
	
	return (m_fTime);
}

void CStopWatch::Display(void)
{
	//CString disp;
	//disp.Format("StopWatch = %3.4f",Read());
	//AfxMessageBox(disp,MB_OK);
}