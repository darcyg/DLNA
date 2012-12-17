#include "DLNACoreOpImpl.h"
#include "DLNAObjectImpl.h"

namespace deejay {

//------------------------------------------------------------------------------
// DLNABrowseOpImpl
//------------------------------------------------------------------------------

DLNABrowseOpImpl::DLNABrowseOpImpl()
	: m_refCount(0), m_waitVar(0), m_abortFlag(false), m_finished(false), m_finishCallback(NULL)
	, m_actInst(NULL), m_succeeded(false), m_hasFaultDetail(false), m_errorCode(0)
{
}

DLNABrowseOpImpl::~DLNABrowseOpImpl()
{
	if (m_actInst) {
		m_actInst->release();
	}
}

int DLNABrowseOpImpl::addRef()
{
	return m_refCount.Increment();
}

int DLNABrowseOpImpl::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

void DLNABrowseOpImpl::abort()
{
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		m_abortFlag = true;
		m_actInst->abort();
	}
}

bool DLNABrowseOpImpl::aborted() const
{
	ReadLocker locker(m_stateLock);
	return m_abortFlag;
}

NPT_Result DLNABrowseOpImpl::wait(NPT_Timeout timeout)
{
	return m_waitVar.WaitWhileEquals(0, timeout);
}

bool DLNABrowseOpImpl::checkFinishedIfNotSetCallback(FinishCallback *callback)
{
	WriteLocker locker(m_stateLock);
	if (m_finished) {
		return true;
	}
	m_finishCallback = callback;
	return false;
}

bool DLNABrowseOpImpl::resetCallback()
{
	WriteLocker locker(m_stateLock);
	m_finishCallback = NULL;
	return m_finished;
}

bool DLNABrowseOpImpl::succeeded() const
{
	return m_succeeded;
}

bool DLNABrowseOpImpl::hasFaultDetail() const
{
	return m_hasFaultDetail;
}

int DLNABrowseOpImpl::errorCode() const
{
	return m_errorCode;
}

const NPT_String& DLNABrowseOpImpl::errorDesc() const
{
	return m_errorDesc;
}

const DLNAObjectList& DLNABrowseOpImpl::objectList() const
{
	return m_result;
}

void DLNABrowseOpImpl::onActionFinished(ActionInstance *instance)
{
	if (NPT_SUCCEEDED(instance->statusCode())) {
		if (instance->errorCode() == 0) {
			if (parseResult(instance->outputValues())) {
				m_succeeded = true;
			} else {
				m_succeeded = false;
				m_hasFaultDetail = false;
			}
		} else {
			m_succeeded = false;
			m_hasFaultDetail = true;
			m_errorCode = instance->errorCode();
			m_errorDesc = instance->errorDescription();
		}
	} else {
		m_succeeded = false;
		m_hasFaultDetail = false;
	}

	{
		WriteLocker locker(m_stateLock);
		if (m_finishCallback) {
			m_finishCallback->onDLNACoreOpFinished(this);
		}
		m_finished = true;
		m_waitVar.SetValue(1);
	}
	release();
}

DLNAPeopleInvolved parseDLNAPeopleInvolved(const NPT_XmlElementNode *el)
{
	DLNAPeopleInvolved pi;
	pi.m_name = getElementText(el);
	const NPT_String *attr = el->GetAttribute("role");
	if (attr) {
		pi.m_role = *attr;
	}
	return pi;
}

void parseDLNAProtocolInfo(DLNAResourceImpl *resImpl, const NPT_String& text)
{
	NPT_List<NPT_String> parts = text.Split(":");
	if (parts.GetItemCount() == 4) {
		const NPT_String& protocol = *parts.GetItem(0);
		NPT_String network = *parts.GetItem(1);
		NPT_String contentFormat = *parts.GetItem(2);
		NPT_String additionalInfo = *parts.GetItem(3);

		resImpl->m_piProtocol = protocol;
		resImpl->m_piNetwork = network;
		resImpl->m_piContentFormat = contentFormat;
		resImpl->m_piAdditionalInfo = additionalInfo;

		if (protocol.Compare("http-get") == 0) {
			resImpl->m_mimeType = contentFormat;
		} else if (protocol.Compare("rtsp-rtp-udp") == 0) {
		} else {
			//NPT_System::GetRandomInteger();
		}
		if (network.Compare("*") == 0) {
		} else {
			//NPT_System::GetRandomInteger();
		}
		if (additionalInfo.Compare("*") == 0) {
			//NPT_System::GetRandomInteger();
		} else {
			NPT_List<NPT_String> parts2 = additionalInfo.Split(";");
			for (NPT_Ordinal i = 0; i < parts2.GetItemCount(); i++) {
				const NPT_String& item = *parts2.GetItem(i);
				int sep = item.Find('=');
				if (sep > 0) {
					NPT_String field = item.Left(sep);
					if (field.Compare("DLNA.ORG_PS") == 0) {
						//NPT_System::GetRandomInteger();
					} else if (field.Compare("DLNA.ORG_PN") == 0) {
						//NPT_System::GetRandomInteger();
					} else if (field.Compare("DLNA.ORG_CI") == 0) {
						//NPT_System::GetRandomInteger();
					} else if (field.Compare("DLNA.ORG_OP") == 0) {
						//NPT_System::GetRandomInteger();
					} else if (field.Compare("DLNA.ORG_FLAGS") == 0) {
						//NPT_System::GetRandomInteger();
					} else if (field.Compare("DLNA.ORG_MAXSP") == 0) {
						//NPT_System::GetRandomInteger();
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else {
					//NPT_System::GetRandomInteger();
				}
			}
		}
	} else {
		//NPT_System::GetRandomInteger();
	}
}

DLNAResource *parseDLNAResource(const NPT_XmlElementNode *el)
{
	DLNAResource *res = new DLNAResource();
	DLNAResourceImpl *d = res->d_ptr();
	for (NPT_Ordinal i = 0; i < el->GetAttributes().GetItemCount(); i++) {
		const NPT_XmlAttribute *attr = *el->GetAttributes().GetItem(i);
		const NPT_String& prefix = attr->GetPrefix();
		const NPT_String& attrName = attr->GetName();
		const NPT_String& attrValue = attr->GetValue();
		const NPT_String *ns;
		if (prefix.IsEmpty()) {
			ns = el->GetNamespace();
		} else {
			ns = el->GetNamespaceUri(prefix);
		}

		if (!ns) {
			continue;
		}

		if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/") == 0) {
			if (attrName.Compare("protocolInfo") == 0) {
				d->m_protocolInfoStr = attrValue;
				parseDLNAProtocolInfo(d, attrValue);
			} else if (attrName.Compare("duration") == 0) {
				d->m_durationStr = attrValue;
			} else if (attrName.Compare("size") == 0) {
				NPT_ParseInteger64(attrValue, d->m_size);
			} else if (attrName.Compare("bitrate") == 0) {
				NPT_ParseInteger(attrValue, d->m_bitrate);
			} else if (attrName.Compare("sampleFrequency") == 0) {
				NPT_ParseInteger(attrValue, d->m_sampleFrequency);
			} else if (attrName.Compare("bitsPerSample") == 0) {
				NPT_ParseInteger(attrValue, d->m_bitsPerSample);
			} else if (attrName.Compare("nrAudioChannels") == 0) {
				NPT_ParseInteger(attrValue, d->m_nrAudioChannels);
			} else if (attrName.Compare("resolution") == 0) {
				parseResolution(attrValue, d->m_resolutionX, d->m_resolutionY);
			} else if (attrName.Compare("colorDepth") == 0) {
				NPT_ParseInteger(attrValue, d->m_colorDepth);
			} else {
				//NPT_System::GetRandomInteger();
			}
		} else if (ns->Compare("urn:schemas-microsoft-com:WMPNSS-1-0/") == 0) {
		} else {
			//NPT_System::GetRandomInteger();
		}
	}
	d->m_url = getElementText(el);
	return res;
}

DLNAItem *parseDLNAItem(const NPT_XmlElementNode *el)
{
	DLNAItem *obj = DLNAItemImpl::newDLNAItem();
	DLNAItemImpl *d = obj->d_ptr();
	for (NPT_Ordinal i = 0; i < el->GetAttributes().GetItemCount(); i++) {
		const NPT_XmlAttribute *attr = *el->GetAttributes().GetItem(i);
		const NPT_String& prefix = attr->GetPrefix();
		const NPT_String& attrName = attr->GetName();
		const NPT_String& attrValue = attr->GetValue();
		const NPT_String *ns;
		if (prefix.IsEmpty()) {
			ns = el->GetNamespace();
		} else {
			ns = el->GetNamespaceUri(prefix);
		}

		if (!ns) {
			continue;
		}

		if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/") == 0) {
			if (attrName.Compare("id") == 0) {
				d->m_objectId = attrValue;
			} else if (attrName.Compare("parentID") == 0) {
				d->m_parentId = attrValue;
			} else if (attrName.Compare("restricted") == 0) {
				d->m_restricted = parseBoolean(attrValue);
			} else if (attrName.Compare("refID") == 0) {
				d->m_refId = attrValue;
			} else {
				//NPT_System::GetRandomInteger();
			}
		} else {
			//NPT_System::GetRandomInteger();
		}
	}

	for (NPT_Ordinal i = 0; i < el->GetChildren().GetItemCount(); i++) {
		if (const NPT_XmlElementNode *childEl = (*el->GetChildren().GetItem(i))->AsElementNode()) {
			const NPT_String *ns = childEl->GetNamespace();
			const NPT_String& tagName = childEl->GetTag();
			NPT_String text = getElementText(childEl);
			if (ns) {
				if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/") == 0) { // DIDL-Lite
					if (tagName.Compare("res") == 0) {
						d->m_resList.Add(parseDLNAResource(childEl));
					} else if (tagName.Compare("desc") == 0) {
						// nothing to do
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/upnp/") == 0) { // upnp
					if (tagName.Compare("class") == 0) {
						d->m_upnpClass = text;
						if (const NPT_String *attr = childEl->GetAttribute("name")) {
							d->m_upnpClassName = *attr;
						}
					} else if (tagName.Compare("writeStatus") == 0) {
						d->m_writeStatus = text;
					} else if (tagName.Compare("genre") == 0) {
						d->m_genreList.Add(text);
					} else if (tagName.Compare("album") == 0) {
						d->m_albumList.Add(text);
					} else if (tagName.Compare("albumArtURI") == 0) {
						d->m_albumArtURIList.Add(text);
					} else if (tagName.Compare("artist") == 0) {
						d->m_artistList.Add(parseDLNAPeopleInvolved(childEl));
					} else if (tagName.Compare("author") == 0) {
						d->m_authorList.Add(parseDLNAPeopleInvolved(childEl));
					} else if (tagName.Compare("actor") == 0) {
						d->m_actorList.Add(parseDLNAPeopleInvolved(childEl));
					} else if (tagName.Compare("originalTrackNumber") == 0) {
						NPT_ParseInteger(text, d->m_originalTrackNumber);
					} else if (tagName.Compare("channelName") == 0) {
						d->m_channelName = text;
					} else if (tagName.Compare("scheduledStartTime") == 0) {
						d->m_scheduledStartTimeStr = text;
					} else if (tagName.Compare("longDescription") == 0) {
						d->m_longDescription = text;
					} else if (tagName.Compare("rating") == 0) {
						d->m_rating = text;
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else if (ns->Compare("http://purl.org/dc/elements/1.1/") == 0) { // dc
					if (tagName.Compare("title") == 0) {
						d->m_title = text;
					} else if (tagName.Compare("creator") == 0) {
						d->m_creator = text;
					} else if (tagName.Compare("date") == 0) {
						d->m_dateStr = text;
					} else if (tagName.Compare("publisher") == 0) {
						d->m_publisherList.Add(text);
					} else if (tagName.Compare("rights") == 0) {
						d->m_rightsList.Add(text);
					} else if (tagName.Compare("description") == 0) {
						d->m_description = text;
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else {
					//NPT_System::GetRandomInteger();
				}
			} else {
				//NPT_System::GetRandomInteger();
			}
		}
	}

	if (d->m_resList.GetItemCount() > 0) {
		d->m_resourceSize = (*d->m_resList.GetFirstItem())->d_ptr()->m_size;
	}

	return obj;
}

DLNAContainer *parseDLNAContainer(const NPT_XmlElementNode *el)
{
	DLNAContainer *obj = DLNAContainerImpl::newDLNAContainer();
	DLNAContainerImpl *d = obj->d_ptr();
	for (NPT_Ordinal i = 0; i < el->GetAttributes().GetItemCount(); i++) {
		const NPT_XmlAttribute *attr = *el->GetAttributes().GetItem(i);
		const NPT_String& prefix = attr->GetPrefix();
		const NPT_String& attrName = attr->GetName();
		const NPT_String& attrValue = attr->GetValue();
		const NPT_String *ns;
		if (prefix.IsEmpty()) {
			ns = el->GetNamespace();
		} else {
			ns = el->GetNamespaceUri(prefix);
		}

		if (!ns) {
			continue;
		}

		if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/") == 0) {
			if (attrName.Compare("id") == 0) {
				d->m_objectId = attrValue;
			} else if (attrName.Compare("parentID") == 0) {
				d->m_parentId = attrValue;
			} else if (attrName.Compare("restricted") == 0) {
				d->m_restricted = parseBoolean(attrValue);
			} else if (attrName.Compare("childCount") == 0) {
				NPT_ParseInteger(attrValue, d->m_childCount);
			} else if (attrName.Compare("searchable") == 0) {
				d->m_searchable = parseBoolean(attrValue);
			} else {
				//NPT_System::GetRandomInteger();
			}
		} else {
			//NPT_System::GetRandomInteger();
		}
	}

	for (NPT_Ordinal i = 0; i < el->GetChildren().GetItemCount(); i++) {
		if (const NPT_XmlElementNode *childEl = (*el->GetChildren().GetItem(i))->AsElementNode()) {
			const NPT_String *ns = childEl->GetNamespace();
			const NPT_String& tagName = childEl->GetTag();
			NPT_String text = getElementText(childEl);
			if (ns) {
				if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/") == 0) { // DIDL-Lite
					if (tagName.Compare("res") == 0) {
						d->m_resList.Add(parseDLNAResource(childEl));
					} else if (tagName.Compare("desc") == 0) {
						// nothing to do
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else if (ns->Compare("urn:schemas-upnp-org:metadata-1-0/upnp/") == 0) { // upnp
					if (tagName.Compare("class") == 0) {
						d->m_upnpClass = text;
						if (const NPT_String *attr = childEl->GetAttribute("name")) {
							d->m_upnpClassName = *attr;
						}
					} else if (tagName.Compare("writeStatus") == 0) {
						d->m_writeStatus = text;
					} else if (tagName.Compare("searchClass") == 0) {
						// TODO:
					} else if (tagName.Compare("genre") == 0) {
						d->m_genreList.Add(text);
					} else if (tagName.Compare("album") == 0) {
						d->m_albumList.Add(text);
					} else if (tagName.Compare("albumArtURI") == 0) {
						d->m_albumArtURIList.Add(text);
					} else if (tagName.Compare("artist") == 0) {
						d->m_artistList.Add(parseDLNAPeopleInvolved(childEl));
					} else if (tagName.Compare("author") == 0) {
						d->m_authorList.Add(parseDLNAPeopleInvolved(childEl));
					} else if (tagName.Compare("actor") == 0) {
						d->m_actorList.Add(parseDLNAPeopleInvolved(childEl));
					} else if (tagName.Compare("storageUsed") == 0) {
						NPT_ParseInteger64(text, d->m_storageUsed);
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else if (ns->Compare("http://purl.org/dc/elements/1.1/") == 0) { // dc
					if (tagName.Compare("title") == 0) {
						d->m_title = text;
					} else if (tagName.Compare("creator") == 0) {
						d->m_creator = text;
					} else if (tagName.Compare("date") == 0) {
						d->m_dateStr = text;
					} else {
						//NPT_System::GetRandomInteger();
					}
				} else {
					//NPT_System::GetRandomInteger();
				}
			} else {
				//NPT_System::GetRandomInteger();
			}
		}
	}

	return obj;
}

static NPT_String regenerateDidl(NPT_XmlElementNode *rootEl, NPT_XmlElementNode *el)
{
	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, false, true);
	xml.StartElement(rootEl->GetPrefix(), rootEl->GetTag());
/*
	const NPT_XmlNamespaceMap *nsMap = rootEl->GetNamespaceMap();
	if (nsMap) {
		const NPT_List<NPT_XmlNamespaceMap::Entry*>& nsls = nsMap->m_Entries;
		for (NPT_Ordinal i = 0; i < nsls.GetItemCount(); i++) {
			const NPT_XmlNamespaceMap::Entry *e = *nsls.GetItem(i);
			if (e->m_Prefix.IsEmpty()) {
				xml.Attribute("", "xmlns", e->m_Uri);
			} else {
				xml.Attribute("xmlns", e->m_Prefix, e->m_Uri);
			}
		}
	}

	const NPT_List<NPT_XmlAttribute*>& attrList = rootEl->GetAttributes();
	for (NPT_Ordinal i = 0; i < attrList.GetItemCount(); i++) {
		NPT_XmlAttribute *attr = *attrList.GetItem(i);
		xml.Attribute(attr->GetPrefix(), attr->GetName(), attr->GetValue());
	}
	
	const char *plh = "placeholder_for_node";

	xml.Text(plh);
	xml.EndElement(rootEl->GetPrefix(), rootEl->GetTag());
	NPT_String result = strm.GetString();

	NPT_XmlWriter w;
	NPT_StringOutputStream strm2;
	w.Serialize(*el, strm2, false);

	result.Replace(plh, strm2.GetString());*/
	NPT_String result = strm.GetString();
	return result;
}

bool DLNABrowseOpImpl::parseResult(const NPT_List<NPT_String>& outputArgs)
{
	if (outputArgs.GetItemCount() < 4) {
		return false;
	}

	NPT_UInt32 numberReturned;
	NPT_UInt32 totalMatches;
	if (NPT_FAILED(NPT_ParseInteger(*outputArgs.GetItem(1), numberReturned))) {
		return false;
	}
	if (NPT_FAILED(NPT_ParseInteger(*outputArgs.GetItem(2), totalMatches))) {
		return false;
	}

	NPT_XmlParser parser;
	const NPT_String& content = *outputArgs.GetItem(0);
	NPT_XmlNode *rootNode;
	if (NPT_FAILED(parser.Parse(content.GetChars(), content.GetLength(), rootNode))) {
		return false;
	}

	PtrHolder<NPT_XmlNode> rootNode1(rootNode);
	NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
	if (!rootEl || !matchTagNamespace(rootEl, "DIDL-Lite", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")) {
		return false;
	}

	DLNAObjectList ls;

	for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
		if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
			if (matchNamespace(el, "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")) {
				if (el->GetTag().Compare("item") == 0) {
					DLNAItem *newItem = parseDLNAItem(el);
					newItem->d_ptr()->m_didlText = regenerateDidl(rootEl, el);
					ls.add(newItem);
				} else if (el->GetTag().Compare("container") == 0) {
					DLNAContainer *newContainer = parseDLNAContainer(el);
					newContainer->d_ptr()->m_didlText = regenerateDidl(rootEl, el);
					ls.add(newContainer);
				}
			}
		}
	}

	m_result.swap(ls);
	return true;
}

//------------------------------------------------------------------------------
// DLNAFullBrowseOpImpl
//------------------------------------------------------------------------------

DLNAFullBrowseOpImpl::DLNAFullBrowseOpImpl(DLNAProgressiveBrowseOp::ResultCallback *callback)
	: m_refCount(0), m_waitVar(0), m_waitVar1(0), m_abortFlag(false), m_finished(false), m_finishCallback(NULL)
	, m_succeeded(false), m_hasFaultDetail(false), m_errorCode(0)
	, m_buddy(NULL), m_resultCallback(callback)
{
}

DLNAFullBrowseOpImpl::~DLNAFullBrowseOpImpl()
{
}

int DLNAFullBrowseOpImpl::addRef()
{
	return m_refCount.Increment();
}

int DLNAFullBrowseOpImpl::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

void DLNAFullBrowseOpImpl::abort()
{
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		m_abortFlag = true;
		if (m_buddy) {
			m_buddy->abort();
		}
	}
}

bool DLNAFullBrowseOpImpl::aborted() const
{
	ReadLocker locker(m_stateLock);
	return m_abortFlag;
}

NPT_Result DLNAFullBrowseOpImpl::wait(NPT_Timeout timeout)
{
	return m_waitVar.WaitWhileEquals(0, timeout);
}

NPT_Result DLNAFullBrowseOpImpl::waitFirstReport(NPT_Timeout timeout)
{
	return m_waitVar1.WaitWhileEquals(0, timeout);
}

bool DLNAFullBrowseOpImpl::checkFinishedIfNotSetCallback(FinishCallback *callback)
{
	WriteLocker locker(m_stateLock);
	if (m_finished) {
		return true;
	}
	m_finishCallback = callback;
	return false;
}

bool DLNAFullBrowseOpImpl::resetCallback()
{
	WriteLocker locker(m_stateLock);
	m_finishCallback = NULL;
	m_resultCallback = NULL;
	return m_finished;
}

bool DLNAFullBrowseOpImpl::succeeded() const
{
	return m_succeeded;
}

bool DLNAFullBrowseOpImpl::hasFaultDetail() const
{
	return m_hasFaultDetail;
}

int DLNAFullBrowseOpImpl::errorCode() const
{
	return m_errorCode;
}

const NPT_String& DLNAFullBrowseOpImpl::errorDesc() const
{
	return m_errorDesc;
}

const DLNAObjectList& DLNAFullBrowseOpImpl::objectList() const
{
	return m_result;
}

void DLNAFullBrowseOpImpl::setBuddy(DLNAFullBrowseTask *buddy)
{
	WriteLocker locker(m_stateLock);
	m_buddy = buddy;
}

void DLNAFullBrowseOpImpl::notifyBrowseResult(NPT_UInt32 startingIndex, NPT_UInt32 numberReturned, NPT_UInt32 totalMatches, const DLNAObjectList& ls)
{
	ReadLocker locker(m_stateLock);
	if (m_resultCallback) {
		m_resultCallback->onDLNAProgressiveBrowseOpResult(this, startingIndex, numberReturned, totalMatches, ls);
	}
}

void DLNAFullBrowseOpImpl::notifyFinished()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_finishCallback) {
			m_finishCallback->onDLNACoreOpFinished(this);
		}
		m_finished = true;
		m_waitVar.SetValue(1);
	}
}

//------------------------------------------------------------------------------
// DLNAFullBrowseTask
//------------------------------------------------------------------------------

DLNAFullBrowseTask::DLNAFullBrowseTask(DLNAFullBrowseOpImpl *op, ControlPoint *cp, const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId, NPT_UInt32 requestedCount)
	: m_op(op), m_cp(cp), m_mediaServerUuid(mediaServerUuid), m_cdsId(cdsId), m_containerId(containerId), m_requestedCount(requestedCount)
	, m_actInst(NULL)
{
	m_op->addRef();
	m_op->setBuddy(this);
}

DLNAFullBrowseTask::~DLNAFullBrowseTask()
{
	m_op->setBuddy(NULL);
	m_op->release();
}

void DLNAFullBrowseTask::exec()
{
	NPT_Result nr;

	DLNAObjectList ls;

	NPT_UInt32 startingIndex = 0;
	NPT_UInt32 requestedCount = m_requestedCount;
	NPT_UInt32 numberReturned;
	bool firstNotify = false;

	for (;;) {
		DLNAObjectList ls1;
		NPT_UInt32 totalMatches;
		nr = browse(m_mediaServerUuid, m_cdsId, m_containerId, startingIndex, requestedCount, ls1, numberReturned, totalMatches);
		if (NPT_SUCCEEDED(nr)) {
			if (numberReturned == 0) {
				break;
			}
			m_op->notifyBrowseResult(startingIndex, numberReturned, totalMatches, ls1);
			if (!firstNotify) {
				firstNotify = true;
				m_op->m_waitVar1.SetValue(1);
			}
			ls.add(ls1);
			startingIndex += numberReturned;
		} else if (nr == NPT_ERROR_CANCELLED) {
			// TODO: 
			break;
		} else {
			// TODO: ignore ???
			break;
		}
	}

	if (!firstNotify) {
		m_op->notifyBrowseResult(0, 0, 0, ls);
		m_op->m_waitVar1.SetValue(1);
	}

	m_op->m_hasFaultDetail = false;
	if (NPT_SUCCEEDED(nr) || nr == NPT_ERROR_CANCELLED) {
		m_op->m_succeeded = true;
		m_op->m_result.swap(ls);
	} else {
		m_op->m_succeeded = false;
	}

	m_op->notifyFinished();
}

NPT_Result DLNAFullBrowseTask::browse(const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, DLNAObjectList& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add(containerId);
	inputArgs.Add("BrowseDirectChildren");
	inputArgs.Add("*");
	inputArgs.Add(NPT_String::FromIntegerU(startingIndex));
	inputArgs.Add(NPT_String::FromIntegerU(requestedCount));
	inputArgs.Add("");

	numberReturned = 0;

	NPT_Result nr;

	ActionInstance *actInst;
	nr = m_cp->invokeAction(mediaServerUuid, cdsId, "Browse", inputArgs, NULL, actInst);
	if (NPT_SUCCEEDED(nr)) {
		setActionInstance(actInst);
		actInst->wait();
		setActionInstance(NULL);

		do {
			if (aborted()) {
				nr = NPT_ERROR_CANCELLED;
				break;
			}

			nr = actInst->statusCode();
			if (NPT_FAILED(nr)) {
				break;
			}

			if (actInst->errorCode() != 0) {
				nr = NPT_FAILURE;
				break;
			}

			const NPT_String& content = *actInst->outputValues().GetItem(0);
			NPT_ParseInteger(*actInst->outputValues().GetItem(1), numberReturned);
			NPT_ParseInteger(*actInst->outputValues().GetItem(2), totalMatches);
			NPT_XmlParser parser;

			NPT_XmlNode *rootNode;
			nr = parser.Parse(content.GetChars(), content.GetLength(), rootNode);
			if (NPT_FAILED(nr)) {
				break;
			}

			PtrHolder<NPT_XmlNode> rootNode1(rootNode);
			NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
			if (!rootEl || !matchTagNamespace(rootEl, "DIDL-Lite", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")) {
				nr = NPT_FAILURE;
				break;
			}

			for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
				if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
					if (matchNamespace(el, "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")) {
						if (el->GetTag().Compare("item") == 0) {
							DLNAItem *item = parseDLNAItem(el);
							item->d_ptr()->m_didlText = regenerateDidl(rootEl, el);
							item->addRef();
							result.add(item);
							item->release();
						} else if (el->GetTag().Compare("container") == 0) {
							DLNAContainer *container = parseDLNAContainer(el);
							container->d_ptr()->m_didlText = regenerateDidl(rootEl, el);
							container->addRef();
							result.add(container);
							container->release();
						}
					}
				}
			}

		} while (false);
		actInst->release();
	}
	return nr;
}

void DLNAFullBrowseTask::setActionInstance(ActionInstance *actInst)
{
	WriteLocker locker(m_stateLock);
	m_actInst = actInst;
}

void DLNAFullBrowseTask::doAbort()
{
	ReadLocker locker(m_stateLock);
	if (m_actInst) {
		m_actInst->abort();
	}
}

//------------------------------------------------------------------------------
// DLNADeepBrowseOpImpl
//------------------------------------------------------------------------------

DLNADeepBrowseOpImpl::DLNADeepBrowseOpImpl()
	: m_refCount(0), m_waitVar(0), m_abortFlag(false), m_finished(false), m_finishCallback(NULL)
	, m_succeeded(false), m_hasFaultDetail(false), m_errorCode(0)
	, m_buddy(NULL)
{
}

DLNADeepBrowseOpImpl::~DLNADeepBrowseOpImpl()
{
}

int DLNADeepBrowseOpImpl::addRef()
{
	return m_refCount.Increment();
}

int DLNADeepBrowseOpImpl::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

void DLNADeepBrowseOpImpl::abort()
{
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		m_abortFlag = true;
		if (m_buddy) {
			m_buddy->abort();
		}
	}
}

bool DLNADeepBrowseOpImpl::aborted() const
{
	ReadLocker locker(m_stateLock);
	return m_abortFlag;
}

NPT_Result DLNADeepBrowseOpImpl::wait(NPT_Timeout timeout)
{
	return m_waitVar.WaitWhileEquals(0, timeout);
}

bool DLNADeepBrowseOpImpl::checkFinishedIfNotSetCallback(FinishCallback *callback)
{
	WriteLocker locker(m_stateLock);
	if (m_finished) {
		return true;
	}
	m_finishCallback = callback;
	return false;
}

bool DLNADeepBrowseOpImpl::resetCallback()
{
	WriteLocker locker(m_stateLock);
	m_finishCallback = NULL;
	return m_finished;
}

bool DLNADeepBrowseOpImpl::succeeded() const
{
	return m_succeeded;
}

bool DLNADeepBrowseOpImpl::hasFaultDetail() const
{
	return m_hasFaultDetail;
}

int DLNADeepBrowseOpImpl::errorCode() const
{
	return m_errorCode;
}

const NPT_String& DLNADeepBrowseOpImpl::errorDesc() const
{
	return m_errorDesc;
}

const DLNAObjectList& DLNADeepBrowseOpImpl::objectList() const
{
	return m_result;
}

void DLNADeepBrowseOpImpl::setBuddy(DLNADeepBrowseTask *buddy)
{
	WriteLocker locker(m_stateLock);
	m_buddy = buddy;
}

void DLNADeepBrowseOpImpl::notifyFinished()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_finishCallback) {
			m_finishCallback->onDLNACoreOpFinished(this);
		}
		m_finished = true;
		m_waitVar.SetValue(1);
	}
}

//------------------------------------------------------------------------------
// DLNADeepBrowseTask
//------------------------------------------------------------------------------

DLNADeepBrowseTask::DLNADeepBrowseTask(DLNADeepBrowseOpImpl *op, ControlPoint *cp, const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId)
	: m_op(op), m_cp(cp), m_mediaServerUuid(mediaServerUuid), m_cdsId(cdsId), m_containerId(containerId)
	, m_actInst(NULL)
{
	m_op->addRef();
	m_op->setBuddy(this);
}

DLNADeepBrowseTask::~DLNADeepBrowseTask()
{
	m_op->setBuddy(NULL);
	m_op->release();
}

void DLNADeepBrowseTask::exec()
{
	NPT_Stack<NPT_String> stack;
	stack.Push(m_containerId);

	NPT_Result nr;

	DLNAObjectList ls;

	NPT_String containerId;
	while (NPT_SUCCEEDED(stack.Pop(containerId))) {
		nr = browse(m_mediaServerUuid, m_cdsId, containerId, stack, ls);
		if (NPT_SUCCEEDED(nr)) {
		} else if (nr == NPT_ERROR_CANCELLED) {
			// TODO: 
			break;
		} else {
			// TODO: ignore ???
			break;
		}
	}

	m_op->m_hasFaultDetail = false;
	if (NPT_SUCCEEDED(nr) || nr == NPT_ERROR_CANCELLED) {
		m_op->m_succeeded = true;
		m_op->m_result.swap(ls);
	} else {
		m_op->m_succeeded = false;
	}

	m_op->notifyFinished();
}

NPT_Result DLNADeepBrowseTask::browse(const UUID& mediaServerUuid, const NPT_String& cdsId, const NPT_String& containerId, NPT_Stack<NPT_String>& stack, DLNAObjectList& result)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add(containerId);
	inputArgs.Add("BrowseDirectChildren");
	inputArgs.Add("*");
	inputArgs.Add("0");
	inputArgs.Add("0");
	inputArgs.Add("");

	NPT_Result nr;

	ActionInstance *actInst;
	nr = m_cp->invokeAction(mediaServerUuid, cdsId, "Browse", inputArgs, NULL, actInst);
	if (NPT_SUCCEEDED(nr)) {
		setActionInstance(actInst);
		actInst->wait();
		setActionInstance(NULL);

		do {
			if (aborted()) {
				nr = NPT_ERROR_CANCELLED;
				break;
			}

			nr = actInst->statusCode();
			if (NPT_FAILED(nr)) {
				break;
			}

			if (actInst->errorCode() != 0) {
				nr = NPT_FAILURE;
				break;
			}

			const NPT_String& content = *actInst->outputValues().GetItem(0);
			NPT_XmlParser parser;

			NPT_XmlNode *rootNode;
			nr = parser.Parse(content.GetChars(), content.GetLength(), rootNode);
			if (NPT_FAILED(nr)) {
				break;
			}

			PtrHolder<NPT_XmlNode> rootNode1(rootNode);
			NPT_XmlElementNode *rootEl = rootNode->AsElementNode();
			if (!rootEl || !matchTagNamespace(rootEl, "DIDL-Lite", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")) {
				nr = NPT_FAILURE;
				break;
			}

			for (NPT_Ordinal i = 0; i < rootEl->GetChildren().GetItemCount(); i++) {
				if (NPT_XmlElementNode *el = (*rootEl->GetChildren().GetItem(i))->AsElementNode()) {
					if (matchNamespace(el, "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")) {
						if (el->GetTag().Compare("item") == 0) {
							DLNAItem *item = parseDLNAItem(el);
							item->d_ptr()->m_didlText = regenerateDidl(rootEl, el);
							item->addRef();
							if (item->refId().IsEmpty()) {
								result.add(item);
								item->release();
							} else {
								item->release();
							}
						} else if (el->GetTag().Compare("container") == 0) {
							DLNAContainer *container = parseDLNAContainer(el);
							container->d_ptr()->m_didlText = regenerateDidl(rootEl, el);
							container->addRef();
							//if (container->childCount() > 0) {
								//qDebug("%s [%s]", container->objectId().GetChars(), container->title().GetChars());
								stack.Add(container->objectId());
							//}
							container->release();
						}
					}
				}
			}

		} while (false);
		actInst->release();
	}
	return nr;
}

void DLNADeepBrowseTask::setActionInstance(ActionInstance *actInst)
{
	WriteLocker locker(m_stateLock);
	m_actInst = actInst;
}

void DLNADeepBrowseTask::doAbort()
{
	ReadLocker locker(m_stateLock);
	if (m_actInst) {
		m_actInst->abort();
	}
}

//------------------------------------------------------------------------------
// DLNABaseMediaOpImpl
//------------------------------------------------------------------------------

DLNABaseMediaOpImpl::DLNABaseMediaOpImpl()
	: m_refCount(0), m_waitVar(0), m_abortFlag(false), m_finished(false), m_finishCallback(NULL)
	, m_succeeded(false), m_buddy(NULL)
{
}

DLNABaseMediaOpImpl::~DLNABaseMediaOpImpl()
{
}

int DLNABaseMediaOpImpl::addRef()
{
	return m_refCount.Increment();
}

int DLNABaseMediaOpImpl::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

void DLNABaseMediaOpImpl::abort()
{
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		m_abortFlag = true;
		if (m_buddy) {
			m_buddy->abort();
		}
	}
}

bool DLNABaseMediaOpImpl::aborted() const
{
	ReadLocker locker(m_stateLock);
	return m_abortFlag;
}

NPT_Result DLNABaseMediaOpImpl::wait(NPT_Timeout timeout)
{
	return m_waitVar.WaitWhileEquals(0, timeout);
}

bool DLNABaseMediaOpImpl::checkFinishedIfNotSetCallback(FinishCallback *callback)
{
	WriteLocker locker(m_stateLock);
	if (m_finished) {
		return true;
	}
	m_finishCallback = callback;
	return false;
}

bool DLNABaseMediaOpImpl::resetCallback()
{
	WriteLocker locker(m_stateLock);
	m_finishCallback = NULL;
	return m_finished;
}

bool DLNABaseMediaOpImpl::succeeded() const
{
	return m_succeeded;
}

void DLNABaseMediaOpImpl::setBuddy(DLNABaseMediaTask *buddy)
{
	WriteLocker locker(m_stateLock);
	m_buddy = buddy;
}

void DLNABaseMediaOpImpl::notifyFinished()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_finishCallback) {
			m_finishCallback->onDLNACoreOpFinished(this);
		}
		m_finished = true;
		m_waitVar.SetValue(1);
	}
}

//------------------------------------------------------------------------------
// DLNABaseMediaTask
//------------------------------------------------------------------------------

DLNABaseMediaTask::DLNABaseMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp)
	: m_op(op), m_cp(cp), m_actInst(NULL)
{
	m_op->addRef();
	m_op->setBuddy(this);
}

DLNABaseMediaTask::~DLNABaseMediaTask()
{
	m_op->setBuddy(NULL);
	m_op->release();
}

void DLNABaseMediaTask::setActionInstance(ActionInstance *actInst)
{
	WriteLocker locker(m_stateLock);
	m_actInst = actInst;
}

void DLNABaseMediaTask::exec()
{
	NPT_Result nr;
	ActionInstance *actInst = NULL;

	do {
		nr = doAction(actInst);
		if (NPT_FAILED(nr)) {
			break;
		}

		setActionInstance(actInst);
		actInst->wait();
		setActionInstance(NULL);
		if (aborted()) {
			nr = NPT_ERROR_CANCELLED;
			break;
		}

		nr = actInst->statusCode();
		if (NPT_FAILED(nr)) {
			break;
		}

		if (actInst->errorCode() != 0) {
			nr = NPT_FAILURE;
			break;
		}

		handleResult(actInst);

		actInst->release();
		actInst = NULL;

	} while (false);

	if (actInst) {
		actInst->release();
	}

	if (NPT_SUCCEEDED(nr)) {
		m_op->m_succeeded = true;
	} else {
		m_op->m_succeeded = false;
	}

	m_op->notifyFinished();
}

void DLNABaseMediaTask::doAbort()
{
	ReadLocker locker(m_stateLock);
	if (m_actInst) {
		m_actInst->abort();
	}
}

//------------------------------------------------------------------------------
// DLNAStopMediaTask
//------------------------------------------------------------------------------

DLNAStopMediaTask::DLNAStopMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId)
{
}

NPT_Result DLNAStopMediaTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	return m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Stop", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNAPauseMediaTask
//------------------------------------------------------------------------------

DLNAPauseMediaTask::DLNAPauseMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId)
{
}

NPT_Result DLNAPauseMediaTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	return m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Pause", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNAPrevMediaTask
//------------------------------------------------------------------------------

DLNAPrevMediaTask::DLNAPrevMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId)
{
}

NPT_Result DLNAPrevMediaTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	return m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Previous", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNANextMediaTask
//------------------------------------------------------------------------------

DLNANextMediaTask::DLNANextMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId)
{
}

NPT_Result DLNANextMediaTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	return m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Next", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNAMuteMediaTask
//------------------------------------------------------------------------------

DLNAMuteMediaTask::DLNAMuteMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& rcsId, bool mute)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_rcsId(rcsId), m_mute(mute)
{
}

NPT_Result DLNAMuteMediaTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	inputArgs.Add("Master");
	inputArgs.Add(m_mute ? "1" : "0");
	return m_cp->invokeAction(m_mediaRendererUuid, m_rcsId, "SetMute", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNAChangeMediaVolumeTask
//------------------------------------------------------------------------------

DLNAChangeMediaVolumeTask::DLNAChangeMediaVolumeTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& rcsId, int volume)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_rcsId(rcsId), m_volume(volume)
{
}

NPT_Result DLNAChangeMediaVolumeTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	inputArgs.Add("Master");
	inputArgs.Add(NPT_String::FromInteger(m_volume));
	return m_cp->invokeAction(m_mediaRendererUuid, m_rcsId, "SetVolume", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNASeekMediaTask
//------------------------------------------------------------------------------

DLNASeekMediaTask::DLNASeekMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_String& mode, const NPT_String& target)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId), m_mode(mode), m_target(target)
{
}

NPT_Result DLNASeekMediaTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	inputArgs.Add(m_mode);
	inputArgs.Add(m_target);
	return m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Seek", inputArgs, NULL, actInst);
}

//------------------------------------------------------------------------------
// DLNAQueryPositionInfoOpImpl
//------------------------------------------------------------------------------

DLNAQueryPositionInfoOpImpl::DLNAQueryPositionInfoOpImpl()
	: m_trackTime(0)
{
}

DLNAQueryPositionInfoOpImpl::~DLNAQueryPositionInfoOpImpl()
{
}

int DLNAQueryPositionInfoOpImpl::addRef()
{
	return DLNABaseMediaOpImpl::addRef();
}

int DLNAQueryPositionInfoOpImpl::release()
{
	return DLNABaseMediaOpImpl::release();
}

void DLNAQueryPositionInfoOpImpl::abort()
{
	DLNABaseMediaOpImpl::abort();
}

bool DLNAQueryPositionInfoOpImpl::aborted() const
{
	return DLNABaseMediaOpImpl::aborted();
}

NPT_Result DLNAQueryPositionInfoOpImpl::wait(NPT_Timeout timeout)
{
	return DLNABaseMediaOpImpl::wait(timeout);
}

bool DLNAQueryPositionInfoOpImpl::checkFinishedIfNotSetCallback(FinishCallback *callback)
{
	return DLNABaseMediaOpImpl::checkFinishedIfNotSetCallback(callback);
}

bool DLNAQueryPositionInfoOpImpl::resetCallback()
{
	return DLNABaseMediaOpImpl::resetCallback();
}

bool DLNAQueryPositionInfoOpImpl::succeeded() const
{
	return DLNABaseMediaOpImpl::succeeded();
}

int DLNAQueryPositionInfoOpImpl::trackTime() const
{
	return m_trackTime;
}

//------------------------------------------------------------------------------
// DLNAQueryPositionInfoTask
//------------------------------------------------------------------------------

DLNAQueryPositionInfoTask::DLNAQueryPositionInfoTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId)
{
}

NPT_Result DLNAQueryPositionInfoTask::doAction(ActionInstance*& actInst)
{
	NPT_List<NPT_String> inputArgs;
	inputArgs.Add("0");
	return m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "GetPositionInfo", inputArgs, NULL, actInst);
}

void DLNAQueryPositionInfoTask::handleResult(ActionInstance *actInst)
{
	NPT_UInt64 trackTime;
	if (NPT_SUCCEEDED(Helper::parseTrackDurationString(*actInst->outputValues().GetItem(4), trackTime))) {
		static_cast<DLNAQueryPositionInfoOpImpl*>(m_op)->m_trackTime = static_cast<int>(trackTime);
	}
}

//------------------------------------------------------------------------------
// DLNAPlayMediaTask
//------------------------------------------------------------------------------

DLNAPlayMediaTask::DLNAPlayMediaTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_String& url, const NPT_String& metaData)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId), m_url(url), m_metaData(metaData)
{
}

void DLNAPlayMediaTask::exec()
{
	NPT_Result nr;
	ActionInstance *actInst = NULL;

	do {
		if (!m_url.IsEmpty()) {
			NPT_List<NPT_String> inputArgs;
			inputArgs.Add("0");
			nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Stop", inputArgs, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}

			setActionInstance(actInst);
			actInst->wait();
			setActionInstance(NULL);

			actInst->release();
		}

		if (!m_url.IsEmpty()) {
			NPT_List<NPT_String> inputArgs;
			inputArgs.Add("0");
			inputArgs.Add(m_url);
			inputArgs.Add(m_metaData);
			nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "SetAVTransportURI", inputArgs, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}

			setActionInstance(actInst);
			actInst->wait();
			setActionInstance(NULL);
			if (aborted()) {
				nr = NPT_ERROR_CANCELLED;
				break;
			}

			nr = actInst->statusCode();
			if (NPT_FAILED(nr)) {
				break;
			}

			if (actInst->errorCode() != 0) {
				nr = NPT_FAILURE;
				break;
			}

			actInst->release();
			actInst = NULL;
		}

		NPT_List<NPT_String> inputArgs;
		inputArgs.Add("0");
		inputArgs.Add("1");
		nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Play", inputArgs, NULL, actInst);
		if (NPT_FAILED(nr)) {
			break;
		}

		setActionInstance(actInst);
		actInst->wait();
		setActionInstance(NULL);
		if (aborted()) {
			nr = NPT_ERROR_CANCELLED;
			break;
		}

		nr = actInst->statusCode();
		if (NPT_FAILED(nr)) {
			break;
		}

		if (actInst->errorCode() != 0) {
			nr = NPT_FAILURE;
			break;
		}

		actInst->release();
		actInst = NULL;

	} while (false);

	if (actInst) {
		actInst->release();
	}

	if (NPT_SUCCEEDED(nr)) {
		m_op->m_succeeded = true;
	} else {
		m_op->m_succeeded = false;
	}

	m_op->notifyFinished();
}

//------------------------------------------------------------------------------
// DLNAPlayMediaListTask
//------------------------------------------------------------------------------

DLNAPlayMediaListTask::DLNAPlayMediaListTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_List<DLNAObject*>& ls)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId)
{
	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		m_mediaList.add(*ls.GetItem(i));
	}
}

NPT_String generateWindowsMediaPlaylist(const DLNAObjectList& ls)
{
	NPT_StringOutputStream strm;
	NPT_XmlSerializer xml(&strm, 0, true, false);

	xml.StartDocument();
	xml.StartElement(NULL, "smil");

	xml.StartElement(NULL, "head");

	const char *plh = "ItemCount_placeholder";

	xml.StartElement(NULL, "meta");
	xml.Attribute(NULL, "name", "IsNetworkFeed");
	xml.Attribute(NULL, "content", "1");
	xml.EndElement(NULL, "meta");

	xml.StartElement(NULL, "meta");
	xml.Attribute(NULL, "name", "ItemCount");
	xml.Attribute(NULL, "content", plh);
	xml.EndElement(NULL, "meta");

	xml.StartElement(NULL, "title");
	xml.Text("playlist1");
	xml.EndElement(NULL, "title");

	xml.EndElement(NULL, "head");

	xml.StartElement(NULL, "body");
	xml.StartElement(NULL, "seq");

	NPT_UInt32 itemCount = 0;

	for (NPT_Ordinal i = 0; i < ls.count(); i++) {
		NPT_String url;
		DLNAObject *obj = ls.itemAt(i);
		if (DLNAItem *item = obj->asItem()) {
			NPT_List<DLNAResource*>::Iterator it = item->d_ptr()->m_resList.GetFirstItem();
			if (it) {
				url = (*it)->d_ptr()->m_url;
			}
		}

		if (!url.IsEmpty()) {
			xml.StartElement(NULL, "media");
			xml.Attribute(NULL, "src", url);
			xml.EndElement(NULL, "media");
			++itemCount;
		}
	}

	xml.EndElement(NULL, "seq");
	xml.EndElement(NULL, "body");

	xml.EndElement(NULL, "smil");
	xml.EndDocument();

	NPT_String result("<?wpl version=\"1.0\"?>");
	result += strm.GetString();
	result.Replace(plh, NPT_String::FromInteger(itemCount));
	return result;
}

void DLNAPlayMediaListTask::exec()
{
	NPT_Result nr;
	ActionInstance *actInst = NULL;

	do {
		{
			NPT_List<NPT_String> inputArgs;
			inputArgs.Add("0");
			nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Stop", inputArgs, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}

			setActionInstance(actInst);
			actInst->wait();
			setActionInstance(NULL);

			actInst->release();
		}

		{
			/*
			NPT_List<NPT_String> inputArgs;
			inputArgs.Add("0");
			inputArgs.Add(m_url);
			inputArgs.Add(m_metaData);
			nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "SetAVTransportURI", inputArgs, NULL, actInst);
			*/
			// application/vnd.ms-wpl
			NPT_String wpl = generateWindowsMediaPlaylist(m_mediaList);
			nr = m_cp->setPlaylistToMediaRenderer(m_mediaRendererUuid, m_avtId, "application/vnd.ms-wpl", wpl, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}

			setActionInstance(actInst);
			actInst->wait();
			setActionInstance(NULL);
			if (aborted()) {
				nr = NPT_ERROR_CANCELLED;
				break;
			}

			nr = actInst->statusCode();
			if (NPT_FAILED(nr)) {
				break;
			}

			if (actInst->errorCode() != 0) {
				nr = NPT_FAILURE;
				break;
			}

			actInst->release();
			actInst = NULL;
		}

		NPT_List<NPT_String> inputArgs;
		inputArgs.Add("0");
		inputArgs.Add("1");
		nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Play", inputArgs, NULL, actInst);
		if (NPT_FAILED(nr)) {
			break;
		}

		setActionInstance(actInst);
		actInst->wait();
		setActionInstance(NULL);
		if (aborted()) {
			nr = NPT_ERROR_CANCELLED;
			break;
		}

		nr = actInst->statusCode();
		if (NPT_FAILED(nr)) {
			break;
		}

		if (actInst->errorCode() != 0) {
			nr = NPT_FAILURE;
			break;
		}

		actInst->release();
		actInst = NULL;

	} while (false);

	if (actInst) {
		actInst->release();
	}

	if (NPT_SUCCEEDED(nr)) {
		m_op->m_succeeded = true;
	} else {
		m_op->m_succeeded = false;
	}

	m_op->notifyFinished();

}

//------------------------------------------------------------------------------
// DLNAPlayFileTask
//------------------------------------------------------------------------------

DLNAPlayFileTask::DLNAPlayFileTask(DLNABaseMediaOpImpl *op, ControlPoint *cp, const UUID& mediaRendererUuid, const NPT_String& avtId, const NPT_String& filePath, const NPT_String& mimeType, const NPT_String& metaData, const NPT_String& placeholder)
	: DLNABaseMediaTask(op, cp), m_mediaRendererUuid(mediaRendererUuid), m_avtId(avtId), m_filePath(filePath), m_mimeType(mimeType), m_metaData(metaData), m_placeholder(placeholder)
{
}

void DLNAPlayFileTask::exec()
{
	NPT_Result nr;
	ActionInstance *actInst = NULL;

	do {
		{
			NPT_List<NPT_String> inputArgs;
			inputArgs.Add("0");
			nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Stop", inputArgs, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}

			setActionInstance(actInst);
			actInst->wait();
			setActionInstance(NULL);

			actInst->release();
		}

		{
			/*
			NPT_List<NPT_String> inputArgs;
			inputArgs.Add("0");
			inputArgs.Add(m_url);
			inputArgs.Add(m_metaData);
			nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "SetAVTransportURI", inputArgs, NULL, actInst);
			*/
			// application/vnd.ms-wpl
			/*NPT_String wpl = generateWindowsMediaPlaylist(m_mediaList);
			nr = m_cp->setPlaylistToMediaRenderer(m_mediaRendererUuid, m_avtId, "application/vnd.ms-wpl", wpl, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}*/
			nr = m_cp->serveFileToMediaRenderer(m_mediaRendererUuid, m_avtId, m_filePath, m_mimeType, m_metaData, m_placeholder, NULL, actInst);
			if (NPT_FAILED(nr)) {
				break;
			}

			setActionInstance(actInst);
			actInst->wait();
			setActionInstance(NULL);
			if (aborted()) {
				nr = NPT_ERROR_CANCELLED;
				break;
			}

			nr = actInst->statusCode();
			if (NPT_FAILED(nr)) {
				break;
			}

			if (actInst->errorCode() != 0) {
				nr = NPT_FAILURE;
				break;
			}

			actInst->release();
			actInst = NULL;
		}

		NPT_List<NPT_String> inputArgs;
		inputArgs.Add("0");
		inputArgs.Add("1");
		nr = m_cp->invokeAction(m_mediaRendererUuid, m_avtId, "Play", inputArgs, NULL, actInst);
		if (NPT_FAILED(nr)) {
			break;
		}

		setActionInstance(actInst);
		actInst->wait();
		setActionInstance(NULL);
		if (aborted()) {
			nr = NPT_ERROR_CANCELLED;
			break;
		}

		nr = actInst->statusCode();
		if (NPT_FAILED(nr)) {
			break;
		}

		if (actInst->errorCode() != 0) {
			nr = NPT_FAILURE;
			break;
		}

		actInst->release();
		actInst = NULL;

	} while (false);

	if (actInst) {
		actInst->release();
	}

	if (NPT_SUCCEEDED(nr)) {
		m_op->m_succeeded = true;
	} else {
		m_op->m_succeeded = false;
	}

	m_op->notifyFinished();

}

} // namespace deejay
