#include "pch.h"
#include "DLNACoreImpl.h"
#include "DLNACoreOpImpl.h"
#include "DLNAObjectImpl.h"

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("deejay.DLNACore")

namespace deejay {

DLNACoreImpl::DLNACoreImpl(DLNACoreDelegate *d)
	: m_delegate(d), m_started(false)
	, m_fe(NULL), m_cp(NULL), m_dms(NULL), m_dmr(NULL)
{
	m_dmsUuid = UUID::generate();
	m_dmrUuid = UUID::generate();

	m_fe = new FrontEnd();
	m_cp = new ControlPoint(this);
	m_dms = new SimpleDMS(m_dmsUuid);
	m_dmr = new SimpleDMR(this, m_dmrUuid);

	m_taskGroup = new TaskGroup();
}

DLNACoreImpl::~DLNACoreImpl()
{
	stop();

	m_taskGroup->abort();
	m_taskGroup->wait();
	delete m_taskGroup;

	delete m_dmr;
	delete m_dms;
	delete m_cp;
	delete m_fe;
}

MediaStore *DLNACoreImpl::mediaStore() const
{
	return m_dms->mediaStore();
}

void DLNACoreImpl::loadConfig(NPT_InputStream *inputStream)
{
	NPT_XmlParser parser;
	NPT_XmlNode *rootNode;
	if (NPT_SUCCEEDED(parser.Parse(*inputStream, rootNode))) {
		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (rootEl && matchTagNamespace(rootEl, "DLNACore", "urn:schemas-netgear-com:dlnacore-1-0")) {
			const NPT_XmlElementNode *dmsEl = rootEl->GetChild("DMS", "urn:schemas-netgear-com:dlnacore-1-0");
			if (dmsEl) {
				const NPT_String *dmsUuid = dmsEl->GetAttribute("uuid");
				if (dmsUuid) {
					m_dmsUuid = UUID::fromString(*dmsUuid);
					m_dms->setUuid(m_dmsUuid);
				}
				const NPT_String *systemUpdateId = dmsEl->GetAttribute("systemUpdateID");
				NPT_UInt32 suid;
				if (systemUpdateId && NPT_SUCCEEDED(NPT_ParseInteger(systemUpdateId->GetChars(), suid))) {
					mediaStore()->setSystemUpdateId(suid);
				}
			}

			const NPT_XmlElementNode *dmrEl = rootEl->GetChild("DMR", "urn:schemas-netgear-com:dlnacore-1-0");
			if (dmrEl) {
				const NPT_String *dmrUuid = dmrEl->GetAttribute("uuid");
				if (dmrUuid) {
					m_dmrUuid = UUID::fromString(*dmrUuid);
					m_dmr->setUuid(m_dmrUuid);
				}
			}
		}
	}
}

void DLNACoreImpl::saveConfig(NPT_OutputStream *outputStream)
{
	NPT_XmlSerializer xml(outputStream, 2, true, true);
	xml.StartDocument();
	xml.StartElement(NULL, "DLNACore");
	xml.Attribute(NULL, "xmlns", "urn:schemas-netgear-com:dlnacore-1-0");
	xml.StartElement(NULL, "DMS");
	xml.Attribute(NULL, "uuid", m_dmsUuid.toString());
	xml.Attribute(NULL, "systemUpdateID", NPT_String::FromIntegerU(mediaStore()->systemUpdateId()));
	xml.EndElement(NULL, "DMS");
	xml.StartElement(NULL, "DMR");
	xml.Attribute(NULL, "uuid", m_dmrUuid.toString());
	xml.EndElement(NULL, "DMR");
	xml.EndElement(NULL, "DLNACore");
	xml.EndDocument();
}

void DLNACoreImpl::setProperty(const NPT_String& name, const NPT_String& value)
{
	if (name.Compare("FriendlyName", true) == 0) {
		m_friendlyName = value;
		m_dms->setFriendlyName(m_friendlyName);
		m_dmr->setFriendlyName(m_friendlyName);
	} else if (name.Compare("DMRProtocolInfo", true) == 0) {
		m_dmr->setSinkProtocolInfo(value);
	} else if (name.Compare("OSVersion", true) == 0) {
		m_fe->setOSVersion(value);
	} else if (name.Compare("PlatformName", true) == 0) {
		m_platformName = value;
		m_dms->setPlatformName(m_platformName);
		m_dmr->setPlatformName(m_platformName);
	}
}

bool DLNACoreImpl::getproperty(const NPT_String& name, NPT_String& value) const
{
	if (name.Compare("FriendlyName", true) == 0) {
		value = m_friendlyName;
		return true;
	} else if (name.Compare("PlatformName", true) == 0) {
		value = m_platformName;
		return true;
	}
	return false;
}

bool DLNACoreImpl::start()
{
	WriteLocker locker(m_stateLock);
	if (m_started) {
		return false;
	}

	if (NPT_FAILED(m_fe->start())) {
		return false;
	}

	m_started = true;
	return true;
}

bool DLNACoreImpl::started() const
{
	ReadLocker locker(m_stateLock);
	return m_started;
}

void DLNACoreImpl::stop()
{
	{
		WriteLocker locker(m_stateLock);
		if (!m_started) {
			return;
		}
	}

	m_fe->stop();

	{
		WriteLocker locker(m_stateLock);
		m_started = false;
	}
}

void DLNACoreImpl::enableFunction(DLNACore::Function function, bool enable)
{
	switch (function) {
	case DLNACore::Function_ControlPoint:
		if (enable) {
			m_fe->addControlPoint(m_cp);
		} else {
			m_fe->removeControlPoint(m_cp);
		}
		break;
	case DLNACore::Function_MediaServer:
		if (enable) {
			m_fe->addDeviceImpl(m_dms);
		} else {
			m_fe->removeDeviceImpl(m_dms);
		}
		break;
	case DLNACore::Function_MediaRenderer:
		if (enable) {
			m_fe->addDeviceImpl(m_dmr);
		} else {
			m_fe->removeDeviceImpl(m_dmr);
		}
		break;
	}
}

bool DLNACoreImpl::isFunctionEnabled(DLNACore::Function function) const
{
	switch (function) {
	case DLNACore::Function_ControlPoint:
		return m_fe->hasControlPoint(m_cp);
	case DLNACore::Function_MediaServer:
		return m_fe->hasDeviceImpl(m_dms);
	case DLNACore::Function_MediaRenderer:
		return m_fe->hasDeviceImpl(m_dmr);
	}
	return false;
}

void DLNACoreImpl::internalEnableLoopback(bool enableLoopback)
{
	m_fe->setIncludeLoopback(enableLoopback);
}

void DLNACoreImpl::flushDeviceList(DLNACore::FlushMode flushMode)
{
	switch (flushMode) {
	case DLNACore::FlushMode_MediaRendererOnly:
		m_cp->forceRemoveDevicesByType("urn:schemas-upnp-org:device:MediaRenderer:*");
		break;
	case DLNACore::FlushMode_MediaServerOnly:
		m_cp->forceRemoveDevicesByType("urn:schemas-upnp-org:device:MediaServer:*");
		break;
	case DLNACore::FlushMode_All:
	default:
		m_cp->forceRemoveAllDevices();
		break;
	}
}

void DLNACoreImpl::searchDevices(NPT_UInt32 mx)
{
	m_cp->ssdpSearch("upnp:rootdevice", mx);
}

DeviceDescList DLNACoreImpl::snapshotMediaServerList() const
{
	ReadLocker locker(m_stateLock);
	DeviceDescList ls = m_dmsDescList;
	return ls;
}

DeviceDescList DLNACoreImpl::snapshotMediaRendererList() const
{
	ReadLocker locker(m_stateLock);
	DeviceDescList ls = m_dmrDescList;
	return ls;
}

NPT_Result DLNACoreImpl::queryStateVariables(const UUID& deviceUuid, const NPT_String& serviceId, const NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList)
{
	valueList.Clear();
	ReadLocker locker(m_stateLock);

	{
		NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(deviceUuid));
		if (it) {
			DMRInfo *dmrInfo = *it;
			if (dmrInfo->m_avt && serviceId.Compare(dmrInfo->m_avt->serviceId()) == 0) {
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					NPT_String *val;
					if (NPT_SUCCEEDED(dmrInfo->m_avtVars.Get(*nameList.GetItem(i), val))) {
						valueList.Add(*val);
					} else {
						valueList.Add(NPT_String());
					}
				}
				return NPT_SUCCESS;
			}

			if (dmrInfo->m_rcs && serviceId.Compare(dmrInfo->m_rcs->serviceId()) == 0) {
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					NPT_String *val;
					if (NPT_SUCCEEDED(dmrInfo->m_rcsVars.Get(*nameList.GetItem(i), val))) {
						valueList.Add(*val);
					} else {
						valueList.Add(NPT_String());
					}
				}
				return NPT_SUCCESS;
			}

			if (dmrInfo->m_cms && serviceId.Compare(dmrInfo->m_cms->serviceId()) == 0) {
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					NPT_String *val;
					if (NPT_SUCCEEDED(dmrInfo->m_cmsVars.Get(*nameList.GetItem(i), val))) {
						valueList.Add(*val);
					} else {
						valueList.Add(NPT_String());
					}
				}
				return NPT_SUCCESS;
			}

			return NPT_ERROR_NO_SUCH_ITEM;
		}
	}

	{
		NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(deviceUuid));
		if (it) {
			DMSInfo *dmsInfo = *it;
			if (dmsInfo->m_cds && serviceId.Compare(dmsInfo->m_cds->serviceId()) == 0) {
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					NPT_String *val;
					if (NPT_SUCCEEDED(dmsInfo->m_cdsVars.Get(*nameList.GetItem(i), val))) {
						valueList.Add(*val);
					} else {
						valueList.Add(NPT_String());
					}
				}
				return NPT_SUCCESS;
			}

			if (dmsInfo->m_cms && serviceId.Compare(dmsInfo->m_cms->serviceId()) == 0) {
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					NPT_String *val;
					if (NPT_SUCCEEDED(dmsInfo->m_cmsVars.Get(*nameList.GetItem(i), val))) {
						valueList.Add(*val);
					} else {
						valueList.Add(NPT_String());
					}
				}
				return NPT_SUCCESS;
			}

			return NPT_ERROR_NO_SUCH_ITEM;
		}
	}

	return NPT_ERROR_NO_SUCH_ITEM;
}

NPT_Result DLNACoreImpl::browseMediaServer(const UUID& mediaServerUuid, const NPT_String& containerId, DLNABrowseOp **op)
{
	*op = NULL;

	ReadLocker locker(m_stateLock);

	NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(mediaServerUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMSInfo *dmsInfo = *it;
	if (!dmsInfo->m_cds) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	NPT_List<NPT_String> inputArgs;
	inputArgs.Add(containerId);
	inputArgs.Add("BrowseDirectChildren");
	inputArgs.Add("*");
	inputArgs.Add("0");
	inputArgs.Add("0");
	inputArgs.Add("");
	DLNABrowseOpImpl *impl = new DLNABrowseOpImpl();
	impl->addRef();

	// addRef() for ActionCallback
	impl->addRef();

	NPT_Result nr = m_cp->invokeAction(mediaServerUuid, dmsInfo->m_cds->serviceId(), "Browse", inputArgs, impl, impl->m_actInst);
	if (NPT_FAILED(nr)) {
		impl->release();
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::browseMediaServerEx(const UUID& mediaServerUuid, const NPT_String& containerId, DLNABrowseOp **op)
{
	*op = NULL;

	ReadLocker locker(m_stateLock);

	NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(mediaServerUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMSInfo *dmsInfo = *it;
	if (!dmsInfo->m_cds) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNAFullBrowseOpImpl *impl = new DLNAFullBrowseOpImpl(NULL);
	impl->addRef();
	DLNAFullBrowseTask *task = new DLNAFullBrowseTask(impl, m_cp, mediaServerUuid, dmsInfo->m_cds->serviceId(), containerId);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::browseMediaServerEx(const UUID& mediaServerUuid, const NPT_String& containerId, NPT_UInt32 step, DLNAProgressiveBrowseOp::ResultCallback *resultCallback, DLNAProgressiveBrowseOp **op)
{
	*op = NULL;

	ReadLocker locker(m_stateLock);

	NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(mediaServerUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMSInfo *dmsInfo = *it;
	if (!dmsInfo->m_cds) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNAFullBrowseOpImpl *impl = new DLNAFullBrowseOpImpl(resultCallback);
	impl->addRef();
	DLNAFullBrowseTask *task = new DLNAFullBrowseTask(impl, m_cp, mediaServerUuid, dmsInfo->m_cds->serviceId(), containerId, step);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::browseMediaServerDeep(const UUID& mediaServerUuid, const NPT_String& containerId, DLNABrowseOp **op)
{
	*op = NULL;

	ReadLocker locker(m_stateLock);

	NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(mediaServerUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMSInfo *dmsInfo = *it;
	if (!dmsInfo->m_cds) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNADeepBrowseOpImpl *impl = new DLNADeepBrowseOpImpl();
	impl->addRef();
	DLNADeepBrowseTask *task = new DLNADeepBrowseTask(impl, m_cp, mediaServerUuid, dmsInfo->m_cds->serviceId(), containerId);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::playMedia(const UUID& mediaRendererUuid, const DLNAItem *mediaItem, DLNACoreOp **op)
{
	*op = NULL;
	DLNAResource *bestResource = NULL;
	if (mediaItem) {
		if (mediaItem->d_ptr()->m_resList.GetItemCount() > 0) {
			bestResource = *mediaItem->d_ptr()->m_resList.GetFirstItem();
		}

		if (!bestResource) {
			return NPT_ERROR_INVALID_FORMAT;
		}
	}

	NPT_String url;
	if (bestResource) {
		url = bestResource->d_ptr()->m_url;
	}

	NPT_String metaData;
	if (mediaItem) {
		metaData = mediaItem->d_ptr()->m_didlText;
	}

	m_cp->remapMediaURL(mediaRendererUuid, url);

	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAPlayMediaTask *task = new DLNAPlayMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId(), url, metaData);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::stopMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAStopMediaTask *task = new DLNAStopMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId());
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::pauseMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAPauseMediaTask *task = new DLNAPauseMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId());
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::prevMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAPrevMediaTask *task = new DLNAPrevMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId());
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::nextMedia(const UUID& mediaRendererUuid, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNANextMediaTask *task = new DLNANextMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId());
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::muteMedia(const UUID& mediaRendererUuid, bool mute, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAMuteMediaTask *task = new DLNAMuteMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_rcs->serviceId(), mute);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::changeMediaVolume(const UUID& mediaRendererUuid, int volume, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAChangeMediaVolumeTask *task = new DLNAChangeMediaVolumeTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_rcs->serviceId(), volume);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::seekMedia(const UUID& mediaRendererUuid, int timeInMillis, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNASeekMediaTask *task = new DLNASeekMediaTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId(), "REL_TIME", Helper::formatTrackDuration(timeInMillis));
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::queryMediaPositionInfo(const UUID& mediaRendererUuid, DLNAQueryPositionInfoOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNAQueryPositionInfoOpImpl *impl = new DLNAQueryPositionInfoOpImpl();
	impl->addRef();
	DLNAQueryPositionInfoTask *task = new DLNAQueryPositionInfoTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId());
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::playMediaList(const UUID& mediaRendererUuid, const NPT_List<DLNAObject*>& ls, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAPlayMediaListTask *task = new DLNAPlayMediaListTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId(), ls);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::playFile(const UUID& mediaRendererUuid, const NPT_String& path, DLNACoreOp **op)
{
	ReadLocker locker(m_stateLock);

	NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(mediaRendererUuid));
	if (!it) {
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	DMRInfo *dmrInfo = *it;
	if (!dmrInfo->m_avt) {
		return NPT_ERROR_NOT_SUPPORTED;
	}

	const char *placeholderStr = "HTTP_URL_PLACEHOLDER";
	NPT_String mimeType;
	NPT_String metaData;
	if (!mediaStore()->buildDidlForFile(path, placeholderStr, mimeType, metaData)) {
		return NPT_FAILURE;
	}

	DLNABaseMediaOpImpl *impl = new DLNABaseMediaOpImpl();
	impl->addRef();
	DLNAPlayFileTask *task = new DLNAPlayFileTask(impl, m_cp, mediaRendererUuid, dmrInfo->m_avt->serviceId(), path, mimeType, metaData, placeholderStr);
	NPT_Result nr = m_cp->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}

	*op = impl;
	return NPT_SUCCESS;
}

NPT_Result DLNACoreImpl::importPhotos(const NPT_String& name, DLNACoreOp **op)
{
	return mediaStore()->importPhotos(name, op);
}

void DLNACoreImpl::controlPointOnDeviceAdded(ControlPoint *controlPoint, DeviceDesc *deviceDesc, bool& subscribe)
{
	WriteLocker locker(m_stateLock);

	subscribe = false;

	const ServiceDesc *cds = deviceDesc->findServiceByType("urn:schemas-upnp-org:service:ContentDirectory:*");
	const ServiceDesc *cms = deviceDesc->findServiceByType("urn:schemas-upnp-org:service:ConnectionManager:*");
	const ServiceDesc *rcs = deviceDesc->findServiceByType("urn:schemas-upnp-org:service:RenderingControl:*");
	const ServiceDesc *avt = deviceDesc->findServiceByType("urn:schemas-upnp-org:service:AVTransport:*");

	if (cds && cms) {
		// Media Server
		subscribe = true;
		DMSInfo *dmsInfo = new DMSInfo();
		dmsInfo->m_cds = cds;
		dmsInfo->m_cms = cms;
		dmsInfo->m_avt = avt;
		deviceDesc->addRef();
		dmsInfo->m_deviceDesc = deviceDesc;

		for (NPT_Ordinal i = 0; i < cds->stateVariableCount(); i++) {
			const StateVariableDesc *varDesc = cds->stateVariableAt(i);
			if (!varDesc->name().StartsWith("A_ARG_TYPE_")) {
				dmsInfo->m_cdsVars.Put(varDesc->name(), varDesc->defaultValue());
			}
		}

		for (NPT_Ordinal i = 0; i < cms->stateVariableCount(); i++) {
			const StateVariableDesc *varDesc = cms->stateVariableAt(i);
			if (!varDesc->name().StartsWith("A_ARG_TYPE_")) {
				dmsInfo->m_cmsVars.Put(varDesc->name(), varDesc->defaultValue());
			}
		}

		m_dmsList.Add(dmsInfo);
		m_dmsDescList.add(deviceDesc);
		invokeDelegate(DelegateMethodIndex_onMediaServerListChanged);
	}

	if (rcs && cms && avt) {
		// Media Renderer
		subscribe = true;
		DMRInfo *dmrInfo = new DMRInfo();
		dmrInfo->m_rcs = rcs;
		dmrInfo->m_cms = cms;
		dmrInfo->m_avt = avt;
		deviceDesc->addRef();
		dmrInfo->m_deviceDesc = deviceDesc;

		for (NPT_Ordinal i = 0; i < rcs->stateVariableCount(); i++) {
			const StateVariableDesc *varDesc = rcs->stateVariableAt(i);
			if (!varDesc->name().StartsWith("A_ARG_TYPE_")) {
				dmrInfo->m_rcsVars.Put(varDesc->name(), varDesc->defaultValue());
			}
		}

		if (avt) {
			for (NPT_Ordinal i = 0; i < avt->stateVariableCount(); i++) {
				const StateVariableDesc *varDesc = avt->stateVariableAt(i);
				if (!varDesc->name().StartsWith("A_ARG_TYPE_")) {
					dmrInfo->m_avtVars.Put(varDesc->name(), varDesc->defaultValue());
				}
			}
		}

		for (NPT_Ordinal i = 0; i < cms->stateVariableCount(); i++) {
			const StateVariableDesc *varDesc = cms->stateVariableAt(i);
			if (!varDesc->name().StartsWith("A_ARG_TYPE_")) {
				dmrInfo->m_cmsVars.Put(varDesc->name(), varDesc->defaultValue());
			}
		}

		m_dmrList.Add(dmrInfo);
		m_dmrDescList.add(deviceDesc);
		invokeDelegate(DelegateMethodIndex_onMediaRendererListChanged);
	}
}

void DLNACoreImpl::controlPointOnDeviceRemoved(ControlPoint *controlPoint, DeviceDesc *deviceDesc)
{
	WriteLocker locker(m_stateLock);

	{
		NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(deviceDesc->uuid()));
		if (it) {
			DMSInfo *dmsInfo = *it;
			m_dmsList.Erase(it);
			delete dmsInfo;
			m_dmsDescList.remove(deviceDesc->uuid());
			invokeDelegate(DelegateMethodIndex_onMediaServerListChanged);
		}
	}

	{
		NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(deviceDesc->uuid()));
		if (it) {
			DMRInfo *dmrInfo = *it;
			m_dmrList.Erase(it);
			delete dmrInfo;
			m_dmrDescList.remove(deviceDesc->uuid());
			invokeDelegate(DelegateMethodIndex_onMediaRendererListChanged);
		}
	}
}

#if 0

void parseLastChangeAVT(const NPT_String& text)
{
	NPT_Result nr;
	do {
		NPT_XmlNode *rootNode;
		NPT_XmlParser parser;
		nr = parser.Parse(text, text.GetLength(), rootNode);
		if (NPT_FAILED(nr)) {
			break;
		}

		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (!rootEl) {
			nr = NPT_FAILURE;
			break;
		}

		if (!matchTagNamespace(rootEl, "Event", "urn:schemas-upnp-org:metadata-1-0/AVT/")) {
			nr = NPT_FAILURE;
			break;
		}

		NPT_XmlElementNode *instEl = NULL;
		for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
				if (matchTagNamespace(el, "InstanceID", "urn:schemas-upnp-org:metadata-1-0/AVT/")) {
					const NPT_String *attrVal = el->GetAttribute("val");
					if (attrVal && attrVal->Compare("0") == 0) {
						instEl = el;
						break;
					}
				}
			}
		}

		if (!instEl) {
			nr = NPT_FAILURE;
			break;
		}

		for (NPT_Ordinal i = 0; i < instEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*instEl->GetChildren().GetItem(i))->AsElementNode()) {
				const NPT_String& tagName = el->GetTag();
				if (tagName.Compare("TransportState") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("TransportStatus") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("PlaybackStorageMedium") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("RecordStorageMedium") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("PossiblePlaybackStorageMedia") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("PossibleRecordStorageMedia") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentPlayMode") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("TransportPlaySpeed") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("RecordMediumWriteStatus") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentRecordQualityMode") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("PossibleRecordQualityModes") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("NumberOfTracks") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentTrack") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentTrackDuration") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentMediaDuration") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentTrackMetaData") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentTrackURI") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("AVTransportURI") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("AVTransportURIMetaData") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("NextAVTransportURI") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("NextAVTransportURIMetaData") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("RelativeTimePosition") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("AbsoluteTimePosition") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("RelativeCounterPosition") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("AbsoluteCounterPosition") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("CurrentTransportActions") == 0) {
					qDebug() << 5;
				} else {
					qDebug() << 5;
				}
			}
		}

	} while (false);
}

void parseLastChangeRCS(const NPT_String& text)
{
	NPT_Result nr;
	do {
		NPT_XmlNode *rootNode;
		NPT_XmlParser parser;
		nr = parser.Parse(text, text.GetLength(), rootNode);
		if (NPT_FAILED(nr)) {
			break;
		}

		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (!rootEl) {
			nr = NPT_FAILURE;
			break;
		}

		if (!matchTagNamespace(rootEl, "Event", "urn:schemas-upnp-org:metadata-1-0/RCS/")) {
			nr = NPT_FAILURE;
			break;
		}

		NPT_XmlElementNode *instEl = NULL;
		for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
				if (matchTagNamespace(el, "InstanceID", "urn:schemas-upnp-org:metadata-1-0/RCS/")) {
					const NPT_String *attrVal = el->GetAttribute("val");
					if (attrVal && attrVal->Compare("0") == 0) {
						instEl = el;
						break;
					}
				}
			}
		}

		if (!instEl) {
			nr = NPT_FAILURE;
			break;
		}

		for (NPT_Ordinal i = 0; i < instEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*instEl->GetChildren().GetItem(i))->AsElementNode()) {
				const NPT_String& tagName = el->GetTag();
				if (tagName.Compare("PresetNameList") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("Brightness") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("Contrast") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("Sharpness") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("RedVideoGain") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("GreenVideoGain") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("BlueVideoGain") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("RedVideoBlackLevel") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("GreenVideoBlackLevel") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("BlueVideoBlackLevel") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("ColorTemperature") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("HorizontalKeystone") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("VerticalKeystone") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("Mute") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("Volume") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("VolumeDB") == 0) {
					qDebug() << 5;
				} else if (tagName.Compare("Loudness") == 0) {
					qDebug() << 5;
				} else {
					qDebug() << 5;
				}
			}
		}

	} while (false);
}

#endif

void DLNACoreImpl::parseLastChangeAVT(DMRInfo *dmrInfo, const NPT_String& text, NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList)
{
	NPT_Result nr;
	do {
		NPT_XmlNode *rootNode;
		NPT_XmlParser parser;
		nr = parser.Parse(text, text.GetLength(), rootNode);
		if (NPT_FAILED(nr)) {
			break;
		}

		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (!rootEl) {
			nr = NPT_FAILURE;
			break;
		}

		if (!matchTagNamespace(rootEl, "Event", "urn:schemas-upnp-org:metadata-1-0/AVT/")) {
			nr = NPT_FAILURE;
			break;
		}

		NPT_XmlElementNode *instEl = NULL;
		for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
				if (matchTagNamespace(el, "InstanceID", "urn:schemas-upnp-org:metadata-1-0/AVT/")) {
					const NPT_String *attrVal = el->GetAttribute("val");
					if (attrVal && attrVal->Compare("0") == 0) {
						instEl = el;
						break;
					}
				}
			}
		}

		if (!instEl) {
			nr = NPT_FAILURE;
			break;
		}

		for (NPT_Ordinal i = 0; i < instEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*instEl->GetChildren().GetItem(i))->AsElementNode()) {
				const NPT_String& tagName = el->GetTag();
				const NPT_String *attrVal = el->GetAttribute("val");
				NPT_String val = attrVal ? *attrVal : NPT_String();
				dmrInfo->m_avtVars.Put(tagName, val);
				nameList.Add(tagName);
				valueList.Add(val);
				NPT_LOG_INFO_2("[%s]=[%s]",tagName.GetChars(), val.GetChars());
			}
		}

	} while (false);
}

void DLNACoreImpl::parseLastChangeRCS(DMRInfo *dmrInfo, const NPT_String& text, NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList)
{
	NPT_Result nr;
	do {
		NPT_XmlNode *rootNode;
		NPT_XmlParser parser;
		nr = parser.Parse(text, text.GetLength(), rootNode);
		if (NPT_FAILED(nr)) {
			break;
		}

		PtrHolder<NPT_XmlNode> rootNode1(rootNode);
		const NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
		if (!rootEl) {
			nr = NPT_FAILURE;
			break;
		}

		if (!matchTagNamespace(rootEl, "Event", "urn:schemas-upnp-org:metadata-1-0/RCS/")) {
			nr = NPT_FAILURE;
			break;
		}

		NPT_XmlElementNode *instEl = NULL;
		for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
				if (matchTagNamespace(el, "InstanceID", "urn:schemas-upnp-org:metadata-1-0/RCS/")) {
					const NPT_String *attrVal = el->GetAttribute("val");
					if (attrVal && attrVal->Compare("0") == 0) {
						instEl = el;
						break;
					}
				}
			}
		}

		if (!instEl) {
			nr = NPT_FAILURE;
			break;
		}

		for (NPT_Ordinal i = 0; i < instEl->GetChildren().GetItemCount(); i++) {
			if (NPT_XmlElementNode *el = (*instEl->GetChildren().GetItem(i))->AsElementNode()) {
				const NPT_String& tagName = el->GetTag();
				const NPT_String *attrVal = el->GetAttribute("val");
				NPT_String val = attrVal ? *attrVal : NPT_String();
				dmrInfo->m_rcsVars.Put(tagName, val);
				nameList.Add(tagName);
				valueList.Add(val);

				NPT_LOG_INFO_2("[%s]=[%s]",tagName.GetChars(), val.GetChars());

				NPT_Map<NPT_String, NPT_String> *channelMap = NULL;
				if (tagName.Compare("Mute") == 0) {
					channelMap = &dmrInfo->m_rcsMuteVars;
				} else if (tagName.Compare("Volume") == 0) {
					channelMap = &dmrInfo->m_rcsVolumeVars;
				} else if (tagName.Compare("VolumeDB") == 0) {
					channelMap = &dmrInfo->m_rcsVolumeDBVars;
				} else if (tagName.Compare("Loudness") == 0) {
					channelMap = &dmrInfo->m_rcsLoudnessVars;
				}
				if (channelMap) {
					const NPT_String *attrChannel = el->GetAttribute("Channel");
					if (!attrChannel) {
						attrChannel = el->GetAttribute("channel");
					}
					if (attrChannel) {
						channelMap->Put(*attrChannel, attrVal ? *attrVal : NPT_String());
					}
				}
			}
		}
	} while (false);
}

void DLNACoreImpl::controlPointOnStateVariablesChanged(ControlPoint *controlPoint, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
{
	ReadLocker locker(m_stateLock);

	{
		NPT_List<DMSInfo*>::Iterator it = m_dmsList.Find(DMSInfoFinder(deviceDesc->uuid()));
		if (it) {
			DMSInfo *dmsInfo = *it;
			NPT_Map<NPT_String, NPT_String> *varMap = NULL;
			if (dmsInfo->m_cms && serviceDesc->serviceId().Compare(dmsInfo->m_cms->serviceId()) == 0) {
				varMap = &dmsInfo->m_cmsVars;
			} else if (dmsInfo->m_cds && serviceDesc->serviceId().Compare(dmsInfo->m_cds->serviceId()) == 0) {
				varMap = &dmsInfo->m_cdsVars;
			}
			if (varMap) {
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					varMap->Put(*nameList.GetItem(i), *valueList.GetItem(i));
				}
				invokeDelegate(DelegateMethodIndex_onMediaServerStateVariablesChanged, deviceDesc, serviceDesc, nameList, valueList);
			}
		}
	}

	{
		NPT_List<DMRInfo*>::Iterator it = m_dmrList.Find(DMRInfoFinder(deviceDesc->uuid()));
		if (it) {
			DMRInfo *dmrInfo = *it;
			NPT_Map<NPT_String, NPT_String> *varMap = NULL;
			if (dmrInfo->m_cms && serviceDesc->serviceId().Compare(dmrInfo->m_cms->serviceId()) == 0) {
				varMap = &dmrInfo->m_cmsVars;
			} else if (dmrInfo->m_rcs && serviceDesc->serviceId().Compare(dmrInfo->m_rcs->serviceId()) == 0) {
				varMap = &dmrInfo->m_rcsVars;
			} else if (dmrInfo->m_avt && serviceDesc->serviceId().Compare(dmrInfo->m_avt->serviceId()) == 0) {
				varMap = &dmrInfo->m_avtVars;
			}
			if (varMap) {
				NPT_List<NPT_String> nameList1;
				NPT_List<NPT_String> valueList1;
				for (NPT_Ordinal i = 0; i < nameList.GetItemCount(); i++) {
					if (nameList.GetItem(i)->Compare("LastChange") == 0) {
						if (serviceDesc->serviceId().Compare(dmrInfo->m_avt->serviceId()) == 0) {
							parseLastChangeAVT(dmrInfo, *valueList.GetItem(i), nameList1, valueList1);
						} else if (serviceDesc->serviceId().Compare(dmrInfo->m_rcs->serviceId()) == 0) {
							parseLastChangeRCS(dmrInfo, *valueList.GetItem(i), nameList1, valueList1);
						}
					} else {
						NPT_LOG_INFO_2("[%s]=[%s]", nameList.GetItem(i)->GetChars(), valueList.GetItem(i)->GetChars());
						varMap->Put(*nameList.GetItem(i), *valueList.GetItem(i));
						nameList1.Add(*nameList.GetItem(i));
						valueList1.Add(*valueList.GetItem(i));
					}
				}
				invokeDelegate(DelegateMethodIndex_onMediaRendererStateVariablesChanged, deviceDesc, serviceDesc, nameList1, valueList1);
			}
		}
	}
}

void DLNACoreImpl::doDmrOpen(SimpleDMR *dmr, const NPT_String& url, const NPT_String& mimeType, const NPT_String& metaData)
{
	m_delegate->dmrOpen(url, mimeType, metaData);
}

void DLNACoreImpl::doDmrPlay(SimpleDMR *dmr)
{
	m_delegate->dmrPlay();
}

void DLNACoreImpl::doDmrPause(SimpleDMR *dmr)
{
	m_delegate->dmrPause();
}

void DLNACoreImpl::doDmrStop(SimpleDMR *dmr)
{
	m_delegate->dmrStop();
}

void DLNACoreImpl::doDmrSeekTo(SimpleDMR *dmr, NPT_Int64 pos)
{
	m_delegate->dmrSeekTo(pos);
}

void DLNACoreImpl::doDmrSetMute(SimpleDMR *dmr, bool mute)
{
	m_delegate->dmrSetMute(mute);
}

void DLNACoreImpl::doDmrSetVolume(SimpleDMR *dmr, int volume)
{
	m_delegate->dmrSetVolume(volume);
}

void DLNACoreImpl::dmrReportState(DLNACore::DMRState state)
{
	const char *ss;
	switch (state) {
	case DLNACore::DMRState_Stopped:
		ss = "STOPPED";
		break;
	case DLNACore::DMRState_Playing:
		ss = "PLAYING";
		break;
	case DLNACore::DMRState_Loading:
		ss = "TRANSITIONING";
		break;
	case DLNACore::DMRState_Paused:
		ss = "PAUSED_PLAYBACK";
		break;
	case DLNACore::DMRState_NoMediaPresent:
	default:
		ss = "NO_MEDIA_PRESENT";
		break;
	}
	m_dmr->setVar(DMRVAR_TransportState, ss);
}

void DLNACoreImpl::dmrReportErrorStatus(bool error)
{
	m_dmr->setVar(DMRVAR_TransportStatus, error ? "ERROR_OCCURRED" : "OK");
}

void DLNACoreImpl::dmrReportProgress(NPT_Int64 currentMillis, NPT_Int64 totalMillis)
{
	m_dmr->setVar(DMRVAR_CurrentTrack, "1");
	m_dmr->setVar(DMRVAR_NumberOfTracks, "1");
	NPT_String durationStr = Helper::formatTrackDuration(totalMillis);
	m_dmr->setVar(DMRVAR_CurrentTrackDuration, durationStr);
	m_dmr->setVar(DMRVAR_CurrentMediaDuration, durationStr);
	NPT_String posStr = Helper::formatTrackDuration(currentMillis);
	m_dmr->setVar(DMRVAR_RelativeTimePosition, posStr);
}

void DLNACoreImpl::dmrReportVolume(int volume, bool muted)
{
	m_dmr->setVar(DMRVAR_Mute, muted ? "1" : "0");
	m_dmr->setVar(DMRVAR_Volume, NPT_String::FromInteger(volume));
}

const UUID& DLNACoreImpl::getMediaServerUuid() const
{
	return m_dms->uuid();
}

const UUID& DLNACoreImpl::getMediaRendererUuid() const
{
	return m_dmr->uuid();
}

void DLNACoreImpl::invokeDelegate(int index)
{
	if (m_delegate) {
		m_taskGroup->startTask(new InvokeDelegateMethodTask(m_delegate, index));
	}
}

void DLNACoreImpl::invokeDelegate(int index, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
{
	if (m_delegate) {
		m_taskGroup->startTask(new InvokeDelegateMethodTask(m_delegate, index, deviceDesc, serviceDesc, nameList, valueList));
	}
}

InvokeDelegateMethodTask::InvokeDelegateMethodTask(DLNACoreDelegate *d, int methodIndex)
	: m_delegate(d), m_methodIndex(methodIndex), m_deviceDesc(NULL)
{
}

InvokeDelegateMethodTask::InvokeDelegateMethodTask(DLNACoreDelegate *d, int methodIndex, DeviceDesc *deviceDesc, ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
	: m_delegate(d), m_methodIndex(methodIndex), m_deviceDesc(deviceDesc), m_serviceDesc(serviceDesc), m_nameList(nameList), m_valueList(valueList)
{
	m_deviceDesc->addRef();
}

InvokeDelegateMethodTask::~InvokeDelegateMethodTask()
{
	if (m_deviceDesc) {
		m_deviceDesc->release();
	}
}

void InvokeDelegateMethodTask::exec()
{
	switch (m_methodIndex) {
	case DLNACoreImpl::DelegateMethodIndex_onMediaServerListChanged:
		m_delegate->onMediaServerListChanged();
		break;
	case DLNACoreImpl::DelegateMethodIndex_onMediaRendererListChanged:
		m_delegate->onMediaRendererListChanged();
		break;
	case DLNACoreImpl::DelegateMethodIndex_onMediaServerStateVariablesChanged:
		m_delegate->onMediaServerStateVariablesChanged(m_deviceDesc, m_serviceDesc, m_nameList, m_valueList);
		break;
	case DLNACoreImpl::DelegateMethodIndex_onMediaRendererStateVariablesChanged:
		m_delegate->onMediaRendererStateVariablesChanged(m_deviceDesc, m_serviceDesc, m_nameList, m_valueList);
		break;
	}
}

} // namespace deejay
