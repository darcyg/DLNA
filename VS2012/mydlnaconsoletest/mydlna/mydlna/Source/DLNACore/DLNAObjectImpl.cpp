#include "DLNAObjectImpl.h"

namespace deejay {

DLNAResourceImpl::DLNAResourceImpl()
	: m_size(0)
	, m_bitrate(0)
	, m_sampleFrequency(0)
	, m_bitsPerSample(0)
	, m_nrAudioChannels(0)
	, m_colorDepth(0)
	, m_resolutionX(0)
	, m_resolutionY(0)
{
}

DLNAResourceImpl::~DLNAResourceImpl()
{
}

DLNAObjectImpl::DLNAObjectImpl()
	: m_restricted(true)
	, m_originalTrackNumber(-1)
	, m_resourceSize(-1)
{
}

DLNAObjectImpl::~DLNAObjectImpl()
{
	m_resList.Apply(NPT_ObjectDeleter<DLNAResource>());
}

DLNAItemImpl::DLNAItemImpl()
{
}

DLNAItemImpl::~DLNAItemImpl()
{
}

DLNAItem *DLNAItemImpl::newDLNAItem()
{
	return new DLNAItem();
}

DLNAContainerImpl::DLNAContainerImpl()
	: m_childCount(0)
	, m_searchable(false)
	, m_storageUsed(-1)
{
}

DLNAContainerImpl::~DLNAContainerImpl()
{
}

DLNAContainer *DLNAContainerImpl::newDLNAContainer()
{
	return new DLNAContainer();
}

} // namespace deejay
