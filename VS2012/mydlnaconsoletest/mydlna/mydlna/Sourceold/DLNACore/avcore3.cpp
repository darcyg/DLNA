#include "avcore3.h"

namespace deejay {

static const char *g_PresetNameAVL[] = {
	"FactoryDefaults", NULL
};

static const char *g_ChannelAVL[] = {
	"Master", NULL
};

static const char *g_VolumeAVR[] = {
	"0", "100", "1"
};

static const StateVariableDecl g_RenderingControl[] = {
	{ "LastChange",            SVDT_STRING,  SVF_EVT },
	{ "PresetNameList",        SVDT_STRING,  SVF_GRP,         g_PresetNameAVL },
	{ "Mute",                  SVDT_BOOLEAN, SVF_GRP|SVF_CHL },
	{ "Volume",                SVDT_UI2,     SVF_GRP|SVF_AVR|SVF_CHL, g_VolumeAVR },
	{ "A_ARG_TYPE_Channel",    SVDT_STRING,  SVF_ARG,         g_ChannelAVL },
	{ "A_ARG_TYPE_InstanceID", SVDT_UI4,     SVF_ARG },
	{ "A_ARG_TYPE_PresetName", SVDT_STRING,  SVF_ARG,         g_PresetNameAVL },
};

static const char *g_TransportStateAVL[] = {
	"STOPPED", "PAUSED_PLAYBACK", "PLAYING", "TRANSITIONING", "NO_MEDIA_PRESENT", NULL
};

static const char *g_TransportStatusAVL[] = {
	"OK", "ERROR_OCCURRED", NULL
};

static const char *g_StorageAVL[] = {
	"UNKNOWN", "CD-DA", "DVD-VIDEO", "HDD", "NETWORK", NULL
};

static const char *g_NotImplAVL[] = {
	"NOT_IMPLEMENTED", NULL
};

static const char *g_PlayModeAVL[] = {
	"NORMAL", "SHUFFLE", "REPEAT_ALL", NULL
};

static const char *g_PlaySpeedAVL[] = {
	"1", NULL
};

static const char *g_TrackAVR[] = {
	"0", "65535", NULL
};

static const char *g_CurrentTrackAVR[] = {
	"0", "65535", "1"
};

static const char *g_SeekModeAVL[] = {
	"ABS_COUNT", "TRACK_NR", "REL_TIME", NULL
};

static const StateVariableDecl g_AVTransport[] = {
	{ "TransportState",               SVDT_STRING, SVF_GRP, g_TransportStateAVL },
	{ "TransportStatus",              SVDT_STRING, SVF_GRP, g_TransportStatusAVL },
	{ "PlaybackStorageMedium",        SVDT_STRING, SVF_GRP, g_StorageAVL },
	{ "RecordStorageMedium",          SVDT_STRING, SVF_GRP, g_NotImplAVL },
	{ "PossiblePlaybackStorageMedia", SVDT_STRING, SVF_GRP, g_StorageAVL },
	{ "PossibleRecordStorageMedia",   SVDT_STRING, SVF_GRP, g_NotImplAVL },
	{ "CurrentPlayMode",              SVDT_STRING, SVF_GRP, g_PlayModeAVL,       "NORMAL" },
	{ "TransportPlaySpeed",           SVDT_STRING, SVF_GRP, g_PlaySpeedAVL },
	{ "RecordMediumWriteStatus",      SVDT_STRING, SVF_GRP, g_NotImplAVL },
	{ "CurrentRecordQualityMode",     SVDT_STRING, SVF_GRP, g_NotImplAVL },
	{ "PossibleRecordQualityModes",   SVDT_STRING, SVF_GRP, g_NotImplAVL },
	{ "NumberOfTracks",               SVDT_UI4,    SVF_GRP|SVF_AVR, g_TrackAVR },
	{ "CurrentTrack",                 SVDT_UI4,    SVF_GRP|SVF_AVR, g_CurrentTrackAVR },
	{ "CurrentTrackDuration",         SVDT_STRING, SVF_GRP },
	{ "CurrentMediaDuration",         SVDT_STRING, SVF_GRP },
	{ "CurrentTrackMetaData",         SVDT_STRING, SVF_GRP },
	{ "CurrentTrackURI",              SVDT_STRING, SVF_GRP },
	{ "AVTransportURI",               SVDT_STRING, SVF_GRP },
	{ "AVTransportURIMetaData",       SVDT_STRING, SVF_GRP },
	{ "NextAVTransportURI",           SVDT_STRING, SVF_GRP },
	{ "NextAVTransportURIMetaData",   SVDT_STRING, SVF_GRP },
	{ "RelativeTimePosition",         SVDT_STRING, SVF_GRP },
	{ "AbsoluteTimePosition",         SVDT_STRING, SVF_GRP },
	{ "RelativeCounterPosition",      SVDT_I4,     SVF_GRP },
	{ "AbsoluteCounterPosition",      SVDT_I4,     SVF_GRP },
	{ "CurrentTransportActions",      SVDT_STRING, SVF_GRP },
	{ "LastChange",                   SVDT_STRING, SVF_EVT },
	{ "A_ARG_TYPE_SeekMode",          SVDT_STRING, SVF_ARG, g_SeekModeAVL },
	{ "A_ARG_TYPE_SeekTarget",        SVDT_STRING, SVF_ARG },
	{ "A_ARG_TYPE_InstanceID",        SVDT_UI4,    SVF_ARG },
};

static const char *g_ConnectionStatusAVL[] = {
	"OK", "ContentFormatMismatch", "InsufficientBandwidth", "UnreliableChannel", "Unknown", NULL
};

static const char *g_DMRDirectionAVL[] = {
	"Input", NULL
};

static const StateVariableDecl g_ConnectionManagerDMR[] = {
	{ "SourceProtocolInfo",           SVDT_STRING, SVF_EVT },
	{ "SinkProtocolInfo",             SVDT_STRING, SVF_EVT },
	{ "CurrentConnectionIDs",         SVDT_STRING, SVF_EVT },
	{ "A_ARG_TYPE_ConnectionStatus",  SVDT_STRING, SVF_ARG, g_ConnectionStatusAVL },
	{ "A_ARG_TYPE_ConnectionManager", SVDT_STRING, SVF_ARG },
	{ "A_ARG_TYPE_Direction",         SVDT_STRING, SVF_ARG, g_DMRDirectionAVL },
	{ "A_ARG_TYPE_ProtocolInfo",      SVDT_STRING, SVF_ARG },
	{ "A_ARG_TYPE_ConnectionID",      SVDT_I4,     SVF_ARG },
	{ "A_ARG_TYPE_AVTransportID",     SVDT_I4,     SVF_ARG },
	{ "A_ARG_TYPE_RcsID",             SVDT_I4,     SVF_ARG },
};

static const char *g_DMSDirectionAVL[] = {
	"Input", "Output", NULL
};

static const StateVariableDecl g_ConnectionManagerDMS[] = {
	{ "SourceProtocolInfo",           SVDT_STRING, SVF_EVT },
	{ "SinkProtocolInfo",             SVDT_STRING, SVF_EVT },
	{ "CurrentConnectionIDs",         SVDT_STRING, SVF_EVT },
	{ "A_ARG_TYPE_ConnectionStatus",  SVDT_STRING, SVF_ARG, g_ConnectionStatusAVL },
	{ "A_ARG_TYPE_ConnectionManager", SVDT_STRING, SVF_ARG },
	{ "A_ARG_TYPE_Direction",         SVDT_STRING, SVF_ARG, g_DMSDirectionAVL },
	{ "A_ARG_TYPE_ProtocolInfo",      SVDT_STRING, SVF_ARG },
	{ "A_ARG_TYPE_ConnectionID",      SVDT_I4,     SVF_ARG },
	{ "A_ARG_TYPE_AVTransportID",     SVDT_I4,     SVF_ARG },
	{ "A_ARG_TYPE_RcsID",             SVDT_I4,     SVF_ARG },
};

static const char *g_BrowseFlagAVL[] = {
	"BrowseMetadata", "BrowseDirectChildren", NULL
};

static const StateVariableDecl g_ContentDirectory[] = {
	{ "A_ARG_TYPE_ObjectID",          SVDT_STRING,  SVF_ARG },
	{ "A_ARG_TYPE_Result",            SVDT_STRING,  SVF_ARG },
	{ "A_ARG_TYPE_SearchCriteria",    SVDT_STRING,  SVF_ARG },
	{ "A_ARG_TYPE_BrowseFlag",        SVDT_STRING,  SVF_ARG, g_BrowseFlagAVL },
	{ "A_ARG_TYPE_Filter",            SVDT_STRING,  SVF_ARG },
	{ "A_ARG_TYPE_SortCriteria",      SVDT_STRING,  SVF_ARG },
	{ "A_ARG_TYPE_Index",             SVDT_UI4,     SVF_ARG },
	{ "A_ARG_TYPE_Count",             SVDT_UI4,     SVF_ARG },
	{ "A_ARG_TYPE_UpdateID",          SVDT_UI4,     SVF_ARG },
	{ "A_ARG_TYPE_TagValueList",      SVDT_STRING,  SVF_ARG },
	{ "SearchCapabilities",           SVDT_STRING, },
	{ "SortCapabilities",             SVDT_STRING, },
	{ "SystemUpdateID",               SVDT_UI4,     SVF_EVT },
	{ "ContainerUpdateIDs",           SVDT_STRING,  SVF_EVT },
//	{ "X_RemoteSharingEnabled",       SVDT_BOOLEAN, SVF_EVT },
};

static const StateVariableDecl g_X_MS_MediaReceiverRegistrar[] = {
	{ "A_ARG_TYPE_DeviceID",            SVDT_STRING, SVF_ARG },
	{ "A_ARG_TYPE_Result",              SVDT_INT,    SVF_ARG },
	{ "A_ARG_TYPE_RegistrationReqMsg",  SVDT_BASE64, SVF_ARG },
	{ "A_ARG_TYPE_RegistrationRespMsg", SVDT_BASE64, SVF_ARG },
	{ "AuthorizationGrantedUpdateID",   SVDT_UI4,    SVF_EVT },
	{ "AuthorizationDeniedUpdateID",    SVDT_UI4,    SVF_EVT },
	{ "ValidationSucceededUpdateID",    SVDT_UI4,    SVF_EVT },
	{ "ValidationRevokedUpdateID",      SVDT_UI4,    SVF_EVT },
};

static const ArgumentDecl g_CM_GetProtocolInfo_Args[] = {
	{ "Source", "SourceProtocolInfo", AF_OUT },
	{ "Sink", "SinkProtocolInfo", AF_OUT },
};

static const ArgumentDecl g_CM_GetCurrentConnectionIDs_Args[] = {
	{ "ConnectionIDs", "CurrentConnectionIDs", AF_OUT },
};

static const ArgumentDecl g_CM_GetCurrentConnectionInfo_Args[] = {
	{ "ConnectionID", "A_ARG_TYPE_ConnectionID", AF_IN },
	{ "RcsID", "A_ARG_TYPE_RcsID", AF_OUT },
	{ "AVTransportID", "A_ARG_TYPE_AVTransportID", AF_OUT },
	{ "ProtocolInfo", "A_ARG_TYPE_ProtocolInfo", AF_OUT },
	{ "PeerConnectionManager", "A_ARG_TYPE_ConnectionManager", AF_OUT },
	{ "PeerConnectionID", "A_ARG_TYPE_ConnectionID", AF_OUT },
	{ "Direction", "A_ARG_TYPE_Direction", AF_OUT },
	{ "Status", "A_ARG_TYPE_ConnectionStatus", AF_OUT },
};

static const ActionDecl g_CM_Actions[] = {
	{ "GetProtocolInfo", 2, 0, g_CM_GetProtocolInfo_Args },
	{ "GetCurrentConnectionIDs", 1, 0, g_CM_GetCurrentConnectionIDs_Args },
	{ "GetCurrentConnectionInfo", 8, 1, g_CM_GetCurrentConnectionInfo_Args },
};

static const ArgumentDecl g_RC_ListPresets_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "CurrentPresetNameList", "PresetNameList", AF_OUT },
};

static const ArgumentDecl g_RC_SelectPreset_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "PresetName", "A_ARG_TYPE_PresetName", AF_IN },
};

static const ArgumentDecl g_RC_GetMute_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Channel", "A_ARG_TYPE_Channel", AF_IN },
	{ "CurrentMute", "Mute", AF_OUT },
};

static const ArgumentDecl g_RC_SetMute_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Channel", "A_ARG_TYPE_Channel", AF_IN },
	{ "DesiredMute", "Mute", AF_IN },
};

static const ArgumentDecl g_RC_GetVolume_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Channel", "A_ARG_TYPE_Channel", AF_IN },
	{ "CurrentVolume", "Volume", AF_OUT },
};

static const ArgumentDecl g_RC_SetVolume_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Channel", "A_ARG_TYPE_Channel", AF_IN },
	{ "DesiredVolume", "Volume", AF_IN },
};

static const ActionDecl g_RC_Actions[] = {
	{ "ListPresets", 2, 1, g_RC_ListPresets_Args },
	{ "SelectPreset", 2, 2, g_RC_SelectPreset_Args },
	{ "GetMute", 3, 2, g_RC_GetMute_Args },
	{ "SetMute", 3, 3, g_RC_SetMute_Args },
	{ "GetVolume", 3, 2, g_RC_GetVolume_Args },
	{ "SetVolume", 3, 3, g_RC_SetVolume_Args },
};

static const ArgumentDecl g_AVT_SetAVTransportURI_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "CurrentURI", "AVTransportURI", AF_IN },
	{ "CurrentURIMetaData", "AVTransportURIMetaData", AF_IN },
};

static const ArgumentDecl g_AVT_GetMediaInfo_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "NrTracks", "NumberOfTracks", AF_OUT },
	{ "MediaDuration", "CurrentMediaDuration", AF_OUT },
	{ "CurrentURI", "AVTransportURI", AF_OUT },
	{ "CurrentURIMetaData", "AVTransportURIMetaData", AF_OUT },
	{ "NextURI", "NextAVTransportURI", AF_OUT },
	{ "NextURIMetaData", "NextAVTransportURIMetaData", AF_OUT },
	{ "PlayMedium", "PlaybackStorageMedium", AF_OUT },
	{ "RecordMedium", "RecordStorageMedium", AF_OUT },
	{ "WriteStatus", "RecordMediumWriteStatus", AF_OUT },
};

static const ArgumentDecl g_AVT_GetTransportInfo_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "CurrentTransportState", "TransportState", AF_OUT },
	{ "CurrentTransportStatus", "TransportStatus", AF_OUT },
	{ "CurrentSpeed", "TransportPlaySpeed", AF_OUT },
};

static const ArgumentDecl g_AVT_GetPositionInfo_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Track", "CurrentTrack", AF_OUT },
	{ "TrackDuration", "CurrentTrackDuration", AF_OUT },
	{ "TrackMetaData", "CurrentTrackMetaData", AF_OUT },
	{ "TrackURI", "CurrentTrackURI", AF_OUT },
	{ "RelTime", "RelativeTimePosition", AF_OUT },
	{ "AbsTime", "AbsoluteTimePosition", AF_OUT },
	{ "RelCount", "RelativeCounterPosition", AF_OUT },
	{ "AbsCount", "AbsoluteCounterPosition", AF_OUT },
};

static const ArgumentDecl g_AVT_GetDeviceCapabilities_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "PlayMedia", "PossiblePlaybackStorageMedia", AF_OUT },
	{ "RecMedia", "PossibleRecordStorageMedia", AF_OUT },
	{ "RecQualityModes", "PossibleRecordQualityModes", AF_OUT },
};

static const ArgumentDecl g_AVT_GetTransportSettings_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "PlayMode", "CurrentPlayMode", AF_OUT },
	{ "RecQualityMode", "CurrentRecordQualityMode", AF_OUT },
};

static const ArgumentDecl g_AVT_Stop_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
};

static const ArgumentDecl g_AVT_Play_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Speed", "TransportPlaySpeed", AF_IN },
};

static const ArgumentDecl g_AVT_Pause_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
};

static const ArgumentDecl g_AVT_Seek_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Unit", "A_ARG_TYPE_SeekMode", AF_IN },
	{ "Target", "A_ARG_TYPE_SeekTarget", AF_IN },
};

static const ArgumentDecl g_AVT_Next_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
};

static const ArgumentDecl g_AVT_Previous_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
};

static const ArgumentDecl g_AVT_GetCurrentTransportActions_Args[] = {
	{ "InstanceID", "A_ARG_TYPE_InstanceID", AF_IN },
	{ "Actions", "CurrentTransportActions", AF_OUT },
};

static const ActionDecl g_AVT_Actions[] = {
	{ "SetAVTransportURI", 3, 3, g_AVT_SetAVTransportURI_Args },
	{ "GetMediaInfo", 10, 1, g_AVT_GetMediaInfo_Args },
	{ "GetTransportInfo", 4, 1, g_AVT_GetTransportInfo_Args },
	{ "GetPositionInfo", 9, 1, g_AVT_GetPositionInfo_Args },
	{ "GetDeviceCapabilities", 4, 1, g_AVT_GetDeviceCapabilities_Args },
	{ "GetTransportSettings", 3, 1, g_AVT_GetTransportSettings_Args },
	{ "Stop", 1, 1, g_AVT_Stop_Args },
	{ "Play", 2, 2, g_AVT_Play_Args },
	{ "Pause", 1, 1, g_AVT_Pause_Args },
	{ "Seek", 3, 3, g_AVT_Seek_Args },
	{ "Next", 1, 1, g_AVT_Next_Args },
	{ "Previous", 1, 1, g_AVT_Previous_Args },
	{ "GetCurrentTransportActions", 2, 1, g_AVT_GetCurrentTransportActions_Args },
};

static const ArgumentDecl g_CD_GetSearchCapabilities_Args[] = {
	{ "SearchCaps", "SearchCapabilities", AF_OUT },
};

static const ArgumentDecl g_CD_GetSortCapabilities_Args[] = {
	{ "SortCaps", "SortCapabilities", AF_OUT },
};

static const ArgumentDecl g_CD_GetSystemUpdateID_Args[] = {
	{ "Id", "SystemUpdateID", AF_OUT },
};

static const ArgumentDecl g_CD_Browse_Args[] = {
	{ "ObjectID", "A_ARG_TYPE_ObjectID", AF_IN },
	{ "BrowseFlag", "A_ARG_TYPE_BrowseFlag", AF_IN },
	{ "Filter", "A_ARG_TYPE_Filter", AF_IN },
	{ "StartingIndex", "A_ARG_TYPE_Index", AF_IN },
	{ "RequestedCount", "A_ARG_TYPE_Count", AF_IN },
	{ "SortCriteria", "A_ARG_TYPE_SortCriteria", AF_IN },
	{ "Result", "A_ARG_TYPE_Result", AF_OUT },
	{ "NumberReturned", "A_ARG_TYPE_Count", AF_OUT },
	{ "TotalMatches", "A_ARG_TYPE_Count", AF_OUT },
	{ "UpdateID", "A_ARG_TYPE_UpdateID", AF_OUT },
};

static const ArgumentDecl g_CD_Search_Args[] = {
	{ "ContainerID", "A_ARG_TYPE_ObjectID", AF_IN },
	{ "SearchCriteria", "A_ARG_TYPE_SearchCriteria", AF_IN },
	{ "Filter", "A_ARG_TYPE_Filter", AF_IN },
	{ "StartingIndex", "A_ARG_TYPE_Index", AF_IN },
	{ "RequestedCount", "A_ARG_TYPE_Count", AF_IN },
	{ "SortCriteria", "A_ARG_TYPE_SortCriteria", AF_IN },
	{ "Result", "A_ARG_TYPE_Result", AF_OUT },
	{ "NumberReturned", "A_ARG_TYPE_Count", AF_OUT },
	{ "TotalMatches", "A_ARG_TYPE_Count", AF_OUT },
	{ "UpdateID", "A_ARG_TYPE_UpdateID", AF_OUT },
};

static const ArgumentDecl g_CD_UpdateObject_Args[] = {
	{ "ObjectID", "A_ARG_TYPE_ObjectID", AF_IN },
	{ "CurrentTagValue", "A_ARG_TYPE_TagValueList", AF_IN },
	{ "NewTagValue", "A_ARG_TYPE_TagValueList", AF_IN },
};
/*
static const ArgumentDecl g_CD_X_GetRemoteSharingStatus_Args[] = {
	{ "Status", "X_RemoteSharingEnabled", AF_OUT },
};
*/
static const ActionDecl g_CD_Actions[] = {
	{ "GetSearchCapabilities", 1, 0, g_CD_GetSearchCapabilities_Args },
	{ "GetSortCapabilities", 1, 0, g_CD_GetSortCapabilities_Args },
	{ "GetSystemUpdateID", 1, 0, g_CD_GetSystemUpdateID_Args },
	{ "Browse", 10, 6, g_CD_Browse_Args },
	{ "Search", 10, 6, g_CD_Search_Args },
//	{ "UpdateObject", 3, g_CD_UpdateObject_Args },
//	{ "X_GetRemoteSharingStatus", 1, g_CD_X_GetRemoteSharingStatus_Args },
};

static const ArgumentDecl g_X_MS_MediaReceiverRegistrar_IsAuthorized_Args[] = {
	{ "DeviceID", "A_ARG_TYPE_DeviceID", AF_IN },
	{ "Result", "A_ARG_TYPE_Result", AF_OUT },
};

static const ArgumentDecl g_X_MS_MediaReceiverRegistrar_RegisterDevice_Args[] = {
	{ "RegistrationReqMsg", "A_ARG_TYPE_RegistrationReqMsg", AF_IN },
	{ "RegistrationRespMsg", "A_ARG_TYPE_RegistrationRespMsg", AF_OUT },
};

static const ArgumentDecl g_X_MS_MediaReceiverRegistrar_IsValidated_Args[] = {
	{ "DeviceID", "A_ARG_TYPE_DeviceID", AF_IN },
	{ "Result", "A_ARG_TYPE_Result", AF_OUT },
};

static const ActionDecl g_X_MS_MediaReceiverRegistrar_Actions[] = {
	{ "IsAuthorized", 2, 1, g_X_MS_MediaReceiverRegistrar_IsAuthorized_Args },
//	{ "RegisterDevice", 2, 1, g_X_MS_MediaReceiverRegistrar_RegisterDevice_Args },
	{ "IsValidated", 2, 1, g_X_MS_MediaReceiverRegistrar_IsValidated_Args },
};

static const ServiceDecl g_DMR_Services[] = {
	{
		"urn:schemas-upnp-org:service:ConnectionManager:1",
		"urn:upnp-org:serviceId:ConnectionManager",
		sizeof(g_ConnectionManagerDMR) / sizeof(g_ConnectionManagerDMR[0]),
		sizeof(g_CM_Actions) / sizeof(g_CM_Actions[0]),
		g_ConnectionManagerDMR,
		g_CM_Actions
	},
	{
		"urn:schemas-upnp-org:service:RenderingControl:1",
		"urn:upnp-org:serviceId:RenderingControl",
		sizeof(g_RenderingControl) / sizeof(g_RenderingControl[0]),
		sizeof(g_RC_Actions) / sizeof(g_RC_Actions[0]),
		g_RenderingControl,
		g_RC_Actions
	},
	{
		"urn:schemas-upnp-org:service:AVTransport:1",
		"urn:upnp-org:serviceId:AVTransport",
		sizeof(g_AVTransport) / sizeof(g_AVTransport[0]),
		sizeof(g_AVT_Actions) / sizeof(g_AVT_Actions[0]),
		g_AVTransport,
		g_AVT_Actions
	}
};

static const ServiceDecl g_DMS_Services[] = {
	{
		"urn:schemas-upnp-org:service:ConnectionManager:1",
		"urn:upnp-org:serviceId:ConnectionManager",
		sizeof(g_ConnectionManagerDMS) / sizeof(g_ConnectionManagerDMS[0]),
		sizeof(g_CM_Actions) / sizeof(g_CM_Actions[0]),
		g_ConnectionManagerDMS,
		g_CM_Actions
	},
	{
		"urn:schemas-upnp-org:service:ContentDirectory:1",
		"urn:upnp-org:serviceId:ContentDirectory",
		sizeof(g_ContentDirectory) / sizeof(g_ContentDirectory[0]),
		sizeof(g_CD_Actions) / sizeof(g_CD_Actions[0]),
		g_ContentDirectory,
		g_CD_Actions
	},
	{
		"urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1",
		"urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar",
		sizeof(g_X_MS_MediaReceiverRegistrar) / sizeof(g_X_MS_MediaReceiverRegistrar[0]),
		sizeof(g_X_MS_MediaReceiverRegistrar_Actions) / sizeof(g_X_MS_MediaReceiverRegistrar_Actions[0]),
		g_X_MS_MediaReceiverRegistrar,
		g_X_MS_MediaReceiverRegistrar_Actions
	},
};

const ServiceDecl *g_DMSServiceTable = g_DMS_Services;
int g_DMSServiceTableSize = sizeof(g_DMS_Services) / sizeof(g_DMS_Services[0]);

const ServiceDecl *g_DMRServiceTable = g_DMR_Services;
int g_DMRServiceTableSize = sizeof(g_DMR_Services) / sizeof(g_DMR_Services[0]);

} // namespace deejay
