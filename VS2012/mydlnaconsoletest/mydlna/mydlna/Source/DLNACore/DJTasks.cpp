#include "DJTasks.h"
#include "DJDescPriv.h"

namespace deejay {

SsdpSearchTask::Callback::~Callback()
{
}

SsdpSearchTask::SsdpSearchTask(const char *st, NPT_UInt32 mx, const NPT_IpAddress& ifAddr, Callback *callback, bool autoDelete)
	: m_st(st), m_mx(mx), m_ifAddr(ifAddr), m_callback(callback), m_autoDelete(autoDelete), m_socket(NPT_SOCKET_FLAG_CANCELLABLE)
{
	if (m_mx == 0) {
		m_mx = 1;
	}

	if (m_mx > 120) {
		m_mx = 120;
	}
}

SsdpSearchTask::~SsdpSearchTask()
{
	if (m_autoDelete) {
		delete m_callback;
	}
}

void SsdpSearchTask::exec()
{
	NPT_Result nr;

	nr = m_socket.Bind(NPT_SocketAddress(m_ifAddr, 0));
	if (NPT_FAILED(nr)) {
		return report(nr);
	}
	m_socket.SetReadTimeout(m_mx * 1000);

	NPT_SocketAddress targetAddr(NPT_IpAddress(239, 255, 255, 250), 1900);
	NPT_String req = NPT_String::Format("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: %d\r\nST: %s\r\n\r\n", m_mx, m_st.GetChars());
	nr = m_socket.Send(NPT_DataBuffer(req.GetChars(), req.GetLength(), false), &targetAddr);
	if (NPT_FAILED(nr)) {
		return report(nr);
	}

	NPT_DataBuffer packet(4096 * 4);
	NPT_SocketAddress fromAddr;
	while (!aborted()) {
		nr = m_socket.Receive(packet, &fromAddr);
		if (aborted()) {
			return report(NPT_ERROR_CANCELLED);
		}
		if (NPT_SUCCEEDED(nr)) {
			if (report(packet)) {
				return report(NPT_SUCCESS);
			}
		} else if (nr == NPT_ERROR_TIMEOUT) {
			return report(NPT_SUCCESS);
		} else {
			return report(nr);
		}
	}
}

void SsdpSearchTask::doAbort()
{
	m_socket.Cancel();
}

void SsdpSearchTask::report(NPT_Result nr)
{
	if (m_callback) {
		m_callback->ssdpSearchTaskFinished(this, nr);
	}
}

bool SsdpSearchTask::report(const NPT_DataBuffer& data)
{
	if (m_callback) {
		return m_callback->ssdpSearchTaskOnPacket(this, data);
	}
	return false;
}

HttpGetTask::Callback::~Callback()
{
}

HttpGetTask::HttpGetTask(const NPT_String& url, Callback *callback, bool autoDelete)
	: m_url(url), m_callback(callback), m_autoDelete(autoDelete), m_resp(NULL)
{
}

HttpGetTask::~HttpGetTask()
{
	if (m_autoDelete) {
		delete m_callback;
	}

	if (m_resp) {
		delete m_resp;
	}
}

void HttpGetTask::exec()
{
	NPT_Result nr;

	m_httpClient.SetTimeouts(10000, 10000, 10000);
	NPT_HttpRequest req(m_url, "GET", NPT_HTTP_PROTOCOL_1_1);
	Helper::setupHttpRequest(req);
	nr = m_httpClient.SendRequest(req, m_resp);
	if (NPT_FAILED(nr)) {
		return report(nr);
	}

	if (m_resp->GetStatusCode() == 200 && m_resp->GetEntity()) {
		NPT_HttpEntity *entity = m_resp->GetEntity();
		//if (entity->GetContentLength() == 0) {
		//	return report(NPT_FAILURE, m_resp);
		//}

		NPT_DataBuffer data;
		nr = entity->Load(data);
		if (NPT_FAILED(nr)) {
			return report(nr, m_resp);
		}

		//if (data.GetDataSize() != entity->GetContentLength()) {
		//	return report(NPT_FAILURE, m_resp);
		//}
		return report(NPT_SUCCESS, m_resp, &data);
	}

	return report(nr, m_resp);
}

void HttpGetTask::doAbort()
{
	m_httpClient.Abort();
}

void HttpGetTask::report(NPT_Result nr, NPT_HttpResponse *resp, const NPT_DataBuffer *data)
{
	if (m_callback) {
		m_callback->httpGetTaskFinished(this, nr, resp, data);
	}
}

DeviceDiscoveryTask::Callback::~Callback()
{
}

DeviceDiscoveryTask::DeviceDiscoveryTask(const NPT_String& location, const UUID& rootDeviceUuid, Callback *callback, bool autoDelete)
	: m_location(location), m_rootDeviceUuid(rootDeviceUuid), m_callback(callback), m_autoDelete(autoDelete)
{
}

DeviceDiscoveryTask::~DeviceDiscoveryTask()
{
	if (m_autoDelete) {
		delete m_callback;
	}
}

const NPT_String& DeviceDiscoveryTask::location() const
{
	return m_location;
}

NPT_String completeURL(const NPT_String& url, const NPT_String& rootURL, const NPT_String& baseURL)
{
	NPT_String result;
	if (url.StartsWith("/")) {
		result = NPT_String::Format("%s%s", rootURL.GetChars(), url.GetChars());
	} else if (url.Find("://") > 0) {
		result = url;
	} else {
		result = NPT_String::Format("%s%s", baseURL.GetChars(), url.GetChars());
	}
	return result;
}

IconDesc *parseIconDesc(const NPT_XmlElementNode *iconEl, const NPT_String& rootURL, const NPT_String& baseURL)
{
	IconDesc *iconDesc = IconDescPrivate::newIconDesc();
	IconDescPrivate *priv = IconDescPrivate::getPrivate(iconDesc);
	for (NPT_Ordinal i = 0; i < iconEl->GetChildren().GetItemCount(); i++) {
		const NPT_XmlNode *childNode = *iconEl->GetChildren().GetItem(i);
		if (const NPT_XmlElementNode *el = childNode->AsElementNode()) {
			const NPT_String *ns = el->GetNamespace();
			if (ns) {
				if (ns->Compare("urn:schemas-upnp-org:device-1-0") == 0) {
					NPT_String text = getElementText(el);
					if (el->GetTag().Compare("mimetype") == 0) {
						priv->m_mimeType = text;
					} else if (el->GetTag().Compare("width") == 0) {
						if (!(NPT_SUCCEEDED(NPT_ParseInteger(text, priv->m_width)) && priv->m_width > 0)) {
							IconDescPrivate::deleteIconDesc(iconDesc);
							return NULL;
						}
					} else if (el->GetTag().Compare("height") == 0) {
						if (!(NPT_SUCCEEDED(NPT_ParseInteger(text, priv->m_height)) && priv->m_height > 0)) {
							IconDescPrivate::deleteIconDesc(iconDesc);
							return NULL;
						}
					} else if (el->GetTag().Compare("depth") == 0) {
						if (!(NPT_SUCCEEDED(NPT_ParseInteger(text, priv->m_depth)) && priv->m_depth > 0)) {
							IconDescPrivate::deleteIconDesc(iconDesc);
							return NULL;
						}
					} else if (el->GetTag().Compare("url") == 0) {
						priv->m_url = completeURL(text, rootURL, baseURL);
					}
				}
			}
		}
	}
	return iconDesc;
}

ServiceDesc *parseServiceDesc(const NPT_XmlElementNode *serviceEl, const NPT_String& rootURL, const NPT_String& baseURL)
{
	ServiceDesc *serviceDesc = ServiceDescPrivate::newServiceDesc();
	ServiceDescPrivate *priv = ServiceDescPrivate::getPrivate(serviceDesc);
	for (NPT_Ordinal i = 0; i < serviceEl->GetChildren().GetItemCount(); i++) {
		const NPT_XmlNode *childNode = *serviceEl->GetChildren().GetItem(i);
		if (const NPT_XmlElementNode *el = childNode->AsElementNode()) {
			const NPT_String *ns = el->GetNamespace();
			if (ns) {
				if (ns->Compare("urn:schemas-upnp-org:device-1-0") == 0) {
					NPT_String text = getElementText(el);
					if (el->GetTag().Compare("serviceType") == 0) {
						priv->m_serviceType = text;
					} else if (el->GetTag().Compare("serviceId") == 0) {
						priv->m_serviceId = text;
					} else if (el->GetTag().Compare("SCPDURL") == 0) {
						priv->m_scpdURL = completeURL(text, rootURL, baseURL);
					} else if (el->GetTag().Compare("controlURL") == 0) {
						priv->m_controlURL = completeURL(text, rootURL, baseURL);
					} else if (el->GetTag().Compare("eventSubURL") == 0) {
						if (!text.IsEmpty()) {
							priv->m_eventSubURL = completeURL(text, rootURL, baseURL);
						}
					}
				}
			}
		}
	}
	return serviceDesc;
}

DeviceDesc *parseDeviceDesc(const NPT_XmlElementNode *deviceEl, const NPT_String& rootURL, const NPT_String& baseURL)
{
	DeviceDesc *deviceDesc = DeviceDescPrivate::newDeviceDesc();
	DeviceDescHolder deviceDesc1(deviceDesc);
	DeviceDescPrivate *priv = DeviceDescPrivate::getPrivate(deviceDesc);
	for (NPT_Ordinal i = 0; i < deviceEl->GetChildren().GetItemCount(); i++) {
		const NPT_XmlNode *childNode = *deviceEl->GetChildren().GetItem(i);
		if (const NPT_XmlElementNode *el = childNode->AsElementNode()) {
			const NPT_String *ns = el->GetNamespace();
			if (ns) {
				if (ns->Compare("urn:schemas-upnp-org:device-1-0") == 0) {
					NPT_String text = getElementText(el);
					if (el->GetTag().Compare("deviceType") == 0) {
						priv->m_deviceType = text;
					} else if (el->GetTag().Compare("friendlyName") == 0) {
						priv->m_friendlyName = text;
					} else if (el->GetTag().Compare("manufacturer") == 0) {
						priv->m_manufacturer = text;
					} else if (el->GetTag().Compare("manufacturerURL") == 0) {
						if (!text.IsEmpty()) {
							priv->m_manufacturerURL = completeURL(text, rootURL, baseURL);
						}
					} else if (el->GetTag().Compare("modelDescription") == 0) {
						priv->m_modelDescription = text;
					} else if (el->GetTag().Compare("modelName") == 0) {
						priv->m_modelName = text;
					} else if (el->GetTag().Compare("modelNumber") == 0) {
						priv->m_modelNumber = text;
					} else if (el->GetTag().Compare("modelURL") == 0) {
						priv->m_modelURL = text;
					} else if (el->GetTag().Compare("serialNumber") == 0) {
						priv->m_serialNumber = text;
					} else if (el->GetTag().Compare("UDN") == 0) {
						if (!extractUuidFromUSN(text, priv->m_uuid)) {
							return NULL;
						}
					} else if (el->GetTag().Compare("UPC") == 0) {
						priv->m_upc = text;
					} else if (el->GetTag().Compare("presentationURL") == 0) {
						if (!text.IsEmpty()) {
							priv->m_presentationURL = completeURL(text, rootURL, baseURL);
						}
					} else if (el->GetTag().Compare("iconList") == 0) {
						for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
							const NPT_XmlNode *iconNode = *el->GetChildren().GetItem(j);
							if (const NPT_XmlElementNode *iconEl = iconNode->AsElementNode()) {
								if (matchTagNamespace(iconEl, "icon", "urn:schemas-upnp-org:device-1-0")) {
									IconDesc *iconDesc = parseIconDesc(iconEl, rootURL, baseURL);
									if (iconDesc) {
										priv->m_iconList.Add(iconDesc);
									} else {
										return NULL;
									}
								}								
							}
						}
					} else if (el->GetTag().Compare("serviceList") == 0) {
						for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
							const NPT_XmlNode *serviceNode = *el->GetChildren().GetItem(j);
							if (const NPT_XmlElementNode *serviceEl = serviceNode->AsElementNode()) {
								if (matchTagNamespace(serviceEl, "service", "urn:schemas-upnp-org:device-1-0")) {
									ServiceDesc *serviceDesc = parseServiceDesc(serviceEl, rootURL, baseURL);
									if (serviceDesc) {
										priv->m_serviceList.Add(serviceDesc);
									} else {
										return NULL;
									}
								}								
							}
						}
					} else if (el->GetTag().Compare("deviceList") == 0) {
						for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
							const NPT_XmlNode *childDeviceNode = *el->GetChildren().GetItem(j);
							if (const NPT_XmlElementNode *childDeviceEl = childDeviceNode->AsElementNode()) {
								if (matchTagNamespace(childDeviceEl, "device", "urn:schemas-upnp-org:device-1-0")) {
									DeviceDesc *childDeviceDesc = parseDeviceDesc(childDeviceEl, rootURL, baseURL);
									if (childDeviceDesc) {
										priv->m_children.Add(childDeviceDesc);
										DeviceDescPrivate::getPrivate(childDeviceDesc)->m_parent = deviceDesc;
									} else {
										return NULL;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	deviceDesc->addRef();
	return deviceDesc;
}

ArgumentDesc *parseArgumentDesc(const NPT_XmlElementNode *argEl)
{
	ArgumentDesc *argDesc = ArgumentDescPrivate::newArgumentDesc();
	ArgumentDescPrivate *priv = ArgumentDescPrivate::getPrivate(argDesc);
	for (NPT_Ordinal i = 0; i < argEl->GetChildren().GetItemCount(); i++) {
		if (const NPT_XmlElementNode *el = (*argEl->GetChildren().GetItem(i))->AsElementNode()) {
			NPT_String text = getElementText(el);
			if (el->GetTag().Compare("name") == 0) {
				priv->m_name = text;
			} else if (el->GetTag().Compare("direction") == 0) {
				priv->m_input = text.Compare("in", true) == 0;
			} else if (el->GetTag().Compare("relatedStateVariable") == 0) {
				priv->m_relatedStateVariableName = text;
			}
		}
	}
	return argDesc;
}

ActionDesc *parseActionDesc(const NPT_XmlElementNode *actionEl)
{
	ActionDesc *actionDesc = ActionDescPrivate::newActionDesc();
	ActionDescPrivate *priv = ActionDescPrivate::getPrivate(actionDesc);
	for (NPT_Ordinal i = 0; i < actionEl->GetChildren().GetItemCount(); i++) {
		if (const NPT_XmlElementNode *el = (*actionEl->GetChildren().GetItem(i))->AsElementNode()) {
			NPT_String text = getElementText(el);
			if (el->GetTag().Compare("name") == 0) {
				priv->m_name = text;
			} else if (el->GetTag().Compare("argumentList") == 0) {
				for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
					if (const NPT_XmlElementNode *argEl = (*el->GetChildren().GetItem(j))->AsElementNode()) {
						if (argEl->GetTag().Compare("argument") == 0) {
							ArgumentDesc *argDesc = parseArgumentDesc(argEl);
							priv->m_argumentList.Add(argDesc);
							if (argDesc->isInputArg()) {
								priv->m_inputArgumentCount++;
							}
						}
					}
				}
			}
		}
	}
	return actionDesc;
}

StateVariableDesc *parseStateVariableDesc(const NPT_XmlElementNode *stateVariableEl)
{
	StateVariableDesc *stateVariableDesc = StateVariableDescPrivate::newStateVariableDesc();
	StateVariableDescPrivate *priv = StateVariableDescPrivate::getPrivate(stateVariableDesc);
	for (NPT_Ordinal i = 0; i < stateVariableEl->GetChildren().GetItemCount(); i++) {
		if (const NPT_XmlElementNode *el = (*stateVariableEl->GetChildren().GetItem(i))->AsElementNode()) {
			NPT_String text = getElementText(el);
			if (el->GetTag().Compare("name") == 0) {
				priv->m_name = text;
			} else if (el->GetTag().Compare("dataType") == 0) {
				priv->m_dataType = text;
			} else if (el->GetTag().Compare("defaultValue") == 0) {
				priv->m_defaultValue = text;
			} else if (el->GetTag().Compare("allowedValueList") == 0) {
				priv->m_allowedValueList.Clear();
				for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
					if (const NPT_XmlElementNode *allowedValueEl = (*el->GetChildren().GetItem(j))->AsElementNode()) {
						if (allowedValueEl->GetTag().Compare("allowedValue") == 0) {
							priv->m_allowedValueList.Add(getElementText(allowedValueEl));
						}
					}
				}
			} else if (el->GetTag().Compare("allowedValueRange") == 0) {
				priv->m_hasAllowedMinimum = false;
				priv->m_hasAllowedMaximum = false;
				priv->m_hasAllowedStep = false;
				for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
					if (const NPT_XmlElementNode *rangeEl = (*el->GetChildren().GetItem(j))->AsElementNode()) {
						if (rangeEl->GetTag().Compare("minimum") == 0) {
							priv->m_hasAllowedMinimum = true;
							priv->m_allowedMinimum = getElementText(rangeEl);
						} else if (rangeEl->GetTag().Compare("maximum") == 0) {
							priv->m_hasAllowedMaximum = true;
							priv->m_allowedMaximum = getElementText(rangeEl);
						} else if (rangeEl->GetTag().Compare("step") == 0) {
							priv->m_hasAllowedStep = true;
							priv->m_allowedStep = getElementText(rangeEl);
						}
					}
				}
			}
		}
	}
	return stateVariableDesc;
}

bool parseScpd(const NPT_String& xml, NPT_List<ActionDesc*>& actionList, NPT_List<StateVariableDesc*>& stateVariableList)
{
	NPT_XmlParser parser;
	NPT_XmlNode *rootNode;
	if (NPT_FAILED(parser.Parse(xml, xml.GetLength(), rootNode))) {
		return false;
	}

	PtrHolder<NPT_XmlNode> rootNode1(rootNode);
	const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
	if (!rootEl || !matchTagNamespace(rootEl, "scpd", "urn:schemas-upnp-org:service-1-0")) {
		return false;
	}

	for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
		if (const NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
			if (matchNamespace(el, "urn:schemas-upnp-org:service-1-0")) {
				if (el->GetTag().Compare("actionList") == 0) {
					for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
						if (const NPT_XmlElementNode *actionEl = (*el->GetChildren().GetItem(j))->AsElementNode()) {
							if (actionEl->GetTag().Compare("action") == 0) {
								ActionDesc *actionDesc = parseActionDesc(actionEl);
								if (actionDesc) {
									actionList.Add(actionDesc);
								} else {
									actionList.Clear();
									return false;
								}
							}
						}
					}
				} else if (el->GetTag().Compare("serviceStateTable") == 0) {
					for (NPT_Ordinal j = 0; j < el->GetChildren().GetItemCount(); j++) {
						if (const NPT_XmlElementNode *stateVariableEl = (*el->GetChildren().GetItem(j))->AsElementNode()) {
							if (stateVariableEl->GetTag().Compare("stateVariable") == 0) {
								StateVariableDesc *stateVariableDesc = parseStateVariableDesc(stateVariableEl);
								if (stateVariableDesc) {
									stateVariableList.Add(stateVariableDesc);
								} else {
									stateVariableList.Clear();
									return false;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;
}

void collectDeviceURLs(const DeviceDesc *deviceDesc, NPT_Map<ServiceDesc*, NPT_String>& scpdMap, NPT_Map<IconDesc*, NPT_String>& iconMap)
{
	for (NPT_Ordinal i = 0; i < deviceDesc->serviceCount(); i++) {
		ServiceDesc *serviceDesc = deviceDesc->serviceAt(i);
		scpdMap.Put(serviceDesc, serviceDesc->scpdURL());
	}

	for (NPT_Ordinal i = 0; i < deviceDesc->iconCount(); i++) {
		IconDesc *iconDesc = deviceDesc->iconAt(i);
		iconMap.Put(iconDesc, iconDesc->url());
	}

	for (NPT_Ordinal i = 0; i < deviceDesc->childCount(); i++) {
		collectDeviceURLs(deviceDesc->childAt(i), scpdMap, iconMap);
	}
}

DeviceDiscoveryTask::HttpGetDescContext::HttpGetDescContext(NPT_DataBuffer& output, NPT_SharedVariable *signalVar)
	: m_output(output), m_signalVar(signalVar)
{
}

DeviceDiscoveryTask::HttpGetDescContext::~HttpGetDescContext()
{
}

void DeviceDiscoveryTask::HttpGetDescContext::httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content)
{
	if (NPT_SUCCEEDED(nr) && resp && content) {
		if (resp->GetEntity() && contentTypeIsUtf8Xml(resp->GetEntity()->GetContentType())) {
			m_output = *content;
		}
	}
	m_signalVar->SetValue(1);
}

DeviceDiscoveryTask::HttpGetScpdContext::HttpGetScpdContext(ServiceDesc *serviceDesc, NPT_AtomicVariable *avar, NPT_SharedVariable *signalVar)
	: m_serviceDesc(serviceDesc), m_avar(avar), m_signalVar(signalVar)
{
}

DeviceDiscoveryTask::HttpGetScpdContext::~HttpGetScpdContext()
{
}

void DeviceDiscoveryTask::HttpGetScpdContext::httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content)
{
	if (NPT_SUCCEEDED(nr) && resp && content) {
		ServiceDescPrivate *priv = ServiceDescPrivate::getPrivate(m_serviceDesc);
		priv->m_scpdXML = NPT_String(reinterpret_cast<const char*>(content->GetData()), content->GetDataSize());
		parseScpd(priv->m_scpdXML, priv->m_actionList, priv->m_stateVariableList);
	}
	if (m_avar->Decrement() == 0) {
		m_signalVar->SetValue(1);
	}
}

DeviceDiscoveryTask::HttpGetIconContext::HttpGetIconContext(IconDesc *iconDesc, NPT_AtomicVariable *avar, NPT_SharedVariable *signalVar)
	: m_iconDesc(iconDesc), m_avar(avar), m_signalVar(signalVar)
{
}

DeviceDiscoveryTask::HttpGetIconContext::~HttpGetIconContext()
{
}

void DeviceDiscoveryTask::HttpGetIconContext::httpGetTaskFinished(HttpGetTask *task, NPT_Result nr, const NPT_HttpResponse *resp, const NPT_DataBuffer *content)
{
	if (NPT_SUCCEEDED(nr) && resp && content) {
		IconDescPrivate::getPrivate(m_iconDesc)->m_iconData = *content;
	}
	if (m_avar->Decrement() == 0) {
		m_signalVar->SetValue(1);
	}
}

void DeviceDiscoveryTask::exec()
{
	NPT_Result nr;

	NPT_DataBuffer content;
	NPT_SharedVariable waitVar;
	waitVar.SetValue(0);
	HttpGetDescContext *httpCtx = new HttpGetDescContext(content, &waitVar);
	nr = taskGroup()->startTask(new HttpGetTask(m_location, httpCtx, true));
	if (NPT_FAILED(nr)) {
		return report(nr);
	}

	waitVar.WaitWhileEquals(0);
	if (aborted()) {
		return report(NPT_ERROR_CANCELLED);
	}

	NPT_XmlParser parser;
	NPT_XmlNode *rootNode;
	NPT_MemoryStream xmlInputStream(content.GetData(), content.GetDataSize());
	nr = parser.Parse(xmlInputStream, rootNode);
	if (NPT_FAILED(nr)) {
		return report(nr);
	}

	PtrHolder<NPT_XmlNode> rootNode1(rootNode);
	const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
	if (!rootEl) {
		return report(NPT_FAILURE);
	}

	if (!matchTagNamespace(rootEl, "root", "urn:schemas-upnp-org:device-1-0")) {
		return report(NPT_FAILURE);
	}

	const NPT_XmlElementNode *rootDeviceEl = rootEl->GetChild("device", "urn:schemas-upnp-org:device-1-0");
	if (!rootDeviceEl) {
		return report(NPT_FAILURE);
	}

	NPT_Url uu(m_location);
	NPT_String rootURL = NPT_String::Format("%s://%s:%d", uu.GetScheme().GetChars(), uu.GetHost().GetChars(), uu.GetPort());
	NPT_String baseURL = m_location;
	int pos = baseURL.ReverseFind('/');
	if (pos <= 0) {
		return report(NPT_FAILURE);
	}

	baseURL = baseURL.Left(pos + 1);

	const NPT_XmlElementNode *urlBaseEl = rootEl->GetChild("URLBase", "urn:schemas-upnp-org:device-1-0");
	if (urlBaseEl) {
		baseURL = getElementText(urlBaseEl);
		if (!baseURL.EndsWith("/")) {
			baseURL.Append("/");
		}
	}

	DeviceDesc *rootDeviceDesc = parseDeviceDesc(rootDeviceEl, rootURL, baseURL);
	if (!rootDeviceDesc) {
		return report(NPT_FAILURE);
	}

	DeviceDescHolder rootDeviceDesc1(rootDeviceDesc);
	if (rootDeviceDesc->uuid() != m_rootDeviceUuid) {
		return report(NPT_FAILURE);
	}

	NPT_Map<ServiceDesc*, NPT_String> scpdMap;
	NPT_Map<IconDesc*, NPT_String> iconMap;
	collectDeviceURLs(rootDeviceDesc, scpdMap, iconMap);

	NPT_AtomicVariable avar(scpdMap.GetEntryCount() + iconMap.GetEntryCount());
	avar.Increment();
	if (avar.GetValue() > 0) {
		waitVar.SetValue(0);

		for (NPT_Ordinal i = 0; i < scpdMap.GetEntries().GetItemCount(); i++) {
			NPT_Map<ServiceDesc*, NPT_String>::Entry *e = *scpdMap.GetEntries().GetItem(i);
			if (NPT_SUCCEEDED(taskGroup()->startTask(new HttpGetTask(e->GetValue(), new HttpGetScpdContext(e->GetKey(), &avar, &waitVar), true)))) {
			} else {
				avar.Decrement();
			}
		}

		for (NPT_Ordinal i = 0; i < iconMap.GetEntries().GetItemCount(); i++) {
			NPT_Map<IconDesc*, NPT_String>::Entry *e = *iconMap.GetEntries().GetItem(i);
			if (NPT_SUCCEEDED(taskGroup()->startTask(new HttpGetTask(e->GetValue(), new HttpGetIconContext(e->GetKey(), &avar, &waitVar), true)))) {
			} else {
				avar.Decrement();
			}
		}

		if (avar.Decrement() == 0) {
			waitVar.SetValue(1);
		}

		waitVar.WaitWhileEquals(0);
	}

	rootDeviceDesc->addRef();
	report(NPT_SUCCESS, rootDeviceDesc);
}

void DeviceDiscoveryTask::doAbort()
{
}

void DeviceDiscoveryTask::report(NPT_Result nr, DeviceDesc *rootDeviceDesc)
{
	if (m_callback) {
		m_callback->deviceDiscoveryTaskFinished(this, nr, rootDeviceDesc, m_rootDeviceUuid);
	}
}

} // namespace deejay
