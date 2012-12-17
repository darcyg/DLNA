#ifndef __avcore4_h__
#define __avcore4_h__

#include <Neptune.h>

namespace deejay {

namespace didl {

class Object;
class Item;
class Container;
class RootContainer;

class Object
{
public:
	const NPT_String& id() const;
	Container *parent() const;
	RootContainer *root() const;

	virtual Item *asItem();
	virtual const Item *asItem() const;

	virtual Container *asContainer();
	virtual const Container *asContainer() const;

	const NPT_String& title() const;
	const NPT_String& creator() const;
	const NPT_String& className() const;
	const NPT_String& classDesc() const;
	const NPT_String& date() const;
	bool restricted() const;
	NPT_UInt64 size() const;
	NPT_UInt64 duration() const;
	NPT_String mimeType() const;
	NPT_String fileSystemPath() const;

	void setTitle(const NPT_String& title);
	void setCreator(const NPT_String& creator);
	void setClassName(const NPT_String& className);
	void setClassDesc(const NPT_String& classDesc);
	void setDate(const NPT_String& date);
	void setRestricted(bool restricted);
	void setSize(NPT_UInt64 size);
	void setDuration(NPT_UInt64 duration);
	void setMimeType(const NPT_String& mimeType);
	void setFileSystemPath(const NPT_String& path);

protected:
	Object(const NPT_String& title, const NPT_String& className, Container *parent, RootContainer *root);
	virtual ~Object();

	void emitContainerModification();

private:
	Object();
	Object(const Object&);
	Object& operator=(const Object&);

private:
	friend class Container;
	friend class RootContainer;

	Container *m_parent;
	RootContainer *m_root;

	NPT_String m_id;                 // object@id
	NPT_String m_title;              // dc:title
	NPT_String m_creator;            // dc:creator
	NPT_String m_className;          // upnp:class
	NPT_String m_classDesc;          // upnp:class@name
	NPT_String m_date;               // dc:date
	bool m_restricted;               // object@restricted
	NPT_UInt64 m_size;               // res@size
	NPT_UInt64 m_duration;           // res@duration
	NPT_String m_mimeType;
	NPT_String m_fsPath;
};

class Item
	: public Object
{
public:
	virtual Item *asItem();
	virtual const Item *asItem() const;

protected:
	Item(const NPT_String& title, const NPT_String& extHint, Container *parent, RootContainer *root);
	virtual ~Item();

private:
	friend class Container;
	friend class RootContainer;

	Object *m_refItem;
	NPT_String m_extHint;
};

class Container
	: public Object
{
public:
	virtual Container *asContainer();
	virtual const Container *asContainer() const;

	NPT_UInt32 updateID() const;

	bool searchable() const;

	void setSearchable(bool searchable);

	Container *newContainer(const NPT_String& title);
	Item *newItem(const NPT_String& title, const NPT_String& extHint = NPT_String());

	bool deleteContainer(Container *child);
	bool deleteItem(Item *child);

	NPT_Cardinal childCount() const;
	Object *childAt(NPT_Ordinal index);
	const Object *childAt(NPT_Ordinal index) const;

	NPT_List<Object*> getChildren();
	NPT_List<const Object*> getChildren() const;

protected:
	Container(const NPT_String& title, Container *parent, RootContainer *root);
	virtual ~Container();

private:
	void notifyContainerModification();
	void onFlushBatchMode();
	void incUpdateID();

private:
	friend class Object;
	friend class RootContainer;

	NPT_List<Object*> m_children;
	NPT_UInt32 m_updateID;
	bool m_pendingUpdate;

	bool m_searchable;               // container@searchable
};

class RootContainer
	: public Container
{
public:
	RootContainer();
	virtual ~RootContainer();

	void beginBatchModification();
	void endBatchModification();

	Object *findObject(const NPT_String& id);
	const Object *findObject(const NPT_String& id) const;
	NPT_UInt32 systemUpdateID() const;

	static NPT_String generateDidl(const NPT_List<Object*>& ls, const NPT_String& resUriTmpl);
	static NPT_String generateDidl(const NPT_List<const Object*>& ls, const NPT_String& resUriTmpl);

private:
	typedef NPT_Map<NPT_String, Object*> ObjectMap;

	void onNewContainer(Container *container, Container *parent);
	void onNewItem(Item *item, Container *parent);
	void onDeleteContainer(Container *container, Container *parent);
	void onDeleteItem(Item *item, Container *parent);
	void onContainerModified(Container *container);
	NPT_String generateContainerId();
	NPT_String generateItemId(Item *item);
	void incSystemUpdateID();

	static void outputItem(NPT_XmlSerializer& xml, const Item *obj, const NPT_String& resUriTmpl);
	static void outputContainer(NPT_XmlSerializer& xml, const Container *obj, const NPT_String& resUriTmpl);
	static void outputObject(NPT_XmlSerializer& xml, const Object *obj, const NPT_String& resUriTmpl);

private:
	friend class Container;

	NPT_UInt64 m_seq;
	bool m_inDestructor;
	bool m_batchMode;
	bool m_batchMode1;
	bool m_pendingUpdate;
	NPT_UInt32 m_systemUpdateID;
	ObjectMap m_objectMap;
	ObjectMap m_containerMap;
};

} // namespace didl

} // namespace deejay

#endif // __avcore4_h__
