#ifndef __DJFrontEnd_h_
#define __DJFrontEnd_h_

#include "DJConnector.h"
#include "DJUtils.h"
#include "DJTaskGroup.h"

namespace deejay {

class ControlPoint;
class DeviceImpl;

class AbortableTask
{
public:
	class Callback
	{
	public:
		virtual void onAborted(AbortableTask *task) = 0;
	};

	virtual bool registerAbortCallback(Callback *callback) = 0;
	virtual void unregisterAbortCallback(Callback *callback) = 0;

protected:
	AbortableTask() {}
	~AbortableTask() {}

private:
	AbortableTask(const AbortableTask&);
	AbortableTask& operator=(const AbortableTask&);
};

class HttpOutput
{
public:
	virtual ~HttpOutput() {}
	virtual void writeResponseHeader(const NPT_HttpResponse& response) = 0;
	virtual void writeData(const NPT_String& data) = 0;
	virtual void writeData(const NPT_DataBuffer& data) = 0;
	virtual void writeData(const void *data, NPT_Size length) = 0;
	virtual void flush() = 0;
};

class FrontEnd
{
public:
	struct InterfaceContext
	{
		NPT_IpAddress m_ifAddr;
		NPT_UInt16 m_httpPort;
		NPT_UInt16 m_ssdpPort;
	};

	struct ControlPointContext
	{
		NPT_String m_httpRoot;
		NPT_String m_serverHeader;
		NPT_String m_userAgentHeader;
	};

	struct DeviceImplContext
	{
		NPT_String m_httpRoot;
		NPT_String m_serverHeader;
		NPT_String m_userAgentHeader;
	};

	enum ClientHint
	{
		CH_Unknown,
		CH_XBox,
	};

	enum TransferMode
	{
		TM_None,
		TM_Streaming,
		TM_Interactive,
		TM_Background,
		TM_Unknown,
	};

	struct RequestContext
	{
		ClientHint clientHint;
		TransferMode transferMode;
		bool getcontentFeaturesReq;
	};

	struct DeviceImplMatch
	{
		DeviceImplMatch(const NPT_String& st, const NPT_String& usn)
			: m_st(st), m_usn(usn)
		{
		}

		NPT_String m_st;
		NPT_String m_usn;
	};

	struct MatchContext
	{
		NPT_String descPath;
		int expireSeconds;
		UUID deviceUuid;
		NPT_String httpRoot;
		NPT_List<DeviceImplMatch> matches;
	};

	FrontEnd();
	~FrontEnd();

	void setIncludeLoopback(bool includeLoopback);
	void setOSVersion(const NPT_String& osver);

	NPT_Result start();
	bool started() const;
	void stop();

	void addControlPoint(ControlPoint *controlPoint);
	void removeControlPoint(ControlPoint *controlPoint);
	bool hasControlPoint(ControlPoint *controlPoint) const;

	void addDeviceImpl(DeviceImpl *deviceImpl);
	void removeDeviceImpl(DeviceImpl *deviceImpl);
	bool hasDeviceImpl(DeviceImpl *deviceImpl) const;

	const NPT_List<const InterfaceContext*>& interfaceContextList() const;

private:
	struct Interface
		: public HttpConnector::Callback
		, public SsdpConnector::Callback
	{
		NPT_NetworkInterface *m_nif;
		HttpConnector *m_httpConnector;
		SsdpConnector *m_ssdpConnector;
		FrontEnd *m_owner;
		InterfaceContext m_context;

		virtual void httpConnectorOnNewClient(HttpConnector *connector, NPT_Socket *client);
		virtual void ssdpConnectorOnPacket(SsdpConnector *connector, const char *data, NPT_Size length, const NPT_SocketAddress& fromAddr);
	};

	enum State {
		State_Stopped,
		State_Running,
		State_Stopping,
	};

	class HttpServerTask
		: public Task
		, public AbortableTask
	{
	public:
		HttpServerTask(FrontEnd *owner, Interface *intf, NPT_Socket *client);
		virtual ~HttpServerTask();

		virtual bool registerAbortCallback(Callback *callback);
		virtual void unregisterAbortCallback(Callback *callback);

		void detach();

	protected:
		virtual void exec();
		virtual void doAbort();

	private:
		FrontEnd *m_owner;
		Interface *m_intf;
		NPT_Socket *m_client;
		ReadWriteLock m_stateLock;
		NPT_List<AbortableTask::Callback*> m_callbackList;
	};

	class SsdpServerTask
		: public Task
		, public AbortableTask
	{
	public:
		SsdpServerTask(FrontEnd *owner, Interface *intf, const char *data, NPT_Size length, const NPT_SocketAddress& fromAddr);
		virtual ~SsdpServerTask();

		virtual bool registerAbortCallback(Callback *callback);
		virtual void unregisterAbortCallback(Callback *callback);

	protected:
		virtual void exec();
		virtual void doAbort();

	private:
		FrontEnd *m_owner;
		Interface *m_intf;
		NPT_DataBuffer m_data;
		NPT_SocketAddress m_fromAddr;
		ReadWriteLock m_stateLock;
		NPT_List<AbortableTask::Callback*> m_callbackList;
	};

	struct ControlPointInfo
	{
		ControlPointContext m_context;
		ControlPoint *m_controlPoint;
	};

	struct ControlPointInfoFinder
	{
		ControlPointInfoFinder(ControlPoint *controlPoint)
			: m_controlPoint(controlPoint)
		{
		}

		bool operator()(const ControlPointInfo *info) const
		{
			return info->m_controlPoint == m_controlPoint;
		}

		ControlPoint *m_controlPoint;
	};

	struct DeviceImplInfo
	{
		DeviceImplContext m_context;
		DeviceImpl *m_deviceImpl;
		NPT_TimeStamp m_updateTS;
	};

	struct DeviceImplInfoFinder
	{
		DeviceImplInfoFinder(DeviceImpl *deviceImpl)
			: m_deviceImpl(deviceImpl)
		{
		}

		bool operator()(const DeviceImplInfo *info) const
		{
			return info->m_deviceImpl == m_deviceImpl;
		}

		DeviceImpl *m_deviceImpl;
	};

	void httpConnectorOnNewClient(HttpServerTask *task, Interface *intf, NPT_Socket *client);
	void ssdpConnectorOnPacket(SsdpServerTask *task, Interface *intf, const NPT_DataBuffer& data, const NPT_SocketAddress& fromAddr);
	void processSsdpSearch(SsdpServerTask *task, Interface *intf, const NPT_DataBuffer& data, const NPT_SocketAddress& fromAddr);
	void broadcastLocked(DeviceImplInfo *deviceInfo, bool avail);
	void broadcastIfNecessary();

	class SsdpSearchAbortCallback
		: public AbortableTask::Callback
	{
	public:
		SsdpSearchAbortCallback(NPT_UdpSocket *socket, NPT_SharedVariable *waitVar);
		virtual void onAborted(AbortableTask *task);

	private:
		NPT_UdpSocket *m_socket;
		NPT_SharedVariable *m_waitVar;
	};

	class SsdpBroadcastTask
		: public Task
	{
	public:
		SsdpBroadcastTask(FrontEnd *owner);
		virtual ~SsdpBroadcastTask();

		virtual void exec();
		virtual void doAbort();

	private:
		FrontEnd *m_owner;
		NPT_SharedVariable m_waitVar;
	};

	class HttpOutputImpl
		: public HttpOutput
	{
	public:
		HttpOutputImpl(FrontEnd *frontEnd, const NPT_SocketInfo& socketInfo, NPT_OutputStreamReference outputStream);
		virtual ~HttpOutputImpl();

	protected:
		virtual void writeResponseHeader(const NPT_HttpResponse& response);
		virtual void writeData(const NPT_String& data);
		virtual void writeData(const NPT_DataBuffer& data);
		virtual void writeData(const void *data, NPT_Size length);
		virtual void flush();

	private:
		FrontEnd *m_frontEnd;
		NPT_SocketInfo m_socketInfo;
		NPT_OutputStreamReference m_outputStream;
	};

	void onHttpRequestHeader(const NPT_SocketInfo& socketInfo, const NPT_HttpRequest *request);
	void onHttpResponseHeader(const NPT_SocketInfo& socketInfo, const NPT_HttpResponse *request);

private:
	ReadWriteLock m_stateLock;
	State m_state;

	NPT_String m_serverHeader;
	NPT_String m_userAgentHeader;

	NPT_List<Interface*> m_ifList;
	TaskGroup *m_taskGroup;
	NPT_List<const InterfaceContext*> m_interfaceContextList;

	ReadWriteLock m_cpLock;
	NPT_List<ControlPointInfo*> m_controlPointList;

	ReadWriteLock m_dsLock;
	NPT_List<DeviceImplInfo*> m_deviceImplList;
	NPT_Map<UUID, DeviceImplInfo*> m_deviceImplIndex;

	bool m_includeLoopback;
};

} // namespace deejay

#endif // __DJFrontEnd_h_
