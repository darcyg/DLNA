#include "pch.h"
#include "DLNACoreImpl.h"
#include "DLNALibraryImpl.h"
#include "DJUtils.h"

namespace deejay {

NPT_Result DLNACore::parseTrackDurationString(const NPT_String& text, NPT_UInt64& duration)
{
	return Helper::parseTrackDurationString(text, duration);
}

DLNACore::DLNACore(DLNACoreDelegate *d)
{
	m_impl = new DLNACoreImpl(d);
}

DLNACore::~DLNACore()
{
	delete m_impl;
}

MediaStore *DLNACore::mediaStore() const
{
	return m_impl->mediaStore();
}

void DLNACore::loadConfig(NPT_InputStream *inputStream)
{
	m_impl->loadConfig(inputStream);
}

void DLNACore::saveConfig(NPT_OutputStream *outputStream)
{
	m_impl->saveConfig(outputStream);
}

void DLNACore::setProperty(const NPT_String& name, const NPT_String& value)
{
	m_impl->setProperty(name, value);
}

bool DLNACore::getproperty(const NPT_String& name, NPT_String& value) const
{
	return m_impl->getproperty(name, value);
}

void DLNACore::importFileSystemToMediaServer(const NPT_String& dir, const NPT_String& name, bool ignoreDot)
{
	mediaStore()->importFileSystem(dir, name, ignoreDot);
}

void DLNACore::clearMediaServerContent()
{
	mediaStore()->reset();
}

bool DLNACore::start()
{
	return m_impl->start();
}

bool DLNACore::started() const
{
	return m_impl->started();
}

void DLNACore::stop()
{
	m_impl->stop();
}

void DLNACore::enableFunction(Function function, bool enable)
{
	m_impl->enableFunction(function, enable);
}

bool DLNACore::isFunctionEnabled(Function function) const
{
	return m_impl->isFunctionEnabled(function);
}

void DLNACore::internalEnableLoopback(bool enable)
{
	m_impl->internalEnableLoopback(enable);
}

void DLNACore::flushDeviceList(FlushMode flushMode)
{
	m_impl->flushDeviceList(flushMode);
}

void DLNACore::searchDevices(NPT_UInt32 mx)
{
	m_impl->searchDevices(mx);
}

DeviceDescList DLNACore::snapshotMediaServerList() const
{
	return m_impl->snapshotMediaServerList();
}

DeviceDescList DLNACore::snapshotMediaRendererList() const
{
	return m_impl->snapshotMediaRendererList();
}

NPT_Result DLNACore::browseMediaServer(const UUID& mediaServerUuid, const NPT_String& containerId, bool deep, DLNABrowseOp **op)
{
	if (deep) {
		return m_impl->browseMediaServerDeep(mediaServerUuid, containerId, op);
	}
	return m_impl->browseMediaServerEx(mediaServerUuid, containerId, op);
}

NPT_Result DLNACore::browseMediaServerEx(const UUID& mediaServerUuid, const NPT_String& containerId, int step, DLNAProgressiveBrowseOp::ResultCallback *resultCallback, DLNAProgressiveBrowseOp **op)
{
	return m_impl->browseMediaServerEx(mediaServerUuid, containerId, step >= 0 ? step : 30, resultCallback, op);
}

/*
DLNALibrary *DLNACore::createMediaLibrary(const UUID& mediaServerUuid)
{
	return DLNALibraryImpl::newDLNALibrary();
}
*/
NPT_Result DLNACore::playMedia(const UUID& mediaRendererUuid, const DLNAItem *mediaItem, DLNACoreOp **op)
{
	return m_impl->playMedia(mediaRendererUuid, mediaItem, op);
}

NPT_Result DLNACore::stopMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	return m_impl->stopMedia(mediaRendererUuid, op);
}

NPT_Result DLNACore::pauseMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	return m_impl->pauseMedia(mediaRendererUuid, op);
}

NPT_Result DLNACore::prevMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	return m_impl->prevMedia(mediaRendererUuid, op);
}

NPT_Result DLNACore::nextMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	return m_impl->nextMedia(mediaRendererUuid, op);
}

NPT_Result DLNACore::muteMedia(const UUID& mediaRendererUuid, bool mute, DLNACoreOp **op)
{
	return m_impl->muteMedia(mediaRendererUuid, mute, op);
}

NPT_Result DLNACore::changeMediaVolume(const UUID& mediaRendererUuid, int volume, DLNACoreOp **op)
{
	return m_impl->changeMediaVolume(mediaRendererUuid, volume, op);
}

NPT_Result DLNACore::seekMedia(const UUID& mediaRendererUuid, int timeInMillis, DLNACoreOp **op)
{
	return m_impl->seekMedia(mediaRendererUuid, timeInMillis, op);
}

NPT_Result DLNACore::queryMediaPositionInfo(const UUID& mediaRendererUuid, DLNAQueryPositionInfoOp **op)
{
	return m_impl->queryMediaPositionInfo(mediaRendererUuid, op);
}

NPT_Result DLNACore::playMediaList(const UUID& mediaRendererUuid, const NPT_List<DLNAObject*>& ls, DLNACoreOp **op)
{
	return m_impl->playMediaList(mediaRendererUuid, ls, op);
}

NPT_Result DLNACore::playFile(const UUID& mediaRendererUuid, const NPT_String& path, DLNACoreOp **op)
{
	return m_impl->playFile(mediaRendererUuid, path, op);
}

NPT_Result DLNACore::importPhotos(const NPT_String& name, DLNACoreOp **op)
{
	return m_impl->importPhotos(name, op);
}

NPT_Result DLNACore::queryStateVariable(const UUID& deviceUuid, const NPT_String& serviceId, const NPT_String& name, NPT_String& value)
{
	NPT_List<NPT_String> nameList;
	nameList.Add(name);
	NPT_List<NPT_String> valueList;
	NPT_Result nr = m_impl->queryStateVariables(deviceUuid, serviceId, nameList, valueList);
	if (NPT_SUCCEEDED(nr)) {
		value = *valueList.GetFirstItem();
	}
	return nr;
}

NPT_Result DLNACore::queryStateVariables(const UUID& deviceUuid, const NPT_String& serviceId, const NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList)
{
	return m_impl->queryStateVariables(deviceUuid, serviceId, nameList, valueList);
}

void DLNACore::dmrReportState(DMRState state)
{
	m_impl->dmrReportState(state);
}

void DLNACore::dmrReportErrorStatus(bool error)
{
	m_impl->dmrReportErrorStatus(error);
}

void DLNACore::dmrReportProgress(NPT_Int64 currentMillis, NPT_Int64 totalMillis)
{
	m_impl->dmrReportProgress(currentMillis, totalMillis);
}

void DLNACore::dmrReportVolume(int volume, bool muted)
{
	m_impl->dmrReportVolume(volume, muted);
}

const UUID& DLNACore::getMediaServerUuid() const
{
	return m_impl->getMediaServerUuid();
}

const UUID& DLNACore::getMediaRendererUuid() const
{
	return m_impl->getMediaRendererUuid();
}

} // namespace deejay
