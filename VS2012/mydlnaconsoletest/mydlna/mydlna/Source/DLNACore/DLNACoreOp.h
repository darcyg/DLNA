#ifndef __DLNACoreOp_h__
#define __DLNACoreOp_h__

#include <Neptune.h>
#include "DLNAObject.h"

namespace deejay {

class DLNACoreOp
{
public:
	class FinishCallback
	{
	public:
		virtual void onDLNACoreOpFinished(DLNACoreOp *op) = 0;
	};

	virtual int addRef() = 0;
	virtual int release() = 0;
	virtual void abort() = 0;
	virtual bool aborted() const = 0;
	virtual NPT_Result wait(NPT_Timeout timeout = NPT_TIMEOUT_INFINITE) = 0;
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback) = 0;
	virtual bool resetCallback() = 0;
	virtual bool succeeded() const = 0;

protected:
	DLNACoreOp();
	virtual ~DLNACoreOp();

private:
	DLNACoreOp(const DLNACoreOp&);
	DLNACoreOp& operator=(const DLNACoreOp&);
};

class DLNABrowseOp
	: public DLNACoreOp
{
public:
	virtual bool hasFaultDetail() const = 0;
	virtual int errorCode() const = 0;
	virtual const NPT_String& errorDesc() const = 0;
	virtual const DLNAObjectList& objectList() const = 0;
};

class DLNAProgressiveBrowseOp
	: public DLNABrowseOp
{
public:
	class ResultCallback
	{
	public:
		virtual void onDLNAProgressiveBrowseOpResult(DLNAProgressiveBrowseOp *op, NPT_UInt32 startingIndex, NPT_UInt32 numberReturned, NPT_UInt32 totalMatches, const DLNAObjectList& ls) = 0;
	};

	virtual NPT_Result waitFirstReport(NPT_Timeout timeout = NPT_TIMEOUT_INFINITE) = 0;
};

class DLNAQueryPositionInfoOp
	: public DLNACoreOp
{
public:
	virtual int trackTime() const = 0;
};

} // namespace deejay

#endif // __DLNACoreOp_h__
