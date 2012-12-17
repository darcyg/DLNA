#ifndef __DJControlPoint_h_
#define __DJControlPoint_h_

#include "DJFrontEnd.h"
#include "DJTasks.h"
#include "DJDesc.h"

namespace deejay {

class ActionInstance
{
public:
	virtual void addRef() = 0;
	virtual void release() = 0;
	virtual void abort() = 0;
	virtual bool aborted() const = 0;
	virtual NPT_Result wait(NPT_Timeout timeout = NPT_TIMEOUT_INFINITE) = 0;
	virtual NPT_Result statusCode() const = 0;
	virtual int errorCode() const = 0;
	virtual const NPT_String& errorDescription() const = 0;
	virtual const NPT_List<NPT_String>& outputNames() const = 0;
	virtual const NPT_List<NPT_String>& outputValues() const = 0;

protected:
	ActionInstance();
	~ActionInstance();

private:
	ActionInstance(const ActionInstance&);
	ActionInstance& operator=(const ActionInstance&);
};

class ActionCallback
{
public:
	virtual void onActionFinished(ActionInstance *instance) = 0;
};

class ControlPoint
{
public:
	class Callback
	{
	public:
		virtual void controlPointOnDeviceAdded(ControlPoint *controlPoint, DeviceDesc *deviceDesc, bool& subscribe) = 0;
		virtual void controlPointOnDeviceRemoved(ControlPoint *controlPoint, DeviceDesc *deviceDesc) = 0;
		virtual void controlPointOnStateVariablesChanged(ControlPoint *controlPoint, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList) = 0;
	};

	ControlPoint(Callback *callback);
	~ControlPoint();

	void ssdpSearch(const NPT_String& st, NPT_UInt32 mx);
	void forceRemoveRootDevice(const UUID& uuid);
	void forceRemoveAllDevices();
	void forceRemoveDevicesByType(const NPT_String& deviceType);
	DeviceDescList snapshotDeviceList() const;
	void subscribeEvents(const UUID& uuid);
	void unsubscribeEvents(const UUID& uuid);

	NPT_Result invokeAction(const UUID& uuid, const NPT_String& serviceId, const NPT_String& actionName, const NPT_List<NPT_String>& inputArgs, ActionCallback *callback, ActionInstance*& instance);
	NPT_Result startTask(Task *task);

	NPT_Result setPlaylistToMediaRenderer(const UUID& uuid, const NPT_String& serviceId, const NPT_String& mimeType, const NPT_String& content, ActionCallback *callback, ActionInstance*& instance);
	NPT_Result serveFileToMediaRenderer(const UUID& uuid, const NPT_String& serviceId, const NPT_String& filePath, const NPT_String& mimeType, const NPT_String& metaData, const NPT_String& placeholder, ActionCallback *callback, ActionInstance*& instance);

	NPT_Result remapMediaURL(const UUID& deviceUuid, NPT_String& url);

private:
	friend class FrontEnd;

	void implAttach(FrontEnd *frontEnd, const FrontEnd::ControlPointContext& context);
	void implDetach();

	void processSsdpNotify(const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet);
	void processSsdpNotifyReal(const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet);
	void processSsdpSearchResponse(const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet);
	void processHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);
	void processHttpRequest(const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket);

	void ssdpSearchLocked(const NPT_String& st, NPT_UInt32 mx);
	void ssdpDiscoverRootDevice(const FrontEnd::InterfaceContext *ifctx, const UUID& uuid, const NPT_String& location, const NPT_String& server, int maxAge);
	void processDiscoveryResult(NPT_Result nr, const FrontEnd::InterfaceContext *ifctx, const NPT_String& location, const NPT_String& server, int maxAge, DeviceDesc *rootDeviceDesc, const UUID& uuid);
	void ssdpRemoveRootDevice(const UUID& uuid);

	void removeExpiredDevicesLoop();
	void removeExpiredDevicesAbort();

	class RootDevice
	{
	public:
		NPT_String m_location;
		NPT_String m_server;
		int m_maxAge;
		NPT_TimeStamp m_registerTS;
		NPT_TimeStamp m_updateTS;
		DeviceDesc *m_deviceDesc;
		const FrontEnd::InterfaceContext *m_ifctx;
	};

	void deleteRootDevice(RootDevice *rootDevice);
	void onAddRootDevice(RootDevice *rootDevice);
	void onRemoveRootDevice(RootDevice *rootDevice);
	void onAddDeviceRecursive(DeviceDesc *deviceDesc, RootDevice *rootDevice);
	void onRemoveDeviceRecursive(DeviceDesc *deviceDesc, RootDevice *rootDevice);
	void onAddDevice(DeviceDesc *deviceDesc, RootDevice *rootDevice);
	void onRemoveDevice(DeviceDesc *deviceDesc, RootDevice *rootDevice);

	class ProcessSsdkNotifyTask
		: public Task
	{
	public:
		ProcessSsdkNotifyTask(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet);
		virtual void exec();

	private:
		ControlPoint *m_owner;
		const FrontEnd::InterfaceContext *m_ifctx;
		NPT_DataBuffer m_packet;
	};

	class ProcessHttpRequestTask
		: public Task
		, public AbortableTask
	{
	public:
		ProcessHttpRequestTask(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket);
		virtual ~ProcessHttpRequestTask();

		virtual bool registerAbortCallback(Callback *callback);
		virtual void unregisterAbortCallback(Callback *callback);

	protected:
		virtual void exec();
		virtual void doAbort();

	private:
		ControlPoint *m_owner;
		const FrontEnd::InterfaceContext *m_ifctx;
		NPT_String m_relPath;
		FrontEnd::RequestContext m_reqCtx;
		NPT_HttpRequest *m_req;
		NPT_HttpResponse *m_resp;
		NPT_InputStream *m_inputStream;
		HttpOutput *m_httpOutput;
		NPT_Socket *m_clientSocket;
		ReadWriteLock m_stateLock;
		NPT_List<AbortableTask::Callback*> m_callbackList;
	};

	class SearchContext
		: public SsdpSearchTask::Callback
	{
	public:
		SearchContext(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx);
		virtual ~SearchContext();

	protected:
		virtual bool ssdpSearchTaskOnPacket(SsdpSearchTask *task, const NPT_DataBuffer& packet);
		virtual void ssdpSearchTaskFinished(SsdpSearchTask *task, NPT_Result nr);

	private:
		ControlPoint *m_owner;
		const FrontEnd::InterfaceContext *m_ifctx;
	};

	class DiscoveryContext
		: public DeviceDiscoveryTask::Callback
	{
	public:
		DiscoveryContext(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_String& server, int maxAge);
		virtual ~DiscoveryContext();

	protected:
		virtual void deviceDiscoveryTaskFinished(DeviceDiscoveryTask *task, NPT_Result nr, DeviceDesc *rootDeviceDesc, const UUID& uuid);

	private:
		ControlPoint *m_owner;
		const FrontEnd::InterfaceContext *m_ifctx;
		NPT_String m_server;
		int m_maxAge;
	};

	class RemoveExpiredDevicesTask
		: public Task
	{
	public:
		RemoveExpiredDevicesTask(ControlPoint *owner)
			: m_owner(owner)
		{
		}

	protected:
		virtual void exec()
		{
			m_owner->removeExpiredDevicesLoop();
		}

		virtual void doAbort()
		{
			m_owner->removeExpiredDevicesAbort();
		}

	private:
		ControlPoint *m_owner;
	};

	enum EventSubState
	{
		EventSubState_Idle,
		EventSubState_Pending,
		EventSubState_On,
		EventSubState_None,
		EventSubState_TempFailure,
	};

	struct StateVariable
	{
		const StateVariableDesc *m_stateVariableDesc;
		NPT_String m_value;
	};

	struct ExtraServiceInfo
	{
		ServiceDesc *m_serviceDesc;
		NPT_Map<NPT_String, StateVariable> m_stateVariableTable;
		NPT_String m_eventSubURL;
		UUID m_sid;
		NPT_String m_callbackURL;
		EventSubState m_eventSubState;
		int m_eventSubTimeout;
		NPT_TimeStamp m_eventSubUpdateTS;
	};

	class ServiceInfoByIdFinder
	{
	public:
		ServiceInfoByIdFinder(const NPT_String& serviceId)
			: m_serviceId(serviceId)
		{
		}

		bool operator()(const ExtraServiceInfo *serviceInfo) const
		{
			return serviceInfo->m_serviceDesc->serviceId().Compare(m_serviceId) == 0;
		}

	private:
		NPT_String m_serviceId;
	};

	struct DeviceInfo
	{
		RootDevice *m_rootDevice;
		DeviceDesc *m_deviceDesc;
		NPT_List<ExtraServiceInfo*> m_extraServiceInfoList;

		~DeviceInfo()
		{
			m_extraServiceInfoList.Apply(NPT_ObjectDeleter<ExtraServiceInfo>());
		}
	};

	struct DeviceInfoFinder
	{
	public:
		DeviceInfoFinder(DeviceDesc *deviceDesc)
			: m_deviceDesc(deviceDesc)
		{
		}

		bool operator()(const DeviceInfo *deviceInfo) const
		{
			return deviceInfo->m_deviceDesc == m_deviceDesc;
		}

		DeviceDesc *m_deviceDesc;
	};

	struct DeviceInfoUuidFinder
	{
	public:
		DeviceInfoUuidFinder(const UUID& uuid)
			: m_uuid(uuid)
		{
		}

		bool operator()(const DeviceInfo *deviceInfo) const
		{
			return deviceInfo->m_deviceDesc->uuid() == m_uuid;
		}

		UUID m_uuid;
	};

	void onSubscribeFinished(const UUID& deviceUuid, const NPT_String& serviceId, NPT_Result status, int timeout, const UUID& sid);

	class SubscribeTask2
		: public Task
	{
	public:
		SubscribeTask2(ControlPoint *owner, const UUID& deviceUuid, const NPT_String& serviceId, const NPT_String& eventSubURL, const NPT_String& callbackURL, int timeoutSeconds, const UUID& sid);
		virtual ~SubscribeTask2();

	protected:
		virtual void exec();
		virtual void doAbort();

	private:
		ControlPoint *m_owner;
		UUID m_deviceUuid;
		NPT_String m_serviceId;
		NPT_String m_eventSubURL;
		NPT_String m_callbackURL;
		int m_timeoutSeconds;
		UUID m_sid;
		NPT_HttpClient m_httpClient;
	};

	enum State {
		State_Attached,
		State_Detaching,
		State_Detached,
	};

	void subscribeEventsLocked(DeviceInfo& deviceInfo);
	void unsubscribeEventsLocked(DeviceInfo& deviceInfo);

	struct SimpleContent
	{
		NPT_String content;
		NPT_String mimeType;
	};

	NPT_Map<NPT_String, SimpleContent> m_simpleContentMap;
	NPT_UInt64 m_simpleContentIndex;

private:
	Callback *m_callback;
	ReadWriteLock m_stateLock;
	State m_state;
	TaskGroup *m_attachedTaskGroup;
	FrontEnd *m_frontEnd;
	FrontEnd::ControlPointContext m_frontEndContext;
	NPT_List<RootDevice*> m_rootDeviceList;
	NPT_Map<UUID, RootDevice*> m_rootDeviceIndexByUUID;
	NPT_SharedVariable m_removeExpiredDevicesVar;
	NPT_List<DeviceInfo*> m_deviceList;
	NPT_Map<UUID, DeviceDiscoveryTask*> m_pendingSsdpDiscoverTasks;
};

class SoapTask;

class MCActionImpl
	: public ActionInstance
{
public:
	MCActionImpl(ActionCallback *callback, const NPT_List<NPT_String>& outputArgNames);
	~MCActionImpl();
	virtual void addRef();
	virtual void release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual NPT_Result statusCode() const;
	virtual int errorCode() const;
	virtual const NPT_String& errorDescription() const;
	virtual const NPT_List<NPT_String>& outputNames() const;
	virtual const NPT_List<NPT_String>& outputValues() const;

	void setBuddy(SoapTask *buddy);
	void onFinished(NPT_Result nr, int errCode, const NPT_String& errDesc, const NPT_List<NPT_String>& outputValues);

	NPT_HttpHeaders m_responseHeaders;
	NPT_HttpStatusCode m_responseStatusCode;
	NPT_String m_responseStatusText;

private:
	ReadWriteLock m_stateLock;
	ActionCallback *m_callback;
	NPT_AtomicVariable m_refCount;
	NPT_SharedVariable m_waitVar;
	SoapTask *m_buddy;
	NPT_List<NPT_String> m_outputArgNames;
	NPT_List<NPT_String> m_outputArgValues;
	NPT_Result m_statusCode;
	int m_errCode;
	NPT_String m_errDesc;
	bool m_abortFlag;
};

class SoapTask
	: public Task
{
public:
	SoapTask(const NPT_String& controlURL, const NPT_String& serviceType, const NPT_String& actionName, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, const NPT_List<NPT_String>& outputArgNames, MCActionImpl *actImpl);
	virtual ~SoapTask();

protected:
	virtual void exec();
	virtual void doAbort();

private:
	NPT_String m_controlURL;
	NPT_String m_serviceType;
	NPT_String m_actionName;
	NPT_List<NPT_String> m_inputArgNames;
	NPT_List<NPT_String> m_inputArgValues;
	NPT_List<NPT_String> m_outputArgNames;
	NPT_List<NPT_String> m_outputArgValues;
	MCActionImpl *m_actImpl;
	NPT_HttpClient m_httpClient;
};

} // namespace deejay

#endif // __DJControlPoint_h_
