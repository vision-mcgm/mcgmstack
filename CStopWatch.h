////////////////////////////////////////////
////////////////////////////////////////////
//
//	StopWatch.h
//
//	StopWatch Class header and declaration
//
//	handles the Performance Timer for accurate profiling	
//	
//	Jason Dale 22/10/2000
////////////////////////////////////////////
////////////////////////////////////////////
#include <windows.h>

#ifndef CSTOPWATCH
#define CSTOPWATCH


class CStopWatch
{
public:
	//member functions
	CStopWatch();
	~CStopWatch();
	
	void Start(void);	//Start the Clock
	float Read(void);	//Read the clock
	void Display(void);	//Display Time in Msg Box
	long Resolution(void);
private:
	bool Reset(void);	//Initialise and Reset Timer
	bool m_bInit;			//True if perf counter is available
	_int64 m_nTimerStart;	//Clock count when started
	_int64 m_nTimerRead;	//Clock count when read
	_int64 m_nFreq;			//Clock Frequency
	_int64 m_nTime;			//Elapsed Time
	float m_fTime;			//Time in ms
};

#endif