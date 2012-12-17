#ifndef __DJDeviceImpl_h_
#define __DJDeviceImpl_h_

#include "DJFrontEnd.h"

namespace deejay {

enum StateVariableDataType
{
	SVDT_STRING,
	SVDT_BOOLEAN,
	SVDT_UI1,
	SVDT_UI2,
	SVDT_UI4,
	SVDT_UI8,
	SVDT_I1,
	SVDT_I2,
	SVDT_I4,
	SVDT_I8,
	SVDT_INT,
	SVDT_BASE64,
};

const NPT_UInt32 SVF_EVT = 0x00000001;
const NPT_UInt32 SVF_GRP = 0x00000002;
const NPT_UInt32 SVF_ARG = 0x00000004;
const NPT_UInt32 SVF_AVR = 0x00000008;
const NPT_UInt32 SVF_CHL = 0x00000010;

struct StateVariableDecl
{
	const char *name;
	StateVariableDataType dataType;
	NPT_UInt32 flags;
	const char **allowedValue;
	const char *defaultValue;
};

const NPT_UInt32 AF_IN = 0x00000001;
const NPT_UInt32 AF_OUT = 0x00000002;
const NPT_UInt32 AF_RETVAL = 0x00000004;

struct ArgumentDecl
{
	const char *name;
	const char *relatedStateVariable;
	NPT_UInt32 flags;
};

struct ActionDecl
{
	const char *name;
	NPT_UInt32 argumentCount;
	NPT_UInt32 inputArgumentCount;
	const ArgumentDecl *arguments;
};

struct ServiceDecl
{
	const char *serviceType;
	const char *serviceId;
	NPT_UInt32 numStateVariables;
	NPT_UInt32 numActions;
	const StateVariableDecl *stateVariables;
	const ActionDecl *actions;
};

class DeliverEventNotifyTask;

class DeviceImpl
{
public:
	DeviceImpl();
	virtual ~DeviceImpl();

	UUID uuid() const;
	NPT_String deviceType() const;

	virtual NPT_String friendlyName() const;
	virtual NPT_String manufacturer() const;
	virtual NPT_String manufacturerURL() const;
	virtual NPT_String modelDescription() const;
	virtual NPT_String modelName() const;
	virtual NPT_String modelNumber() const;
	virtual NPT_String modelURL() const;
	virtual NPT_String serialNumber() const;
	virtual NPT_String upc() const;
	virtual NPT_String presentationURL() const;

protected:
	void registerServices(const ServiceDecl *decls, NPT_UInt32 numServices);
	void registerStaticContent(const NPT_String& path, const NPT_String& mimeType, const NPT_DataBuffer& data, bool copy);
	bool setStateValue(const NPT_String& serviceId, const NPT_String& name, const NPT_String& value);
	bool getStateValue(const NPT_String& serviceId, const NPT_String& name, NPT_String& value) const;
	virtual NPT_String generateDeviceDescriptionXml(const FrontEnd::InterfaceContext *ifctx, const FrontEnd::DeviceImplContext *fectx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req);
	virtual void outputMoreDeviceDescription(NPT_XmlSerializer *xml);
	virtual int onAction(const ServiceDecl *serviceDecl, const ActionDecl *actionDecl, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, NPT_List<NPT_String>& outputArgValues) = 0;
	virtual bool onHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);
	const FrontEnd::DeviceImplContext *frontEndContext() const;

private:
	void registerService(const ServiceDecl *decl);
	bool match(const NPT_String& st, NPT_List<FrontEnd::DeviceImplMatch>& ls);

protected:
	UUID m_uuid;
	NPT_String m_deviceType;
	NPT_String m_friendlyName;
	NPT_String m_manufacturer;
	NPT_String m_manufacturerURL;
	NPT_String m_modelDescription;
	NPT_String m_modelName;
	NPT_String m_modelNumber;
	NPT_String m_modelURL;
	NPT_String m_serialNumber;
	NPT_String m_upc;
	NPT_String m_presentationURL;

	int m_expireSeconds;
	NPT_String m_descPath;

private:
	friend class FrontEnd;

	enum State {
		State_Attached,
		State_Detaching,
		State_Detached,
	};

	void implAttach(FrontEnd *frontEnd, const FrontEnd::DeviceImplContext& context);
	void implDetach();

	struct StaticContentInfo
	{
		NPT_String m_path;
		NPT_String m_mimeType;
		NPT_DataBuffer m_data;
	};

	NPT_List<StaticContentInfo> m_staticContentList;

	struct EventSubInfo
	{
		NPT_List<NPT_String> m_callbackList;
		int m_timeout;
		NPT_TimeStamp m_updateTS;
		UUID m_sid;
		NPT_UInt32 m_eventKey;
	};

	class EventSubInfoFinder
	{
	public:
		EventSubInfoFinder(const UUID& sid)
			: m_sid(sid)
		{
		}

		bool operator()(const EventSubInfo *info) const
		{
			return info->m_sid == m_sid;
		}

	private:
		UUID m_sid;
	};

	struct StateVarInfo
	{
		const StateVariableDecl *m_varDecl;
		NPT_String m_value;
		NPT_String m_cachedValue;
	};

	struct ServiceInfo
	{
		const ServiceDecl *m_serviceDecl;
		NPT_String m_scpdXml;
		NPT_String m_scpdPath;
		NPT_String m_controlPath;
		NPT_String m_eventSubPath;

		NPT_List<EventSubInfo*> m_eventSubList;
		NPT_Map<UUID, EventSubInfo*> m_eventSubIndex;

		NPT_List<StateVarInfo*> m_stateVarList;
		NPT_Map<NPT_String, StateVarInfo*> m_stateVarIndex;

		~ServiceInfo()
		{
			m_eventSubList.Apply(NPT_ObjectDeleter<EventSubInfo>());
			m_stateVarList.Apply(NPT_ObjectDeleter<StateVarInfo>());
		}
	};

	class ServiceInfoByIdFinder
	{
	public:
		ServiceInfoByIdFinder(const NPT_String& serviceId)
			: m_serviceId(serviceId)
		{
		}

		bool operator()(const ServiceInfo *serviceInfo) const
		{
			return m_serviceId.Compare(serviceInfo->m_serviceDecl->serviceId) == 0;
		}

	private:
		NPT_String m_serviceId;
	};

	void removeExpiredEventSubLoop();
	void removeExpiredEventSubAbort();

	class RemoveExpiredEventSubTask
		: public Task
	{
	public:
		RemoveExpiredEventSubTask(DeviceImpl *owner)
			: m_owner(owner)
		{
		}

		virtual void exec()
		{
			m_owner->removeExpiredEventSubLoop();
		}

		virtual void doAbort()
		{
			m_owner->removeExpiredEventSubAbort();
		}

	private:
		DeviceImpl *m_owner;
	};

	class ProcessHttpRequestTask
		: public Task
		, public AbortableTask
	{
	public:
		ProcessHttpRequestTask(DeviceImpl *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket);
		virtual ~ProcessHttpRequestTask();

		virtual bool registerAbortCallback(Callback *callback);
		virtual void unregisterAbortCallback(Callback *callback);

	protected:
		virtual void exec();
		virtual void doAbort();

	private:
		DeviceImpl *m_owner;
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


	void processHttpRequest(const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket);
	void processHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);
	void processControlRequest(ServiceInfo *serviceInfo, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);
	void processEventSubRequest(ServiceInfo *serviceInfo, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);
	void serveSimpleContent(const NPT_String& content, const NPT_String& contentType, const FrontEnd::InterfaceContext *ifctx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);

	bool addEventSub(ServiceInfo *serviceInfo, const NPT_List<NPT_String>& callbackList, int& timeout, UUID& sid);
	bool renewEventSub(ServiceInfo *serviceInfo, const UUID& sid, int& timeout);
	bool removeEventSub(ServiceInfo *serviceInfo, const UUID& sid);
	void onInitialEventNotifyDelivered(const NPT_String& serviceId, const UUID& sid);

	NPT_String composeLastChangeAVT(const ServiceInfo *serviceInfo);
	NPT_String composeLastChangeRCS(const ServiceInfo *serviceInfo);

	void monitorStateVarsLoop();
	void monitorStateVarsAbort();

	NPT_SharedVariable m_monitorVar;

	class StateVarMonitorTask
		: public Task
	{
	public:
		StateVarMonitorTask(DeviceImpl *owner)
			: m_owner(owner)
		{
		}

		virtual void exec()
		{
			m_owner->monitorStateVarsLoop();
		}

		virtual void doAbort()
		{
			m_owner->monitorStateVarsAbort();
		}

	private:
		DeviceImpl *m_owner;

	};

private:
	friend class DeliverEventNotifyTask;

	ReadWriteLock m_stateLock;
	State m_state;
	TaskGroup *m_attachedTaskGroup;
	FrontEnd *m_frontEnd;
	FrontEnd::DeviceImplContext m_frontEndContext;

	NPT_List<ServiceInfo*> m_serviceInfoList;
	NPT_SharedVariable m_removeExpiredEventSubVar;
};

class DeliverEventNotifyTask
	: public Task
{
public:
	DeliverEventNotifyTask(DeviceImpl *deviceImpl, const NPT_List<NPT_String>& callbackList, NPT_UInt32 eventKey, const NPT_String& serviceId, const UUID& sid, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList);
	virtual ~DeliverEventNotifyTask();

protected:
	virtual void exec();
	virtual void doAbort();

private:
	bool deliver(const NPT_String& url, const NPT_String& content);

private:
	DeviceImpl *m_deviceImpl;
	NPT_List<NPT_String> m_callbackList;
	NPT_UInt32 m_eventKey;
	NPT_String m_serviceId;
	UUID m_sid;
	NPT_List<NPT_String> m_nameList;
	NPT_List<NPT_String> m_valueList;
	NPT_HttpClient m_httpClient;
};

} // namespace deejay

#endif // __DJDeviceImpl_h_
