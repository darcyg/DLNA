#include "avcore2.h"

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

extern const ServiceDecl *g_DMRServiceTable;
extern int g_DMRServiceTableSize;

SimpleDMR::SimpleDMR(Callback *callback, const UUID& uuid)
	: m_callback(callback)
{
	m_uuid = uuid;
	m_deviceType = "urn:schemas-upnp-org:device:MediaRenderer:1";
	m_friendlyName = "SimpleDMR[WTF]";
	m_manufacturer = "NETGEAR Inc.";
	m_manufacturerURL = "http://www.netgear.com";
	m_modelDescription = "GENIE SimpleDMR";
	m_modelName = "SimpleDMR";
	m_modelNumber = "DJ-DMR";
	m_modelURL = "http://www.netgear.com";
	m_serialNumber = "262471";
	m_upc = "400240112011";
	m_presentationURL = "http://www.netgear.com";

	registerServices(g_DMRServiceTable, g_DMRServiceTableSize);
	registerStaticContent("icon1", "image/png", NPT_DataBuffer(g_icon120_png, g_icon120_png_size, false), false);
	registerStaticContent("icon2", "image/png", NPT_DataBuffer(g_icon48_png, g_icon48_png_size, false), false);
	registerStaticContent("icon3", "image/png", NPT_DataBuffer(g_icon32_png, g_icon32_png_size, false), false);
	registerStaticContent("icon4", "image/jpeg", NPT_DataBuffer(g_icon120_jpg, g_icon120_jpg_size, false), false);
	registerStaticContent("icon5", "image/jpeg", NPT_DataBuffer(g_icon48_jpg, g_icon48_jpg_size, false), false);
	registerStaticContent("icon6", "image/jpeg", NPT_DataBuffer(g_icon32_jpg, g_icon32_jpg_size, false), false);
/*
	const char *sinkProtocolInfo =
		"http-get:*:image/png:*,"
		"http-get:*:image/jpeg:*,"
		"http-get:*:image/bmp:*,"
		"http-get:*:image/gif:*,"
		"http-get:*:audio/mpeg:*,"
		"http-get:*:audio/3gpp:*,"
		"http-get:*:audio/mp4:*,"
		"http-get:*:audio/x-ms-wma:*,"
		"http-get:*:audio/wav:*,"
		"http-get:*:video/mp4:*,"
		"http-get:*:video/mpeg:*,"
		"http-get:*:video/x-ms-wmv:*,"
		"http-get:*:video/x-ms-asf:*,"
		"http-get:*:video/3gpp:*,"
		"http-get:*:video/avi:*,"
		"http-get:*:video/quicktime:*"
		;
*/
	const char *cms = "urn:upnp-org:serviceId:ConnectionManager";
	setStateValue(cms, "SourceProtocolInfo", "");
//	setStateValue(cms, "SinkProtocolInfo", sinkProtocolInfo);
	setStateValue(cms, "CurrentConnectionIDs", "0");
/*
	const char *avt = "urn:upnp-org:serviceId:AVTransport";
	setStateValue(avt, "TransportState", "NO_MEDIA_PRESENT");
	setStateValue(avt, "TransportStatus", "OK");
	setStateValue(avt, "PlaybackStorageMedium", "NONE");
	setStateValue(avt, "RecordStorageMedium", "NOT_IMPLEMENTED");
	setStateValue(avt, "PossiblePlaybackStorageMedia", "NONE,NETWORK,UNKNOWN,HDD");
	setStateValue(avt, "PossibleRecordStorageMedia", "NOT_IMPLEMENTED");
	setStateValue(avt, "CurrentPlayMode", "NORMAL");
	setStateValue(avt, "TransportPlaySpeed", "1");
	setStateValue(avt, "RecordMediumWriteStatus", "NOT_IMPLEMENTED");
	setStateValue(avt, "CurrentRecordQualityMode", "NOT_IMPLEMENTED");
	setStateValue(avt, "PossibleRecordQualityModes", "NOT_IMPLEMENTED");
	setStateValue(avt, "NumberOfTracks", "0");
	setStateValue(avt, "CurrentTrack", "0");
	setStateValue(avt, "CurrentTrackDuration", "00:00:00");
	setStateValue(avt, "CurrentMediaDuration", "00:00:00");
	setStateValue(avt, "CurrentTrackMetaData", "");
	setStateValue(avt, "CurrentTrackURI", "");
	setStateValue(avt, "AVTransportURI", "");
	setStateValue(avt, "AVTransportURIMetaData", "");
	setStateValue(avt, "NextAVTransportURI", "NOT_IMPLEMENTED");
	setStateValue(avt, "NextAVTransportURIMetaData", "NOT_IMPLEMENTED");
	setStateValue(avt, "RelativeTimePosition", "00:00:00");
	setStateValue(avt, "AbsoluteTimePosition", "NOT_IMPLEMENTED");
	setStateValue(avt, "RelativeCounterPosition", "-1");
	setStateValue(avt, "AbsoluteCounterPosition", "-1");
	setStateValue(avt, "CurrentTransportActions", "");

	const char *rcs = "urn:upnp-org:serviceId:RenderingControl";
	setStateValue(rcs, "PresetNameList", "FactoryDefaults");
	setStateValue(rcs, "Mute", "0");
	setStateValue(rcs, "Volume", "100");
*/
	setVar(DMRVAR_TransportState, "NO_MEDIA_PRESENT");
	setVar(DMRVAR_TransportStatus, "OK");
	setVar(DMRVAR_PlaybackStorageMedium, "NONE");
	setVar(DMRVAR_RecordStorageMedium, "NOT_IMPLEMENTED");
	setVar(DMRVAR_PossiblePlaybackStorageMedia, "NONE,NETWORK,UNKNOWN,HDD");
	setVar(DMRVAR_PossibleRecordStorageMedia, "NOT_IMPLEMENTED");
	setVar(DMRVAR_CurrentPlayMode, "NORMAL");
	setVar(DMRVAR_TransportPlaySpeed, "1");
	setVar(DMRVAR_RecordMediumWriteStatus, "NOT_IMPLEMENTED");
	setVar(DMRVAR_CurrentRecordQualityMode, "NOT_IMPLEMENTED");
	setVar(DMRVAR_PossibleRecordQualityModes, "NOT_IMPLEMENTED");
	setVar(DMRVAR_NumberOfTracks, "0");
	setVar(DMRVAR_CurrentTrack, "0");
	setVar(DMRVAR_CurrentTrackDuration, "00:00:00");
	setVar(DMRVAR_CurrentMediaDuration, "00:00:00");
	setVar(DMRVAR_CurrentTrackMetaData, "");
	setVar(DMRVAR_CurrentTrackURI, "");
	setVar(DMRVAR_AVTransportURI, "");
	setVar(DMRVAR_AVTransportURIMetaData, "");
	setVar(DMRVAR_NextAVTransportURI, "NOT_IMPLEMENTED");
	setVar(DMRVAR_NextAVTransportURIMetaData, "NOT_IMPLEMENTED");
	setVar(DMRVAR_RelativeTimePosition, "00:00:00");
	setVar(DMRVAR_AbsoluteTimePosition, "NOT_IMPLEMENTED");
	setVar(DMRVAR_RelativeCounterPosition, "-1");
	setVar(DMRVAR_AbsoluteCounterPosition, "-1");
	setVar(DMRVAR_CurrentTransportActions, "");

	setVar(DMRVAR_PresetNameList, "FactoryDefaults");
	setVar(DMRVAR_Mute, "0");
	setVar(DMRVAR_Volume, "100");
}

SimpleDMR::~SimpleDMR()
{
}

void SimpleDMR::setUuid(const UUID& uuid)
{
	m_uuid = uuid;
}

const UUID& SimpleDMR::uuid() const
{
	return m_uuid;
}

void SimpleDMR::setFriendlyName(const NPT_String& friendlyName)
{
	m_friendlyName = NPT_String::Format("Genie Media Player (%s)", friendlyName.GetChars());
}

void SimpleDMR::setPlatformName(const NPT_String& platformName)
{
	m_modelDescription = NPT_String::Format("%s Genie DMR", platformName.GetChars());
}

void SimpleDMR::setSinkProtocolInfo(const NPT_String& value)
{
	setStateValue("urn:upnp-org:serviceId:ConnectionManager", "SinkProtocolInfo", value);
}

void SimpleDMR::setVarAVT(const NPT_String& name, const NPT_String& value)
{
	setStateValue("urn:upnp-org:serviceId:AVTransport", name, value);
}

void SimpleDMR::setVarRCS(const NPT_String& name, const NPT_String& value)
{
	setStateValue("urn:upnp-org:serviceId:RenderingControl", name, value);
}

void SimpleDMR::setVar(DMRVAR varIndex, const NPT_String& value)
{
	if (varIndex >= DMRVAR_BaseIndexAVT && varIndex <= DMRVAR_LastIndexAVT) {
		const char *varName;
		m_avtVars[varIndex - DMRVAR_BaseIndexAVT] = value;
		switch (varIndex) {
		case DMRVAR_TransportState:
			varName = "TransportState";
			break;
		case DMRVAR_TransportStatus:
			varName = "TransportStatus";
			break;
		case DMRVAR_PlaybackStorageMedium:
			varName = "PlaybackStorageMedium";
			break;
		case DMRVAR_RecordStorageMedium:
			varName = "RecordStorageMedium";
			break;
		case DMRVAR_PossiblePlaybackStorageMedia:
			varName = "PossiblePlaybackStorageMedia";
			break;
		case DMRVAR_PossibleRecordStorageMedia:
			varName = "PossibleRecordStorageMedia";
			break;
		case DMRVAR_CurrentPlayMode:
			varName = "CurrentPlayMode";
			break;
		case DMRVAR_TransportPlaySpeed:
			varName = "TransportPlaySpeed";
			break;
		case DMRVAR_RecordMediumWriteStatus:
			varName = "RecordMediumWriteStatus";
			break;
		case DMRVAR_CurrentRecordQualityMode:
			varName = "CurrentRecordQualityMode";
			break;
		case DMRVAR_PossibleRecordQualityModes:
			varName = "PossibleRecordQualityModes";
			break;
		case DMRVAR_NumberOfTracks:
			varName = "NumberOfTracks";
			break;
		case DMRVAR_CurrentTrack:
			varName = "CurrentTrack";
			break;
		case DMRVAR_CurrentTrackDuration:
			varName = "CurrentTrackDuration";
			break;
		case DMRVAR_CurrentMediaDuration:
			varName = "CurrentMediaDuration";
			break;
		case DMRVAR_CurrentTrackMetaData:
			varName = "CurrentTrackMetaData";
			break;
		case DMRVAR_CurrentTrackURI:
			varName = "CurrentTrackURI";
			break;
		case DMRVAR_AVTransportURI:
			varName = "AVTransportURI";
			break;
		case DMRVAR_AVTransportURIMetaData:
			varName = "AVTransportURIMetaData";
			break;
		case DMRVAR_NextAVTransportURI:
			varName = "NextAVTransportURI";
			break;
		case DMRVAR_NextAVTransportURIMetaData:
			varName = "NextAVTransportURIMetaData";
			break;
		case DMRVAR_RelativeTimePosition:
			varName = "RelativeTimePosition";
			break;
		case DMRVAR_AbsoluteTimePosition:
			varName = "AbsoluteTimePosition";
			break;
		case DMRVAR_RelativeCounterPosition:
			varName = "RelativeCounterPosition";
			break;
		case DMRVAR_AbsoluteCounterPosition:
			varName = "AbsoluteCounterPosition";
			break;
		case DMRVAR_CurrentTransportActions:
		default:
			varName = "CurrentTransportActions";
			break;
		}
		setStateValue("urn:upnp-org:serviceId:AVTransport", varName, value);
	} else if (varIndex >= DMRVAR_BaseIndexRCS && varIndex <= DMRVAR_LastIndexRCS) {
		const char *varName;
		m_rcsVars[varIndex - DMRVAR_BaseIndexRCS] = value;
		switch (varIndex) {
		case DMRVAR_PresetNameList:
			varName = "PresetNameList";
			break;
		case DMRVAR_Mute:
			varName = "Mute";
			break;
		case DMRVAR_Volume:
			varName = "Volume";
			break;
		}
		setStateValue("urn:upnp-org:serviceId:RenderingControl", varName, value);
	}
}

void SimpleDMR::outputMoreDeviceDescription(NPT_XmlSerializer *xml)
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

class HttpClientAbortCallback
	: public AbortableTask::Callback
{
public:
	HttpClientAbortCallback(NPT_HttpClient *httpClient)
		: m_httpClient(httpClient)
	{
	}

	virtual void onAborted(AbortableTask *task)
	{
		m_httpClient->Abort();
	}

private:
	NPT_HttpClient *m_httpClient;
};

int SimpleDMR::onAction(const ServiceDecl *serviceDecl, const ActionDecl *actionDecl, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, NPT_List<NPT_String>& outputArgValues)
{
	WriteLocker locker(m_stateLock);
	if (NPT_String::Compare(serviceDecl->serviceId, "urn:upnp-org:serviceId:AVTransport") == 0) {

		if (NPT_String::Compare(actionDecl->name, "SetAVTransportURI") == 0) {
			if (inputArgValues.GetItemCount() != 3) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			NPT_HttpClient httpClient;
			NPT_HttpRequest req(*inputArgValues.GetItem(1), NPT_HTTP_METHOD_GET, NPT_HTTP_PROTOCOL_1_1);
			Helper::setupHttpRequest(req);
			NPT_HttpResponse *resp;
			NPT_Result nr;
			HttpClientAbortCallback abortCallback(&httpClient);
			if (task->registerAbortCallback(&abortCallback)) {
				nr = httpClient.SendRequest(req, resp);
				task->unregisterAbortCallback(&abortCallback);
			} else {
				return 715;
			}

			if (NPT_FAILED(nr)) {
				return 716;
			}

			PtrHolder<NPT_HttpResponse> resp1(resp);
			if (resp->GetStatusCode() != 200) {
				return 716;
			}

			NPT_HttpHeader *hdrContentType = resp->GetHeaders().GetHeader(NPT_HTTP_HEADER_CONTENT_TYPE);
			if (!hdrContentType) {
				return 714;
			}

			setVar(DMRVAR_AVTransportURI, *inputArgValues.GetItem(1));
			setVar(DMRVAR_AVTransportURIMetaData, *inputArgValues.GetItem(2));
			setVar(DMRVAR_CurrentTrackMetaData, *inputArgValues.GetItem(2));
			m_callback->doDmrOpen(this, *inputArgValues.GetItem(1), hdrContentType->GetValue(), *inputArgValues.GetItem(2));
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetMediaInfo") == 0) {
			if (inputArgValues.GetItemCount() != 1) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			*outputArgValues.GetItem(0) = m_avtVars[DMRVAR_NumberOfTracks - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(1) = m_avtVars[DMRVAR_CurrentMediaDuration - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(2) = m_avtVars[DMRVAR_AVTransportURI - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(3) = m_avtVars[DMRVAR_AVTransportURIMetaData - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(4) = m_avtVars[DMRVAR_NextAVTransportURI - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(5) = m_avtVars[DMRVAR_NextAVTransportURIMetaData - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(6) = m_avtVars[DMRVAR_PlaybackStorageMedium - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(7) = m_avtVars[DMRVAR_RecordStorageMedium - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(8) = m_avtVars[DMRVAR_RecordMediumWriteStatus - DMRVAR_BaseIndexAVT];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetTransportInfo") == 0) {
			if (inputArgValues.GetItemCount() != 1) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			*outputArgValues.GetItem(0) = m_avtVars[DMRVAR_TransportState - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(1) = m_avtVars[DMRVAR_TransportStatus - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(2) = m_avtVars[DMRVAR_TransportPlaySpeed - DMRVAR_BaseIndexAVT];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetPositionInfo") == 0) {
			if (inputArgValues.GetItemCount() != 1) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			*outputArgValues.GetItem(0) = m_avtVars[DMRVAR_CurrentTrack - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(1) = m_avtVars[DMRVAR_CurrentTrackDuration - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(2) = m_avtVars[DMRVAR_CurrentTrackMetaData - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(3) = m_avtVars[DMRVAR_CurrentTrackURI - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(4) = m_avtVars[DMRVAR_RelativeTimePosition - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(5) = m_avtVars[DMRVAR_AbsoluteTimePosition - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(6) = m_avtVars[DMRVAR_RelativeCounterPosition - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(7) = m_avtVars[DMRVAR_AbsoluteCounterPosition - DMRVAR_BaseIndexAVT];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetDeviceCapabilities") == 0) {
			if (inputArgValues.GetItemCount() != 1) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			*outputArgValues.GetItem(0) = m_avtVars[DMRVAR_PossiblePlaybackStorageMedia - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(1) = m_avtVars[DMRVAR_PossibleRecordStorageMedia - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(2) = m_avtVars[DMRVAR_PossibleRecordQualityModes - DMRVAR_BaseIndexAVT];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetTransportSettings") == 0) {
			if (inputArgValues.GetItemCount() != 1) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			*outputArgValues.GetItem(0) = m_avtVars[DMRVAR_CurrentPlayMode - DMRVAR_BaseIndexAVT];
			*outputArgValues.GetItem(1) = m_avtVars[DMRVAR_CurrentRecordQualityMode - DMRVAR_BaseIndexAVT];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "Stop") == 0) {
			m_callback->doDmrStop(this);
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "Play") == 0) {
			m_callback->doDmrPlay(this);
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "Pause") == 0) {
			m_callback->doDmrPause(this);
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "Seek") == 0) {
			if (inputArgValues.GetItemCount() != 3) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 718;
			}

			const NPT_String& seekMode = *inputArgValues.GetItem(1);
			const NPT_String& seekTarget = *inputArgValues.GetItem(2);
			if (seekMode.Compare("TRACK_NR") == 0) {
				// TODO:
			} else if (seekMode.Compare("REL_TIME") == 0) {
				NPT_UInt64 pos;
				if (NPT_FAILED(Helper::parseTrackDurationString(seekTarget, pos))) {
					// Illegal seek target
					return 711;
				}
				m_callback->doDmrSeekTo(this, pos);
			} else {
				// Seek mode not supported
				return 710;
			}

			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "Next") == 0) {
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "Previous") == 0) {
			return 0;
		}

		return 602;
	}

	if (NPT_String::Compare(serviceDecl->serviceId, "urn:upnp-org:serviceId:RenderingControl") == 0) {

		if (NPT_String::Compare(actionDecl->name, "ListPresets") == 0) {
			if (inputArgValues.GetItemCount() != 1) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 702;
			}

			*outputArgValues.GetItem(0) = m_rcsVars[DMRVAR_PresetNameList - DMRVAR_BaseIndexRCS];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "SelectPreset") == 0) {
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetMute") == 0) {
			if (inputArgValues.GetItemCount() != 2) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 702;
			}

			*outputArgValues.GetItem(0) = m_rcsVars[DMRVAR_Mute - DMRVAR_BaseIndexRCS];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "SetMute") == 0) {
			if (inputArgValues.GetItemCount() != 3) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 702;
			}

			const NPT_String& desiredMute = *inputArgValues.GetItem(2);
			m_callback->doDmrSetMute(this, desiredMute.Compare("true", true) == 0 || desiredMute.Compare("1") == 0);
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "GetVolume") == 0) {
			if (inputArgValues.GetItemCount() != 2) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 702;
			}

			*outputArgValues.GetItem(0) = m_rcsVars[DMRVAR_Volume - DMRVAR_BaseIndexRCS];
			return 0;
		}

		if (NPT_String::Compare(actionDecl->name, "SetVolume") == 0) {
			if (inputArgValues.GetItemCount() != 3) {
				// Invalid Args
				return 402;
			}

			if (inputArgValues.GetFirstItem()->Compare("0") != 0) {
				// Invalid InstanceID
				return 702;
			}

			const NPT_String& desiredVolume = *inputArgValues.GetItem(2);
			int volume;
			if (NPT_FAILED(NPT_ParseInteger(desiredVolume, volume))) {
				// Invalid Args
				return 402;
			}
			m_callback->doDmrSetVolume(this, volume);
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

	return 501;
}

} // namespace deejay
