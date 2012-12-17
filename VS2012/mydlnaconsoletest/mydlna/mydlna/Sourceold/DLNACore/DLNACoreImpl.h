#ifndef __DLNACoreImpl_h__
#define __DLNACoreImpl_h__

#include "DLNACore.h"
#include "DJControlPoint.h"
#include "DJFrontEnd.h"
#include "DJUtils.h"
#include "avcore1.h"
#include "avcore2.h"

namespace deejay {

class DLNACoreImpl
	: public ControlPoint::Callback
	, public SimpleDMR::Callback
{
public:
	DLNACoreImpl(DLNACoreDelegate *d);
	~DLNACoreImpl();

	enum DelegateMethodIndex
	{
		DelegateMethodIndex_onMediaServerListChanged,
		DelegateMethodIndex_onMediaRendererListChanged,
		DelegateMethodIndex_onMediaServerStateVariablesChanged,
		DelegateMethodIndex_onMediaRendererStateVariablesChanged,
	};

	bool start();
	bool started() const;
	void stop();

	MediaStore *mediaStore() const;

	void loadConfig(NPT_InputStream *inputStream);
	void saveConfig(NPT_OutputStream *outputStream);

	void setProperty(const NPT_String& name, const NPT_String& value);
	bool getproperty(const NPT_String& name, NPT_String& value) const;

	void enableFunction(DLNACore::Function function, bool enable);
	bool isFunctionEnabled(DLNACore::Function function) const;

	void internalEnableLoopback(bool enable);

	void flushDeviceList(DLNACore::FlushMode flushMode);
	void searchDevices(NPT_UInt32 mx = 3);

	DeviceDescList snapshotMediaServerList() const;
	DeviceDescList snapshotMediaRendererList() const;

	NPT_Result queryStateVariables(const UUID& deviceUuid, const NPT_String& serviceId, const NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList);

	NPT_Result browseMediaServer(const UUID& mediaServerUuid, const NPT_String& containerId, DLNABrowseOp **op);
	NPT_Result browseMediaServerEx(const UUID& mediaServerUuid, const NPT_String& containerId, DLNABrowseOp **op);
	NPT_Result browseMediaServerEx(const UUID& mediaServerUuid, const NPT_String& containerId, NPT_UInt32 step, DLNAProgressiveBrowseOp::ResultCallback *resultCallback, DLNAProgressiveBrowseOp **op);
	NPT_Result browseMediaServerDeep(const UUID& mediaServerUuid, const NPT_String& containerId, DLNABrowseOp **op);
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

	void dmrReportState(DLNACore::DMRState state);
	void dmrReportErrorStatus(bool error);
	void dmrReportProgress(NPT_Int64 currentMillis, NPT_Int64 totalMillis);
	void dmrReportVolume(int volume, bool muted);

	const UUID& getMediaServerUuid() const;
	const UUID& getMediaRendererUuid() const;

protected:
	virtual void controlPointOnDeviceAdded(ControlPoint *controlPoint, DeviceDesc *deviceDesc, bool& subscribe);
	virtual void controlPointOnDeviceRemoved(ControlPoint *controlPoint, DeviceDesc *deviceDesc);
	virtual void controlPointOnStateVariablesChanged(ControlPoint *controlPoint, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList);

	virtual void doDmrOpen(SimpleDMR *dmr, const NPT_String& url, const NPT_String& mimeType, const NPT_String& metaData);
	virtual void doDmrPlay(SimpleDMR *dmr);
	virtual void doDmrPause(SimpleDMR *dmr);
	virtual void doDmrStop(SimpleDMR *dmr);
	virtual void doDmrSeekTo(SimpleDMR *dmr, NPT_Int64 pos);
	virtual void doDmrSetMute(SimpleDMR *dmr, bool mute);
	virtual void doDmrSetVolume(SimpleDMR *dmr, int volume);

private:
	struct DMSInfo
	{
		DeviceDesc *m_deviceDesc;
		const ServiceDesc *m_cds;
		const ServiceDesc *m_cms;
		const ServiceDesc *m_avt;

		~DMSInfo()
		{
			m_deviceDesc->release();
		}

		NPT_Map<NPT_String, NPT_String> m_cdsVars;
		NPT_Map<NPT_String, NPT_String> m_cmsVars;
	};

	class DMSInfoFinder
	{
	public:
		DMSInfoFinder(const UUID& uuid)
			: m_uuid(uuid)
		{
		}

		bool operator()(const DMSInfo *dmsInfo) const
		{
			return dmsInfo->m_deviceDesc->uuid() == m_uuid;
		}

	private:
		UUID m_uuid;
	};

	struct DMRInfo
	{
		DeviceDesc *m_deviceDesc;
		const ServiceDesc *m_rcs;
		const ServiceDesc *m_cms;
		const ServiceDesc *m_avt;

		~DMRInfo()
		{
			m_deviceDesc->release();
		}

		NPT_Map<NPT_String, NPT_String> m_cmsVars;
		NPT_Map<NPT_String, NPT_String> m_rcsVars;
		NPT_Map<NPT_String, NPT_String> m_avtVars;

		NPT_Map<NPT_String, NPT_String> m_rcsMuteVars;
		NPT_Map<NPT_String, NPT_String> m_rcsVolumeVars;
		NPT_Map<NPT_String, NPT_String> m_rcsVolumeDBVars;
		NPT_Map<NPT_String, NPT_String> m_rcsLoudnessVars;
	};

	class DMRInfoFinder
	{
	public:
		DMRInfoFinder(const UUID& uuid)
			: m_uuid(uuid)
		{
		}

		bool operator()(const DMRInfo *dmrInfo) const
		{
			return dmrInfo->m_deviceDesc->uuid() == m_uuid;
		}

	private:
		UUID m_uuid;
	};

	void parseLastChangeAVT(DMRInfo *dmrInfo, const NPT_String& text, NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList);
	void parseLastChangeRCS(DMRInfo *dmrInfo, const NPT_String& text, NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList);

	void invokeDelegate(int index);
	void invokeDelegate(int index, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList);


	ReadWriteLock m_stateLock;
	DLNACoreDelegate *m_delegate;
	FrontEnd *m_fe;
	ControlPoint *m_cp;
	SimpleDMS *m_dms;
	SimpleDMR *m_dmr;
	bool m_started;

	NPT_List<DMSInfo*> m_dmsList;
	NPT_List<DMRInfo*> m_dmrList;
	DeviceDescList m_dmrDescList;
	DeviceDescList m_dmsDescList;

	UUID m_dmsUuid;
	UUID m_dmrUuid;

	TaskGroup *m_taskGroup;

	NPT_String m_friendlyName;
	NPT_String m_platformName;
};

class InvokeDelegateMethodTask
	: public Task
{
public:
	InvokeDelegateMethodTask(DLNACoreDelegate *d, int methodIndex);
	InvokeDelegateMethodTask(DLNACoreDelegate *d, int methodIndex, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList);
	virtual ~InvokeDelegateMethodTask();

protected:
	virtual void exec();

private:
	DLNACoreDelegate *m_delegate;
	int m_methodIndex;
	DeviceDesc *m_deviceDesc;
	ServiceDesc *m_serviceDesc;
	NPT_List<NPT_String> m_nameList;
	NPT_List<NPT_String> m_valueList;
};

} // namespace deejay

#endif // __DLNACoreImpl_h__
