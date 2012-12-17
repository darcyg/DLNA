#ifndef __avcore2_h__
#define __avcore2_h__

#include "DJDeviceImpl.h"

namespace deejay {

enum DMRVAR 
{
	// AVTransport
	DMRVAR_BaseIndexAVT = 1000,
	DMRVAR_TransportState = DMRVAR_BaseIndexAVT,
	DMRVAR_TransportStatus,
	DMRVAR_PlaybackStorageMedium,
	DMRVAR_RecordStorageMedium,
	DMRVAR_PossiblePlaybackStorageMedia,
	DMRVAR_PossibleRecordStorageMedia,
	DMRVAR_CurrentPlayMode, ///
	DMRVAR_TransportPlaySpeed,
	DMRVAR_RecordMediumWriteStatus,
	DMRVAR_CurrentRecordQualityMode, ///
	DMRVAR_PossibleRecordQualityModes,
	DMRVAR_NumberOfTracks,
	DMRVAR_CurrentTrack,
	DMRVAR_CurrentTrackDuration,
	DMRVAR_CurrentMediaDuration,
	DMRVAR_CurrentTrackMetaData,
	DMRVAR_CurrentTrackURI,
	DMRVAR_AVTransportURI,
	DMRVAR_AVTransportURIMetaData,
	DMRVAR_NextAVTransportURI,
	DMRVAR_NextAVTransportURIMetaData,
	DMRVAR_RelativeTimePosition,
	DMRVAR_AbsoluteTimePosition,
	DMRVAR_RelativeCounterPosition,
	DMRVAR_AbsoluteCounterPosition,
	DMRVAR_CurrentTransportActions, ///
	DMRVAR_LastIndexAVT = DMRVAR_CurrentTransportActions,

	// RenderingControl
	DMRVAR_BaseIndexRCS = 1200,
	DMRVAR_PresetNameList = DMRVAR_BaseIndexRCS,
	DMRVAR_Mute,
	DMRVAR_Volume,
	DMRVAR_LastIndexRCS = DMRVAR_Volume,
};

class SimpleDMR
	: public DeviceImpl
{
public:
	class Callback
	{
	public:
		virtual void doDmrOpen(SimpleDMR *dmr, const NPT_String& url, const NPT_String& mimeType, const NPT_String& metaData) = 0;
		virtual void doDmrPlay(SimpleDMR *dmr) = 0;
		virtual void doDmrPause(SimpleDMR *dmr) = 0;
		virtual void doDmrStop(SimpleDMR *dmr) = 0;
		virtual void doDmrSeekTo(SimpleDMR *dmr, NPT_Int64 pos) = 0;
		virtual void doDmrSetMute(SimpleDMR *dmr, bool mute) = 0;
		virtual void doDmrSetVolume(SimpleDMR *dmr, int volume) = 0;
	};

	SimpleDMR(Callback *callback, const UUID& uuid);
	virtual ~SimpleDMR();

	void setUuid(const UUID& uuid);
	const UUID& uuid() const;
	void setFriendlyName(const NPT_String& friendlyName);
	void setPlatformName(const NPT_String& platformName);

	void setVar(DMRVAR varIndex, const NPT_String& value);
	void setSinkProtocolInfo(const NPT_String& value);

protected:
	virtual void outputMoreDeviceDescription(NPT_XmlSerializer *xml);
	virtual int onAction(const ServiceDecl *serviceDecl, const ActionDecl *actionDecl, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, NPT_List<NPT_String>& outputArgValues);

	void setVarAVT(const NPT_String& name, const NPT_String& value);
	void setVarRCS(const NPT_String& name, const NPT_String& value);

private:
	NPT_String m_avtVars[DMRVAR_LastIndexAVT - DMRVAR_BaseIndexAVT + 1];
	NPT_String m_rcsVars[DMRVAR_LastIndexRCS - DMRVAR_BaseIndexRCS + 1];
	Callback *m_callback;
	ReadWriteLock m_stateLock;
};

} // namespace deejay

#endif // __avcore2_h__
