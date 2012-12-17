#ifndef __DLNACore_h__
#define __DLNACore_h__

#include "DJDesc.h"
#include "DJMediaStore.h"
#include "DLNACoreOp.h"
#include "DLNALibrary.h"
#include "avcore4.h"

namespace deejay {

class DLNACoreImpl;

class DLNACoreDelegate
{
public:
	virtual void onMediaServerListChanged() = 0;
	virtual void onMediaRendererListChanged() = 0;
	virtual void onMediaServerStateVariablesChanged(DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList) = 0;
	virtual void onMediaRendererStateVariablesChanged(DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList) = 0;

	virtual void dmrOpen(const NPT_String& url, const NPT_String& mimeType, const NPT_String& metaData) = 0;
	virtual void dmrPlay() = 0;
	virtual void dmrPause() = 0;
	virtual void dmrStop() = 0;
	virtual void dmrSeekTo(NPT_Int64 timeInMillis) = 0;
	virtual void dmrSetMute(bool mute) = 0;
	virtual void dmrSetVolume(int volume) = 0;
};

class DLNACore
{
public:
	enum Function {
		Function_ControlPoint,
		Function_MediaServer,
		Function_MediaRenderer,
	};

	static NPT_Result parseTrackDurationString(const NPT_String& text, NPT_UInt64& duration);

	DLNACore(DLNACoreDelegate *d);
	~DLNACore();

	void loadConfig(NPT_InputStream *inputStream);
	void saveConfig(NPT_OutputStream *outputStream);

	void setProperty(const NPT_String& name, const NPT_String& value);
	bool getproperty(const NPT_String& name, NPT_String& value) const;

	void importFileSystemToMediaServer(const NPT_String& dir, const NPT_String& name, bool ignoreDot = false);
	const UUID& getMediaServerUuid() const;
	const UUID& getMediaRendererUuid() const;
	void clearMediaServerContent();

	bool start();
	bool started() const;
	void stop();

	void enableFunction(Function function, bool enable);
	bool isFunctionEnabled(Function function) const;

	void internalEnableLoopback(bool enable);

	enum FlushMode {
		FlushMode_All,
		FlushMode_MediaRendererOnly,
		FlushMode_MediaServerOnly,
	};

	void flushDeviceList(FlushMode flushMode = FlushMode_All);
	void searchDevices(NPT_UInt32 mx = 3);

	DeviceDescList snapshotMediaServerList() const;
	DeviceDescList snapshotMediaRendererList() const;

	MediaStore *mediaStore() const;

	NPT_Result queryStateVariable(const UUID& deviceUuid, const NPT_String& serviceId, const NPT_String& name, NPT_String& value);
	NPT_Result queryStateVariables(const UUID& deviceUuid, const NPT_String& serviceId, const NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList);

	NPT_Result browseMediaServer(const UUID& mediaServerUuid, const NPT_String& containerId, bool deep, DLNABrowseOp **op);
	NPT_Result browseMediaServerEx(const UUID& mediaServerUuid, const NPT_String& containerId, int step, DLNAProgressiveBrowseOp::ResultCallback *resultCallback, DLNAProgressiveBrowseOp **op);
	NPT_Result playMedia(const UUID& mediaRendererUuid, const DLNAItem *mediaItem, DLNACoreOp **op);
	NPT_Result stopMedia(const UUID& mediaRendererUuid, DLNACoreOp **op);
	NPT_Result pauseMedia(const UUID& mediaRendererUuid, DLNACoreOp **op);
	NPT_Result prevMedia(const UUID& mediaRendererUuid, DLNACoreOp **op);
	NPT_Result nextMedia(const UUID& mediaRendererUuid, DLNACoreOp **op);
	NPT_Result muteMedia(const UUID& mediaRendererUuid, bool mute, DLNACoreOp **op);
	NPT_Result changeMediaVolume(const UUID& mediaRendererUuid, int volume, DLNACoreOp **op);
	NPT_Result seekMedia(const UUID& mediaRendererUuid, int timeInMillis, DLNACoreOp **op);
	NPT_Result queryMediaPositionInfo(const UUID& mediaRendererUuid, DLNAQueryPositionInfoOp **op);

	NPT_Result playMediaList(const UUID& mediaRendererUuid, const NPT_List<DLNAObject*>& ls, DLNACoreOp **op);
	NPT_Result playFile(const UUID& mediaRendererUuid, const NPT_String& path, DLNACoreOp **op);

	NPT_Result importPhotos(const NPT_String& name, DLNACoreOp **op);

	enum DMRState {
		DMRState_Stopped,
		DMRState_Playing,
		DMRState_Loading,
		DMRState_Paused,
		DMRState_NoMediaPresent,
	};

	void dmrReportState(DMRState state);
	void dmrReportErrorStatus(bool error);
	void dmrReportProgress(NPT_Int64 currentMillis, NPT_Int64 totalMillis);
	void dmrReportVolume(int volume, bool muted);

private:
	DLNACoreImpl *m_impl;
};

} // namespace deejay

#endif // __DLNACore_h__
