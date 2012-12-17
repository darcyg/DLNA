#include "DJDeviceImpl.h"

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("deejay.DeviceImpl")

namespace deejay {

DeviceImpl::DeviceImpl()
	: m_state(State_Detached)
	, m_frontEnd(NULL)
	, m_expireSeconds(300)
	, m_descPath("desc")
{
	m_attachedTaskGroup = new TaskGroup();
}

DeviceImpl::~DeviceImpl()
{
	delete m_attachedTaskGroup;
	m_serviceInfoList.Apply(NPT_ObjectDeleter<ServiceInfo>());
}

UUID DeviceImpl::uuid() const
{
	return m_uuid;
}

NPT_String DeviceImpl::deviceType() const
{
	return m_deviceType;
}

NPT_String DeviceImpl::friendlyName() const
{
	return m_friendlyName;
}

NPT_String DeviceImpl::manufacturer() const
{
	return m_manufacturer;
}

NPT_String DeviceImpl::manufacturerURL() const
{
	return m_manufacturerURL;
}

NPT_String DeviceImpl::modelDescription() const
{
	return m_modelDescription;
}

NPT_String DeviceImpl::modelName() const
{
	return m_modelName;
}

NPT_String DeviceImpl::modelNumber() const
{
	return m_modelNumber;
}

NPT_String DeviceImpl::modelURL() const
{
	return m_modelURL;
}

NPT_String DeviceImpl::serialNumber() const
{
	return m_serialNumber;
}

NPT_String DeviceImpl::upc() const
{
	return m_upc;
}

NPT_String DeviceImpl::presentationURL() const
{
	return m_presentationURL;
}

const FrontEnd::DeviceImplContext *DeviceImpl::frontEndContext() const
{
	return &m_frontEndContext;
}

void DeviceImpl::registerServices(const ServiceDecl *decls, NPT_UInt32 numServices)
{
	for (NPT_UInt32 i = 0; i < numServices; i++) {
		registerService(decls + i);
	}
}

void DeviceImpl::registerStaticContent(const NPT_String& path, const NPT_String& mimeType, const NPT_DataBuffer& data, bool copy)
{
	m_staticContentList.Add(StaticContentInfo());
	NPT_List<StaticContentInfo>::Iterator it = m_staticContentList.GetLastItem();
	it->m_path = path;
	it->m_mimeType = mimeType;
	it->m_data = NPT_DataBuffer(data.GetData(), data.GetDataSize(), copy);
}

NPT_String varDataTypeStr(StateVariableDataType dataType)
{
	const char *desc = NULL;
	switch (dataType) {
	case SVDT_STRING:
		desc = "string";
		break;
	case SVDT_BOOLEAN:
		desc = "boolean";
		break;
	case SVDT_UI1:
		desc = "ui1";
		break;
	case SVDT_UI2:
		desc = "ui2";
		break;
	case SVDT_UI4:
		desc = "ui4";
		break;
	case SVDT_UI8:
		desc = "ui8";
		break;
	case SVDT_I1:
		desc = "i1";
		break;
	case SVDT_I2:
		desc = "i2";
		break;
	case SVDT_I4:
		desc = "i4";
		break;
	case SVDT_I8:
		desc = "i8";
		break;
	case SVDT_INT:
		desc = "int";
		break;
	case SVDT_BASE64:
		desc = "bin.base64";
		break;
	}
	return desc;
}

NPT_String generateSCPD(const ServiceDecl *serviceDecl)
{
	NPT_StringOutputStream outputStream;
	NPT_XmlSerializer xml(&outputStream, 0, true, true);

	xml.StartDocument();
	xml.StartElement(NULL, "scpd");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:service-1-0");

	xml.StartElement(NULL, "specVersion");
	xml.StartElement(NULL, "major");
	xml.Text("1");
	xml.EndElement(NULL, "major");
	xml.StartElement(NULL, "minor");
	xml.Text("0");
	xml.EndElement(NULL, "minor");
	xml.EndElement(NULL, "specVersion");

	if (serviceDecl->numActions > 0) {
		xml.StartElement(NULL, "actionList");

		for (NPT_UInt32 i = 0; i < serviceDecl->numActions; i++) {
			const ActionDecl *actDecl = serviceDecl->actions + i;
			xml.StartElement(NULL, "action");

			xml.StartElement(NULL, "name");
			xml.Text(actDecl->name);
			xml.EndElement(NULL, "name");

			if (actDecl->argumentCount > 0) {
				xml.StartElement(NULL, "argumentList");

				for (NPT_UInt32 j = 0; j < actDecl->argumentCount; j++) {
					const ArgumentDecl *argDecl = actDecl->arguments + j;
					xml.StartElement(NULL, "argument");

					xml.StartElement(NULL, "name");
					xml.Text(argDecl->name);
					xml.EndElement(NULL, "name");

					xml.StartElement(NULL, "direction");
					xml.Text((argDecl->flags & AF_IN) != 0 ? "in" : "out");
					xml.EndElement(NULL, "direction");

					xml.StartElement(NULL, "relatedStateVariable");
					xml.Text(argDecl->relatedStateVariable);
					xml.EndElement(NULL, "relatedStateVariable");

					if ((argDecl->flags & AF_RETVAL)) {
						xml.StartElement(NULL, "retval");
						xml.EndElement(NULL, "retval");
					}

					xml.EndElement(NULL, "argument");
				}

				xml.EndElement(NULL, "argumentList");
			}

			xml.EndElement(NULL, "action");
		}

		xml.EndElement(NULL, "actionList");
	}

	if (serviceDecl->numStateVariables > 0) {
		xml.StartElement(NULL, "serviceStateTable");

		for (NPT_UInt32 i = 0; i < serviceDecl->numStateVariables; i++) {
			const StateVariableDecl *varDecl = serviceDecl->stateVariables + i;
			xml.StartElement(NULL, "stateVariable");
			xml.Attribute(NULL, "sendEvents", (varDecl->flags & SVF_EVT) != 0 ? "yes" : "no");

			xml.StartElement(NULL, "name");
			xml.Text(varDecl->name);
			xml.EndElement(NULL, "name");

			xml.StartElement(NULL, "dataType");
			xml.Text(varDataTypeStr(varDecl->dataType));
			xml.EndElement(NULL, "dataType");

			if (varDecl->defaultValue) {
				xml.StartElement(NULL, "defaultValue");
				xml.Text(varDecl->defaultValue);
				xml.EndElement(NULL, "defaultValue");
			}

			if (varDecl->allowedValue) {
				if ((varDecl->flags & SVF_AVR)) {
					xml.StartElement(NULL, "allowedValueRange");

					if (varDecl->allowedValue[0]) {
						xml.StartElement(NULL, "minimum");
						xml.Text(varDecl->allowedValue[0]);
						xml.EndElement(NULL, "minimum");
					}

					if (varDecl->allowedValue[1]) {
						xml.StartElement(NULL, "maximum");
						xml.Text(varDecl->allowedValue[1]);
						xml.EndElement(NULL, "maximum");
					}

					if (varDecl->allowedValue[2]) {
						xml.StartElement(NULL, "step");
						xml.Text(varDecl->allowedValue[2]);
						xml.EndElement(NULL, "step");
					}

					xml.EndElement(NULL, "allowedValueRange");
				} else {
					xml.StartElement(NULL, "allowedValueList");

					const char **ag = varDecl->allowedValue;
					while (*ag) {
						xml.StartElement(NULL, "allowedValue");
						xml.Text(*ag);
						xml.EndElement(NULL, "allowedValue");
						ag++;
					}

					xml.EndElement(NULL, "allowedValueList");
				}
			}

			xml.EndElement(NULL, "stateVariable");
		}

		xml.EndElement(NULL, "serviceStateTable");
	}

	xml.EndElement(NULL, "scpd");
	xml.EndDocument();

	return outputStream.GetString();
}

void DeviceImpl::registerService(const ServiceDecl *decl)
{
	ServiceInfo *serviceInfo = new ServiceInfo();
	m_serviceInfoList.Add(serviceInfo);
	serviceInfo->m_serviceDecl = decl;
	serviceInfo->m_scpdXml = generateSCPD(decl);
	serviceInfo->m_scpdPath = NPT_String::Format("services/%s/scpd", decl->serviceId);
	serviceInfo->m_controlPath = NPT_String::Format("services/%s/control", decl->serviceId);
	serviceInfo->m_eventSubPath = NPT_String::Format("services/%s/eventSub", decl->serviceId);

	for (NPT_UInt32 i = 0; i < decl->numStateVariables; i++) {
		const StateVariableDecl *varDecl = decl->stateVariables + i;
		if ((varDecl->flags & SVF_ARG) == 0) {
			StateVarInfo *varInfo = new StateVarInfo();
			serviceInfo->m_stateVarList.Add(varInfo);
			serviceInfo->m_stateVarIndex.Put(varDecl->name, varInfo);
			varInfo->m_varDecl = varDecl;
			if (varDecl->defaultValue) {
				varInfo->m_value = varDecl->defaultValue;
			}
			varInfo->m_cachedValue = varInfo->m_value;
		}
	}
}

bool DeviceImpl::setStateValue(const NPT_String& serviceId, const NPT_String& name, const NPT_String& value)
{
	WriteLocker locker(m_stateLock);
	NPT_List<ServiceInfo*>::Iterator it = m_serviceInfoList.Find(ServiceInfoByIdFinder(serviceId));
	if (it) {
		ServiceInfo *serviceInfo = *it;
		StateVarInfo **ppvalue;
		if (NPT_SUCCEEDED(serviceInfo->m_stateVarIndex.Get(name, ppvalue))) {
			StateVarInfo *varInfo = *ppvalue;
			varInfo->m_value = value;
			return true;
		}
	}
	return false;
}

bool DeviceImpl::getStateValue(const NPT_String& serviceId, const NPT_String& name, NPT_String& value) const
{
	ReadLocker locker(m_stateLock);
	NPT_List<ServiceInfo*>::Iterator it = m_serviceInfoList.Find(ServiceInfoByIdFinder(serviceId));
	if (it) {
		ServiceInfo *serviceInfo = *it;
		StateVarInfo **ppvalue;
		if (NPT_SUCCEEDED(serviceInfo->m_stateVarIndex.Get(name, ppvalue))) {
			const StateVarInfo *varInfo = *ppvalue;
			value = varInfo->m_value;
			return true;
		}
	}
	return false;
}

bool DeviceImpl::match(const NPT_String& st, NPT_List<FrontEnd::DeviceImplMatch>& ls)
{
	NPT_String uuidStr = NPT_String::Format("uuid:%s", uuid().toString().GetChars());
	if (st.Compare("ssdp:all") == 0) {
		ls.Add(FrontEnd::DeviceImplMatch("upnp:rootdevice", NPT_String::Format("%s::upnp:rootdevice", uuidStr.GetChars())));
		ls.Add(FrontEnd::DeviceImplMatch(uuidStr, uuidStr));
		ls.Add(FrontEnd::DeviceImplMatch(deviceType(), NPT_String::Format("%s::%s", uuidStr.GetChars(), deviceType().GetChars())));
		for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
			ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
			ls.Add(FrontEnd::DeviceImplMatch(serviceInfo->m_serviceDecl->serviceType, NPT_String::Format("%s::%s", uuidStr.GetChars(), serviceInfo->m_serviceDecl->serviceType)));
		}
		return true;
	}

	if (st.Compare("upnp:rootdevice") == 0) {
		ls.Add(FrontEnd::DeviceImplMatch("upnp:rootdevice", NPT_String::Format("%s::upnp:rootdevice", uuidStr.GetChars())));
		return true;
	}

	if (st.Compare(uuidStr) == 0) {
		ls.Add(FrontEnd::DeviceImplMatch(uuidStr, uuidStr));
		return true;
	}

	// TODO: match version number ??
	if (st.Compare(deviceType()) == 0) {
		ls.Add(FrontEnd::DeviceImplMatch(deviceType(), NPT_String::Format("%s::%s", uuidStr.GetChars(), deviceType().GetChars())));
		return true;
	}

	for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
		ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
		const ServiceDecl *serviceDecl = serviceInfo->m_serviceDecl;
		// TODO: match version number ??
		if (st.Compare(serviceDecl->serviceType) == 0) {
			ls.Add(FrontEnd::DeviceImplMatch(serviceDecl->serviceType, NPT_String::Format("%s::%s", uuidStr.GetChars(), serviceDecl->serviceType)));
			return true;
		}
	}

	return false;
}

void DeviceImpl::implAttach(FrontEnd *frontEnd, const FrontEnd::DeviceImplContext& context)
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Detached) {
		return;
	}

	m_attachedTaskGroup->reset();

	m_frontEnd = frontEnd;
	m_frontEndContext = context;

	m_removeExpiredEventSubVar.SetValue(0);
	m_attachedTaskGroup->startTask(new RemoveExpiredEventSubTask(this));
	m_monitorVar.SetValue(0);
	m_attachedTaskGroup->startTask(new StateVarMonitorTask(this));

	m_state = State_Attached;
}

void DeviceImpl::implDetach()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_state != State_Attached) {
			return;
		}
		m_state = State_Detaching;
	}

	m_attachedTaskGroup->abort();
	NPT_LOG_INFO_3("DeviceImpl for %p[%s] waiting TaskGroup %p", this, friendlyName().GetChars(), &m_attachedTaskGroup);
	m_attachedTaskGroup->wait();

	{
		WriteLocker locker(m_stateLock);
		m_state = State_Detached;
		m_frontEnd = NULL;
	}
}

void DeviceImpl::serveSimpleContent(const NPT_String& content, const NPT_String& contentType, const FrontEnd::InterfaceContext *ifctx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	bool isGetMethod = req->GetMethod().Compare("GET") == 0;
	bool isHeadMethod = req->GetMethod().Compare("HEAD") == 0;

	if (isGetMethod || isHeadMethod) {
		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, contentType);
		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromInteger(content.GetLength()));
		httpOutput->writeResponseHeader(resp);

		if (isGetMethod) {
			httpOutput->writeData(content);
		}
	} else {
		setStatusCode(resp, 405);
		resp.GetHeaders().SetHeader("Allow", "GET, HEAD");
		httpOutput->writeResponseHeader(resp);
	}
	httpOutput->flush();
}

DeviceImpl::ProcessHttpRequestTask::ProcessHttpRequestTask(DeviceImpl *owner, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket)
	: m_owner(owner), m_ifctx(ifctx), m_relPath(relPath), m_reqCtx(reqCtx), m_req(req), m_resp(resp), m_inputStream(inputStream), m_httpOutput(httpOutput), m_clientSocket(clientSocket)
{
}

DeviceImpl::ProcessHttpRequestTask::~ProcessHttpRequestTask()
{
	delete m_httpOutput;
	delete m_inputStream;
	delete m_resp;
	delete m_req;
	delete m_clientSocket;
}

void DeviceImpl::ProcessHttpRequestTask::exec()
{
	m_owner->processHttpRequest(this, m_ifctx, m_relPath, m_reqCtx, m_req, *m_resp, m_inputStream, m_httpOutput);
}

void DeviceImpl::ProcessHttpRequestTask::doAbort()
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

bool DeviceImpl::ProcessHttpRequestTask::registerAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	if (aborted()) {
		return false;
	}
	m_callbackList.Add(callback);
	return true;
}

void DeviceImpl::ProcessHttpRequestTask::unregisterAbortCallback(Callback *callback)
{
	WriteLocker locker(m_stateLock);
	m_callbackList.Remove(callback, true);
}

static const NPT_String g_ctXMLUTF8("text/xml; charset=\"utf-8\"");

void DeviceImpl::processHttpRequest(const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, NPT_HttpRequest *req, NPT_HttpResponse *resp, NPT_InputStream *inputStream, HttpOutput *httpOutput, NPT_Socket *clientSocket)
{
	m_attachedTaskGroup->startTask(new ProcessHttpRequestTask(this, ifctx, relPath, reqCtx, req, resp, inputStream, httpOutput, clientSocket));
}

void DeviceImpl::processHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	if (relPath.Compare(m_descPath) == 0) {
		NPT_String descXml = generateDeviceDescriptionXml(ifctx, &m_frontEndContext, reqCtx, req);
		return serveSimpleContent(descXml, g_ctXMLUTF8, ifctx, req, resp, inputStream, httpOutput);
	}

	for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
		ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
		if (relPath.Compare(serviceInfo->m_scpdPath) == 0) {
			return serveSimpleContent(serviceInfo->m_scpdXml, g_ctXMLUTF8, ifctx, req, resp, inputStream, httpOutput);
		} else if (relPath.Compare(serviceInfo->m_controlPath) == 0) {
			return processControlRequest(serviceInfo, task, ifctx, reqCtx, req, resp, inputStream, httpOutput);
		} else if (relPath.Compare(serviceInfo->m_eventSubPath) == 0) {
			return processEventSubRequest(serviceInfo, task, ifctx, reqCtx, req, resp, inputStream, httpOutput);
		}
	}

	for (NPT_Ordinal i = 0; i < m_staticContentList.GetItemCount(); i++) {
		NPT_List<StaticContentInfo>::Iterator it = m_staticContentList.GetItem(i);
		if (it->m_path.Compare(relPath) == 0) {
			setStatusCode(resp, 200);
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, it->m_mimeType);
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromInteger(it->m_data.GetDataSize()));
			httpOutput->writeResponseHeader(resp);
			httpOutput->writeData(it->m_data.GetData(), it->m_data.GetDataSize());
			httpOutput->flush();
			return;
		}
	}

	if (!onHttpRequest(task, ifctx, relPath, reqCtx, req, resp, inputStream, httpOutput)) {
		setStatusCode(resp, 404);
		httpOutput->writeResponseHeader(resp);
		httpOutput->flush();
	}
}

bool DeviceImpl::onHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	return false;
}

NPT_String mapUPnPErrorDesc(int errorCode)
{
	NPT_String errDesc;
	switch (errorCode) {
	case 401:
		errDesc = "Invalid action";
		break;
	case 402:
		errDesc = "Invalid args";
		break;
	case 501:
		errDesc = "Action failed";
		break;
	case 600:
		errDesc = "Argument value invalid";
		break;
	case 601:
		errDesc = "Argument value out of range";
		break;
	case 602:
		errDesc = "Optional action not implemented";
		break;
	case 603:
		errDesc = "Out of memory";
		break;
	case 604:
		errDesc = "Human intervention required";
		break;
	case 605:
		errDesc = "String argument too long";
		break;
	case 606:
		errDesc = "Action not authorized";
		break;
	case 607:
		errDesc = "Signature failure";
		break;
	case 608:
		errDesc = "Signature missing";
		break;
	case 609:
		errDesc = "Not encrypted";
		break;
	case 610:
		errDesc = "Invalid sequence";
		break;
	case 611:
		errDesc = "Invalid control URL";
		break;
	case 612:
		errDesc = "No such session";
		break;
	case 701:
		errDesc = "No such object";
		break;
	case 709:
		errDesc = "Unsupported or invalid sort criteria";
		break;
	case 720:
		errDesc = "Cannot process the request";
		break;
	default:
		errDesc = "WTF!!!";
		break;
	}
	return errDesc;
}

void emitActionResponse(NPT_HttpResponse& resp, HttpOutput *httpOutput, const ServiceDecl *serviceDecl, const ActionDecl *actionDecl, const NPT_List<NPT_String>& outputArgValues)
{
	NPT_StringOutputStream xmlOutputStream;
	NPT_XmlSerializer xml(&xmlOutputStream, 0, true, true);
	xml.StartDocument();
	xml.StartElement("s", "Envelope");
	xml.Attribute("xmlns", "s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.Attribute("s", "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
	xml.StartElement("s", "Body");

	NPT_String actionResponse = NPT_String::Format("%sResponse", actionDecl->name);
	xml.StartElement("u", actionResponse);
	xml.Attribute("xmlns", "u", serviceDecl->serviceType);
	NPT_UInt32 outputArgCount = actionDecl->argumentCount - actionDecl->inputArgumentCount;
	const ArgumentDecl *argDecl = actionDecl->arguments + actionDecl->inputArgumentCount;
	for (NPT_UInt32 i = 0; i < outputArgCount; i++, argDecl++) {
		xml.StartElement(NULL, argDecl->name);
		if (i < outputArgValues.GetItemCount()) {
			xml.Text(*outputArgValues.GetItem(i));
		} else {
			xml.Text("");
		}
		xml.EndElement(NULL, argDecl->name);
	}
	xml.EndElement("u", actionResponse);

	xml.EndElement("s", "Body");
	xml.EndElement("s", "Envelope");
	xml.EndDocument();

	const NPT_String& content = xmlOutputStream.GetString();

	setStatusCode(resp, 200);
	resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromInteger(content.GetLength()));
	resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, "text/xml; charset=\"utf-8\"");
	resp.GetHeaders().SetHeader("EXT", "");
	httpOutput->writeResponseHeader(resp);
	httpOutput->writeData(content);
}

void emitUPnPError(NPT_HttpResponse& resp, int errorCode, HttpOutput *httpOutput)
{
	NPT_StringOutputStream xmlOutputStream;
	NPT_XmlSerializer xml(&xmlOutputStream, 0, true, true);
	xml.StartDocument();
	xml.StartElement("s", "Envelope");
	xml.Attribute("xmlns", "s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.Attribute("s", "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
	xml.StartElement("s", "Body");
	xml.StartElement("s", "Fault");

	xml.StartElement(NULL, "faultcode");
	xml.Text("s:Client");
	xml.EndElement(NULL, "faultcode");

	xml.StartElement(NULL, "faultstring");
	xml.Text("UPnPError");
	xml.EndElement(NULL, "faultstring");

	xml.StartElement(NULL, "detail");
	xml.StartElement(NULL, "UPnPError");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:control-1-0");

	xml.StartElement(NULL, "errorCode");
	xml.Text(NPT_String::FromInteger(errorCode));
	xml.EndElement(NULL, "errorCode");

	xml.StartElement(NULL, "errorDescription");
	xml.Text(mapUPnPErrorDesc(errorCode));
	xml.EndElement(NULL, "errorDescription");

	xml.EndElement(NULL, "UPnPError");
	xml.EndElement(NULL, "detail");

	xml.EndElement("s", "Fault");
	xml.EndElement("s", "Body");
	xml.EndElement("s", "Envelope");
	xml.EndDocument();

	const NPT_String& content = xmlOutputStream.GetString();

	setStatusCode(resp, 500);
	resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromInteger(content.GetLength()));
	resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, "text/xml; charset=\"utf-8\"");
	resp.GetHeaders().SetHeader("EXT", "");
	httpOutput->writeResponseHeader(resp);
	httpOutput->writeData(content);
}

void DeviceImpl::processControlRequest(ServiceInfo *serviceInfo, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	bool isPostMethod = req->GetMethod().Compare("POST") == 0;
	bool isMPostMethod = req->GetMethod().Compare("M-POST") == 0;

	if (isPostMethod || isMPostMethod) {
		int statusCode = 200;
		bool httpOk = false;
		do {
			NPT_HttpHeader *hdrContentLength = req->GetHeaders().GetHeader("CONTENT-LENGTH");
			NPT_Size contentLength;
			if (!hdrContentLength || NPT_FAILED(NPT_ParseInteger(hdrContentLength->GetValue(), contentLength))) {
				statusCode = 400;
				break;
			}

			NPT_HttpHeader *hdrContentType = req->GetHeaders().GetHeader("CONTENT-TYPE");
			if (!hdrContentType || !contentTypeIsUtf8Xml(hdrContentType->GetValue())) {
				statusCode = 400;
				break;
			}

			NPT_String soapActionHeader;
			if (isMPostMethod) {
				NPT_HttpHeader *hdrMAN = req->GetHeaders().GetHeader("MAN");
				if (!hdrMAN || !hdrMAN->GetValue().StartsWith("\"http://schemas.xmlsoap.org/soap/envelope/\"")) {
					statusCode = 400;
					break;
				}

				int sep = hdrMAN->GetValue().Find("ns=");
				if (sep <= 0) {
					statusCode = 400;
					break;
				}

				soapActionHeader = hdrMAN->GetValue().SubString(sep+3) + "-SOAPACTION";

			} else {
				soapActionHeader = "SOAPACTION";
			}

			NPT_HttpHeader *hdrSoapAction = req->GetHeaders().GetHeader(soapActionHeader);
			if (!hdrSoapAction || !hdrSoapAction->GetValue().StartsWith("\"") || !hdrSoapAction->GetValue().EndsWith("\"")) {
				statusCode = 400;
				break;
			}

			NPT_String soapActionStr = hdrSoapAction->GetValue();
			soapActionStr = soapActionStr.SubString(1, soapActionStr.GetLength() - 2);

			int sep2 = soapActionStr.Find('#');
			if (sep2 <= 0) {
				statusCode = 400;
				break;
			}

			NPT_String serviceType = soapActionStr.Left(sep2);
			NPT_String actionName = soapActionStr.SubString(sep2 + 1);

			if (serviceType.Compare(serviceInfo->m_serviceDecl->serviceType) != 0) {
				statusCode = 400;
				break;
			}

			NPT_DataBuffer content(contentLength);
			if (NPT_FAILED(inputStream->Load(content, contentLength))) {
				// aborted, return ASAP!!!
				return;
			}
			
			if (contentLength != content.GetDataSize()) {
				statusCode = 400;
				break;
			}

			NPT_XmlParser parser;
			NPT_XmlNode *rootNode;
			if (NPT_FAILED(parser.Parse(reinterpret_cast<const char*>(content.GetData()), contentLength, rootNode))) {
				statusCode = 400;
				break;
			}

			PtrHolder<NPT_XmlNode> rootNode1(rootNode);

			const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
			if (!rootEl) {
				statusCode = 400;
				break;
			}

			if (!matchTagNamespace(rootEl, "Envelope", "http://schemas.xmlsoap.org/soap/envelope/")) {
				statusCode = 400;
				break;
			}

			const NPT_XmlElementNode *bodyEl = rootEl->GetChild("Body", "http://schemas.xmlsoap.org/soap/envelope/");
			if (!bodyEl) {
				statusCode = 400;
				break;
			}

			const NPT_XmlElementNode *actionEl = bodyEl->GetChild(actionName, serviceType);
			if (!actionEl) {
				statusCode = 400;
				break;
			}

			const ActionDecl *actionDecl = NULL;
			for (NPT_UInt32 i = 0; i < serviceInfo->m_serviceDecl->numActions; i++) {
				if (actionName.Compare((serviceInfo->m_serviceDecl->actions + i)->name) == 0) {
					actionDecl = serviceInfo->m_serviceDecl->actions + i;
					break;
				}
			}

			if (!actionDecl) {
				// Invalid action
				emitUPnPError(resp, 401, httpOutput);
				httpOk = true;
				break;
			}

			const ArgumentDecl *argDecl = actionDecl->arguments;
			const ArgumentDecl *argDeclE = argDecl + actionDecl->argumentCount;
			NPT_List<NPT_String> nameList;
			NPT_List<NPT_String> valueList;

			const NPT_List<NPT_XmlNode*>& args = actionEl->GetChildren();
			for (NPT_Ordinal i = 0; i < args.GetItemCount() && argDecl != argDeclE; i++) {
				if (const NPT_XmlElementNode *argEl = (*args.GetItem(i))->AsElementNode()) {
					NPT_String argName = argEl->GetTag();
					// xbox 360 uses "ContainerID" instead of "ObjectID"
					if (reqCtx.clientHint == FrontEnd::CH_XBox) {
						if (actionName.Compare("Browse") == 0 && actionDecl->inputArgumentCount == 6 && actionDecl->argumentCount == 10 && nameList.GetItemCount() == 0 && argName.Compare("ContainerID") == 0) {
							argName = "ObjectID";
						}
					}

					if (argName.Compare(argDecl->name) == 0 && ((argDecl->flags & AF_IN) != 0)) {
						nameList.Add(argDecl->name);
						valueList.Add(getElementText(argEl));
						argDecl++;
					}
				}
			}

			httpOk = true;

			if (nameList.GetItemCount() != actionDecl->inputArgumentCount) {
				// Invalid args
				emitUPnPError(resp, 402, httpOutput);
				break;
			}

			NPT_List<NPT_String> outputValueList;
			for (NPT_UInt32 i = 0; i < actionDecl->argumentCount - actionDecl->inputArgumentCount; i++) {
				outputValueList.Add(NPT_String());
			}
			int errorCode = onAction(serviceInfo->m_serviceDecl, actionDecl, task, ifctx, reqCtx, req, nameList, valueList, outputValueList);
			if (errorCode == 0) {
				emitActionResponse(resp, httpOutput, serviceInfo->m_serviceDecl, actionDecl, outputValueList);
			} else {
				emitUPnPError(resp, errorCode, httpOutput);
			}
		} while (false);

		if (!httpOk) {
			setStatusCode(resp, statusCode);
			httpOutput->writeResponseHeader(resp);
		}
	} else {
		setStatusCode(resp, 405);
		resp.GetHeaders().SetHeader("Allow", "SUBSCRIBE, UNSUBSCRIBE");
		httpOutput->writeResponseHeader(resp);
	}
	httpOutput->flush();
}

static bool parseEventSubCallback(const NPT_String& text, NPT_List<NPT_String>& callbackList)
{
	int pos1 = 0, pos2 = 0;
	for (;;) {
		pos1 = text.Find('<', pos2);
		if (pos1 >= 0) {
			pos2 = text.Find('>', pos1);
			if (pos2 > pos1) {
				NPT_String url = text.SubString(pos1 + 1, pos2 - pos1 - 1);
				if (url.StartsWith("http://", true)) {
					callbackList.Add(url);
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			break;
		}
	}
	return true;
}

void DeviceImpl::processEventSubRequest(ServiceInfo *serviceInfo, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	bool isSubscribeMethod = req->GetMethod().Compare("SUBSCRIBE") == 0;
	bool isUnsubscribeMethod = req->GetMethod().Compare("UNSUBSCRIBE") == 0;
	DeliverEventNotifyTask *deliverInitialNotifyTask = NULL;

	if (isSubscribeMethod) {
		int statusCode = 200;
		do {
			NPT_HttpHeader *hdrCallback = req->GetHeaders().GetHeader("CALLBACK");
			NPT_HttpHeader *hdrNT = req->GetHeaders().GetHeader("NT");
			NPT_HttpHeader *hdrSID = req->GetHeaders().GetHeader("SID");
			NPT_HttpHeader *hdrTimeout = req->GetHeaders().GetHeader("TIMEOUT");

			int timeout = -1;
			if (hdrTimeout && !parseTimeoutSecond(hdrTimeout->GetValue(), timeout)) {
				statusCode = 400;
				break;
			}

			if (hdrNT) {
				if (hdrSID) {
					statusCode = 400;
					break;
				}

				if (hdrNT->GetValue().Compare("upnp:event") != 0) {
					statusCode = 412;
					break;
				}

				NPT_List<NPT_String> callbackList;
				if (!hdrCallback || !parseEventSubCallback(hdrCallback->GetValue(), callbackList)) {
					statusCode = 412;
					break;
				}

				UUID sid;
				if (addEventSub(serviceInfo, callbackList, timeout, sid)) {
					resp.GetHeaders().SetHeader("SID", NPT_String::Format("uuid:%s", sid.toString().GetChars()));
					resp.GetHeaders().SetHeader("TIMEOUT", NPT_String::Format("Second-%d", timeout));
					{
						ReadLocker locker(m_stateLock);
						NPT_List<NPT_String> nameList;
						NPT_List<NPT_String> valueList;
						for (NPT_Ordinal vv = 0; vv < serviceInfo->m_stateVarList.GetItemCount(); vv++) {
							StateVarInfo *varInfo = *serviceInfo->m_stateVarList.GetItem(vv);
							if ((varInfo->m_varDecl->flags & SVF_EVT) != 0) {
								if (NPT_String::Compare(varInfo->m_varDecl->name, "LastChange") == 0) {
									if (NPT_String::Compare(serviceInfo->m_serviceDecl->serviceId, "urn:upnp-org:serviceId:AVTransport") == 0) {
										nameList.Add(varInfo->m_varDecl->name);
										valueList.Add(composeLastChangeAVT(serviceInfo));
									} else if (NPT_String::Compare(serviceInfo->m_serviceDecl->serviceId, "urn:upnp-org:serviceId:RenderingControl") == 0) {
										nameList.Add(varInfo->m_varDecl->name);
										valueList.Add(composeLastChangeRCS(serviceInfo));
									} else {
										nameList.Add(varInfo->m_varDecl->name);
										valueList.Add(varInfo->m_value);
									}
								} else {
									nameList.Add(varInfo->m_varDecl->name);
									valueList.Add(varInfo->m_value);
								}
							}
						}

						deliverInitialNotifyTask = new DeliverEventNotifyTask(this, callbackList, 0, serviceInfo->m_serviceDecl->serviceId, sid, nameList, valueList);
					}
				} else {
					statusCode = 500;
					break;
				}
			} else {
				if (hdrCallback) {
					statusCode = 400;
					break;
				}

				UUID sid;
				if (!hdrSID || !extractUuidFromUSN(hdrSID->GetValue(), sid)) {
					statusCode = 412;
					break;
				}

				if (renewEventSub(serviceInfo, sid, timeout)) {
					resp.GetHeaders().SetHeader("SID", NPT_String::Format("uuid:%s", sid.toString().GetChars()));
					resp.GetHeaders().SetHeader("TIMEOUT", NPT_String::Format("Second-%d", timeout));
				} else {
					statusCode = 412;
					break;
				}
			}
		} while (false);
		setStatusCode(resp, statusCode);
		httpOutput->writeResponseHeader(resp);
	} else if (isUnsubscribeMethod) {
		int statusCode = 200;
		do {
			UUID sid;
			NPT_HttpHeader *hdrSID = req->GetHeaders().GetHeader("SID");
			if (!hdrSID || !extractUuidFromUSN(hdrSID->GetValue(), sid)) {
				statusCode = 412;
				break;
			}

			if (req->GetHeaders().GetHeader("NT") || req->GetHeaders().GetHeader("CALLBACK")) {
				statusCode = 400;
				break;
			}

			if (!removeEventSub(serviceInfo, sid)) {
				statusCode = 412;
				break;
			}
		} while (false);
		setStatusCode(resp, statusCode);
		httpOutput->writeResponseHeader(resp);
	} else {
		setStatusCode(resp, 405);
		resp.GetHeaders().SetHeader("Allow", "SUBSCRIBE, UNSUBSCRIBE");
		httpOutput->writeResponseHeader(resp);
	}
	httpOutput->flush();

	if (deliverInitialNotifyTask) {
		m_attachedTaskGroup->startTask(deliverInitialNotifyTask);
	}
}

NPT_String DeviceImpl::generateDeviceDescriptionXml(const FrontEnd::InterfaceContext *ifctx, const FrontEnd::DeviceImplContext *fectx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req)
{
	NPT_StringOutputStream outputStream0;
	NPT_XmlSerializer xml(&outputStream0, 0, true, true);

	xml.StartDocument();
	xml.StartElement(NULL, "root");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:device-1-0");

	xml.StartElement(NULL, "specVersion");
	xml.StartElement(NULL, "major");
	xml.Text("1");
	xml.EndElement(NULL, "major");
	xml.StartElement(NULL, "minor");
	xml.Text("0");
	xml.EndElement(NULL, "minor");
	xml.EndElement(NULL, "specVersion");

	xml.StartElement(NULL, "device");

	xml.StartElement(NULL, "deviceType");
	xml.Text(m_deviceType);
	xml.EndElement(NULL, "deviceType");

	xml.StartElement(NULL, "UDN");
	xml.Text(NPT_String::Format("uuid:%s", uuid().toString().GetChars()));
	xml.EndElement(NULL, "UDN");

	NPT_String friendlyName;
	NPT_String modelName;
	NPT_String modelNumber;

	if (reqCtx.clientHint == FrontEnd::CH_XBox) {
		friendlyName = NPT_String::Format("%s: 1", m_friendlyName.GetChars());
		modelName = NPT_String::Format("Windows Media Connect compatible (%s)", m_friendlyName.GetChars());
		modelNumber = "1";
	} else {
		friendlyName = m_friendlyName;
		modelName = m_modelName;
		modelNumber = m_modelNumber;
	}

	{
		xml.StartElement(NULL, "friendlyName");
		xml.Text(friendlyName);
		xml.EndElement(NULL, "friendlyName");

		xml.StartElement(NULL, "manufacturer");
		xml.Text(m_manufacturer);
		xml.EndElement(NULL, "manufacturer");

		xml.StartElement(NULL, "manufacturerURL");
		xml.Text(m_manufacturerURL);
		xml.EndElement(NULL, "manufacturerURL");

		xml.StartElement(NULL, "modelDescription");
		xml.Text(m_modelDescription);
		xml.EndElement(NULL, "modelDescription");

		xml.StartElement(NULL, "modelName");
		xml.Text(modelName);
		xml.EndElement(NULL, "modelName");

		xml.StartElement(NULL, "modelNumber");
		xml.Text(modelNumber);
		xml.EndElement(NULL, "modelNumber");

		xml.StartElement(NULL, "modelURL");
		xml.Text(m_modelURL);
		xml.EndElement(NULL, "modelURL");

		xml.StartElement(NULL, "serialNumber");
		xml.Text(m_serialNumber);
		xml.EndElement(NULL, "serialNumber");

		xml.StartElement(NULL, "UPC");
		xml.Text(m_upc);
		xml.EndElement(NULL, "UPC");

		xml.StartElement(NULL, "presentationURL");
		xml.Text(m_presentationURL);
		xml.EndElement(NULL, "presentationURL");

	}

	outputMoreDeviceDescription(&xml);

	xml.StartElement(NULL, "serviceList");
	for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
		ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
		xml.StartElement(NULL, "service");

		xml.StartElement(NULL, "serviceType");
		xml.Text(serviceInfo->m_serviceDecl->serviceType);
		xml.EndElement(NULL, "serviceType");

		xml.StartElement(NULL, "serviceId");
		xml.Text(serviceInfo->m_serviceDecl->serviceId);
		xml.EndElement(NULL, "serviceId");

		xml.StartElement(NULL, "SCPDURL");
		xml.Text(NPT_String::Format("%s%s", fectx->m_httpRoot.GetChars(), serviceInfo->m_scpdPath.GetChars()));
		xml.EndElement(NULL, "SCPDURL");

		xml.StartElement(NULL, "controlURL");
		xml.Text(NPT_String::Format("%s%s", fectx->m_httpRoot.GetChars(), serviceInfo->m_controlPath.GetChars()));
		xml.EndElement(NULL, "controlURL");

		xml.StartElement(NULL, "eventSubURL");
		xml.Text(NPT_String::Format("%s%s", fectx->m_httpRoot.GetChars(), serviceInfo->m_eventSubPath.GetChars()));
		xml.EndElement(NULL, "eventSubURL");

		xml.EndElement(NULL, "service");
	}
	xml.EndElement(NULL, "serviceList");

	xml.EndElement(NULL, "device");

	xml.EndElement(NULL, "root");
	xml.EndDocument();
	return outputStream0.GetString();
}

void DeviceImpl::outputMoreDeviceDescription(NPT_XmlSerializer *xml)
{
}

bool DeviceImpl::addEventSub(ServiceInfo *serviceInfo, const NPT_List<NPT_String>& callbackList, int& timeout, UUID& sid)
{
	WriteLocker locker(m_stateLock);
	if (timeout < 300) {
		timeout = 300;
	}
	sid = UUID::generate();
	EventSubInfo *info = new EventSubInfo();
	info->m_callbackList = callbackList;
	info->m_timeout = timeout;
	info->m_sid = sid;
	info->m_eventKey = 0;
	serviceInfo->m_eventSubList.Add(info);
	serviceInfo->m_eventSubIndex.Put(sid, info);
	NPT_System::GetCurrentTimeStamp(info->m_updateTS);
	return true;
}

bool DeviceImpl::renewEventSub(ServiceInfo *serviceInfo, const UUID& sid, int& timeout)
{
	WriteLocker locker(m_stateLock);
	if (timeout < 300) {
		timeout = 300;
	}
	EventSubInfo **ppinfo;
	if (NPT_FAILED(serviceInfo->m_eventSubIndex.Get(sid, ppinfo))) {
		return false;
	}
	EventSubInfo *info = *ppinfo;
	info->m_timeout = timeout;
	NPT_System::GetCurrentTimeStamp(info->m_updateTS);
	return true;
}

bool DeviceImpl::removeEventSub(ServiceInfo *serviceInfo, const UUID& sid)
{
	WriteLocker locker(m_stateLock);
	NPT_List<EventSubInfo*>::Iterator it = serviceInfo->m_eventSubList.Find(EventSubInfoFinder(sid));
	if (!it) {
		return false;
	}

	EventSubInfo *info = *it;
	serviceInfo->m_eventSubIndex.Erase(sid);
	serviceInfo->m_eventSubList.Erase(it);
	delete info;
	return true;
}

void DeviceImpl::onInitialEventNotifyDelivered(const NPT_String& serviceId, const UUID& sid)
{
	WriteLocker locker(m_stateLock);
	NPT_List<ServiceInfo*>::Iterator it = m_serviceInfoList.Find(ServiceInfoByIdFinder(serviceId));
	if (it) {
		ServiceInfo *serviceInfo = *it;
		EventSubInfo **ppes;
		if (NPT_SUCCEEDED(serviceInfo->m_eventSubIndex.Get(sid, ppes))) {
			EventSubInfo *eventSub = *ppes;
			eventSub->m_eventKey = 1;
		}
	}
}

void DeviceImpl::removeExpiredEventSubLoop()
{
	for (;;) {
		if (NPT_SUCCEEDED(m_removeExpiredEventSubVar.WaitWhileEquals(0, 5000))) {
			break;
		}

		{
			NPT_TimeStamp ts;
			NPT_System::GetCurrentTimeStamp(ts);
			NPT_Int64 tsMillis = ts.ToMillis();
			WriteLocker locker(m_stateLock);
			for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
				ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
				NPT_Cardinal count = serviceInfo->m_eventSubList.GetItemCount();
				for (NPT_Ordinal j = 0; j < count; ) {
					NPT_List<EventSubInfo*>::Iterator it = serviceInfo->m_eventSubList.GetItem(j);
					EventSubInfo *eventSub = *it;

					bool removeIt = false;

					// 6000 ms more
					if (eventSub->m_updateTS.ToMillis() + (eventSub->m_timeout + 16) * 1000 < tsMillis) {
						removeIt = true;
					}

					if (removeIt) {
						serviceInfo->m_eventSubIndex.Erase(eventSub->m_sid);
						serviceInfo->m_eventSubList.Erase(it);
						delete eventSub;
						count--;
					} else {
						j++;
					}
				}
			}
		}
	}
}

void DeviceImpl::removeExpiredEventSubAbort()
{
	m_removeExpiredEventSubVar.SetValue(1);
}

NPT_String DeviceImpl::composeLastChangeAVT(const ServiceInfo *serviceInfo)
{
	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, true, true);
	xml.StartDocument();
	xml.StartElement(NULL, "Event");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/AVT/");
	xml.StartElement(NULL, "InstanceID");
	xml.Attribute(NULL, "val", "0");
	for (NPT_Ordinal i = 0; i < serviceInfo->m_stateVarList.GetItemCount(); i++) {
		StateVarInfo *varInfo = *serviceInfo->m_stateVarList.GetItem(i);
		if ((varInfo->m_varDecl->flags & SVF_GRP) != 0) {
			xml.StartElement(NULL, varInfo->m_varDecl->name);
			xml.Attribute(NULL, "val", varInfo->m_value);
			xml.EndElement(NULL, varInfo->m_varDecl->name);
		}
	}
	xml.EndElement(NULL, "InstanceID");
	xml.EndElement(NULL, "Event");
	xml.EndDocument();
	return strm.GetString();
}

NPT_String DeviceImpl::composeLastChangeRCS(const ServiceInfo *serviceInfo)
{
	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, true, true);
	xml.StartDocument();
	xml.StartElement(NULL, "Event");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/RCS/");
	xml.StartElement(NULL, "InstanceID");
	xml.Attribute(NULL, "val", "0");
	for (NPT_Ordinal i = 0; i < serviceInfo->m_stateVarList.GetItemCount(); i++) {
		StateVarInfo *varInfo = *serviceInfo->m_stateVarList.GetItem(i);
		if ((varInfo->m_varDecl->flags & SVF_GRP) != 0) {
			xml.StartElement(NULL, varInfo->m_varDecl->name);
			xml.Attribute(NULL, "val", varInfo->m_value);
			if ((varInfo->m_varDecl->flags & SVF_CHL) != 0) {
				xml.Attribute(NULL, "channel", "Master");
			}
			xml.EndElement(NULL, varInfo->m_varDecl->name);
		}
	}
	xml.EndElement(NULL, "InstanceID");
	xml.EndElement(NULL, "Event");
	xml.EndDocument();
	return strm.GetString();
}

static NPT_String composeLastChangeAVT_L(const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
{
	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, true, true);
	xml.StartDocument();
	xml.StartElement(NULL, "Event");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/AVT/");
	xml.StartElement(NULL, "InstanceID");
	xml.Attribute(NULL, "val", "0");
	for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
		const NPT_String& name = *nameList.GetItem(i);
		const NPT_String& value = *valueList.GetItem(i);
		xml.StartElement(NULL, name);
		xml.Attribute(NULL, "val", value);
		xml.EndElement(NULL, name);
	}
	xml.EndElement(NULL, "InstanceID");
	xml.EndElement(NULL, "Event");
	xml.EndDocument();
	return strm.GetString();
}

static NPT_String composeLastChangeRCS_L(const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
{
	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, true, true);
	xml.StartDocument();
	xml.StartElement(NULL, "Event");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/RCS/");
	xml.StartElement(NULL, "InstanceID");
	xml.Attribute(NULL, "val", "0");
	for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
		const NPT_String& name = *nameList.GetItem(i);
		const NPT_String& value = *valueList.GetItem(i);
		xml.StartElement(NULL, name);
		xml.Attribute(NULL, "val", value);
		// TODO:
		if (name.Compare("Mute") == 0 || name.Compare("Volume") == 0) {
			xml.Attribute(NULL, "channel", "Master");
		}
		xml.EndElement(NULL, name);
	}
	xml.EndElement(NULL, "InstanceID");
	xml.EndElement(NULL, "Event");
	xml.EndDocument();
	return strm.GetString();
}

struct MonitorStateVarsInfo
{
	bool m_bAVT;
	bool m_bRCS;
	NPT_List<NPT_String> m_nameList;
	NPT_List<NPT_String> m_valueList;
	NPT_List<NPT_String> m_lastChangedNameList;
	NPT_List<NPT_String> m_lastChangedValueList;
};

void DeviceImpl::monitorStateVarsLoop()
{
	for (;;) {
		if (NPT_SUCCEEDED(m_monitorVar.WaitWhileEquals(0, 500))) {
			break;
		}

		NPT_List<MonitorStateVarsInfo*> monitorList;

		{
			ReadLocker locker1(m_stateLock);
			for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
				ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
				MonitorStateVarsInfo *mi = new MonitorStateVarsInfo();
				monitorList.Add(mi);
				mi->m_bAVT = NPT_String::Compare(serviceInfo->m_serviceDecl->serviceId, "urn:upnp-org:serviceId:AVTransport") == 0;
				mi->m_bRCS = NPT_String::Compare(serviceInfo->m_serviceDecl->serviceId, "urn:upnp-org:serviceId:RenderingControl") == 0;

				for (NPT_Ordinal j = 0; j < serviceInfo->m_stateVarList.GetItemCount(); j++) {
					StateVarInfo *varInfo = *serviceInfo->m_stateVarList.GetItem(j);
					if (varInfo->m_cachedValue != varInfo->m_value) {
						// IMPORTANT!!!!
						// We are modifying m_cachedValue, while hold m_stateLock for readonly (using ReadLocker instead of WriteLocker)
						// Since this is the only line to modify m_cachedValue, using ReadLocker is safe!
						varInfo->m_cachedValue = varInfo->m_value;

						if ((mi->m_bAVT || mi->m_bRCS) && ((varInfo->m_varDecl->flags & SVF_GRP) != 0)) {
							mi->m_lastChangedNameList.Add(varInfo->m_varDecl->name);
							mi->m_lastChangedValueList.Add(varInfo->m_value);
						} else {
							mi->m_nameList.Add(varInfo->m_varDecl->name);
							mi->m_valueList.Add(varInfo->m_value);
						}
					}
				}
			}
		}

		for (NPT_Ordinal i = 0; i < monitorList.GetItemCount(); i++) {
			MonitorStateVarsInfo *mi = *monitorList.GetItem(i);
			if (mi->m_lastChangedNameList.GetItemCount() > 0) {
				if (mi->m_bAVT) {
					mi->m_nameList.Add("LastChange");
					mi->m_valueList.Add(composeLastChangeAVT_L(mi->m_lastChangedNameList, mi->m_lastChangedValueList));
				} else if (mi->m_bRCS) {
					mi->m_nameList.Add("LastChange");
					mi->m_valueList.Add(composeLastChangeRCS_L(mi->m_lastChangedNameList, mi->m_lastChangedValueList));
				}
			}
		}

		{
			ReadLocker locker2(m_stateLock);
			for (NPT_Ordinal i = 0; i < m_serviceInfoList.GetItemCount(); i++) {
				ServiceInfo *serviceInfo = *m_serviceInfoList.GetItem(i);
				MonitorStateVarsInfo *mi = *monitorList.GetItem(i);
				if (mi->m_nameList.GetItemCount() > 0) {
					for (NPT_Ordinal j = 0; j < serviceInfo->m_eventSubList.GetItemCount(); j++) {
						EventSubInfo *eventSub = *serviceInfo->m_eventSubList.GetItem(j);
						if (eventSub->m_eventKey != 0) {
							if (eventSub->m_eventKey == 0xFFFFFFFF) {
								eventSub->m_eventKey = 1;
							} else {
								++eventSub->m_eventKey;
							}
							m_attachedTaskGroup->startTask(new DeliverEventNotifyTask(this, eventSub->m_callbackList, eventSub->m_eventKey, serviceInfo->m_serviceDecl->serviceId, eventSub->m_sid, mi->m_nameList, mi->m_valueList));
						}
					}
				}
			}
		}

		monitorList.Apply(NPT_ObjectDeleter<MonitorStateVarsInfo>());
	}
}

void DeviceImpl::monitorStateVarsAbort()
{
	m_monitorVar.SetValue(1);
}

DeliverEventNotifyTask::DeliverEventNotifyTask(DeviceImpl *deviceImpl, const NPT_List<NPT_String>& callbackList, NPT_UInt32 eventKey, const NPT_String& serviceId, const UUID& sid, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
	: m_deviceImpl(deviceImpl), m_callbackList(callbackList), m_eventKey(eventKey), m_serviceId(serviceId), m_sid(sid), m_nameList(nameList), m_valueList(valueList)
{
}

DeliverEventNotifyTask::~DeliverEventNotifyTask()
{
}

void DeliverEventNotifyTask::exec()
{
	NPT_StringOutputStream outputStream;
	NPT_XmlSerializer xml(&outputStream, 0, true, true);
	xml.StartDocument();
	xml.StartElement("e", "propertyset");
	xml.Attribute("xmlns", "e", "urn:schemas-upnp-org:event-1-0");

	for (NPT_Ordinal i = 0; i < m_nameList.GetItemCount(); i++) {
		NPT_List<NPT_String>::Iterator it = m_nameList.GetItem(i);
		xml.StartElement("e", "property");
		xml.StartElement(NULL, *it);
		xml.Text(*m_valueList.GetItem(i));
		xml.EndElement(NULL, *it);
		xml.EndElement("e", "property");
	}

	xml.EndElement("e", "propertyset");
	xml.EndDocument();

	bool delivered = false;
	for (NPT_Ordinal i = 0; i < m_callbackList.GetItemCount(); i++) {
		NPT_String url = *m_callbackList.GetItem(i);
		if (deliver(url, outputStream.GetString())) {
			delivered = true;
			break;
		}
	}

	if (delivered && m_deviceImpl) {
		m_deviceImpl->onInitialEventNotifyDelivered(m_serviceId, m_sid);
	}
}

void DeliverEventNotifyTask::doAbort()
{
	m_httpClient.Abort();
}

bool DeliverEventNotifyTask::deliver(const NPT_String& url, const NPT_String& content)
{
	NPT_HttpRequest req(url, "NOTIFY", NPT_HTTP_PROTOCOL_1_1);
	Helper::setupHttpRequest(req);
	req.GetHeaders().SetHeader("NT", "upnp:event");
	req.GetHeaders().SetHeader("NTS", "upnp:propchange");
	req.GetHeaders().SetHeader("SID", NPT_String::Format("uuid:%s", m_sid.toString().GetChars()));
	req.GetHeaders().SetHeader("SEQ", NPT_String::FromInteger(m_eventKey));
	req.SetEntity(new NPT_HttpEntity());
	req.GetEntity()->SetInputStream(content);
	req.GetEntity()->SetContentType("text/xml");

	NPT_HttpResponse *resp;
	NPT_Result nr = m_httpClient.SendRequest(req, resp);
	if (NPT_SUCCEEDED(nr)) {
		PtrHolder<NPT_HttpResponse> resp1(resp);
		return true;
	}

	return false;
}

} // namespace deejay
