/*****************************************************************
|
|      Neptune - System :: Posix Implementation
|
|      (c) 2001-2003 Gilles Boccon-Gibod
|      Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <memory.h>

#include "NptConfig.h"
#include "NptTypes.h"
#include "NptSystem.h"
#include "NptResults.h"
#include "NptDebug.h"
#include "NptThreads.h"

/*----------------------------------------------------------------------
|   NPT_PosixSystem
+---------------------------------------------------------------------*/
class NPT_PosixSystem
{
public:
    // class variables
    static NPT_PosixSystem System;
    
    // methods
    NPT_PosixSystem();
   ~NPT_PosixSystem();

    // members
    pthread_mutex_t m_SleepMutex;
    pthread_cond_t  m_SleepCondition;
};
NPT_PosixSystem NPT_PosixSystem::System;

/*----------------------------------------------------------------------
|   NPT_PosixSystem::NPT_PosixSystem
+---------------------------------------------------------------------*/
NPT_PosixSystem::NPT_PosixSystem()
{
    pthread_mutex_init(&m_SleepMutex, NULL);
    pthread_cond_init(&m_SleepCondition, NULL);
}

/*----------------------------------------------------------------------
|   NPT_PosixSystem::~NPT_PosixSystem
+---------------------------------------------------------------------*/
NPT_PosixSystem::~NPT_PosixSystem()
{
    pthread_cond_destroy(&m_SleepCondition);
    pthread_mutex_destroy(&m_SleepMutex);
}

/*----------------------------------------------------------------------
|   NPT_System::GetProcessId
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::GetProcessId(NPT_UInt32& id)
{
    id = getpid();
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_System::GetCurrentTimeStamp
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::GetCurrentTimeStamp(NPT_TimeStamp& now)
{
    struct timeval now_tv;

    // get current time from system
    if (gettimeofday(&now_tv, NULL)) {
        now.SetNanos(0);
        return NPT_FAILURE;
    }
    
    // convert format
    now.SetNanos((NPT_UInt64)now_tv.tv_sec  * 1000000000 + 
                 (NPT_UInt64)now_tv.tv_usec * 1000);

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_System::Sleep
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::Sleep(const NPT_TimeInterval& duration)
{
    struct timespec time_req;
    struct timespec time_rem;
    int             result;

    // setup the time value
    time_req.tv_sec  = duration.ToNanos()/1000000000;
    time_req.tv_nsec = duration.ToNanos()%1000000000;

    // sleep
    do {
        result = nanosleep(&time_req, &time_rem);
        time_req = time_rem;
    } while (result == -1 && errno == EINTR);

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_System::SleepUntil
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::SleepUntil(const NPT_TimeStamp& when)
{
    struct timespec timeout;
    struct timeval  now;
    int             result;

    // get current time from system
    if (gettimeofday(&now, NULL)) {
        return NPT_FAILURE;
    }

    // setup timeout
    NPT_UInt64 limit = (NPT_UInt64)now.tv_sec*1000000000 +
                       (NPT_UInt64)now.tv_usec*1000 +
                       when.ToNanos();
    timeout.tv_sec  = limit/1000000000;
    timeout.tv_nsec = limit%1000000000;

    // sleep
    do {
        result = pthread_cond_timedwait(&NPT_PosixSystem::System.m_SleepCondition, 
                                        &NPT_PosixSystem::System.m_SleepMutex, 
                                        &timeout);
        if (result == ETIMEDOUT) {
            return NPT_SUCCESS;
        }
    } while (result == EINTR);

    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   NPT_System::SetRandomSeed
+---------------------------------------------------------------------*/
NPT_Result  
NPT_System::SetRandomSeed(unsigned int seed)
{
    srand(seed);
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_System::GetRandomInteger
+---------------------------------------------------------------------*/
NPT_UInt32 
NPT_System::GetRandomInteger()
{
    static bool seeded = false;
    if (seeded == false) {
        NPT_TimeStamp now;
        GetCurrentTimeStamp(now);
        SetRandomSeed((NPT_UInt32)now.ToNanos());
        seeded = true;
    }

    return rand();
}

/*----------------------------------------------------------------------
|   NPT_System::GenerateUUID
+---------------------------------------------------------------------*/

static NPT_Mutex g_uuidLock;
static NPT_UInt64 g_uuidLastTime = 0;
static NPT_UInt16 g_uuidSeq;
static NPT_UInt8 g_uuidNodeId[6];

NPT_Result NPT_System::GenerateUUID(NPT_UInt8 data[16])
{
	NPT_UInt64 ts;

	timeval now;
	gettimeofday(&now, NULL);
	ts = now.tv_sec;
	ts *= 1000000;
	ts += now.tv_usec;

	NPT_UInt16 seqpart;

	g_uuidLock.Lock();

	if (g_uuidLastTime == 0) {
		g_uuidSeq = GetRandomInteger() % 0x3FFF;
	} else if (ts <= g_uuidLastTime) {
		if (g_uuidSeq == 0x3FFF) {
			g_uuidSeq = 0;
		} else {
			g_uuidSeq++;
		}
	}

	g_uuidNodeId[0] = GetRandomInteger() % 0xFF;
	g_uuidNodeId[1] = GetRandomInteger() % 0xFF;
	g_uuidNodeId[2] = GetRandomInteger() % 0xFF;
	g_uuidNodeId[3] = GetRandomInteger() % 0xFF;
	g_uuidNodeId[4] = GetRandomInteger() % 0xFF;
	g_uuidNodeId[5] = GetRandomInteger() % 0xFF;

	g_uuidLastTime = ts;

	seqpart = g_uuidSeq;
	seqpart |= 0x4000;

	NPT_UInt64 tspart = ts;
	tspart |= 0x1000000000000000LL;

	NPT_UInt32 tslow = tspart & 0xFFFFFFFF;
	NPT_UInt16 tsmid = (tspart >> 32) & 0xFFFF;
	NPT_UInt16 tshigh = (tspart >> 48) & 0xFFFF;

	memcpy(&data[0], &tslow, 4);
	memcpy(&data[4], &tsmid, 2);
	memcpy(&data[6], &tshigh, 2);
	memcpy(&data[8], &seqpart, 2);
	memcpy(&data[10], &g_uuidNodeId[0], 6);

	g_uuidLock.Unlock();

	return NPT_SUCCESS;
}
