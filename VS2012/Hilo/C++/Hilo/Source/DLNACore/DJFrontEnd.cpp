#include "pch.h"
#include "DJFrontEnd.h"
#include "DJControlPoint.h"
#include "DJDeviceImpl.h"

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("deejay.FrontEnd")

namespace deejay {

FrontEnd::FrontEnd()
	: m_state(State_Stopped)
	, m_includeLoopback(false)
{
	m_taskGroup = new TaskGroup();
	m_userAgentHeader = NPT_String("DEEJAY/1.0");
	setOSVersion("WTF/1.0");
}

FrontEnd::~FrontEnd()
{
	stop();
	delete m_taskGroup;
}

void FrontEnd::Interface::httpConnectorOnNewClient(HttpConnector *connector, NPT_Socket *client)
{
	ReadLocker locker(m_owner->m_stateLock);
	if (m_owner->m_state == State_Running) {
		m_owner->m_taskGroup->startTask(new HttpServerTask(m_owner, this, client));
	}
}

void FrontEnd::Interface::ssdpConnectorOnPacket(SsdpConnector *connector, const char *data, NPT_Size length, const NPT_SocketAddress& fromAddr)
{
	ReadLocker locker(m_owner->m_stateLock);
	if (m_owner->m_state == State_Running) {
		m_owner->m_taskGroup->startTask(new SsdpServerTask(m_owner, this, data, length, fromAddr));
	}
}

void FrontEnd::setIncludeLoopback(bool includeLoopback)
{
	m_includeLoopback = includeLoopback;
}

void FrontEnd::setOSVersion(const NPT_String& osver)
{
	WriteLocker locker(m_stateLock);
	m_serverHeader = NPT_String::Format("%s DLNADOC/1.50 UPnP/1.0 DEEJAY/1.0", osver.GetChars());
	Helper::setUserAgentString(m_serverHeader);
}

bool FrontEnd::started() const
{
	ReadLocker locker(m_stateLock);
	return m_state == State_Running;
}

NPT_Result FrontEnd::start()
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Stopped) {
		return NPT_ERROR_INVALID_STATE;
	}

	NPT_Result nr;

	NPT_List<NPT_NetworkInterface*> ifList;
	nr = NPT_NetworkInterface::GetNetworkInterfaces(ifList);
	if (NPT_FAILED(nr)) {
		return nr;
	}

	for (NPT_Ordinal i = 0; i < ifList.GetItemCount(); i++) {
		NPT_NetworkInterface *nif = *ifList.GetItem(i);
		if (nif->GetAddresses().GetFirstItem() && (m_includeLoopback || ((nif->GetFlags() & NPT_NETWORK_INTERFACE_FLAG_LOOPBACK) == 0))) {
			Interface *intf = new Interface();
			intf->m_owner = this;
			intf->m_nif = nif;
			intf->m_context.m_ifAddr = nif->GetAddresses().GetFirstItem()->GetPrimaryAddress();
			intf->m_httpConnector = new HttpConnector(intf, intf->m_context.m_ifAddr);
			intf->m_ssdpConnector = new SsdpConnector(intf, intf->m_context.m_ifAddr);
			intf->m_context.m_httpPort = 0;
			intf->m_context.m_ssdpPort = 0;
			m_ifList.Add(intf);
			m_interfaceContextList.Add(&intf->m_context);
		} else {
			delete nif;
		}
	}

	if (m_ifList.GetItemCount() == 0) {
		return NPT_FAILURE;
	}

	for (NPT_Ordinal i = 0; i < m_ifList.GetItemCount(); i++) {
		Interface *intf = *m_ifList.GetItem(i);
		if (NPT_SUCCEEDED(intf->m_httpConnector->start())) {
			intf->m_context.m_httpPort = intf->m_httpConnector->port();
		}
		if (NPT_SUCCEEDED(intf->m_ssdpConnector->start())) {
			intf->m_context.m_ssdpPort = intf->m_ssdpConnector->port();
		}
	}

	m_taskGroup->reset();

	{
		ReadLocker locker1(m_cpLock);
		for (NPT_Ordinal i = 0; i < m_controlPointList.GetItemCount(); i++) {
			ControlPointInfo *info = *m_controlPointList.GetItem(i);
			info->m_controlPoint->implAttach(this, info->m_context);
		}

		for (NPT_Ordinal i = 0; i < m_deviceImplList.GetItemCount(); i++) {
			DeviceImplInfo *info = *m_deviceImplList.GetItem(i);
			info->m_deviceImpl->implAttach(this, info->m_context);
		}

		broadcastLocked(NULL, true);
	}

	m_taskGroup->startTask(new SsdpBroadcastTask(this));

	m_state = State_Running;
	return NPT_SUCCESS;
}

void FrontEnd::stop()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_state != State_Running) {
			return;
		}
		m_state = State_Stopping;

		ReadLocker locker1(m_cpLock);
		broadcastLocked(NULL, false);
	}

	m_taskGroup->abort();

	{
		for (NPT_Ordinal i = 0; i < m_controlPointList.GetItemCount(); i++) {
			ControlPointInfo *info = *m_controlPointList.GetItem(i);
			info->m_controlPoint->implDetach();
		}

		for (NPT_Ordinal i = 0; i < m_deviceImplList.GetItemCount(); i++) {
			DeviceImplInfo *info = *m_deviceImplList.GetItem(i);
			info->m_deviceImpl->implDetach();
		}
	}

	m_taskGroup->wait();

	for (NPT_Ordinal i = 0; i != m_ifList.GetItemCount(); i++) {
		Interface *intf = *m_ifList.GetItem(i);
		intf->m_httpConnector->stop();
		intf->m_ssdpConnector->stop();
	}

	{
		WriteLocker locker(m_stateLock);

		m_interfaceContextList.Clear();

		for (NPT_Ordinal i = 0; i != m_ifList.GetItemCount(); i++) {
			Interface *intf = *m_ifList.GetItem(i);
			delete intf->m_httpConnector;
			delete intf->m_ssdpConnector;
			delete intf->m_nif;
			delete intf;
		}
		m_ifList.Clear();
		m_state = State_Stopped;
	}
}

const NPT_List<const FrontEnd::InterfaceContext*>& FrontEnd::interfaceContextList() const
{
	return m_interfaceContextList;
}

void FrontEnd::addControlPoint(ControlPoint *controlPoint)
{
	ReadLocker locker1(m_stateLock);
	WriteLocker locker2(m_cpLock);
	if (controlPoint) {
		NPT_List<ControlPointInfo*>::Iterator it = m_controlPointList.Find(ControlPointInfoFinder(controlPoint));
		if (!it) {
			ControlPointInfo *controlPointInfo = new ControlPointInfo();
			controlPointInfo->m_controlPoint = controlPoint;
			controlPointInfo->m_context.m_httpRoot = NPT_String::Format("/cp%p/", controlPoint);
			controlPointInfo->m_context.m_userAgentHeader = m_userAgentHeader;
			controlPointInfo->m_context.m_serverHeader = m_serverHeader;
			m_controlPointList.Add(controlPointInfo);
			if (m_state == State_Running) {
				controlPoint->implAttach(this, controlPointInfo->m_context);
			}
		}
	}
}

void FrontEnd::removeControlPoint(ControlPoint *controlPoint)
{
	ReadLocker locker1(m_stateLock);
	WriteLocker locker2(m_cpLock);
	if (controlPoint) {
		NPT_List<ControlPointInfo*>::Iterator it = m_controlPointList.Find(ControlPointInfoFinder(controlPoint));
		if (it) {
			ControlPointInfo *controlPointInfo = *it;
			if (m_state == State_Running) {
				controlPoint->implDetach();
			}
			m_controlPointList.Erase(it);
			delete controlPointInfo;
		}
	}
}

bool FrontEnd::hasControlPoint(ControlPoint *controlPoint) const
{
	ReadLocker locker(m_cpLock);
	NPT_List<ControlPointInfo*>::Iterator it = m_controlPointList.Find(ControlPointInfoFinder(controlPoint));
	if (it) {
		return true;
	}
	return false;
}

void FrontEnd::addDeviceImpl(DeviceImpl *deviceImpl)
{
	ReadLocker locker1(m_stateLock);
	WriteLocker locker2(m_dsLock);
	if (deviceImpl) {
		NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.Find(DeviceImplInfoFinder(deviceImpl));
		if (!it) {
			DeviceImplInfo *deviceImplInfo = new DeviceImplInfo;
			deviceImplInfo->m_deviceImpl = deviceImpl;
						deviceImplInfo->m_context.m_httpRoot = NPT_String::Format("/devices/%s/", deviceImpl->uuid().toString().GetChars());
			deviceImplInfo->m_context.m_userAgentHeader = m_userAgentHeader;
			deviceImplInfo->m_context.m_serverHeader = m_serverHeader;
			m_deviceImplList.Add(deviceImplInfo);
			m_deviceImplIndex.Put(deviceImpl->uuid(), deviceImplInfo);
			if (m_state == State_Running) {
				deviceImpl->implAttach(this, deviceImplInfo->m_context);
				//broadcastLocked(deviceImplInfo, false);
				broadcastLocked(deviceImplInfo, true);
			}
		}
	}
}

void FrontEnd::removeDeviceImpl(DeviceImpl *deviceImpl)
{
	ReadLocker locker1(m_stateLock);
	WriteLocker locker2(m_dsLock);
	if (deviceImpl) {
		NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.Find(DeviceImplInfoFinder(deviceImpl));
		if (it) {
			DeviceImplInfo *deviceImplInfo = *it;
			if (m_state == State_Running) {
				broadcastLocked(deviceImplInfo, false);
				deviceImpl->implDetach();
			}
			m_deviceImplIndex.Erase(deviceImpl->uuid());
			m_deviceImplList.Erase(it);
			delete deviceImplInfo;
		}
	}
}

bool FrontEnd::hasDeviceImpl(DeviceImpl *deviceImpl) const
{
	ReadLocker locker(m_dsLock);
	NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.Find(DeviceImplInfoFinder(deviceImpl));
	if (it) {
		return true;
	}
	return false;
}

FrontEnd::HttpServerTask::HttpServerTask(FrontEnd *owner, Interface *intf, NPT_Socket *client)
	: m_owner(owner), m_intf(intf), m_client(client)
{
}

FrontEnd::HttpServerTask::~HttpServerTask()
{
	if (m_client) {
		delete m_client;
	}
}

void FrontEnd::HttpServerTask::detach()
{
	WriteLocker locker(m_stateLock);
	if (m_client) {
		m_client = NULL;
	}
}

void FrontEnd::HttpServerTask::exec()
{
	if (m_owner->m_state == State_Running) {
		m_owner->httpConnectorOnNewClient(this, m_intf, m_client);
	}
}

void FrontEnd::HttpServerTask::doAbort()
{
	ReadLocker locker(m_stateLock);
	for (NPT_Ordinal i = 0; i < m_callbackList.GetItemCount(); i++) {
		AbortableTask::Callback *callback = *m_callbackList.GetItem(i);
		callback->onAborted(this);
	}
	if (m_client) {
		m_client->Cancel();
	}
}

bool FrontEnd::HttpServerTask::registerAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	if (aborted()) {
		return false;
	}
	m_callbackList.Add(callback);
	return true;
}

void FrontEnd::HttpServerTask::unregisterAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	m_callbackList.Remove(callback, true);
}

FrontEnd::SsdpServerTask::SsdpServerTask(FrontEnd *owner, Interface *intf, const char *data, NPT_Size length, const NPT_SocketAddress& fromAddr)
	: m_owner(owner), m_intf(intf), m_data(data, length, true), m_fromAddr(fromAddr)
{
}

FrontEnd::SsdpServerTask::~SsdpServerTask()
{
}

void FrontEnd::SsdpServerTask::exec()
{
	if (m_owner->m_state == State_Running) {
		m_owner->ssdpConnectorOnPacket(this, m_intf, m_data, m_fromAddr);
	}
}

void FrontEnd::SsdpServerTask::doAbort()
{
	ReadLocker locker(m_stateLock);
	for (NPT_Ordinal i = 0; i < m_callbackList.GetItemCount(); i++) {
		AbortableTask::Callback *callback = *m_callbackList.GetItem(i);
		callback->onAborted(this);
	}
}

bool FrontEnd::SsdpServerTask::registerAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	if (aborted()) {
		return false;
	}
	m_callbackList.Add(callback);
	return true;
}

void FrontEnd::SsdpServerTask::unregisterAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	m_callbackList.Remove(callback, true);
}
/*
class HttpReadRequestCallback
	: public AbortableTask::Callback
{
public:
	HttpReadRequestCallback(NPT_Socket *client)
		: m_client(client)
	{
	}

	virtual void onAborted(AbortableTask *task)
	{
		m_client->Cancel();
	}

private:
	NPT_Socket *m_client;
};
*/

void FrontEnd::httpConnectorOnNewClient(HttpServerTask *task, Interface *intf, NPT_Socket *client)
{
	NPT_Result nr;
	NPT_InputStreamReference inputStream0;
	nr = client->GetInputStream(inputStream0);
	if (NPT_FAILED(nr)) {
		return;
	}

	NPT_OutputStreamReference outputStream;
	nr = client->GetOutputStream(outputStream);
	if (NPT_FAILED(nr)) {
		return;
	}

	NPT_BufferedInputStreamReference inputStream(new NPT_BufferedInputStream(inputStream0));

	NPT_HttpRequest *req;

	nr = NPT_HttpRequest::Parse(*inputStream.AsPointer(), NULL, req);
	if (NPT_FAILED(nr)) {
		return;
	}

	// TODO: validate "HOST" ???

	RequestContext reqCtx;
	reqCtx.clientHint = CH_Unknown;
	reqCtx.transferMode = TM_None;
	reqCtx.getcontentFeaturesReq = false;

	NPT_HttpHeader *hdrUserAgent = req->GetHeaders().GetHeader(NPT_HTTP_HEADER_USER_AGENT);
	if (hdrUserAgent) {
		if (hdrUserAgent->GetValue().Find("xbox", 0, true) >= 0) {
			NPT_LOG_INFO_1("XBox found [User-Agent: %s]", hdrUserAgent->GetValue().GetChars());
			reqCtx.clientHint = CH_XBox;
		}
	}

	NPT_HttpHeader *hdrTransferMode = req->GetHeaders().GetHeader("transferMode.dlna.org");
	if (hdrTransferMode) {
		const NPT_String& transferMode = hdrTransferMode->GetValue();
		if (transferMode.Compare("Streaming", true) == 0) {
			reqCtx.transferMode = TM_Streaming;
		} else if (transferMode.Compare("Interactive", true) == 0) {
			reqCtx.transferMode = TM_Interactive;
		} else if (transferMode.Compare("Background", true) == 0) {
			reqCtx.transferMode = TM_Background;
		} else {
			reqCtx.transferMode = TM_Unknown;
		}
	}

	NPT_HttpHeader *hdrGetContentFeatures = req->GetHeaders().GetHeader("getcontentFeatures.dlna.org");
	if (hdrGetContentFeatures) {
		NPT_String getContentFeatures = hdrGetContentFeatures->GetValue();
		if (getContentFeatures.Trim().Compare("1") == 0) {
			reqCtx.getcontentFeaturesReq = true;
		}
	}

	NPT_SocketInfo si;
	client->GetInfo(si);
	onHttpRequestHeader(si, req);

	PtrHolder<NPT_HttpRequest> req1(req);
	NPT_String reqPath(req->GetUrl().GetPath());

	NPT_TimeStamp ts;
	NPT_System::GetCurrentTimeStamp(ts);
	NPT_String dateStr = NPT_DateTime(ts).ToString(NPT_DateTime::FORMAT_RFC_1123);

	NPT_HttpResponse *resp = new NPT_HttpResponse(200, "OK", NPT_HTTP_PROTOCOL_1_1);
	PtrHolder<NPT_HttpResponse> resp1(resp);
	resp->GetHeaders().SetHeader(NPT_HTTP_HEADER_SERVER, m_serverHeader);
	resp->GetHeaders().SetHeader("Date", dateStr);
	resp->GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, "0");
	resp->GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, "text/xml");

	HttpOutput *httpOutput = new HttpOutputImpl(this, si, outputStream);
	PtrHolder<HttpOutput> httpOutput1(httpOutput);

	{
		ReadLocker locker(m_cpLock);
		for (NPT_Ordinal i = 0; i < m_controlPointList.GetItemCount(); i++) {
			NPT_List<ControlPointInfo*>::Iterator it = m_controlPointList.GetItem(i);
			ControlPointInfo *info = *it;
			if (reqPath.StartsWith(info->m_context.m_httpRoot)) {
				NPT_InputStream *input = inputStream.AsPointer();
				inputStream.Detach();
				httpOutput1.detach();
				resp1.detach();
				req1.detach();
				task->detach();
				return info->m_controlPoint->processHttpRequest(&intf->m_context, reqPath.SubString(info->m_context.m_httpRoot.GetLength()), reqCtx, req, resp, input, httpOutput, client);
			}
		}
	}

	{
		ReadLocker locker(m_dsLock);
		for (NPT_Ordinal i = 0; i < m_deviceImplList.GetItemCount(); i++) {
			NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.GetItem(i);
			DeviceImplInfo *info = *it;
			if (reqPath.StartsWith(info->m_context.m_httpRoot)) {
				NPT_InputStream *input = inputStream.AsPointer();
				inputStream.Detach();
				httpOutput1.detach();
				resp1.detach();
				req1.detach();
				task->detach();
				return info->m_deviceImpl->processHttpRequest(&intf->m_context, reqPath.SubString(info->m_context.m_httpRoot.GetLength()), reqCtx, req, resp, input, httpOutput, client);
			}
		}
	}

	setStatusCode(*resp, 404);
	httpOutput->writeResponseHeader(*resp);
	httpOutput->flush();

}

void FrontEnd::ssdpConnectorOnPacket(SsdpServerTask *task, Interface *intf, const NPT_DataBuffer& data, const NPT_SocketAddress& fromAddr)
{
	const char *dataStr = reinterpret_cast<const char*>(data.GetData());
	if (data.GetDataSize() > 10) {
		if (NPT_String::CompareN(dataStr, "NOTIFY", 6) == 0) {
			ReadLocker locker(m_cpLock);
			for (NPT_Ordinal i = 0; i < m_controlPointList.GetItemCount(); i++) {
				NPT_List<ControlPointInfo*>::Iterator it = m_controlPointList.GetItem(i);
				ControlPointInfo *info = *it;
				if (task->aborted()) {
					return;
				}
				info->m_controlPoint->processSsdpNotify(&intf->m_context, data);
			}
		} else if (NPT_String::CompareN(dataStr, "M-SEARCH", 8) == 0) {
			processSsdpSearch(task, intf, data, fromAddr);
		}
	}
}

FrontEnd::SsdpSearchAbortCallback::SsdpSearchAbortCallback(NPT_UdpSocket *socket, NPT_SharedVariable *waitVar)
	: m_socket(socket), m_waitVar(waitVar)
{
}

void FrontEnd::SsdpSearchAbortCallback::onAborted(AbortableTask *task)
{
	m_socket->Cancel();
	m_waitVar->SetValue(1);
}

void FrontEnd::processSsdpSearch(SsdpServerTask *task, Interface *intf, const NPT_DataBuffer& data, const NPT_SocketAddress& fromAddr)
{
	do {
		NPT_HttpRequest *req;
		NPT_InputStreamReference inputStream0(new NPT_MemoryStream(data.GetData(), data.GetDataSize()));
		NPT_BufferedInputStream inputStream(inputStream0);
		if (NPT_FAILED(NPT_HttpRequest::Parse(inputStream, NULL, req))) {
			break;
		}

		PtrHolder<NPT_HttpRequest> req1(req);
		if (req->GetMethod().Compare("M-SEARCH") != 0 || req->GetProtocol().Compare(NPT_HTTP_PROTOCOL_1_1) != 0 || req->GetUrl().GetPath().Compare("*") != 0) {
			break;
		}

		NPT_HttpHeader *hdrMan = req->GetHeaders().GetHeader("MAN");
		if (!hdrMan || hdrMan->GetValue().Compare("\"ssdp:discover\"") != 0) {
			break;
		}

		NPT_HttpHeader *hdrHost = req->GetHeaders().GetHeader("HOST");
		if (!hdrHost || (hdrHost->GetValue().Compare("239.255.255.250:1900") != 0 && hdrHost->GetValue().Compare("239.255.255.250") != 0)) {
			break;
		}

		int mx;
		NPT_HttpHeader *hdrMX = req->GetHeaders().GetHeader("MX");
		if (!hdrMX || NPT_FAILED(NPT_ParseInteger(hdrMX->GetValue(), mx)) || mx < 1) {
			break;
		}

		if (mx > 120) {
			mx = 120;
		}

		NPT_HttpHeader *hdrST = req->GetHeaders().GetHeader("ST");
		if (!hdrST) {
			break;
		}

		NPT_List<MatchContext*> matchList;

		NPT_UdpSocket sock(NPT_SOCKET_FLAG_CANCELLABLE);
		sock.Bind(NPT_SocketAddress(intf->m_context.m_ifAddr, 0));
		NPT_SharedVariable waitVar;
		waitVar.SetValue(0);

		{
			ReadLocker locker(m_dsLock);
			for (NPT_Ordinal i = 0; i < m_deviceImplList.GetItemCount(); i++) {
				NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.GetItem(i);
				DeviceImplInfo *info = *it;
				MatchContext *matchContext = new MatchContext();
				if (info->m_deviceImpl->match(hdrST->GetValue(), matchContext->matches)) {
					matchList.Add(matchContext);
					matchContext->deviceUuid = info->m_deviceImpl->uuid();
					matchContext->expireSeconds = info->m_deviceImpl->m_expireSeconds;
					matchContext->descPath = info->m_deviceImpl->m_descPath;
					matchContext->httpRoot = info->m_context.m_httpRoot;
				} else {
					delete matchContext;
				}
			}
		}

		SsdpSearchAbortCallback abortCallback(&sock, &waitVar);
		if (task->registerAbortCallback(&abortCallback)) {

			for (NPT_Ordinal i = 0; i < matchList.GetItemCount(); i++) {
				MatchContext *matchContext = *matchList.GetItem(i);

				NPT_String location = NPT_String::Format("http://%s:%d%s%s", intf->m_context.m_ifAddr.ToString().GetChars(), intf->m_context.m_httpPort, matchContext->httpRoot.GetChars(), matchContext->descPath.GetChars());

				bool broken = false;

				for (NPT_Ordinal j = 0; j < matchContext->matches.GetItemCount(); j++) {
					NPT_List<DeviceImplMatch>::Iterator it2 = matchContext->matches.GetItem(j);

					NPT_Timeout timeout = NPT_System::GetRandomInteger() % (mx * 1000);
					// TODO: wait or not ???
					timeout = 0;
					if (NPT_SUCCEEDED(waitVar.WaitWhileEquals(0, timeout))) {
						break;
					}

					{
						ReadLocker locker(m_dsLock);
						if (m_deviceImplIndex.HasKey(matchContext->deviceUuid)) {
							NPT_TimeStamp ts;
							NPT_System::GetCurrentTimeStamp(ts);
							NPT_String dateStr = NPT_DateTime(ts).ToString(NPT_DateTime::FORMAT_RFC_1123);
							NPT_String resp = NPT_String::Format("HTTP/1.1 200 OK\r\nCACHE-CONTROL: max-age=%d\r\nDATE: %s\r\nEXT: \r\nLOCATION: %s\r\nSERVER: %s\r\nST: %s\r\nUSN: %s\r\n\r\n", matchContext->expireSeconds, dateStr.GetChars(), location.GetChars(), m_serverHeader.GetChars(), it2->m_st.GetChars(), it2->m_usn.GetChars());
							NPT_DataBuffer packet(resp.GetChars(), resp.GetLength(), false);
							sock.Send(packet, &fromAddr);
						}
					}
				}

				if (broken) {
					break;
				}
			}

			task->unregisterAbortCallback(&abortCallback);
		}

		matchList.Apply(NPT_ObjectDeleter<MatchContext>());
	} while (false);
}

void FrontEnd::broadcastLocked(DeviceImplInfo *deviceInfo, bool avail)
{
	NPT_TimeStamp ts;
	NPT_System::GetCurrentTimeStamp(ts);

	NPT_List<MatchContext*> matchList;

	if (deviceInfo) {
		deviceInfo->m_updateTS = ts;
		MatchContext *matchContext = new MatchContext();
		if (deviceInfo->m_deviceImpl->match("ssdp:all", matchContext->matches)) {
			matchList.Add(matchContext);
			matchContext->deviceUuid = deviceInfo->m_deviceImpl->uuid();
			matchContext->expireSeconds = deviceInfo->m_deviceImpl->m_expireSeconds;
			matchContext->descPath = deviceInfo->m_deviceImpl->m_descPath;
			matchContext->httpRoot = deviceInfo->m_context.m_httpRoot;
		} else {
			delete matchContext;
		}
	} else {
		for (NPT_Ordinal i = 0; i < m_deviceImplList.GetItemCount(); i++) {
			NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.GetItem(i);
			DeviceImplInfo *info = *it;
			info->m_updateTS = ts;
			MatchContext *matchContext = new MatchContext();
			if (info->m_deviceImpl->match("ssdp:all", matchContext->matches)) {
				matchList.Add(matchContext);
				matchContext->deviceUuid = info->m_deviceImpl->uuid();
				matchContext->expireSeconds = info->m_deviceImpl->m_expireSeconds;
				matchContext->descPath = info->m_deviceImpl->m_descPath;
				matchContext->httpRoot = info->m_context.m_httpRoot;
			} else {
				delete matchContext;
			}
		}
	}

	NPT_SocketAddress targetAddr(NPT_IpAddress(239, 255, 255, 250), 1900);

	for (NPT_Ordinal i = 0; i < m_ifList.GetItemCount(); i++) {
		Interface *nif = *m_ifList.GetItem(i);
		NPT_UdpSocket sock(NPT_SOCKET_FLAG_CANCELLABLE);
		sock.Bind(NPT_SocketAddress(nif->m_context.m_ifAddr, 0));
		for (NPT_Ordinal j = 0; j < matchList.GetItemCount(); j++) {
			MatchContext *matchContext = *matchList.GetItem(j);
			NPT_String location = NPT_String::Format("http://%s:%d%s%s", nif->m_context.m_ifAddr.ToString().GetChars(), nif->m_context.m_httpPort, matchContext->httpRoot.GetChars(), matchContext->descPath.GetChars());
			for (NPT_Ordinal k = 0; k < matchContext->matches.GetItemCount(); k++) {
				NPT_List<DeviceImplMatch>::Iterator it2 = matchContext->matches.GetItem(k);
				NPT_String msg;
				if (avail) {
					msg = NPT_String::Format("NOTIFY * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nCACHE-CONTROL: max-age=%d\r\nLOCATION: %s\r\nNT: %s\r\nNTS: ssdp:alive\r\nSERVER: %s\r\nUSN: %s\r\n\r\n", matchContext->expireSeconds, location.GetChars(), it2->m_st.GetChars(), m_serverHeader.GetChars(), it2->m_usn.GetChars());
				} else {
					msg = NPT_String::Format("NOTIFY * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nNT: %s\r\nNTS: ssdp:byebye\r\nUSN: %s\r\n\r\n", it2->m_st.GetChars(), it2->m_usn.GetChars());
				}
				NPT_DataBuffer packet(msg.GetChars(), msg.GetLength(), false);
				sock.Send(packet, &targetAddr);
			}
		}
	}

	matchList.Apply(NPT_ObjectDeleter<MatchContext>());
}

void FrontEnd::broadcastIfNecessary()
{
	ReadLocker locker1(m_stateLock);
	if (m_state != State_Running) {
		return;
	}

	NPT_TimeStamp ts;
	NPT_System::GetCurrentTimeStamp(ts);
	NPT_Int64 tsMillis = ts.ToMillis();
	WriteLocker locker2(m_dsLock);
	for (NPT_Ordinal i = 0; i < m_deviceImplList.GetItemCount(); i++) {
		NPT_List<DeviceImplInfo*>::Iterator it = m_deviceImplList.GetItem(i);
		DeviceImplInfo *info = *it;
		if (info->m_updateTS.ToMillis() + info->m_deviceImpl->m_expireSeconds * 1000 - 11000 < tsMillis) {
			broadcastLocked(info, true);
		}
	}
}

FrontEnd::SsdpBroadcastTask::SsdpBroadcastTask(FrontEnd *owner)
	: m_owner(owner)
{
	m_waitVar.SetValue(0);
}

FrontEnd::SsdpBroadcastTask::~SsdpBroadcastTask()
{
}

void FrontEnd::SsdpBroadcastTask::exec()
{
	while (!aborted()) {
		if (NPT_SUCCEEDED(m_waitVar.WaitWhileEquals(0, 5000))) {
			break;
		}
		m_owner->broadcastIfNecessary();
	}
}

void FrontEnd::SsdpBroadcastTask::doAbort()
{
	m_waitVar.SetValue(1);
}

FrontEnd::HttpOutputImpl::HttpOutputImpl(FrontEnd *frontEnd, const NPT_SocketInfo& socketInfo, NPT_OutputStreamReference outputStream)
	: m_frontEnd(frontEnd), m_socketInfo(socketInfo), m_outputStream(outputStream)
{
}

FrontEnd::HttpOutputImpl::~HttpOutputImpl()
{
	m_outputStream->Flush();
}

void FrontEnd::HttpOutputImpl::writeResponseHeader(const NPT_HttpResponse& response)
{
	m_frontEnd->onHttpResponseHeader(m_socketInfo, &response);
	response.Emit(*m_outputStream);
	m_outputStream->Flush();
}

void FrontEnd::HttpOutputImpl::writeData(const NPT_String& data)
{
	m_outputStream->WriteFully(data.GetChars(), data.GetLength());
}

void FrontEnd::HttpOutputImpl::writeData(const NPT_DataBuffer& data)
{
	m_outputStream->WriteFully(data.GetData(), data.GetDataSize());
}

void FrontEnd::HttpOutputImpl::writeData(const void *data, NPT_Size length)
{
	m_outputStream->WriteFully(data, length);
}

void FrontEnd::HttpOutputImpl::flush()
{
	m_outputStream->Flush();
}

void FrontEnd::onHttpRequestHeader(const NPT_SocketInfo& socketInfo, const NPT_HttpRequest *request)
{
	NPT_LOG_FINER_2("%s %s", request->GetMethod().GetChars(), request->GetUrl().ToString().GetChars());
	NPT_LOG_FINER_2("from %s:%d", socketInfo.remote_address.GetIpAddress().ToString().GetChars(), socketInfo.remote_address.GetPort());
	const NPT_List<NPT_HttpHeader*>& ls0 = request->GetHeaders().GetHeaders();
	for (NPT_Ordinal i = 0; i < ls0.GetItemCount(); i++) {
		NPT_HttpHeader *hh = *ls0.GetItem(i);
		NPT_LOG_FINER_2("[%s]=[%s]", hh->GetName().GetChars(), hh->GetValue().GetChars());
	}

}

void FrontEnd::onHttpResponseHeader(const NPT_SocketInfo& socketInfo, const NPT_HttpResponse *response)
{
	const NPT_List<NPT_HttpHeader*>& ls = response->GetHeaders().GetHeaders();
	NPT_LOG_FINER_2("RESP: %d %s", response->GetStatusCode(), response->GetReasonPhrase().GetChars());
	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		NPT_HttpHeader *hh = *ls.GetItem(i);
		NPT_LOG_FINER_2("[%s]=[%s]", hh->GetName().GetChars(), hh->GetValue().GetChars());
	}
}

} // namespace deejay
