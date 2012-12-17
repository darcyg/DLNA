#include "DLNAObjectImpl.h"

namespace deejay {

DLNAResource::DLNAResource()
{
	m_impl = new DLNAResourceImpl();
}

DLNAResource::~DLNAResource()
{
	delete m_impl;
}

DLNAResourceImpl *DLNAResource::d_ptr() const
{
	return m_impl;
}

DLNAObject::DLNAObject(DLNAObjectImpl *impl)
	: m_impl(impl)
{
}

DLNAObject::~DLNAObject()
{
	delete m_impl;
}

int DLNAObject::addRef()
{
	return m_impl->m_refCount.Increment();
}

int DLNAObject::release()
{
	int rc = m_impl->m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

DLNAItem *DLNAObject::asItem()
{
	return NULL;
}

const DLNAItem *DLNAObject::asItem() const
{
	return NULL;
}

DLNAContainer *DLNAObject::asContainer()
{
	return NULL;
}

const DLNAContainer *DLNAObject::asContainer() const
{
	return NULL;
}

DLNAObjectImpl *DLNAObject::d_ptr() const
{
	return m_impl;
}

const NPT_String& DLNAObject::objectId() const
{
	return d_ptr()->m_objectId;
}

const NPT_String& DLNAObject::parentId() const
{
	return d_ptr()->m_parentId;
}

const NPT_String& DLNAObject::title() const
{
	return d_ptr()->m_title;
}

const NPT_String& DLNAObject::creator() const
{
	return d_ptr()->m_creator;
}

const NPT_String& DLNAObject::upnpClass() const
{
	return d_ptr()->m_upnpClass;
}

const NPT_String& DLNAObject::upnpClassName() const
{
	return d_ptr()->m_upnpClassName;
}

const NPT_String& DLNAObject::writeStatus() const
{
	return d_ptr()->m_writeStatus;
}

bool DLNAObject::restricted() const
{
	return d_ptr()->m_restricted;
}

const NPT_String& DLNAObject::dateStr() const
{
	return d_ptr()->m_dateStr;
}

NPT_Int64 DLNAObject::resourceSize() const
{
	return d_ptr()->m_resourceSize;
}

const NPT_List<NPT_String>& DLNAObject::albumArtURIList() const
{
	return d_ptr()->m_albumArtURIList;
}

const NPT_List<DLNAResource*> DLNAObject::resourceList() const
{
	return d_ptr()->m_resList;
}

bool DLNAObject::findThumbnailURL(int width, int height, const NPT_List<NPT_String>* preferredMimeTypes, NPT_String& url)
{
	NPT_List<NPT_String>::Iterator it = d_ptr()->m_albumArtURIList.GetFirstItem();
	if (it) {
		url = NPT_Uri::PercentDecode(*it);
		return true;
	}

	for (NPT_Ordinal i = 0; i < d_ptr()->m_resList.GetItemCount(); i++) {
		DLNAResourceImpl *resImpl = (*d_ptr()->m_resList.GetItem(i))->d_ptr();
		if (resImpl->m_mimeType.Compare("image/png") == 0
			|| resImpl->m_mimeType.Compare("image/jpeg") == 0
			|| resImpl->m_mimeType.Compare("image/gif") == 0
//			|| resImpl->m_mimeType.Compare("image/tiff") == 0
			|| resImpl->m_mimeType.Compare("image/bmp") == 0)
		{
			if (!preferredMimeTypes || (preferredMimeTypes->Find(NPT_ObjectComparator<NPT_String>(resImpl->m_mimeType)))) {
				url = NPT_Uri::PercentDecode(resImpl->m_url);
				return true;
			}
		}
	}

	return false;
}

DLNAItem::DLNAItem()
	: DLNAObject(new DLNAItemImpl())
{
}

DLNAItem::~DLNAItem()
{
}

DLNAItem *DLNAItem::asItem()
{
	return this;
}

const DLNAItem *DLNAItem::asItem() const
{
	return this;
}

DLNAItemImpl *DLNAItem::d_ptr() const
{
	return static_cast<DLNAItemImpl*>(DLNAObject::d_ptr());
}

const NPT_String& DLNAItem::refId() const
{
	return d_ptr()->m_refId;
}

DLNAContainer::DLNAContainer()
	: DLNAObject(new DLNAContainerImpl())
{
}

DLNAContainer::~DLNAContainer()
{
}

DLNAContainer *DLNAContainer::asContainer()
{
	return this;
}

const DLNAContainer *DLNAContainer::asContainer() const
{
	return this;
}

DLNAContainerImpl *DLNAContainer::d_ptr() const
{
	return static_cast<DLNAContainerImpl*>(DLNAObject::d_ptr());
}

int DLNAContainer::childCount() const
{
	return d_ptr()->m_childCount;
}

bool DLNAContainer::searchable() const
{
	return d_ptr()->m_searchable;
}

DLNAObjectList::DLNAObjectList()
{
	m_ls = new NPT_List<DLNAObject*>();
}

DLNAObjectList::DLNAObjectList(const DLNAObjectList& other)
{
	m_ls = new NPT_List<DLNAObject*>();
	for (NPT_Ordinal i = 0; i < other.m_ls->GetItemCount(); i++) {
		DLNAObject *obj = *other.m_ls->GetItem(i);
		obj->addRef();
		m_ls->Add(obj);
	}
}

DLNAObjectList& DLNAObjectList::operator=(const DLNAObjectList& other)
{
	clear();
	for (NPT_Ordinal i = 0; i < other.m_ls->GetItemCount(); i++) {
		DLNAObject *obj = *other.m_ls->GetItem(i);
		obj->addRef();
		m_ls->Add(obj);
	}
	return *this;
}

DLNAObjectList::~DLNAObjectList()
{
	clear();
	delete m_ls;
}

void DLNAObjectList::add(DLNAObject *obj)
{
	obj->addRef();
	m_ls->Add(obj);
}

void DLNAObjectList::add(const DLNAObjectList& ls)
{
	for (NPT_Ordinal i = 0; i < ls.m_ls->GetItemCount(); i++) {
		add(*ls.m_ls->GetItem(i));
	}
}

NPT_Cardinal DLNAObjectList::count() const
{
	return m_ls->GetItemCount();
}

DLNAObject *DLNAObjectList::itemAt(NPT_Ordinal i) const
{
	if (i < count()) {
		return *m_ls->GetItem(i);
	}
	return NULL;
}

void DLNAObjectList::swap(DLNAObjectList& other)
{
	NPT_List<DLNAObject*> *temp = m_ls;
	m_ls = other.m_ls;
	other.m_ls = temp;
}

void DLNAObjectList::clear()
{
	for (NPT_Ordinal i = 0; i < m_ls->GetItemCount(); i++) {
		DLNAObject *obj = *m_ls->GetItem(i);
		obj->release();
	}
	m_ls->Clear();
}

} // namespace deejay
