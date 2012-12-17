/*****************************************************************
|
|   Neptune - System :: Win32 Implementation
|
|   (c) 2001-2006 Gilles Boccon-Gibod
|   Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#if defined(_XBOX)
#include <xtl.h>
#else
#include <windows.h>
#endif

#if !defined(_WIN32_WCE)
#include <sys/timeb.h>
#endif

#include "NptConfig.h"
#include "NptTypes.h"
#include "NptSystem.h"
#include "NptResults.h"
#include "NptDebug.h"

/*----------------------------------------------------------------------
|   NPT_System::GetProcessId
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::GetProcessId(NPT_UInt32& id)
{
    //id = getpid();
    id = 0;
    return NPT_SUCCESS;
}

#if defined(_WIN32_WCE)
/*----------------------------------------------------------------------
|   NPT_System::GetCurrentTimeStamp
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::GetCurrentTimeStamp(NPT_TimeStamp& now)
{
    SYSTEMTIME stime;
    FILETIME   ftime;
    __int64    time64;
    GetSystemTime(&stime);
    SystemTimeToFileTime(&stime, &ftime);

    /* convert to 64-bits 100-nanoseconds value */
    time64 = (((unsigned __int64)ftime.dwHighDateTime)<<32) | ((unsigned __int64)ftime.dwLowDateTime);
    time64 -= 116444736000000000; /* convert from the Windows epoch (Jan. 1, 1601) to the 
                                   * Unix epoch (Jan. 1, 1970) */
    
    now.m_Seconds = (NPT_Int32)(time64/10000000);
    now.m_NanoSeconds = 100*(NPT_Int32)(time64-((unsigned __int64)now.m_Seconds*10000000));

    return NPT_SUCCESS;
}
#else
/*----------------------------------------------------------------------
|   NPT_System::GetCurrentTimeStamp
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::GetCurrentTimeStamp(NPT_TimeStamp& now)
{
    struct _timeb time_stamp;

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    _ftime_s(&time_stamp);
#else
    _ftime(&time_stamp);
#endif
    now.SetNanos(((NPT_UInt64)time_stamp.time)     * 1000000000UL +
                  ((NPT_UInt64)time_stamp.millitm) * 1000000);

    return NPT_SUCCESS;
}
#endif

/*----------------------------------------------------------------------
|   NPT_System::Sleep
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::Sleep(const NPT_TimeInterval& duration)
{
    ::Sleep((NPT_UInt32)duration.ToMillis());

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_System::SleepUntil
+---------------------------------------------------------------------*/
NPT_Result
NPT_System::SleepUntil(const NPT_TimeStamp& when)
{
    NPT_TimeStamp now;
    GetCurrentTimeStamp(now);
    if (when > now) {
        NPT_TimeInterval duration = when-now;
        return Sleep(duration);
    } else {
        return NPT_SUCCESS;
    }
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
        srand((NPT_UInt32)now.ToNanos());
        seeded = true;
    }

    return rand();
}

/*----------------------------------------------------------------------
|   NPT_System::GenerateUUID
+---------------------------------------------------------------------*/
#if 1

NPT_Result NPT_System::GenerateUUID(NPT_UInt8 data[16])
{
	UUID uuid;
	RPC_STATUS status = UuidCreate(&uuid);
	switch (status) {
	case RPC_S_OK:
	case RPC_S_UUID_LOCAL_ONLY:
	case RPC_S_UUID_NO_ADDRESS:
		*reinterpret_cast<NPT_UInt32*>(data) = uuid.Data1;
		*reinterpret_cast<NPT_UInt16*>(data + 4) = uuid.Data2;
		*reinterpret_cast<NPT_UInt16*>(data + 6) = uuid.Data3;
		memcpy(data + 8, &uuid.Data4[0], 8);
		break;
	default:
		return NPT_FAILURE;
	}
	return NPT_SUCCESS;
}

#else

#include <sys/types.h>
#include <sys/timeb.h>

static NPT_Mutex g_uuidLock;
static NPT_UInt64 g_uuidLastTime = 0;
static NPT_UInt16 g_uuidSeq;
static NPT_UInt8 g_uuidNodeId[6];

NPT_Result NPT_System::GenerateUUID(NPT_UInt8 data[16])
{
	NPT_UInt64 ts;

	timeb tb;
	ftime(&tb);
	ts = tb.time * 1000 + tb.millitm;

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

#endif
