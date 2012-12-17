#include "avcore4.h"

namespace deejay {

namespace didl {

static const NPT_UInt64 NO_VALUE = 0xFFFFFFFFFFFFFFFFLL;

static NPT_String formatDuration(NPT_UInt64 duration)
{
	NPT_UInt64 seconds = duration / 1000;
	NPT_UInt64 ms = duration - (seconds * 1000);
	NPT_UInt64 minutes = seconds / 60;
	seconds -= minutes * 60;
	NPT_UInt64 hours = minutes / 60;
	minutes -= hours * 60;
	//return NPT_String::Format("%d:%02d:%02d:%03d", static_cast<NPT_UInt32>(hours), static_cast<NPT_UInt32>(minutes), static_cast<NPT_UInt32>(seconds), static_cast<NPT_UInt32>(ms));
	return NPT_String::Format("%d:%02d:%02d", static_cast<NPT_UInt32>(hours), static_cast<NPT_UInt32>(minutes), static_cast<NPT_UInt32>(seconds));
}

Object::Object(const NPT_String& title, const NPT_String& className, Container *parent, RootContainer *root)
	: m_title(title), m_className(className), m_parent(parent), m_root(root), m_restricted(true), m_size(NO_VALUE), m_duration(NO_VALUE)
{
}

Object::~Object()
{
}

const NPT_String& Object::id() const
{
	return m_id;
}

Container *Object::parent() const
{
	return m_parent;
}

RootContainer *Object::root() const
{
	return m_root;
}

Item *Object::asItem()
{
	return NULL;
}

const Item *Object::asItem() const
{
	return NULL;
}

Container *Object::asContainer()
{
	return NULL;
}

const Container *Object::asContainer() const
{
	return NULL;
}

void Object::emitContainerModification()
{
	if (parent()) {
		parent()->notifyContainerModification();
	}
	if (asContainer()) {
		asContainer()->notifyContainerModification();
	}
}

const NPT_String& Object::title() const
{
	return m_title;
}

const NPT_String& Object::creator() const
{
	return m_creator;
}

const NPT_String& Object::className() const
{
	return m_className;
}

const NPT_String& Object::classDesc() const
{
	return m_classDesc;
}

const NPT_String& Object::date() const
{
	return m_date;
}

bool Object::restricted() const
{
	return m_restricted;
}

NPT_UInt64 Object::size() const
{
	return m_size;
}

NPT_UInt64 Object::duration() const
{
	return m_duration;
}

NPT_String Object::mimeType() const
{
	return m_mimeType;
}

NPT_String Object::fileSystemPath() const
{
	return m_fsPath;
}

void Object::setTitle(const NPT_String& title)
{
	if (m_title != title) {
		m_title = title;
		emitContainerModification();
	}
}

void Object::setCreator(const NPT_String& creator)
{
	if (m_creator != creator) {
		m_creator = creator;
		emitContainerModification();
	}
}

void Object::setClassName(const NPT_String& className)
{
	if (m_className != className) {
		m_className = className;
		emitContainerModification();
	}
}

void Object::setClassDesc(const NPT_String& classDesc)
{
	if (m_classDesc != classDesc) {
		m_classDesc = classDesc;
		emitContainerModification();
	}
}

void Object::setDate(const NPT_String& date)
{
	if (m_date != date) {
		m_date = date;
		emitContainerModification();
	}
}

void Object::setRestricted(bool restricted)
{
	if (m_restricted != restricted) {
		m_restricted = restricted;
		emitContainerModification();
	}
}

void Object::setSize(NPT_UInt64 size)
{
	if (m_size != size) {
		m_size = size;
		emitContainerModification();
	}
}

void Object::setDuration(NPT_UInt64 duration)
{
	if (m_duration != duration) {
		m_duration = duration;
		emitContainerModification();
	}
}

void Object::setMimeType(const NPT_String& mimeType)
{
	if (m_mimeType != mimeType) {
		m_mimeType = mimeType;
		emitContainerModification();
	}
}

void Object::setFileSystemPath(const NPT_String& path)
{
	m_fsPath = path;
}

Item::Item(const NPT_String& title, const NPT_String& extHint, Container *parent, RootContainer *root)
	: Object(title, "object.item", parent, root), m_extHint(extHint), m_refItem(NULL)
{
}

Item::~Item()
{
}

Item *Item::asItem()
{
	return this;
}

const Item *Item::asItem() const
{
	return this;
}

Container::Container(const NPT_String& title, Container *parent, RootContainer *root)
	: Object(title, "object.container", parent, root), m_updateID(0), m_pendingUpdate(false)
	, m_searchable(true)
{
}

Container::~Container()
{
	for (NPT_Ordinal i = 0; i < m_children.GetItemCount(); i++) {
		Object *childObj = *m_children.GetItem(i);
		if (childObj->asContainer()) {
			root()->onDeleteContainer(childObj->asContainer(), this);
		} else {
			root()->onDeleteItem(childObj->asItem(), this);
		}
		delete childObj;
	}
	m_children.Clear();
}

void Container::notifyContainerModification()
{
	if (root()->m_batchMode) {
		m_pendingUpdate = true;
	} else {
		incUpdateID();
	}
}

void Container::onFlushBatchMode()
{
	if (m_pendingUpdate) {
		m_pendingUpdate = false;
		incUpdateID();
	}
}

void Container::incUpdateID()
{
	if (m_updateID == 0xFFFFFFFF) {
		m_updateID = 0;
	} else {
		++m_updateID;
	}
	root()->onContainerModified(this);
}

Container *Container::asContainer()
{
	return this;
}

const Container *Container::asContainer() const
{
	return this;
}

NPT_UInt32 Container::updateID() const
{
	return m_updateID;
}

bool Container::searchable() const
{
	return m_searchable;
}

void Container::setSearchable(bool searchable)
{
	if (m_searchable != searchable) {
		m_searchable = searchable;
		emitContainerModification();
	}
}

Container *Container::newContainer(const NPT_String& title)
{
	Container *child = new Container(title, this, root());
	m_children.Add(child);
	root()->onNewContainer(child, this);
	notifyContainerModification();
	return child;
}

Item *Container::newItem(const NPT_String& title, const NPT_String& extHint)
{
	Item *child = new Item(title, extHint, this, root());
	m_children.Add(child);
	root()->onNewItem(child, this);
	notifyContainerModification();
	return child;
}

bool Container::deleteContainer(Container *child)
{
	Object *o = child;
	NPT_List<Object*>::Iterator it = m_children.Find(NPT_ObjectComparator<Object*>(o));
	if (it) {
		root()->onDeleteContainer(child, this);
		m_children.Erase(it);
		delete child;
		notifyContainerModification();
		return true;
	}
	return false;
}

bool Container::deleteItem(Item *child)
{
	Object *o = child;
	NPT_List<Object*>::Iterator it = m_children.Find(NPT_ObjectComparator<Object*>(o));
	if (it) {
		root()->onDeleteItem(child, this);
		m_children.Erase(it);
		delete child;
		notifyContainerModification();
		return true;
	}
	return false;
}

NPT_Cardinal Container::childCount() const
{
	return m_children.GetItemCount();
}

Object *Container::childAt(NPT_Ordinal index)
{
	Object *child = NULL;
	if (index < childCount()) {
		child = *m_children.GetItem(index);
	}
	return child;
}

const Object *Container::childAt(NPT_Ordinal index) const
{
	Object *child = NULL;
	if (index < childCount()) {
		child = *m_children.GetItem(index);
	}
	return child;
}

NPT_List<Object*> Container::getChildren()
{
	NPT_List<Object*> ls;
	for (NPT_Ordinal i = 0; i < childCount(); i++) {
		ls.Add(childAt(i));
	}
	return ls;
}

NPT_List<const Object*> Container::getChildren() const
{
	NPT_List<const Object*> ls;
	for (NPT_Ordinal i = 0; i < childCount(); i++) {
		ls.Add(childAt(i));
	}
	return ls;
}

RootContainer::RootContainer()
	: Container("root", NULL, this), m_seq(0), m_inDestructor(false), m_batchMode(false), m_batchMode1(false), m_pendingUpdate(false)
	, m_systemUpdateID(0)
{
	m_id = "0";
	m_objectMap.Put(m_id, this);
	m_containerMap.Put(m_id, this);
}

RootContainer::~RootContainer()
{
	m_inDestructor = true;
}

void RootContainer::beginBatchModification()
{
	if (!m_batchMode) {
		m_batchMode = true;
		m_batchMode1 = true;
	}
}

void RootContainer::endBatchModification()
{
	if (m_batchMode) {
		m_batchMode = false;

		const NPT_List<ObjectMap::Entry*>& ls = m_containerMap.GetEntries();
		for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
			ObjectMap::Entry *e = *ls.GetItem(i);
			Container *container = e->GetValue()->asContainer();
			container->onFlushBatchMode();
		}

		m_batchMode1 = false;

		if (m_pendingUpdate) {
			m_pendingUpdate = false;
			incSystemUpdateID();
		}
	}
}

void RootContainer::onNewContainer(Container *container, Container *parent)
{
	container->m_id = generateContainerId();
	m_objectMap.Put(container->m_id, container);
	m_containerMap.Put(container->m_id, container);
}

void RootContainer::onNewItem(Item *item, Container *parent)
{
	item->m_id = generateItemId(item);
	m_objectMap.Put(item->m_id, item);
}

void RootContainer::onDeleteContainer(Container *container, Container *parent)
{
	if (!m_inDestructor) {
		m_objectMap.Erase(container->m_id);
		m_containerMap.Erase(container->m_id);
	}
}

void RootContainer::onDeleteItem(Item *item, Container *parent)
{
	if (!m_inDestructor) {
		m_objectMap.Erase(item->m_id);
	}
}

void RootContainer::onContainerModified(Container *container)
{
	if (m_batchMode1) {
		m_pendingUpdate = true;
	} else {
		incSystemUpdateID();
	}
}

void RootContainer::incSystemUpdateID()
{
	if (m_systemUpdateID == 0xFFFFFFFF) {
		m_systemUpdateID = 0;
	} else {
		++m_systemUpdateID;
	}
}

NPT_String RootContainer::generateContainerId()
{
	static const char ab[] = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const NPT_UInt32 ac = sizeof(ab) - 1;
	NPT_UInt64 seq = m_seq++;
	NPT_UInt64 q;
	NPT_UInt64 r;

	NPT_String result;

	for (;;) {
		q = seq / ac;
		r = seq - (q * ac);
		result.Append(ab + r, 1);
		if (q == 0) {
			break;
		}
		seq = q;
	}
	return result;
}

NPT_String RootContainer::generateItemId(Item *item)
{
	return generateContainerId() + item->m_extHint;
}

Object *RootContainer::findObject(const NPT_String& id)
{
	Object **obj;
	if (NPT_SUCCEEDED(m_objectMap.Get(id, obj))) {
		return *obj;
	}
	return NULL;
}

const Object *RootContainer::findObject(const NPT_String& id) const
{
	Object **obj;
	if (NPT_SUCCEEDED(m_objectMap.Get(id, obj))) {
		return *obj;
	}
	return NULL;
}

NPT_UInt32 RootContainer::systemUpdateID() const
{
	return m_systemUpdateID;
}

NPT_String RootContainer::generateDidl(const NPT_List<Object*>& ls, const NPT_String& resUriTmpl)
{
	NPT_StringOutputStream outputStream;
	NPT_XmlSerializer xml(&outputStream, 0, true, true);

	xml.StartDocument();
	xml.StartElement(NULL, "DIDL-Lite");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
	xml.Attribute("xmlns", "dc", "http://purl.org/dc/elements/1.1/");
	xml.Attribute("xmlns", "upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");

	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		Object *obj = *ls.GetItem(i);
		if (Item *item = obj->asItem()) {
			outputItem(xml, item, resUriTmpl);
		} else if (Container *container = obj->asContainer()) {
			outputContainer(xml, container, resUriTmpl);
		}
	}

	xml.EndElement(NULL, "DIDL-Lite");
	xml.EndDocument();
	return outputStream.GetString();
}

NPT_String RootContainer::generateDidl(const NPT_List<const Object*>& ls, const NPT_String& resUriTmpl)
{
	NPT_StringOutputStream outputStream;
	NPT_XmlSerializer xml(&outputStream, 0, true, true);

	xml.StartDocument();
	xml.StartElement(NULL, "DIDL-Lite");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
	xml.Attribute("xmlns", "dc", "http://purl.org/dc/elements/1.1/");
	xml.Attribute("xmlns", "upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");

	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		const Object *obj = *ls.GetItem(i);
		if (const Item *item = obj->asItem()) {
			outputItem(xml, item, resUriTmpl);
		} else if (const Container *container = obj->asContainer()) {
			outputContainer(xml, container, resUriTmpl);
		}
	}

	xml.EndElement(NULL, "DIDL-Lite");
	xml.EndDocument();
	return outputStream.GetString();
}

void RootContainer::outputObject(NPT_XmlSerializer& xml, const Object *obj, const NPT_String& resUriTmpl)
{
	xml.StartElement("dc", "title");
	xml.Text(obj->title());
	xml.EndElement("dc", "title");

	if (!obj->creator().IsEmpty()) {
		xml.StartElement("dc", "creator");
		xml.Text(obj->creator());
		xml.EndElement("dc", "creator");
	}

	if (!obj->date().IsEmpty()) {
		xml.StartElement("dc", "date");
		xml.Text(obj->date());
		xml.EndElement("dc", "date");
	}

	xml.StartElement("upnp", "class");
	if (!obj->classDesc().IsEmpty()) {
		xml.Attribute(NULL, "name", obj->classDesc());
	}
	xml.Text(obj->className());
	xml.EndElement("upnp", "class");

	if (!obj->mimeType().IsEmpty() && !obj->fileSystemPath().IsEmpty()) {
		xml.StartElement(NULL, "res");
		//xml.Attribute(NULL, "protocolInfo", NPT_String::Format("http-get:*:%s:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=00d00000000000000000000000000000", obj->mimeType().GetChars()));
		xml.Attribute(NULL, "protocolInfo", NPT_String::Format("http-get:*:%s:*", obj->mimeType().GetChars()));
		if (obj->size() != NO_VALUE) {
			xml.Attribute(NULL, "size", NPT_String::FromIntegerU(obj->size()));
		}

		if (obj->duration() != NO_VALUE) {
			xml.Attribute(NULL, "duration", formatDuration(obj->duration()));
		}

		NPT_String uu = NPT_String::Format(resUriTmpl, obj->id().GetChars());
		xml.Text(uu);
/*		if (obj->className().StartsWith("object.item.imageItem")) {
			xml.StartElement("upnp", "album");
			xml.Text("[unknown]");
			xml.EndElement("upnp", "album");
			xml.StartElement("upnp", "albumArtURI");
			xml.Text(uu);
			xml.EndElement("upnp", "albumArtURI");
		}*/
		//xml.Text("http://img0.pconline.com.cn/pconline/test/focus/bbs/1111/88.jpg");

		xml.EndElement(NULL, "res");
	}

}

void RootContainer::outputItem(NPT_XmlSerializer& xml, const Item *obj, const NPT_String& resUriTmpl)
{
	xml.StartElement(NULL, "item");
	xml.Attribute(NULL, "id", obj->id());
	xml.Attribute(NULL, "parentID", obj->parent() ? obj->parent()->id() : "-1");
	xml.Attribute(NULL, "restricted", "0");//obj->restricted() ? "true" : "false");

	outputObject(xml, obj, resUriTmpl);

	xml.EndElement(NULL, "item");
}

void RootContainer::outputContainer(NPT_XmlSerializer& xml, const Container *obj, const NPT_String& resUriTmpl)
{
	xml.StartElement(NULL, "container");
	xml.Attribute(NULL, "id", obj->id());
	xml.Attribute(NULL, "parentID", obj->parent() ? obj->parent()->id() : "-1");
	xml.Attribute(NULL, "childCount", NPT_String::FromInteger(obj->childCount()));
	xml.Attribute(NULL, "searchable", obj->searchable() ? "true" : "false");
	xml.Attribute(NULL, "restricted", obj->restricted() ? "true" : "false");

	outputObject(xml, obj, resUriTmpl);

	xml.EndElement(NULL, "container");
}

} // namespace didl

} // namespace deejay
