#include "avcore1.h"

extern unsigned int g_icon120_png_size;
extern unsigned char g_icon120_png[];

extern unsigned int g_icon48_png_size;
extern unsigned char g_icon48_png[];

extern unsigned int g_icon32_png_size;
extern unsigned char g_icon32_png[];

extern unsigned int g_icon120_jpg_size;
extern unsigned char g_icon120_jpg[];

extern unsigned int g_icon48_jpg_size;
extern unsigned char g_icon48_jpg[];

extern unsigned int g_icon32_jpg_size;
extern unsigned char g_icon32_jpg[];

namespace deejay {

extern const ServiceDecl *g_DMSServiceTable;
extern int g_DMSServiceTableSize;

SimpleDMS::SimpleDMS(const UUID& uuid)
{
	m_uuid = uuid;
	m_deviceType = "urn:schemas-upnp-org:device:MediaServer:1";
	m_friendlyName = "SimpleDMS[WTF]";
	m_manufacturer = "NETGEAR Inc.";
	m_manufacturerURL = "http://www.netgear.com";
	m_modelDescription = "GENIE SimpleDMS";
	m_modelName = "SimpleDMS";
	m_modelNumber = "DJ-DMS";
	m_modelURL = "http://www.netgear.com";
	m_serialNumber = "260471";
	m_upc = "400240112013";
	m_presentationURL = "http://www.netgear.com";

/*	m_modelName = "Windows Media Player Sharing";
	m_modelNumber = "12.0";
	m_friendlyName = "SimpleDMS[WTF] : 2";*/
/*	m_modelURL = "http://go.microsoft.com/fwlink/?LinkId=105926";
	m_manufacturer = "Microsoft Corporation";
	m_manufacturerURL = "http://www.microsoft.com";*/

//	NPT_String friendlyName = "SimpleDMS248";

//	m_friendlyName = NPT_String::Format("%s: 1", friendlyName.GetChars());
//	m_modelName = NPT_String::Format("Windows Media Connect compatible (%s)", m_friendlyName.GetChars());
//	m_modelNumber = "1";
	//m_modelURL = "http://www.netgear.com/";
	//m_manufacturer = "NETGEAR32";
	//m_manufacturerURL = "http://www.siteview.com/";
	//m_serialNumber = "12345678";
	//m_modelDescription = friendlyName;

	registerServices(g_DMSServiceTable, g_DMSServiceTableSize);
	registerStaticContent("icon1", "image/png", NPT_DataBuffer(g_icon120_png, g_icon120_png_size, false), false);
	registerStaticContent("icon2", "image/png", NPT_DataBuffer(g_icon48_png, g_icon48_png_size, false), false);
	registerStaticContent("icon3", "image/png", NPT_DataBuffer(g_icon32_png, g_icon32_png_size, false), false);
	registerStaticContent("icon4", "image/jpeg", NPT_DataBuffer(g_icon120_jpg, g_icon120_jpg_size, false), false);
	registerStaticContent("icon5", "image/jpeg", NPT_DataBuffer(g_icon48_jpg, g_icon48_jpg_size, false), false);
	registerStaticContent("icon6", "image/jpeg", NPT_DataBuffer(g_icon32_jpg, g_icon32_jpg_size, false), false);

	const char *cds = "urn:upnp-org:serviceId:ContentDirectory";
	setStateValue(cds, "SearchCapabilities", "*");
	setStateValue(cds, "SortCapabilities", "*");
	setStateValue(cds, "SystemUpdateID", "0");
	setStateValue(cds, "ContainerUpdateIDs", "");

	const char *cms = "urn:upnp-org:serviceId:ConnectionManager";
	setStateValue(cms, "SourceProtocolInfo", "");
	setStateValue(cms, "SinkProtocolInfo", "");
	setStateValue(cms, "CurrentConnectionIDs", "0");

	m_store = new MediaStore(this);
}

SimpleDMS::~SimpleDMS()
{
	delete m_store;
}

MediaStore *SimpleDMS::mediaStore() const
{
	return m_store;
}

void SimpleDMS::setUuid(const UUID& uuid)
{
	m_uuid = uuid;
}

const UUID& SimpleDMS::uuid() const
{
	return m_uuid;
}

void SimpleDMS::setFriendlyName(const NPT_String& friendlyName)
{
	m_friendlyName = NPT_String::Format("Genie Media Server (%s)", friendlyName.GetChars());
}

void SimpleDMS::setPlatformName(const NPT_String& platformName)
{
	m_modelDescription = NPT_String::Format("%s Genie DMS", platformName.GetChars());
}

void SimpleDMS::outputMoreDeviceDescription(NPT_XmlSerializer *xml)
{
	struct IconDataInfo
	{
		const char *mimeType;
		int width;
		int height;
		int depth;
	};

	static const IconDataInfo iconData[] = {
		{ "image/png", 120, 120, 24 },
		{ "image/png", 48, 48, 24 },
		{ "image/png", 32, 32, 24 },
		{ "image/jpeg", 120, 120, 24 },
		{ "image/jpeg", 48, 48, 24 },
		{ "image/jpeg", 32, 32, 24 },
	};
/*
	xml->StartElement("dlna", "X_DLNADOC");
	xml->Attribute("xmlns", "dlna", "urn:schemas-dlna-org:device-1-0");
	xml->Text("DMS-1.50");
	xml->EndElement("dlna", "X_DLNADOC");
*/
	xml->StartElement(NULL, "iconList");
	for (size_t i = 0; i < sizeof(iconData) / sizeof(iconData[0]); i++) {
		const IconDataInfo *info = iconData + i;
		xml->StartElement(NULL, "icon");

		xml->StartElement(NULL, "mimetype");
		xml->Text(info->mimeType);
		xml->EndElement(NULL, "mimetype");

		xml->StartElement(NULL, "width");
		xml->Text(NPT_String::FromInteger(info->width));
		xml->EndElement(NULL, "width");

		xml->StartElement(NULL, "height");
		xml->Text(NPT_String::FromInteger(info->height));
		xml->EndElement(NULL, "height");

		xml->StartElement(NULL, "depth");
		xml->Text(NPT_String::FromInteger(info->depth));
		xml->EndElement(NULL, "depth");

		xml->StartElement(NULL, "url");
		xml->Text(NPT_String::Format("icon%d", i+1));
		xml->EndElement(NULL, "url");

		xml->EndElement(NULL, "icon");
	}
	xml->EndElement(NULL, "iconList");
}

int SimpleDMS::onAction(const ServiceDecl *serviceDecl, const ActionDecl *actionDecl, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, NPT_List<NPT_String>& outputArgValues)
{
	if (NPT_String::Compare(serviceDecl->serviceId, "urn:upnp-org:serviceId:ContentDirectory") == 0) {

		if (NPT_String::Compare(actionDecl->name, "Browse") == 0) {
			NPT_UInt32 startingIndex;
			NPT_UInt32 requestedCount;
			NPT_ParseInteger(*inputArgValues.GetItem(3), startingIndex);
			NPT_ParseInteger(*inputArgValues.GetItem(4), requestedCount);
			NPT_String result;
			NPT_UInt32 numberReturned;
			NPT_UInt32 totalMatches;
			NPT_UInt32 updateID;
			int errorCode = m_store->browse(task, ifctx, reqCtx, NPT_String::Format("http://%s:%d%sdms/%%s", ifctx->m_ifAddr.ToString().GetChars(), ifctx->m_httpPort, frontEndContext()->m_httpRoot.GetChars()), *inputArgValues.GetItem(0), *inputArgValues.GetItem(1), *inputArgValues.GetItem(2), startingIndex, requestedCount, *inputArgValues.GetItem(5), result, numberReturned, totalMatches, updateID);
			if (errorCode == 0) {
				*outputArgValues.GetItem(0) = result;
				*outputArgValues.GetItem(1) = NPT_String::FromIntegerU(numberReturned);
				*outputArgValues.GetItem(2) = NPT_String::FromIntegerU(totalMatches);
				*outputArgValues.GetItem(3) = NPT_String::FromIntegerU(updateID);
			}
			return errorCode;
		}

		if (NPT_String::Compare(actionDecl->name, "Search") == 0) {
			NPT_UInt32 startingIndex;
			NPT_UInt32 requestedCount;
			NPT_ParseInteger(*inputArgValues.GetItem(3), startingIndex);
			NPT_ParseInteger(*inputArgValues.GetItem(4), requestedCount);
			NPT_String result;
			NPT_UInt32 numberReturned;
			NPT_UInt32 totalMatches;
			NPT_UInt32 updateID;
			int errorCode = m_store->search(task, ifctx, reqCtx, NPT_String::Format("http://%s:%d%sdms/%%s", ifctx->m_ifAddr.ToString().GetChars(), ifctx->m_httpPort, frontEndContext()->m_httpRoot.GetChars()), *inputArgValues.GetItem(0), *inputArgValues.GetItem(1), *inputArgValues.GetItem(2), startingIndex, requestedCount, *inputArgValues.GetItem(5), result, numberReturned, totalMatches, updateID);
			if (errorCode == 0) {
				*outputArgValues.GetItem(0) = result;
				*outputArgValues.GetItem(1) = NPT_String::FromIntegerU(numberReturned);
				*outputArgValues.GetItem(2) = NPT_String::FromIntegerU(totalMatches);
				*outputArgValues.GetItem(3) = NPT_String::FromIntegerU(updateID);
			}
			return errorCode;
		}

		if (NPT_String::Compare(actionDecl->name, "GetSearchCapabilities") == 0) {
			NPT_String v;
			if (getStateValue(serviceDecl->serviceId, "SearchCapabilities", v)) {
				*outputArgValues.GetItem(0) = v;
			}
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetSortCapabilities") == 0) {
			NPT_String v;
			if (getStateValue(serviceDecl->serviceId, "SortCapabilities", v)) {
				*outputArgValues.GetItem(0) = v;
			}
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetSystemUpdateID") == 0) {
			NPT_String v;
			if (getStateValue(serviceDecl->serviceId, "SystemUpdateID", v)) {
				*outputArgValues.GetItem(0) = v;
			}
			return 0;
		}

		return 602;
	}

	if (NPT_String::Compare(serviceDecl->serviceId, "urn:upnp-org:serviceId:ConnectionManager") == 0) {

		if (NPT_String::Compare(actionDecl->name, "GetProtocolInfo") == 0) {
			NPT_String v;
			if (getStateValue(serviceDecl->serviceId, "SourceProtocolInfo", v)) {
				*outputArgValues.GetItem(0) = v;
			}

			if (getStateValue(serviceDecl->serviceId, "SinkProtocolInfo", v)) {
				*outputArgValues.GetItem(1) = v;
			}
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetCurrentConnectionIDs") == 0) {
			NPT_String v;
			if (getStateValue(serviceDecl->serviceId, "SourceProtocolInfo", v)) {
				*outputArgValues.GetItem(0) = v;
			}
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetCurrentConnectionInfo") == 0) {
			*outputArgValues.GetItem(0) = "0";
			*outputArgValues.GetItem(1) = "0";
			*outputArgValues.GetItem(2) = "";
			*outputArgValues.GetItem(3) = "";
			*outputArgValues.GetItem(4) = "-1";
			*outputArgValues.GetItem(5) = "Input"; // or "Output"? WTF!
			*outputArgValues.GetItem(6) = "OK";
			return 0;
		}

		return 602;
	}

	if (NPT_String::Compare(serviceDecl->serviceId, "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar") == 0) {
		if (NPT_String::Compare(actionDecl->name, "IsAuthorized") == 0) {
			*outputArgValues.GetItem(0) = "1";
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "IsValidated") == 0) {
			*outputArgValues.GetItem(0) = "1";
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "RegisterDevice") == 0) {
			return 0;
		}
		return 602;
	}

	return 501;
}

void serveIOSAsset(AbortableTask *task, const MediaStore::FileDetail& detail, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput);
void serveFile(AbortableTask *task, const NPT_String& filePath, const NPT_String& mimeType, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput);

#ifndef DLNACORE_PLATFORM_IOS

void serveIOSAsset(AbortableTask *task, const MediaStore::FileDetail& detail, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput)
{
	setStatusCode(resp, 404);
	httpOutput->writeResponseHeader(resp);
}

#endif // DLNACORE_PLATFORM_IOS

bool SimpleDMS::onHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput)
{
	MediaStore::FileDetail detail;
	bool found = false;
	if (relPath.StartsWith("dms/")) {
		found = m_store->findFileDetail(relPath.SubString(4), detail);
	}

	if (found) {
		if (detail.m_type == MediaStore::FileDetail::ALAsset) {
			serveIOSAsset(task, detail, reqCtx, req, resp, httpOutput);
		} else {
			serveFile(task, detail.m_path, detail.m_mimeType, reqCtx, req, resp, httpOutput);
		}
	} else {
		setStatusCode(resp, 404);
		httpOutput->writeResponseHeader(resp);
	}
	return true;
}

void SimpleDMS::onMediaStoreUpdated(MediaStore *mediaStore)
{
	setStateValue("urn:upnp-org:serviceId:ContentDirectory", "SystemUpdateID", NPT_String::FromIntegerU(mediaStore->systemUpdateId()));
}

} // namespace deejay
