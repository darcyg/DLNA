#include "DJControlPoint.h"
#include "DJDescPriv.h"

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("deejay.DeviceImpl")

namespace deejay {

const int DEFAULT_SUBSCRIPTION_TIMEOUT = 300;

ControlPoint::ControlPoint(Callback *callback)
	: m_callback(callback), m_state(State_Detached)
	, m_frontEnd(NULL), m_simpleContentIndex(1000)
{
	m_attachedTaskGroup = new TaskGroup();
}

ControlPoint::~ControlPoint()
{
	delete m_attachedTaskGroup;
}

ControlPoint::SearchContext::SearchContext(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx)
	: m_owner(owner), m_ifctx(ifctx)
{
}

ControlPoint::SearchContext::~SearchContext()
{
}

bool ControlPoint::SearchContext::ssdpSearchTaskOnPacket(SsdpSearchTask *task, const NPT_DataBuffer& packet)
{
	m_owner->processSsdpSearchResponse(m_ifctx, packet);
	return false;
}

void ControlPoint::SearchContext::ssdpSearchTaskFinished(SsdpSearchTask *task, NPT_Result nr)
{
}

ControlPoint::DiscoveryContext::DiscoveryContext(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_String& server, int maxAge)
	: m_owner(owner), m_ifctx(ifctx), m_server(server), m_maxAge(maxAge)
{
}

ControlPoint::DiscoveryContext::~DiscoveryContext()
{
}

void ControlPoint::DiscoveryContext::deviceDiscoveryTaskFinished(DeviceDiscoveryTask *task, NPT_Result nr, DeviceDesc *rootDeviceDesc, const UUID& uuid)
{
	m_owner->processDiscoveryResult(nr, m_ifctx, task->location(), m_server, m_maxAge, rootDeviceDesc, uuid);
}

void ControlPoint::implAttach(FrontEnd *frontEnd, const FrontEnd::ControlPointContext& context)
{
	NPT_LOG_FINEST("implAttach checkpoint 1");
	WriteLocker locker(m_stateLock);
	NPT_LOG_FINEST("implAttach checkpoint 2");
	if (m_state != State_Detached) {
		NPT_LOG_FINEST("implAttach checkpoint 3");
		return;
	}

	NPT_LOG_FINEST("implAttach checkpoint 4");
	m_attachedTaskGroup->reset();
	NPT_LOG_FINEST("implAttach checkpoint 5");

	m_frontEnd = frontEnd;
	m_frontEndContext = context;
	// TODO: necessary ??
	NPT_LOG_FINEST("implAttach checkpoint 6");
	ssdpSearchLocked("ssdp:all", 3);
	NPT_LOG_FINEST("implAttach checkpoint 7");

	m_removeExpiredDevicesVar.SetValue(0);
	NPT_LOG_FINEST("implAttach checkpoint 8");
	m_attachedTaskGroup->startTask(new RemoveExpiredDevicesTask(this));
	NPT_LOG_FINEST("implAttach checkpoint 9");

	m_state = State_Attached;
	NPT_LOG_FINEST("implAttach checkpoint 10");
}

void ControlPoint::implDetach()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_state != State_Attached) {
			return;
		}
		m_state = State_Detaching;
	}

	m_attachedTaskGroup->abort();
	NPT_LOG_INFO_2("ControlPoint for %p waiting TaskGroup %p", this, &m_attachedTaskGroup);
	m_attachedTaskGroup->wait();

	{
		WriteLocker locker(m_stateLock);
		m_state = State_Detached;
		m_frontEnd = NULL;

		for (NPT_Ordinal i = 0; i < m_rootDeviceList.GetItemCount(); i++) {
			RootDevice *rootDevice = *m_rootDeviceList.GetItem(m_rootDeviceList.GetItemCount() - i - 1);
			deleteRootDevice(rootDevice);
		}

		m_rootDeviceList.Clear();
		m_rootDeviceIndexByUUID.Clear();
	}
}

void ControlPoint::ssdpSearch(const NPT_String& st, NPT_UInt32 mx)
{
	ReadLocker locker(m_stateLock);
	if (m_state == State_Attached) {
		ssdpSearchLocked(st, mx);
	}
}

void ControlPoint::forceRemoveRootDevice(const UUID& uuid)
{
	ssdpRemoveRootDevice(uuid);
}

void ControlPoint::forceRemoveAllDevices()
{
	WriteLocker locker(m_stateLock);

	for (NPT_Ordinal i = 0; i < m_rootDeviceList.GetItemCount(); i++) {
		RootDevice *rootDevice = *m_rootDeviceList.GetItem(m_rootDeviceList.GetItemCount() - i - 1);
		deleteRootDevice(rootDevice);
	}

	m_rootDeviceList.Clear();
	m_rootDeviceIndexByUUID.Clear();
}

void ControlPoint::forceRemoveDevicesByType(const NPT_String& deviceType)
{
	WriteLocker locker(m_stateLock);

	NPT_Cardinal count = m_rootDeviceList.GetItemCount();
	for (NPT_Ordinal i = 0; i < count; ) {
		NPT_List<RootDevice*>::Iterator it = m_rootDeviceList.GetItem(i);
		RootDevice *rootDevice = *it;
		if (rootDevice->m_deviceDesc->matchDeviceType(deviceType)) {
			m_rootDeviceIndexByUUID.Erase(rootDevice->m_deviceDesc->uuid());
			m_rootDeviceList.Erase(it);
			deleteRootDevice(rootDevice);
			count--;
		} else {
			i++;
		}
	}
}

DeviceDescList ControlPoint::snapshotDeviceList() const
{
	ReadLocker locker(m_stateLock);
	DeviceDescList ls;
	for (NPT_Ordinal i = 0; i < m_deviceList.GetItemCount(); i++) {
		DeviceDesc *deviceDesc = (*m_deviceList.GetItem(i))->m_deviceDesc;
		ls.add(deviceDesc);
	}
	return ls;
}

void ControlPoint::subscribeEvents(const UUID& uuid)
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Attached) {
		return;
	}

	NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(uuid));
	if (it) {
		subscribeEventsLocked(*(*it));
	}
}

void ControlPoint::unsubscribeEvents(const UUID& uuid)
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Attached) {
		return;
	}

	NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(uuid));
	if (it) {
		unsubscribeEventsLocked(*(*it));
	}
}

NPT_Result ControlPoint::invokeAction(const UUID& uuid, const NPT_String& serviceId, const NPT_String& actionName, const NPT_List<NPT_String>& inputArgs, ActionCallback *callback, ActionInstance*& instance)
{
	ReadLocker locker(m_stateLock);
	if (m_state != State_Attached) {
		return NPT_ERROR_INVALID_STATE;
	}

	NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(uuid));
	if (!it) {
		// no device
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DeviceInfo *deviceInfo = *it;
	const ServiceDesc *serviceDesc = NULL;
	for (NPT_Ordinal i = 0; i < deviceInfo->m_deviceDesc->serviceCount(); i++) {
		serviceDesc = deviceInfo->m_deviceDesc->serviceAt(i);
		if (serviceDesc->serviceId().Compare(serviceId) == 0) {
			break;
		}
	}

	if (!serviceDesc) {
		// no service
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	const ActionDesc *actionDesc = NULL;
	for (NPT_Ordinal i = 0; i < serviceDesc->actionCount(); i++) {
		if (serviceDesc->actionAt(i)->name().Compare(actionName) == 0) {
			actionDesc = serviceDesc->actionAt(i);
			break;
		}
	}

	if (!actionDesc) {
		// no action
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	if (actionDesc->inputArgumentCount() != inputArgs.GetItemCount()) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	NPT_List<NPT_String> inputArgNames;
	NPT_List<NPT_String> outputArgNames;
	for (NPT_Ordinal i = 0; i < actionDesc->argumentCount(); i++) {
		ArgumentDesc *argDesc = actionDesc->argumentAt(i);
		if (argDesc->isInputArg()) {
			inputArgNames.Add(argDesc->name());
		} else {
			outputArgNames.Add(argDesc->name());
		}
	}

	NPT_Result nr;
	MCActionImpl *actImpl = new MCActionImpl(callback, outputArgNames);
	actImpl->addRef();
	SoapTask *task = new SoapTask(serviceDesc->controlURL(), serviceDesc->serviceType(), actionName, inputArgNames, inputArgs, outputArgNames, actImpl);
	nr = m_attachedTaskGroup->startTask(task);
	if (NPT_FAILED(nr)) {
		actImpl->release();
		return nr;
	}

	instance = actImpl;
	return NPT_SUCCESS;
}

NPT_Result ControlPoint::setPlaylistToMediaRenderer(const UUID& uuid, const NPT_String& serviceId, const NPT_String& mimeType, const NPT_String& content, ActionCallback *callback, ActionInstance*& instance)
{
	NPT_Result nr;
	NPT_String url;
	NPT_String metaData;

	{
		WriteLocker locker(m_stateLock);
		if (m_state != State_Attached) {
			return NPT_ERROR_INVALID_STATE;
		}

		NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(uuid));
		if (!it) {
			// no device
			return NPT_ERROR_NO_SUCH_ITEM;
		}

		DeviceInfo *deviceInfo = *it;
		const ServiceDesc *serviceDesc = NULL;
		for (NPT_Ordinal i = 0; i < deviceInfo->m_deviceDesc->serviceCount(); i++) {
			serviceDesc = deviceInfo->m_deviceDesc->serviceAt(i);
			if (serviceDesc->serviceId().Compare(serviceId) == 0) {
				break;
			}
		}

		if (!serviceDesc) {
			// no service
			return NPT_ERROR_NO_SUCH_ITEM;
		}

		NPT_String playlistName = NPT_String::FromIntegerU(m_simpleContentIndex++);
		playlistName += ".wpl";
		url = NPT_String::Format("http://%s:%d%spls/%s", deviceInfo->m_rootDevice->m_ifctx->m_ifAddr.ToString().GetChars(), deviceInfo->m_rootDevice->m_ifctx->m_httpPort, m_frontEndContext.m_httpRoot.GetChars(), playlistName.GetChars());

		SimpleContent sc;
		sc.content = content;
		sc.mimeType = mimeType;
		m_simpleContentMap.Put(playlistName, sc);
	}

	NPT_String protocolInfo = NPT_String::Format("http-get:*:%s:*", mimeType.GetChars());

	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, true, true);
	xml.StartDocument();
	xml.StartElement(NULL, "DIDL-Lite");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
	xml.Attribute("xmlns", "upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
	xml.Attribute("xmlns", "dc", "http://purl.org/dc/elements/1.1/");
	xml.StartElement(NULL, "item");
	xml.Attribute(NULL, "id", "2");
	xml.Attribute(NULL, "parentID", "1");
	xml.Attribute(NULL, "restricted", "1");

	xml.StartElement("dc", "title");
	xml.Text("playlist1");
	xml.EndElement("dc", "title");
/*
	xml.StartElement("dc", "creator");
	xml.Text("DeeJay");
	xml.EndElement("dc", "creator");
*/
	xml.StartElement("upnp", "class");
	xml.Text("object.item.playlist");
	xml.EndElement("upnp", "class");

	xml.StartElement(NULL, "res");
	xml.Attribute(NULL, "protocolInfo", protocolInfo);
	xml.Text(url);
	xml.EndElement(NULL, "res");

	xml.EndElement(NULL, "item");
	xml.EndElement(NULL, "DIDL-Lite");
	xml.EndDocument();

	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	inputArgs.Add(url);
	inputArgs.Add(metaData);
	nr = invokeAction(uuid, serviceId, "SetAVTransportURI", inputArgs, callback, instance);
	return nr;
}

NPT_Result ControlPoint::serveFileToMediaRenderer(const UUID& uuid, const NPT_String& serviceId, const NPT_String& filePath, const NPT_String& mimeType, const NPT_String& metaData, const NPT_String& placeholder, ActionCallback *callback, ActionInstance*& instance)
{
	NPT_Result nr;
	NPT_String url;

	{
		WriteLocker locker(m_stateLock);
		if (m_state != State_Attached) {
			return NPT_ERROR_INVALID_STATE;
		}

		NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(uuid));
		if (!it) {
			// no device
			return NPT_ERROR_NO_SUCH_ITEM;
		}

		DeviceInfo *deviceInfo = *it;
		const ServiceDesc *serviceDesc = NULL;
		for (NPT_Ordinal i = 0; i < deviceInfo->m_deviceDesc->serviceCount(); i++) {
			serviceDesc = deviceInfo->m_deviceDesc->serviceAt(i);
			if (serviceDesc->serviceId().Compare(serviceId) == 0) {
				break;
			}
		}

		if (!serviceDesc) {
			// no service
			return NPT_ERROR_NO_SUCH_ITEM;
		}

		NPT_String fName = NPT_String::FromIntegerU(m_simpleContentIndex++);
		fName += NPT_FilePath::FileExtension(filePath);
		url = NPT_String::Format("http://%s:%d%sfiles/%s", deviceInfo->m_rootDevice->m_ifctx->m_ifAddr.ToString().GetChars(), deviceInfo->m_rootDevice->m_ifctx->m_httpPort, m_frontEndContext.m_httpRoot.GetChars(), fName.GetChars());

		SimpleContent sc;
		sc.content = filePath;
		sc.mimeType = mimeType;
		m_simpleContentMap.Put(fName, sc);
	}

	NPT_String metaData2 = metaData;
	metaData2.Replace(placeholder, url);

	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	inputArgs.Add(url);
	inputArgs.Add(metaData2);
	nr = invokeAction(uuid, serviceId, "SetAVTransportURI", inputArgs, callback, instance);
	return nr;
}

NPT_Result ControlPoint::remapMediaURL(const UUID& deviceUuid, NPT_String& url)
{
/*	ReadLocker locker(m_stateLock);
	NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(deviceUuid));
	if (!it) {
		// no device
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	NPT_Url uu(url);




	DeviceInfo *devInfo = *it;
	if (devInfo->m_rootDevice->m_ifctx->m_ifAddr.ToString().Compare(uu.GetHost()) != 0) {
		uu.GetHost();
	}
*/
	return NPT_SUCCESS;
}

NPT_Result ControlPoint::startTask(Task *task)
{
	return m_attachedTaskGroup->startTask(task);
}

ControlPoint::SubscribeTask2::SubscribeTask2(ControlPoint *owner, const UUID& deviceUuid, const NPT_String& serviceId, const NPT_String& eventSubURL, const NPT_String& callbackURL, int timeoutSeconds, const UUID& sid)
	: m_owner(owner), m_deviceUuid(deviceUuid), m_serviceId(serviceId), m_eventSubURL(eventSubURL), m_callbackURL(callbackURL), m_timeoutSeconds(timeoutSeconds), m_sid(sid)
{
}

ControlPoint::SubscribeTask2::~SubscribeTask2()
{
}

void ControlPoint::SubscribeTask2::exec()
{
	NPT_HttpRequest req(m_eventSubURL, "SUBSCRIBE", NPT_HTTP_PROTOCOL_1_1);
	Helper::setupHttpRequest(req);
	if (m_sid.isNull()) {
		req.GetHeaders().SetHeader("CALLBACK", m_callbackURL);
		req.GetHeaders().SetHeader("NT", "upnp:event");
		req.GetHeaders().SetHeader("TIMEOUT", NPT_String::Format("Second-%d", m_timeoutSeconds));
	} else {
		req.GetHeaders().SetHeader("SID", NPT_String::Format("uuid:%s", m_sid.toString().GetChars()));
		req.GetHeaders().SetHeader("TIMEOUT", NPT_String::Format("Second-%d", m_timeoutSeconds));
	}

	NPT_HttpResponse *resp;
	NPT_Result nr;
	nr = m_httpClient.SendRequest(req, resp);
	if (NPT_SUCCEEDED(nr)) {
		PtrHolder<NPT_HttpResponse> resp1(resp);
		if (resp->GetStatusCode() == 200) {
			NPT_HttpHeader *hdrSid = resp->GetHeaders().GetHeader("SID");
			NPT_HttpHeader *hdrTimeout = resp->GetHeaders().GetHeader("TIMEOUT");
			UUID sid;
			int timeoutSeconds;
			if (hdrSid && hdrTimeout && extractUuidFromUSN(hdrSid->GetValue(), sid) && parseTimeoutSecond(hdrTimeout->GetValue(), timeoutSeconds)) {
				if (timeoutSeconds <= 0) {
					timeoutSeconds = m_timeoutSeconds;
				}
				m_owner->onSubscribeFinished(m_deviceUuid, m_serviceId, NPT_SUCCESS, timeoutSeconds, sid);
			} else {
				m_owner->onSubscribeFinished(m_deviceUuid, m_serviceId, NPT_FAILURE, -1, UUID());
			}
		} else {
			m_owner->onSubscribeFinished(m_deviceUuid, m_serviceId, NPT_FAILURE, -1, UUID());
		}
	} else {
		m_owner->onSubscribeFinished(m_deviceUuid, m_serviceId, nr, -1, UUID());
	}
}

void ControlPoint::SubscribeTask2::doAbort()
{
	m_httpClient.Abort();
}

void ControlPoint::subscribeEventsLocked(DeviceInfo& deviceInfo)
{
	for (NPT_Ordinal i = 0; i < deviceInfo.m_extraServiceInfoList.GetItemCount(); i++) {
		ExtraServiceInfo *extraServiceInfo = *deviceInfo.m_extraServiceInfoList.GetItem(i);
		if (extraServiceInfo->m_eventSubState == EventSubState_Idle || extraServiceInfo->m_eventSubState == EventSubState_TempFailure) {
			extraServiceInfo->m_eventSubState = EventSubState_Pending;
			NPT_String callbackURL(NPT_String::Format("<http://%s:%d%s>", deviceInfo.m_rootDevice->m_ifctx->m_ifAddr.ToString().GetChars(), deviceInfo.m_rootDevice->m_ifctx->m_httpPort, extraServiceInfo->m_callbackURL.GetChars()));
			m_attachedTaskGroup->startTask(new SubscribeTask2(this, deviceInfo.m_deviceDesc->uuid(), extraServiceInfo->m_serviceDesc->serviceId(), extraServiceInfo->m_eventSubURL, callbackURL, DEFAULT_SUBSCRIPTION_TIMEOUT, extraServiceInfo->m_sid));
		}
	}
}

void ControlPoint::onSubscribeFinished(const UUID& deviceUuid, const NPT_String& serviceId, NPT_Result status, int timeout, const UUID& sid)
{
	WriteLocker locker(m_stateLock);
	NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoUuidFinder(deviceUuid));
	if (it) {
		DeviceInfo *deviceInfo = *it;
		NPT_List<ExtraServiceInfo*>::Iterator it2 = deviceInfo->m_extraServiceInfoList.Find(ServiceInfoByIdFinder(serviceId));
		if (it2) {
			ExtraServiceInfo *extraServiceInfo = *it2;
			if (status == NPT_SUCCESS) {
				extraServiceInfo->m_eventSubState = EventSubState_On;
				extraServiceInfo->m_eventSubTimeout = timeout;
				extraServiceInfo->m_sid = sid;
				NPT_System::GetCurrentTimeStamp(extraServiceInfo->m_eventSubUpdateTS);
			} else if (status == NPT_ERROR_EOS) {
				extraServiceInfo->m_eventSubState = EventSubState_None;
			} else {
				extraServiceInfo->m_eventSubState = EventSubState_TempFailure;
			}
		}
	}
}

void ControlPoint::unsubscribeEventsLocked(DeviceInfo& deviceInfo)
{
	for (NPT_Ordinal i = 0; i < deviceInfo.m_extraServiceInfoList.GetItemCount(); i++) {
		ExtraServiceInfo *extraServiceInfo = *deviceInfo.m_extraServiceInfoList.GetItem(i);
		extraServiceInfo->m_eventSubState = EventSubState_Idle;
		extraServiceInfo->m_sid = UUID();
	}
}

void ControlPoint::ssdpSearchLocked(const NPT_String& st, NPT_UInt32 mx)
{
	for (NPT_Ordinal i = 0; i < m_frontEnd->interfaceContextList().GetItemCount(); i++) {
		const FrontEnd::InterfaceContext *ifctx = *m_frontEnd->interfaceContextList().GetItem(i);
		m_attachedTaskGroup->startTask(new SsdpSearchTask(st, mx, ifctx->m_ifAddr, new SearchContext(this, ifctx), true));
	}
}

void ControlPoint::ssdpDiscoverRootDevice(const FrontEnd::InterfaceContext *ifctx, const UUID& uuid, const NPT_String& location, const NPT_String& server, int maxAge)
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Attached) {
		return;
	}

	RootDevice **dummy;
	if (NPT_SUCCEEDED(m_rootDeviceIndexByUUID.Get(uuid, dummy))) {
		RootDevice *rootDevice = *dummy;
		NPT_System::GetCurrentTimeStamp(rootDevice->m_updateTS);
		return;
	}

	DeviceDiscoveryTask **dummy2;
	if (NPT_SUCCEEDED(m_pendingSsdpDiscoverTasks.Get(uuid, dummy2))) {
		return;
	}

	DeviceDiscoveryTask *discoveryTask = new DeviceDiscoveryTask(location, uuid, new DiscoveryContext(this, ifctx, server, maxAge), true);
	m_pendingSsdpDiscoverTasks.Put(uuid, discoveryTask);
	m_attachedTaskGroup->startTask(discoveryTask);
}

void ControlPoint::processDiscoveryResult(NPT_Result nr, const FrontEnd::InterfaceContext *ifctx, const NPT_String& location, const NPT_String& server, int maxAge, DeviceDesc *rootDeviceDesc, const UUID& uuid)
{
	if (NPT_SUCCEEDED(nr) && rootDeviceDesc) {
		DeviceDescHolder rootDeviceDesc1(rootDeviceDesc);

		WriteLocker locker(m_stateLock);
		{
			DeviceDiscoveryTask **dummy1;
			if (NPT_FAILED(m_pendingSsdpDiscoverTasks.Get(uuid, dummy1))) {
				return;
			}
		}

		if (m_state != State_Attached) {
			m_pendingSsdpDiscoverTasks.Erase(uuid);
			return;
		}

		RootDevice **dummy;
		if (NPT_SUCCEEDED(m_rootDeviceIndexByUUID.Get(rootDeviceDesc->uuid(), dummy))) {
			RootDevice *rootDevice = *dummy;
			NPT_System::GetCurrentTimeStamp(rootDevice->m_updateTS);
			m_pendingSsdpDiscoverTasks.Erase(uuid);
			return;
		}

		RootDevice *rootDevice = new RootDevice();
		m_rootDeviceList.Add(rootDevice);
		m_rootDeviceIndexByUUID.Put(rootDeviceDesc->uuid(), rootDevice);
		rootDeviceDesc->addRef();
		rootDevice->m_deviceDesc = rootDeviceDesc;
		rootDevice->m_location = location;
		rootDevice->m_server = server;
		rootDevice->m_maxAge = maxAge;
		rootDevice->m_ifctx = ifctx;
		NPT_System::GetCurrentTimeStamp(rootDevice->m_registerTS);
		rootDevice->m_updateTS = rootDevice->m_registerTS;
		onAddRootDevice(rootDevice);
		m_pendingSsdpDiscoverTasks.Erase(uuid);
	} else {
		WriteLocker locker(m_stateLock);
		m_pendingSsdpDiscoverTasks.Erase(uuid);
	}
}

void ControlPoint::ssdpRemoveRootDevice(const UUID& uuid)
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Attached) {
		return;
	}

	RootDevice **dummy;
	if (NPT_SUCCEEDED(m_rootDeviceIndexByUUID.Get(uuid, dummy))) {
		RootDevice *rootDevice = *dummy;
		m_rootDeviceList.Remove(rootDevice);
		m_rootDeviceIndexByUUID.Erase(uuid);
		deleteRootDevice(rootDevice);
	}
}

void ControlPoint::removeExpiredDevicesLoop()
{
	for (;;) {
		if (NPT_SUCCEEDED(m_removeExpiredDevicesVar.WaitWhileEquals(0, 5000))) {
			break;
		}

		{
			NPT_TimeStamp ts;
			NPT_System::GetCurrentTimeStamp(ts);
			NPT_Int64 tsMillis = ts.ToMillis();
			WriteLocker locker(m_stateLock);
			NPT_Cardinal count = m_rootDeviceList.GetItemCount();
			for (NPT_Ordinal i = 0; i < count; ) {
				NPT_List<RootDevice*>::Iterator it = m_rootDeviceList.GetItem(i);
				bool removeIt = false;
				RootDevice *rootDevice = *it;

				// 6000 ms more
				if (rootDevice->m_updateTS.ToMillis() + (rootDevice->m_maxAge + 6) * 1000 < tsMillis) {
					removeIt = true;
				}

				if (removeIt) {
					m_rootDeviceList.Erase(it);
					m_rootDeviceIndexByUUID.Erase(rootDevice->m_deviceDesc->uuid());
					deleteRootDevice(rootDevice);
					count--;
				} else {
					i++;
				}
			}

			for (NPT_Ordinal i = 0; i < m_deviceList.GetItemCount(); i++) {
				DeviceInfo *deviceInfo = *m_deviceList.GetItem(i);
				for (NPT_Ordinal j = 0; j < deviceInfo->m_extraServiceInfoList.GetItemCount(); j++) {
					ExtraServiceInfo *extraServiceInfo = *deviceInfo->m_extraServiceInfoList.GetItem(j);
					if ((extraServiceInfo->m_eventSubState == EventSubState_TempFailure) || (extraServiceInfo->m_eventSubState == EventSubState_On && extraServiceInfo->m_eventSubUpdateTS.ToMillis() + extraServiceInfo->m_eventSubTimeout * 1000 - 26000 < tsMillis)) {
						int timeout;
						if (extraServiceInfo->m_eventSubState == EventSubState_On) {
							timeout = extraServiceInfo->m_eventSubTimeout;
						} else {
							timeout = DEFAULT_SUBSCRIPTION_TIMEOUT;
						}
						extraServiceInfo->m_eventSubState = EventSubState_Pending;
						NPT_String callbackURL(NPT_String::Format("<http://%s:%d%s>", deviceInfo->m_rootDevice->m_ifctx->m_ifAddr.ToString().GetChars(), deviceInfo->m_rootDevice->m_ifctx->m_httpPort, extraServiceInfo->m_callbackURL.GetChars()));
						m_attachedTaskGroup->startTask(new SubscribeTask2(this, deviceInfo->m_deviceDesc->uuid(), extraServiceInfo->m_serviceDesc->serviceId(), extraServiceInfo->m_eventSubURL, callbackURL, timeout, extraServiceInfo->m_sid));
					}
				}
			}
		}
	}
}

void ControlPoint::removeExpiredDevicesAbort()
{
	m_removeExpiredDevicesVar.SetValue(1);
}

void ControlPoint::deleteRootDevice(RootDevice *rootDevice)
{
	m_pendingSsdpDiscoverTasks.Erase(rootDevice->m_deviceDesc->uuid());
	onRemoveRootDevice(rootDevice);
	rootDevice->m_deviceDesc->release();
	delete rootDevice;
}

void ControlPoint::onAddRootDevice(RootDevice *rootDevice)
{
	onAddDeviceRecursive(rootDevice->m_deviceDesc, rootDevice);
}

void ControlPoint::onRemoveRootDevice(RootDevice *rootDevice)
{
	onRemoveDeviceRecursive(rootDevice->m_deviceDesc, rootDevice);
}

void ControlPoint::onAddDeviceRecursive(DeviceDesc *deviceDesc, RootDevice *rootDevice)
{
	onAddDevice(deviceDesc, rootDevice);
	for (NPT_Ordinal i = 0; i < deviceDesc->childCount(); i++) {
		onAddDeviceRecursive(deviceDesc->childAt(i), rootDevice);
	}
}

void ControlPoint::onRemoveDeviceRecursive(DeviceDesc *deviceDesc, RootDevice *rootDevice)
{
	for (NPT_Ordinal i = 0; i < deviceDesc->childCount(); i++) {
		onRemoveDeviceRecursive(deviceDesc->childAt(deviceDesc->childCount() - i - 1), rootDevice);
	}
	onRemoveDevice(deviceDesc, rootDevice);
}

void ControlPoint::onAddDevice(DeviceDesc *deviceDesc, RootDevice *rootDevice)
{
	DeviceDescPrivate::getPrivate(deviceDesc)->m_rootDescURL = rootDevice->m_location;
	DeviceInfo *deviceInfo = new DeviceInfo();
	m_deviceList.Add(deviceInfo);
	deviceInfo->m_deviceDesc = deviceDesc;
	deviceInfo->m_rootDevice = rootDevice;
	for (NPT_Ordinal i = 0; i < deviceDesc->serviceCount(); i++) {
		ServiceDesc *serviceDesc = deviceDesc->serviceAt(i);
		ExtraServiceInfo *extraServiceInfo = new ExtraServiceInfo();
		deviceInfo->m_extraServiceInfoList.Add(extraServiceInfo);
		extraServiceInfo->m_serviceDesc = serviceDesc;
		extraServiceInfo->m_eventSubURL = serviceDesc->eventSubURL();
		extraServiceInfo->m_callbackURL = NPT_String::Format("%sslot/%s/%s", m_frontEndContext.m_httpRoot.GetChars(), deviceDesc->uuid().toString().GetChars(), serviceDesc->serviceId().GetChars());
		if (extraServiceInfo->m_eventSubURL.IsEmpty()) {
			extraServiceInfo->m_eventSubState = EventSubState_None;
		} else {
			extraServiceInfo->m_eventSubState = EventSubState_Idle;
		}
		for (NPT_Ordinal kk = 0; kk < serviceDesc->stateVariableCount(); kk++) {
			const StateVariableDesc *stateVariableDesc = serviceDesc->stateVariableAt(kk);
			extraServiceInfo->m_stateVariableTable.Put(stateVariableDesc->name(), StateVariable());
			StateVariable *svar;
			extraServiceInfo->m_stateVariableTable.Get(stateVariableDesc->name(), svar);
			svar->m_stateVariableDesc = stateVariableDesc;
			svar->m_value = stateVariableDesc->defaultValue();
		}
	}
	if (m_callback) {
		bool subscribe = true;
		m_callback->controlPointOnDeviceAdded(this, deviceDesc, subscribe);
		if (subscribe) {
			this->subscribeEventsLocked(*deviceInfo);
		}
	}
}

void ControlPoint::onRemoveDevice(DeviceDesc *deviceDesc, RootDevice *rootDevice)
{
	if (m_callback) {
		m_callback->controlPointOnDeviceRemoved(this, deviceDesc);
	}
	NPT_List<DeviceInfo*>::Iterator it = m_deviceList.Find(DeviceInfoFinder(deviceDesc));
	if (it) {
		DeviceInfo *deviceInfo = *it;
		unsubscribeEventsLocked(*deviceInfo);
		m_deviceList.Erase(it);
		delete deviceInfo;
	}
}

ControlPoint::ProcessSsdkNotifyTask::ProcessSsdkNotifyTask(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet)
	: m_owner(owner), m_ifctx(ifctx), m_packet(packet)
{
}

void ControlPoint::ProcessSsdkNotifyTask::exec()
{
	m_owner->processSsdpNotifyReal(m_ifctx, m_packet);
}

void ControlPoint::processSsdpNotify(const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet)
{
	this->m_attachedTaskGroup->startTask(new ProcessSsdkNotifyTask(this, ifctx, packet));
}

void ControlPoint::processSsdpNotifyReal(const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet)
{
	NPT_HttpRequest *req;
	NPT_InputStreamReference inputStream(new NPT_MemoryStream(packet.GetData(), packet.GetDataSize()));
	NPT_BufferedInputStream xmlInputStream(inputStream);
	if (NPT_SUCCEEDED(NPT_HttpRequest::Parse(xmlInputStream, NULL, req))) {
		PtrHolder<NPT_HttpRequest> req1(req);
		do {
			if (req->GetProtocol().Compare(NPT_HTTP_PROTOCOL_1_1) != 0 || req->GetMethod().Compare("NOTIFY") != 0 || req->GetUrl().GetPath().Compare("*") != 0) {
				break;
			}

			NPT_HttpHeader *hdrHost = req->GetHeaders().GetHeader("HOST");
			if (!hdrHost || (hdrHost->GetValue().Compare("239.255.255.250:1900") != 0 && hdrHost->GetValue().Compare("239.255.255.250") != 0)) {
				break;
			}

			NPT_HttpHeader *hdrNTS = req->GetHeaders().GetHeader("NTS");
			if (!hdrNTS) {
				break;
			}

			NPT_HttpHeader *hdrNT = req->GetHeaders().GetHeader("NT");
			if (!hdrNT) {
				break;
			}

			UUID rootDeviceUuid;
			NPT_HttpHeader *hdrUSN = req->GetHeaders().GetHeader("USN");
			if (!hdrUSN || !extractUuidFromUSN(hdrUSN->GetValue(), rootDeviceUuid)) {
				break;
			}

			if (hdrNTS->GetValue().Compare("ssdp:alive") == 0) {
				NPT_HttpHeader *hdrLocation = req->GetHeaders().GetHeader("LOCATION");
				if (!hdrLocation) {
					break;
				}

				NPT_HttpHeader *hdrServer = req->GetHeaders().GetHeader("SERVER");
				if (!hdrServer) {
					break;
				}

				int maxAge;
				NPT_HttpHeader *hdrCacheControl = req->GetHeaders().GetHeader("CACHE-CONTROL");
				if (!hdrCacheControl || !parseCacheControl(hdrCacheControl->GetValue(), maxAge)) {
					break;
				}

				ssdpDiscoverRootDevice(ifctx, rootDeviceUuid, hdrLocation->GetValue(), hdrServer->GetValue(), maxAge);

			} else if (hdrNTS->GetValue().Compare("ssdp:byebye") == 0) {
				ssdpRemoveRootDevice(rootDeviceUuid);
			} else {
				break;
			}

		} while (false);
	}
}

void ControlPoint::processSsdpSearchResponse(const FrontEnd::InterfaceContext *ifctx, const NPT_DataBuffer& packet)
{
	NPT_HttpResponse *resp;
	NPT_InputStreamReference inputStream(new NPT_MemoryStream(packet.GetData(), packet.GetDataSize()));
	NPT_BufferedInputStream xmlInputStream(inputStream);
	if (NPT_SUCCEEDED(NPT_HttpResponse::Parse(xmlInputStream, resp))) {
		PtrHolder<NPT_HttpResponse> resp1(resp);
		do {
			if (resp->GetProtocol().Compare(NPT_HTTP_PROTOCOL_1_1) != 0 || resp->GetStatusCode() != 200) {
				break;
			}

			NPT_HttpHeader *hdrExt = resp->GetHeaders().GetHeader("EXT");
			if (!hdrExt || hdrExt->GetValue().GetLength() > 0) {
				break;
			}

			NPT_HttpHeader *hdrLocation = resp->GetHeaders().GetHeader("LOCATION");
			if (!hdrLocation) {
				break;
			}

			NPT_HttpHeader *hdrServer = resp->GetHeaders().GetHeader("SERVER");
			if (!hdrServer) {
				break;
			}

			int maxAge;
			NPT_HttpHeader *hdrCacheControl = resp->GetHeaders().GetHeader("CACHE-CONTROL");
			if (!hdrCacheControl || !parseCacheControl(hdrCacheControl->GetValue(), maxAge)) {
				break;
			}

			NPT_HttpHeader *hdrST = resp->GetHeaders().GetHeader("ST");
			if (!hdrST) {
				break;
			}

			UUID rootDeviceUuid;
			NPT_HttpHeader *hdrUSN = resp->GetHeaders().GetHeader("USN");
			if (!hdrUSN || !extractUuidFromUSN(hdrUSN->GetValue(), rootDeviceUuid)) {
				break;
			}

			ssdpDiscoverRootDevice(ifctx, rootDeviceUuid, hdrLocation->GetValue(), hdrServer->GetValue(), maxAge);

		} while (false);
	}
}

ControlPoint::ProcessHttpRequestTask::ProcessHttpRequestTask(ControlPoint *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket)
	: m_owner(owner), m_ifctx(ifctx), m_relPath(relPath), m_reqCtx(reqCtx), m_req(req), m_resp(resp), m_inputStream(inputStream), m_httpOutput(httpOutput), m_clientSocket(clientSocket)
{
}

ControlPoint::ProcessHttpRequestTask::~ProcessHttpRequestTask()
{
	delete m_httpOutput;
	delete m_inputStream;
	delete m_resp;
	delete m_req;
	delete m_clientSocket;
}

void ControlPoint::ProcessHttpRequestTask::exec()
{
	m_owner->processHttpRequest(this, m_ifctx, m_relPath, m_reqCtx, m_req, *m_resp, m_inputStream, m_httpOutput);
}

void ControlPoint::ProcessHttpRequestTask::doAbort()
{
	ReadLocker locker(m_stateLock);
	for (NPT_Ordinal i = 0; i < m_callbackList.GetItemCount(); i++) {
		AbortableTask::Callback *callback = *m_callbackList.GetItem(i);
		callback->onAborted(this);
	}
	if (m_clientSocket) {
		m_clientSocket->Cancel();
	}
}

bool ControlPoint::ProcessHttpRequestTask::registerAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	if (aborted()) {
		return false;
	}
	m_callbackList.Add(callback);
	return true;
}

void ControlPoint::ProcessHttpRequestTask::unregisterAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	m_callbackList.Remove(callback, true);
}

void ControlPoint::processHttpRequest(const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket)
{
	m_attachedTaskGroup->startTask(new ProcessHttpRequestTask(this, ifctx, relPath, reqCtx, req, resp, inputStream, httpOutput, clientSocket));
}

void serveFile(AbortableTask *task, const NPT_String& filePath, const NPT_String& mimeType, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput);

void ControlPoint::processHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	do {
		if (req->GetMethod().Compare("NOTIFY") != 0) {
			break;
		}

		NPT_HttpHeader *hdrContentType = req->GetHeaders().GetHeader("CONTENT-TYPE");
		if (!hdrContentType || !contentTypeIsUtf8Xml(hdrContentType->GetValue())) {
			break;
		}

		NPT_Size contentLength;
		NPT_HttpHeader *hdrContentLength = req->GetHeaders().GetHeader("CONTENT-LENGTH");
		if (!hdrContentLength || NPT_FAILED(NPT_ParseInteger(hdrContentLength->GetValue(), contentLength))) {
			break;
		}

		NPT_HttpHeader *hdrNT = req->GetHeaders().GetHeader("NT");
		if (!hdrNT || hdrNT->GetValue().Compare("upnp:event") != 0) {
			break;
		}

		NPT_HttpHeader *hdrNTS = req->GetHeaders().GetHeader("NTS");
		if (!hdrNTS || hdrNTS->GetValue().Compare("upnp:propchange") != 0) {
			break;
		}

		UUID sid;
		NPT_HttpHeader *hdrSID = req->GetHeaders().GetHeader("SID");
		if (!hdrSID || !extractUuidFromUSN(hdrSID->GetValue(), sid)) {
			break;
		}

		NPT_UInt32 seq;
		NPT_HttpHeader *hdrSeq = req->GetHeaders().GetHeader("SEQ");
		if (!hdrSeq || NPT_FAILED(NPT_ParseInteger(hdrSeq->GetValue(), seq))) {
			break;
		}

		NPT_DataBuffer content;
		if (NPT_FAILED(inputStream->Load(content, contentLength))) {
			// may be aborted ???
			break;
		}

		if (content.GetDataSize() != contentLength) {
			break;
		}

		NPT_XmlParser parser;
		NPT_XmlNode *rootNode;
		if (NPT_FAILED(parser.Parse(reinterpret_cast<const char*>(content.GetData()), content.GetDataSize(), rootNode))) {
			break;
		}

		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (!rootEl || !matchTagNamespace(rootEl, "propertyset", "urn:schemas-upnp-org:event-1-0")) {
			break;
		}

		{
			DeviceInfo *matchDeviceInfo = NULL;
			ExtraServiceInfo *matchExtraServiceInfo;

			WriteLocker locker(m_stateLock);
			if (m_state != State_Attached) {
				return;
			}

			NPT_String path = req->GetUrl().GetPath();

			if (relPath.StartsWith("slot/")) {
				for (NPT_Ordinal i = 0; i < m_deviceList.GetItemCount(); i++) {
					DeviceInfo *deviceInfo = *m_deviceList.GetItem(i);
					bool found = false;
					for (NPT_Ordinal j = 0; j < deviceInfo->m_extraServiceInfoList.GetItemCount(); j++) {
						ExtraServiceInfo *extraServiceInfo = *deviceInfo->m_extraServiceInfoList.GetItem(j);
						if (extraServiceInfo->m_callbackURL.Compare(path) == 0) {
							matchExtraServiceInfo = extraServiceInfo;
							matchDeviceInfo = deviceInfo;
							found = true;
							break;
						}
					}
					if (found) {
						break;
					}
				}
			}

			if (matchDeviceInfo){

				NPT_List<NPT_String> changedVarNames;
				NPT_List<NPT_String> changedVarValues;
				// alreay locked!!
				//WriteLocker locker1(m_stateLock);
				ExtraServiceInfo *extraServiceInfo = matchExtraServiceInfo;
				for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
					const NPT_XmlNode *node = *rootEl->GetChildren().GetItem(i);
					if (const NPT_XmlElementNode *propertyEl = node->AsElementNode()) {
						if (matchTagNamespace(propertyEl, "property", "urn:schemas-upnp-org:event-1-0")) {
							for (NPT_Ordinal j = 0; j < propertyEl->GetChildren().GetItemCount(); j++) {
								if (const NPT_XmlElementNode *el = (*propertyEl->GetChildren().GetItem(j))->AsElementNode()) {
									NPT_String text = getElementText(el);
									StateVariable *svar;
									if (NPT_SUCCEEDED(extraServiceInfo->m_stateVariableTable.Get(el->GetTag(), svar))) {
										if (svar->m_value != text) {
											svar->m_value = text;
											changedVarNames.Add(svar->m_stateVariableDesc->name());
											changedVarValues.Add(svar->m_value);
										}
									}
								}
							}
						}
					}
				}

				if (m_callback && changedVarNames.GetItemCount() > 0) {
					m_callback->controlPointOnStateVariablesChanged(this, matchDeviceInfo->m_deviceDesc, extraServiceInfo->m_serviceDesc, changedVarNames, changedVarValues);
				}

			}
		}

//		if (!task->aborted()) {
			setStatusCode(resp, 200);
			httpOutput->writeResponseHeader(resp);
			httpOutput->flush();
//		}

		return;

	} while (false);

	if (req->GetMethod().Compare("GET") == 0 && relPath.StartsWith("pls/")) {
		NPT_String playlistName = relPath.SubString(4);
		ReadLocker locker(m_stateLock);
		SimpleContent *sc = NULL;
		if (NPT_SUCCEEDED(m_simpleContentMap.Get(playlistName, sc))) {
			setStatusCode(resp, 200);
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, NPT_String::Format("; charset=\"UTF-8\"", sc->mimeType.GetChars()));
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromInteger(sc->content.GetLength()));
			httpOutput->writeResponseHeader(resp);
			httpOutput->writeData(sc->content);
			httpOutput->flush();
			return;
		}
	}

	if (relPath.StartsWith("files/")) {
		NPT_String filePath;
		NPT_String mimeType;
		{
			NPT_String fileName = relPath.SubString(6);
			ReadLocker locker(m_stateLock);
			SimpleContent *sc = NULL;
			if (NPT_SUCCEEDED(m_simpleContentMap.Get(fileName, sc))) {
				filePath = sc->content;
				mimeType = sc->mimeType;
			}
		}

		if (!filePath.IsEmpty()) {
			serveFile(task, filePath, mimeType, reqCtx, req, resp, httpOutput);
			return;
		}
	}


//	if (!task->aborted()) {
		setStatusCode(resp, 404);
		httpOutput->writeResponseHeader(resp);
		httpOutput->flush();
//	}
}

ActionInstance::ActionInstance()
{
}

ActionInstance::~ActionInstance()
{
}

MCActionImpl::MCActionImpl(ActionCallback *callback, const NPT_List<NPT_String>& outputArgNames)
	: m_callback(callback), m_abortFlag(false), m_buddy(NULL), m_outputArgNames(outputArgNames)
{
}

MCActionImpl::~MCActionImpl()
{
}

void MCActionImpl::addRef()
{
	m_refCount.Increment();
}

void MCActionImpl::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
	}
}

void MCActionImpl::abort()
{
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		m_abortFlag = true;
		m_callback = NULL;
		if (m_buddy) {
			m_buddy->abort();
		}
	}
}

bool MCActionImpl::aborted() const
{
	ReadLocker locker(m_stateLock);
	return m_abortFlag;
}

NPT_Result MCActionImpl::wait(NPT_Timeout timeout)
{
	return m_waitVar.WaitWhileEquals(0, timeout);
}

NPT_Result MCActionImpl::statusCode() const
{
	return m_statusCode;
}

int MCActionImpl::errorCode() const
{
	return m_errCode;
}

const NPT_String& MCActionImpl::errorDescription() const
{
	return m_errDesc;
}

const NPT_List<NPT_String>& MCActionImpl::outputNames() const
{
	return m_outputArgNames;
}

const NPT_List<NPT_String>& MCActionImpl::outputValues() const
{
	return m_outputArgValues;
}

void MCActionImpl::setBuddy(SoapTask *buddy)
{
	WriteLocker locker(m_stateLock);
	m_buddy = buddy;
}

void MCActionImpl::onFinished(NPT_Result nr, int errCode, const NPT_String& errDesc, const NPT_List<NPT_String>& outputValues)
{
	ReadLocker locker(m_stateLock);

	m_statusCode = nr;
	m_errCode = errCode;
	m_errDesc = errDesc;
	m_outputArgValues = outputValues;

	m_waitVar.SetValue(1);
	if (m_callback) {
		m_callback->onActionFinished(this);
	}
}

SoapTask::SoapTask(const NPT_String& controlURL, const NPT_String& serviceType, const NPT_String& actionName, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, const NPT_List<NPT_String>& outputArgNames, MCActionImpl *actImpl)
	: m_controlURL(controlURL), m_serviceType(serviceType), m_actionName(actionName), m_inputArgNames(inputArgNames), m_inputArgValues(inputArgValues), m_outputArgNames(outputArgNames), m_actImpl(actImpl)
{
	m_actImpl->addRef();
	m_actImpl->setBuddy(this);
}

SoapTask::~SoapTask()
{
	m_actImpl->setBuddy(NULL);
	m_actImpl->release();
}

static bool extractFaultDetail(const NPT_XmlElementNode *bodyEl, int& errCode, NPT_String& errDesc)
{
	do {
		const NPT_XmlElementNode *faultEl = bodyEl->GetChild("Fault", "http://schemas.xmlsoap.org/soap/envelope/");
		if (!faultEl) {
			break;
		}

		const NPT_XmlElementNode *detailEl = faultEl->GetChild("detail");
		if (!detailEl) {
			break;
		}

		const NPT_XmlElementNode *upnpErrorEl = detailEl->GetChild("UPnPError", "urn:schemas-upnp-org:control-1-0");
		if (!upnpErrorEl) {
			break;
		}

		const NPT_XmlElementNode *errorCodeEl = upnpErrorEl->GetChild("errorCode", "urn:schemas-upnp-org:control-1-0");
		if (!errorCodeEl) {
			break;
		}

		if (NPT_FAILED(NPT_ParseInteger(getElementText(errorCodeEl), errCode))) {
			break;
		}

		const NPT_XmlElementNode *errorDescEl = upnpErrorEl->GetChild("errorDescription", "urn:schemas-upnp-org:control-1-0");
		if (errorDescEl) {
			errDesc = getElementText(errorDescEl);
		}

		return true;
	} while (false);
	return false;
}

void SoapTask::exec()
{
	NPT_Result nr;

	NPT_StringOutputStream stream0;
	NPT_XmlSerializer xml(&stream0, 0, true, true);
	xml.StartDocument();
	xml.StartElement("s", "Envelope");
	xml.Attribute("xmlns", "s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.Attribute("s", "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
	xml.StartElement("s", "Body");
	xml.StartElement("u", m_actionName);
	xml.Attribute("xmlns", "u", m_serviceType);
	for (NPT_Ordinal i = 0; i < m_inputArgNames.GetItemCount(); i++) {
		NPT_List<NPT_String>::Iterator it = m_inputArgNames.GetItem(i);
		NPT_List<NPT_String>::Iterator it2 = m_inputArgValues.GetItem(i);
		xml.StartElement(NULL, *it);
		xml.Text(*it2);
		xml.EndElement(NULL, *it);
	}
	xml.EndElement("u", m_actionName);
	xml.EndElement("s", "Body");
	xml.EndElement("s", "Envelope");
	xml.EndDocument();

	NPT_HttpRequest req(m_controlURL, "POST", NPT_HTTP_PROTOCOL_1_1);
	Helper::setupHttpRequest(req);
	req.GetHeaders().AddHeader("SOAPACTION", NPT_String::Format("\"%s#%s\"", m_serviceType.GetChars(), m_actionName.GetChars()));
	req.SetEntity(new NPT_HttpEntity());
	req.GetEntity()->SetContentType("text/xml; charset=\"utf-8\"");
	req.GetEntity()->SetInputStream(stream0.GetString());

	do {
		m_httpClient.SetTimeouts(10000, 40000, 10000);
		NPT_HttpResponse *resp;
		nr = m_httpClient.SendRequest(req, resp);
		if (NPT_FAILED(nr)) {
			break;
		}

		const NPT_List<NPT_HttpHeader*>& ls0 = resp->GetHeaders().GetHeaders();
		for (NPT_Ordinal i = 0; i < ls0.GetItemCount(); i++) {
			NPT_HttpHeader *hh = *ls0.GetItem(i);
			m_actImpl->m_responseHeaders.SetHeader(hh->GetName(), hh->GetValue());
		}

		m_actImpl->m_responseStatusCode = resp->GetStatusCode();
		m_actImpl->m_responseStatusText = resp->GetReasonPhrase();

		PtrHolder<NPT_HttpResponse> resp1(resp);
		if (!resp->GetEntity()/* || resp->GetEntity()->GetContentLength() == 0*/ || !contentTypeIsUtf8Xml(resp->GetEntity()->GetContentType())) {
			nr = NPT_FAILURE;
			break;
		}

		NPT_DataBuffer outputData;
		if (NPT_FAILED(resp->GetEntity()->Load(outputData))/* || outputData.GetDataSize() != resp->GetEntity()->GetContentLength()*/) {
			nr = NPT_FAILURE;
			break;
		}

		NPT_XmlParser parser;
		NPT_XmlNode *rootNode;
		nr = parser.Parse(reinterpret_cast<const char*>(outputData.GetData()), outputData.GetDataSize(), rootNode);
		if (NPT_FAILED(nr)) {
			break;
		}

		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (!rootEl || !matchNamespace(rootEl, "http://schemas.xmlsoap.org/soap/envelope/")) {
			nr = NPT_FAILURE;
			break;
		}

		const NPT_XmlElementNode *bodyEl = rootEl->GetChild("Body", "http://schemas.xmlsoap.org/soap/envelope/");
		if (!bodyEl) {
			nr = NPT_FAILURE;
			break;
		}

		if (resp->GetStatusCode() == 200) {
			// success
			NPT_String tag = NPT_String::Format("%sResponse", m_actionName.GetChars());
			const NPT_XmlElementNode *respEl = bodyEl->GetChild(tag, m_serviceType);
			if (!respEl) {
				nr = NPT_FAILURE;
				break;
			}

			int outputIndex = 0;
			for (NPT_Ordinal vv = 0; vv < respEl->GetChildren().GetItemCount(); vv++) {
				if (const NPT_XmlElementNode *argEl = (*respEl->GetChildren().GetItem(vv))->AsElementNode()) {
					if (argEl->GetTag().Compare(*m_outputArgNames.GetItem(outputIndex)) == 0) {
						m_outputArgValues.Add(getElementText(argEl));
						outputIndex++;
					}
				}
			}

			if (m_outputArgValues.GetItemCount() != m_outputArgNames.GetItemCount()) {
				nr = NPT_FAILURE;
				break;
			}

			m_actImpl->onFinished(NPT_SUCCESS, 0, NPT_String(), m_outputArgValues);
			return;
		}

		if (resp->GetStatusCode() == 500) {
			int errCode;
			NPT_String errDesc;
			if (extractFaultDetail(bodyEl, errCode, errDesc)) {
				m_actImpl->onFinished(NPT_SUCCESS, errCode, errDesc, NPT_List<NPT_String>());
				return;
			}

			nr = NPT_FAILURE;
			break;
		} else {
			nr = NPT_FAILURE;
			break;
		}

	} while (false);

	m_actImpl->onFinished(nr, 0, NPT_String(), NPT_List<NPT_String>());
}

void SoapTask::doAbort()
{
	m_httpClient.Abort();
}

} // namespace deejay
