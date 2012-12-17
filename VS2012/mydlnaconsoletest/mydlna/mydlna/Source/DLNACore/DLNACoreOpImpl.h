#ifndef __DLNACoreOpImpl_h__
#define __DLNACoreOpImpl_h__

#include "DLNACoreOp.h"
#include "DJControlPoint.h"

namespace deejay {

class DLNABrowseOpImpl
	: public DLNABrowseOp
	, public ActionCallback
{
public:
	DLNABrowseOpImpl();
	virtual ~DLNABrowseOpImpl();

	virtual int addRef();
	virtual int release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback);
	virtual bool resetCallback();
	virtual bool succeeded() const;

	virtual bool hasFaultDetail() const;
	virtual int errorCode() const;
	virtual const NPT_String& errorDesc() const;
	virtual const DLNAObjectList& objectList() const;

	virtual void onActionFinished(ActionInstance *instance);

	bool parseResult(const NPT_List<NPT_String>& outputArgs);

	ReadWriteLock m_stateLock;
	NPT_AtomicVariable m_refCount;
	NPT_SharedVariable m_waitVar;
	bool m_abortFlag;
	bool m_finished;
	FinishCallback *m_finishCallback;
	ActionInstance *m_actInst;
	bool m_succeeded;
	bool m_hasFaultDetail;
	int m_errorCode;
	NPT_String m_errorDesc;
	DLNAObjectList m_result;
};

class DLNAFullBrowseTask;

class DLNAFullBrowseOpImpl
	: public DLNAProgressiveBrowseOp
{
public:
	DLNAFullBrowseOpImpl(DLNAProgressiveBrowseOp::ResultCallback *callback);
	virtual ~DLNAFullBrowseOpImpl();

	virtual int addRef();
	virtual int release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback);
	virtual bool resetCallback();
	virtual bool succeeded() const;

	virtual NPT_Result waitFirstReport(NPT_Timeout timeout);

	virtual bool hasFaultDetail() const;
	virtual int errorCode() const;
	virtual const NPT_String& errorDesc() const;
	virtual const DLNAObjectList& objectList() const;

	void setBuddy(DLNAFullBrowseTask *buddy);
	void notifyBrowseResult(NPT_UInt32 startingIndex, NPT_UInt32 numberReturned, NPT_UInt32 totalMatches, const DLNAObjectList& ls);
	void notifyFinished();

	ReadWriteLock m_stateLock;
	NPT_AtomicVariable m_refCount;
	NPT_SharedVariable m_waitVar;
	NPT_SharedVariable m_waitVar1;
	bool m_abortFlag;
	bool m_finished;
	FinishCallback *m_finishCallback;
	bool m_succeeded;
	bool m_hasFaultDetail;
	int m_errorCode;
	NPT_String m_errorDesc;
	DLNAObjectList m_result;
	DLNAFullBrowseTask *m_buddy;
	DLNAProgressiveBrowseOp::ResultCallback *m_resultCallback;
};

class DLNAFullBrowseTask
	: public Task
{
public:
	DLNAFullBrowseTask(DLNAFullBrowseOpImpl *op, ControlPoint *cp, const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId, NPT_UInt32 requestedCount = 20);
	virtual ~DLNAFullBrowseTask();

	virtual void exec();
	virtual void doAbort();

	void setActionInstance(ActionInstance *actInst);

	NPT_Result browse(const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, DLNAObjectList& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches);

private:
	DLNAFullBrowseOpImpl *m_op;
	ControlPoint *m_cp;
	UUID m_mediaServerUuid;
	NPT_String m_cdsId;
	NPT_String m_containerId;
	ActionInstance *m_actInst;
	ReadWriteLock m_stateLock;
	NPT_UInt32 m_requestedCount;
};


class DLNADeepBrowseTask;

class DLNADeepBrowseOpImpl
	: public DLNABrowseOp
{
public:
	DLNADeepBrowseOpImpl();
	virtual ~DLNADeepBrowseOpImpl();

	virtual int addRef();
	virtual int release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback);
	virtual bool resetCallback();
	virtual bool succeeded() const;

	virtual bool hasFaultDetail() const;
	virtual int errorCode() const;
	virtual const NPT_String& errorDesc() const;
	virtual const DLNAObjectList& objectList() const;

	void setBuddy(DLNADeepBrowseTask *buddy);
	void notifyFinished();

	ReadWriteLock m_stateLock;
	NPT_AtomicVariable m_refCount;
	NPT_SharedVariable m_waitVar;
	bool m_abortFlag;
	bool m_finished;
	FinishCallback *m_finishCallback;
	bool m_succeeded;
	bool m_hasFaultDetail;
	int m_errorCode;
	NPT_String m_errorDesc;
	DLNAObjectList m_result;
	DLNADeepBrowseTask *m_buddy;
};

class DLNADeepBrowseTask
	: public Task
{
public:
	DLNADeepBrowseTask(DLNADeepBrowseOpImpl *op, ControlPoint *cp, const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId);
	virtual ~DLNADeepBrowseTask();

	virtual void exec();
	virtual void doAbort();

	void setActionInstance(ActionInstance *actInst);

	NPT_Result browse(const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId, NPT_Stack<NPT_String>& stack, DLNAObjectList& result);

private:
	DLNADeepBrowseOpImpl *m_op;
	ControlPoint *m_cp;
	UUID m_mediaServerUuid;
	NPT_String m_cdsId;
	NPT_String m_containerId;
	ActionInstance *m_actInst;
	ReadWriteLock m_stateLock;
};

class DLNABaseMediaTask;

class DLNABaseMediaOpImpl
	: public DLNACoreOp
{
public:
	DLNABaseMediaOpImpl();
	virtual ~DLNABaseMediaOpImpl();

	virtual int addRef();
	virtual int release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback);
	virtual bool resetCallback();
	virtual bool succeeded() const;

	void setBuddy(DLNABaseMediaTask *buddy);
	void notifyFinished();

	ReadWriteLock m_stateLock;
	NPT_AtomicVariable m_refCount;
	NPT_SharedVariable m_waitVar;
	bool m_abortFlag;
	bool m_finished;
	FinishCallback *m_finishCallback;
	bool m_succeeded;
	DLNABaseMediaTask *m_buddy;
};

class DLNABaseMediaTask
	: public Task
{
public:
	DLNABaseMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp);
	virtual ~DLNABaseMediaTask();

	virtual void exec();
	virtual void doAbort();
	virtual NPT_Result doAction(ActionInstance*& actInst) { return NPT_ERROR_NOT_IMPLEMENTED; }
	virtual void handleResult(ActionInstance *actInst) {}

	void setActionInstance(ActionInstance *actInst);

protected:
	DLNABaseMediaOpImpl *m_op;
	ControlPoint *m_cp;
	ActionInstance *m_actInst;
	ReadWriteLock m_stateLock;
};

class DLNAStopMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNAStopMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
};

class DLNAPauseMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNAPauseMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
};

class DLNAPrevMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNAPrevMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
};

class DLNANextMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNANextMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
};

class DLNAMuteMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNAMuteMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& rcsId, bool mute);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_rcsId;
	bool m_mute;
};

class DLNAChangeMediaVolumeTask
	: public DLNABaseMediaTask
{
public:
	DLNAChangeMediaVolumeTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& rcsId, int volume);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_rcsId;
	int m_volume;
};

class DLNASeekMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNASeekMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_String& mode, const NPT_String& target);
	virtual NPT_Result doAction(ActionInstance*& actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
	NPT_String m_mode;
	NPT_String m_target;
};

class DLNAPlayMediaTask
	: public DLNABaseMediaTask
{
public:
	DLNAPlayMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_String& url, const NPT_String& metaData);
	virtual void exec();

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
	NPT_String m_url;
	NPT_String m_metaData;
};

class DLNAPlayMediaListTask
	: public DLNABaseMediaTask
{
public:
	DLNAPlayMediaListTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_List<DLNAObject*>& ls);
	virtual void exec();

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
	DLNAObjectList m_mediaList;
};

class DLNAPlayFileTask
	: public DLNABaseMediaTask
{
public:
	DLNAPlayFileTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_String& filePath, const NPT_String& mimeType, const NPT_String& metaData, const NPT_String& placeholder);
	virtual void exec();

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
	NPT_String m_filePath;
	NPT_String m_mimeType;
	NPT_String m_metaData;
	NPT_String m_placeholder;
};

class DLNAQueryPositionInfoOpImpl
	: public DLNABaseMediaOpImpl
	, public DLNAQueryPositionInfoOp
{
public:
	DLNAQueryPositionInfoOpImpl();
	virtual ~DLNAQueryPositionInfoOpImpl();

	virtual int addRef();
	virtual int release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback);
	virtual bool resetCallback();
	virtual bool succeeded() const;

	virtual int trackTime() const;

	int m_trackTime;
};

class DLNAQueryPositionInfoTask
	: public DLNABaseMediaTask
{
public:
	DLNAQueryPositionInfoTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId);
	virtual NPT_Result doAction(ActionInstance*& actInst);
	virtual void handleResult(ActionInstance *actInst);

private:
	UUID m_mediaRendererUuid;
	NPT_String m_avtId;
	NPT_String m_mode;
	NPT_String m_target;
};

} // namespace deejay

#endif // __DLNACoreOpImpl_h__
