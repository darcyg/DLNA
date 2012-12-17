#ifndef __DJTasks_h__
#define __DJTasks_h__

#include "DJTaskGroup.h"
#include "DJDesc.h"

namespace deejay {

class SsdpSearchTask
	: public Task
{
public:
	class Callback
	{
	public:
		virtual ~Callback();
		virtual bool ssdpSearchTaskOnPacket(SsdpSearchTask *task, const NPT_DataBuffer& packet) = 0;
		virtual void ssdpSearchTaskFinished(SsdpSearchTask *task, NPT_Result nr) = 0;
	};

	SsdpSearchTask(const char *st, NPT_UInt32 mx, const NPT_IpAddress& ifAddr, Callback *callback, bool autoDelete = true);
	virtual ~SsdpSearchTask();

protected:
	virtual void exec();
	virtual void doAbort();

private:
	void report(NPT_Result nr);
	bool report(const NPT_DataBuffer& data);

private:
	NPT_String m_st;
	NPT_UInt32 m_mx;
	NPT_IpAddress m_ifAddr;
	Callback *m_callback;
	bool m_autoDelete;
	NPT_UdpSocket m_socket;
};

class HttpGetTask
	: public Task
{
public:
	class Callback
	{
	public:
		virtual ~Callback();
		virtual void httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content) = 0;
	};

	HttpGetTask(const NPT_String& url, Callback *callback, bool autoDelete = true);
	virtual ~HttpGetTask();

protected:
	virtual void exec();
	virtual void doAbort();

private:
	void report(NPT_Result nr, NPT_HttpResponse *resp = NULL, const NPT_DataBuffer *data = NULL);

private:
	NPT_String m_url;
	Callback *m_callback;
	bool m_autoDelete;
	NPT_HttpClient m_httpClient;
	NPT_HttpResponse *m_resp;
};

class DeviceDiscoveryTask
	: public Task
{
public:
	class Callback
	{
	public:
		virtual ~Callback();
		virtual void deviceDiscoveryTaskFinished(DeviceDiscoveryTask *task, NPT_Result nr, DeviceDesc *rootDeviceDesc, const UUID& uuid) = 0;
	};

	DeviceDiscoveryTask(const NPT_String& location, const UUID& rootDeviceUuid, Callback *callback, bool autoDelete = true);
	virtual ~DeviceDiscoveryTask();

	const NPT_String& location() const;

protected:
	virtual void exec();
	virtual void doAbort();

private:
	class HttpGetDescContext
		: public HttpGetTask::Callback
	{
	public:
		HttpGetDescContext(NPT_DataBuffer& output, NPT_SharedVariable *signalVar);
		virtual ~HttpGetDescContext();

	protected:
		virtual void httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content);

	private:
		NPT_SharedVariable *m_signalVar;
		NPT_DataBuffer& m_output;
	};

	class HttpGetScpdContext
		: public HttpGetTask::Callback
	{
	public:
		HttpGetScpdContext(ServiceDesc *serviceDesc, NPT_AtomicVariable *avar, NPT_SharedVariable *signalVar);
		virtual ~HttpGetScpdContext();

	protected:
		virtual void httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content);

	private:
		ServiceDesc *m_serviceDesc;
		NPT_AtomicVariable *m_avar;
		NPT_SharedVariable *m_signalVar;
	};

	class HttpGetIconContext
		: public HttpGetTask::Callback
	{
	public:
		HttpGetIconContext(IconDesc *iconDesc, NPT_AtomicVariable *avar, NPT_SharedVariable *signalVar);
		virtual ~HttpGetIconContext();

	protected:
		virtual void httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content);

	private:
		IconDesc *m_iconDesc;
		NPT_AtomicVariable *m_avar;
		NPT_SharedVariable *m_signalVar;
	};

	void report(NPT_Result nr, DeviceDesc *rootDeviceDesc = NULL);

private:
	NPT_String m_location;
	UUID m_rootDeviceUuid;
	Callback *m_callback;
	bool m_autoDelete;
};

} // namespace deejay

#endif // __DJTasks_h__
