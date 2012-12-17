#ifndef __DLNAObject_h__
#define __DLNAObject_h__

#include <Neptune.h>

namespace deejay {

class DLNAObjectImpl;
class DLNAItemImpl;
class DLNAContainerImpl;
class DLNAResourceImpl;

class DLNAItem;
class DLNAContainer;

class DLNAResource
{
public:
	DLNAResource();
	~DLNAResource();

	DLNAResourceImpl *d_ptr() const;

private:
	friend class DLNAResourceImpl;
	DLNAResourceImpl *m_impl;
};

struct DLNAPeopleInvolved
{
	NPT_String m_name;
	NPT_String m_role;
};

class DLNAObject
{
public:
	int addRef();
	int release();

	virtual DLNAItem *asItem();
	virtual const DLNAItem *asItem() const;
	virtual DLNAContainer *asContainer();
	virtual const DLNAContainer *asContainer() const;

	DLNAObjectImpl *d_ptr() const;

	const NPT_String& objectId() const;
	const NPT_String& parentId() const;
	const NPT_String& title() const;
	const NPT_String& creator() const;
	const NPT_String& upnpClass() const;
	const NPT_String& upnpClassName() const;
	const NPT_String& writeStatus() const;
	bool restricted() const;
	const NPT_String& dateStr() const;

	const NPT_List<NPT_String>& albumArtURIList() const;
	const NPT_List<DLNAResource*> resourceList() const;

	bool findThumbnailURL(int width, int height, const NPT_List<NPT_String>* preferredMimeTypes, NPT_String& url);

	NPT_Int64 resourceSize() const;

protected:
	DLNAObject(DLNAObjectImpl *impl);
	virtual ~DLNAObject();

private:
	DLNAObjectImpl *m_impl;
};

class DLNAItem
	: public DLNAObject
{
	friend class DLNAItemImpl;

public:
	virtual DLNAItem *asItem();
	virtual const DLNAItem *asItem() const;

	DLNAItemImpl *d_ptr() const;

	const NPT_String& refId() const;

protected:
	DLNAItem();
	virtual ~DLNAItem();
};

class DLNAContainer
	: public DLNAObject
{
	friend class DLNAContainerImpl;

public:
	virtual DLNAContainer *asContainer();
	virtual const DLNAContainer *asContainer() const;

	DLNAContainerImpl *d_ptr() const;

	int childCount() const;
	bool searchable() const;

protected:
	DLNAContainer();
	virtual ~DLNAContainer();
};

class DLNAObjectList
{
public:
	DLNAObjectList();
	DLNAObjectList(const DLNAObjectList& other);
	DLNAObjectList& operator=(const DLNAObjectList& other);
	~DLNAObjectList();

	void add(DLNAObject *obj);
	void add(const DLNAObjectList& ls);

	NPT_Cardinal count() const;
	DLNAObject *itemAt(NPT_Ordinal i) const;

	void swap(DLNAObjectList& other);
	void clear();

private:
	NPT_List<DLNAObject*> *m_ls;
};

} // namespace deejay

#endif // __DLNAObject_h__
