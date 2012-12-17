#include "DJDesc.h"
#include "DJDescPriv.h"

namespace deejay {

DeviceDesc::DeviceDesc()
{
	m_private = new DeviceDescPrivate();
}

DeviceDesc::~DeviceDesc()
{
	delete m_private;
}

void DeviceDesc::addRef()
{
	if (m_private->m_parent) {
		m_private->m_parent->addRef();
	} else {
		m_private->m_refCount.Increment();
	}
}

void DeviceDesc::release()
{
	if (m_private->m_parent) {
		m_private->m_parent->release();
	} else {
		if (m_private->m_refCount.Decrement() == 0) {
			delete this;
		}
	}
}

const NPT_String& DeviceDesc::deviceType() const
{
	return m_private->m_deviceType;
}

const NPT_String& DeviceDesc::friendlyName() const
{
	return m_private->m_friendlyName;
}

const NPT_String& DeviceDesc::manufacturer() const
{
	return m_private->m_manufacturer;
}

const NPT_String& DeviceDesc::manufacturerURL() const
{
	return m_private->m_manufacturerURL;
}

const NPT_String& DeviceDesc::modelDescription() const
{
	return m_private->m_modelDescription;
}

const NPT_String& DeviceDesc::modelName() const
{
	return m_private->m_modelName;
}

const NPT_String& DeviceDesc::modelNumber() const
{
	return m_private->m_modelNumber;
}

const NPT_String& DeviceDesc::modelURL() const
{
	return m_private->m_modelURL;
}

const NPT_String& DeviceDesc::serialNumber() const
{
	return m_private->m_serialNumber;
}

const NPT_String& DeviceDesc::upc() const
{
	return m_private->m_upc;
}

const NPT_String& DeviceDesc::presentationURL() const
{
	return m_private->m_presentationURL;
}

const UUID& DeviceDesc::uuid() const
{
	return m_private->m_uuid;
}

const NPT_String& DeviceDesc::rootDescURL() const
{
	return m_private->m_rootDescURL;
}

NPT_Cardinal DeviceDesc::childCount() const
{
	return m_private->m_children.GetItemCount();
}

DeviceDesc *DeviceDesc::childAt(NPT_Ordinal index) const
{
	if (index < childCount()) {
		return *m_private->m_children.GetItem(index);
	}
	return NULL;
}

bool DeviceDesc::getChildAt(NPT_Ordinal index, DeviceDesc*& child) const
{
	if (index < childCount()) {
		child = *m_private->m_children.GetItem(index);
		child->addRef();
		return true;
	}
	return false;
}

NPT_Cardinal DeviceDesc::iconCount() const
{
	return m_private->m_iconList.GetItemCount();
}

IconDesc *DeviceDesc::iconAt(NPT_Ordinal index) const
{
	if (index < iconCount()) {
		return *m_private->m_iconList.GetItem(index);
	}
	return NULL;
}

NPT_Cardinal DeviceDesc::serviceCount() const
{
	return m_private->m_serviceList.GetItemCount();
}

ServiceDesc *DeviceDesc::serviceAt(NPT_Ordinal index) const
{
	if (index < serviceCount()) {
		return *m_private->m_serviceList.GetItem(index);
	}
	return NULL;
}

ServiceDesc *DeviceDesc::findServiceById(const NPT_String& serviceId) const
{
	NPT_List<ServiceDesc*>::Iterator it = m_private->m_serviceList.Find(ServiceDescByIdFinder(serviceId));
	if (it) {
		return *it;
	}
	return NULL;
}

ServiceDesc *DeviceDesc::findServiceByType(const NPT_String& serviceType) const
{
	NPT_List<ServiceDesc*>::Iterator it = m_private->m_serviceList.Find(ServiceDescByTypeFinder(serviceType));
	if (it) {
		return *it;
	}
	return NULL;
}

bool DeviceDesc::hasServiceType(const NPT_String& serviceType) const
{
	return findServiceByType(serviceType) != NULL;
}

bool DeviceDesc::matchDeviceType(const NPT_String& deviceType) const
{
	if (deviceType.EndsWith(":*")) {
		return m_private->m_deviceType.CompareN(deviceType, deviceType.GetLength() - 1) == 0;
	}
	return m_private->m_deviceType.Compare(deviceType) == 0;
}

ServiceDesc::ServiceDesc()
{
	m_private = new ServiceDescPrivate();
}

ServiceDesc::~ServiceDesc()
{
	delete m_private;
}

const NPT_String& ServiceDesc::serviceId() const
{
	return m_private->m_serviceId;
}

const NPT_String& ServiceDesc::serviceType() const
{
	return m_private->m_serviceType;
}

const NPT_String& ServiceDesc::scpdURL() const
{
	return m_private->m_scpdURL;
}

const NPT_String& ServiceDesc::eventSubURL() const
{
	return m_private->m_eventSubURL;
}

const NPT_String& ServiceDesc::controlURL() const
{
	return m_private->m_controlURL;
}

NPT_Cardinal ServiceDesc::actionCount() const
{
	return m_private->m_actionList.GetItemCount();
}

ActionDesc *ServiceDesc::actionAt(NPT_Ordinal index) const
{
	if (index < actionCount()) {
		return *m_private->m_actionList.GetItem(index);
	}
	return NULL;
}

ActionDesc *ServiceDesc::findAction(const NPT_String& name) const
{
	for (NPT_Ordinal i = 0; i < m_private->m_actionList.GetItemCount(); i++) {
		ActionDesc *actionDesc = *m_private->m_actionList.GetItem(i);
		if (actionDesc->name().Compare(name, true) == 0) {
			return actionDesc;
		}
	}
	return NULL;
}

bool ServiceDesc::hasAction(const NPT_String& name) const
{
	return findAction(name) != NULL;
}

NPT_Cardinal ServiceDesc::stateVariableCount() const
{
	return m_private->m_stateVariableList.GetItemCount();
}

StateVariableDesc *ServiceDesc::stateVariableAt(NPT_Ordinal index) const
{
	if (index < stateVariableCount()) {
		return *m_private->m_stateVariableList.GetItem(index);
	}
	return NULL;
}

StateVariableDesc *ServiceDesc::findStateVariable(const NPT_String& name) const
{
	for (NPT_Ordinal i = 0; i < m_private->m_stateVariableList.GetItemCount(); i++) {
		StateVariableDesc *stateVariableDesc = *m_private->m_stateVariableList.GetItem(i);
		if (stateVariableDesc->name().Compare(name, true) == 0) {
			return stateVariableDesc;
		}
	}
	return NULL;
}

bool ServiceDesc::hasStateVariable(const NPT_String name) const
{
	return findStateVariable(name) != NULL;
}

IconDesc::IconDesc()
{
	m_private = new IconDescPrivate();
}

IconDesc::~IconDesc()
{
	delete m_private;
}

const NPT_String& IconDesc::mimeType() const
{
	return m_private->m_mimeType;
}

NPT_UInt32 IconDesc::width() const
{
	return m_private->m_width;
}

NPT_UInt32 IconDesc::height() const
{
	return m_private->m_height;
}

NPT_UInt32 IconDesc::depth() const
{
	return m_private->m_depth;
}

const NPT_String& IconDesc::url() const
{
	return m_private->m_url;
}

const NPT_DataBuffer& IconDesc::iconData() const
{
	return m_private->m_iconData;
}

ActionDesc::ActionDesc()
{
	m_private = new ActionDescPrivate();
}

ActionDesc::~ActionDesc()
{
	delete m_private;
}

const NPT_String& ActionDesc::name() const
{
	return m_private->m_name;
}

NPT_Cardinal ActionDesc::argumentCount() const
{
	return m_private->m_argumentList.GetItemCount();
}

NPT_Cardinal ActionDesc::inputArgumentCount() const
{
	return m_private->m_inputArgumentCount;
}

NPT_Cardinal ActionDesc::outputArgumentCount() const
{
	return argumentCount() - m_private->m_inputArgumentCount;
}

ArgumentDesc *ActionDesc::argumentAt(NPT_Ordinal index) const
{
	if (index < argumentCount()) {
		return *m_private->m_argumentList.GetItem(index);
	}
	return NULL;
}

ArgumentDesc::ArgumentDesc()
{
	m_private = new ArgumentDescPrivate();
}

ArgumentDesc::~ArgumentDesc()
{
	delete m_private;
}

const NPT_String& ArgumentDesc::name() const
{
	return m_private->m_name;
}

bool ArgumentDesc::isInputArg() const
{
	return m_private->m_input;
}

bool ArgumentDesc::isOutputArg() const
{
	return !m_private->m_input;
}

const NPT_String& ArgumentDesc::relatedStateVariableName() const
{
	return m_private->m_relatedStateVariableName;
}

StateVariableDesc::StateVariableDesc()
{
	m_private = new StateVariableDescPrivate();
}

StateVariableDesc::~StateVariableDesc()
{
	delete m_private;
}

const NPT_String& StateVariableDesc::name() const
{
	return m_private->m_name;
}

const NPT_String& StateVariableDesc::dataType() const
{
	return m_private->m_dataType;
}

const NPT_String& StateVariableDesc::defaultValue() const
{
	return m_private->m_defaultValue;
}

NPT_Cardinal StateVariableDesc::allowedValueCount() const
{
	return m_private->m_allowedValueList.GetItemCount();
}

const NPT_String *StateVariableDesc::allowedValueAt(NPT_Ordinal index) const
{
	if (index < allowedValueCount()) {
		return &(*m_private->m_allowedValueList.GetItem(index));
	}
	return NULL;
}

bool StateVariableDesc::hasAllowedValueRange() const
{
	return m_private->m_hasAllowedMinimum || m_private->m_hasAllowedMaximum || m_private->m_hasAllowedStep;
}

bool StateVariableDesc::hasAllowedValueRangeMinimum() const
{
	return m_private->m_hasAllowedMinimum;
}

bool StateVariableDesc::hasAllowedValueRangeMaximum() const
{
	return m_private->m_hasAllowedMaximum;
}

bool StateVariableDesc::hasAllowedValueRangeStep() const
{
	return m_private->m_hasAllowedStep;
}

const NPT_String *StateVariableDesc::allowedValueRangeMinimum() const
{
	return m_private->m_hasAllowedMinimum ? &m_private->m_allowedMinimum : NULL;
}

const NPT_String *StateVariableDesc::allowedValueRangeMaximum() const
{
	return m_private->m_hasAllowedMaximum ? &m_private->m_allowedMaximum : NULL;
}

const NPT_String *StateVariableDesc::allowedValueRangeStep() const
{
	return m_private->m_hasAllowedStep ? &m_private->m_allowedStep : NULL;
}

DeviceDescList::DeviceDescList()
{
	m_ls = new NPT_List<DeviceDesc*>();
}

DeviceDescList::DeviceDescList(const DeviceDescList& other)
{
	m_ls = new NPT_List<DeviceDesc*>();
	for (NPT_Ordinal i = 0; i < other.m_ls->GetItemCount(); i++) {
		DeviceDesc *deviceDesc = *other.m_ls->GetItem(i);
		deviceDesc->addRef();
		m_ls->Add(deviceDesc);
	}
}

DeviceDescList& DeviceDescList::operator=(const DeviceDescList& other)
{
	clear();
	for (NPT_Ordinal i = 0; i < other.m_ls->GetItemCount(); i++) {
		DeviceDesc *deviceDesc = *other.m_ls->GetItem(i);
		deviceDesc->addRef();
		m_ls->Add(deviceDesc);
	}
	return *this;
}

DeviceDescList::~DeviceDescList()
{
	clear();
	delete m_ls;
}

void DeviceDescList::add(DeviceDesc *deviceDesc)
{
	deviceDesc->addRef();
	m_ls->Add(deviceDesc);
}

void DeviceDescList::clear()
{
	for (NPT_Ordinal i = 0; i < m_ls->GetItemCount(); i++) {
		DeviceDesc *deviceDesc = *m_ls->GetItem(i);
		deviceDesc->release();
	}
	m_ls->Clear();
}

void DeviceDescList::remove(const UUID& uuid)
{
	for (NPT_Ordinal i = 0; i < m_ls->GetItemCount(); i++) {
		NPT_List<DeviceDesc*>::Iterator it = m_ls->GetItem(i);
		DeviceDesc *deviceDesc = *it;
		if (deviceDesc->uuid() == uuid) {
			m_ls->Erase(it);
			deviceDesc->release();
			break;
		}
	}
}

NPT_Cardinal DeviceDescList::count() const
{
	return m_ls->GetItemCount();
}

DeviceDesc *DeviceDescList::itemAt(NPT_Ordinal i) const
{
	if (i < count()) {
		return *m_ls->GetItem(i);
	}
	return NULL;
}

DeviceDesc *DeviceDescList::find(const UUID& uuid) const
{
	for (NPT_Ordinal i = 0; i < m_ls->GetItemCount(); i++) {
		DeviceDesc *deviceDesc = *m_ls->GetItem(i);
		if (deviceDesc->uuid() == uuid) {
			return deviceDesc;
		}
	}
	return NULL;
}

void DeviceDescList::differ(const DeviceDescList& oldList, const DeviceDescList& newList, DeviceDescList& removedList, DeviceDescList& addedList, DeviceDescList& commonList)
{
	removedList.clear();
	addedList.clear();
	commonList.clear();
	for (NPT_Ordinal i = 0; i < newList.count(); i++) {
		deejay::DeviceDesc *commonItem = oldList.find(newList.itemAt(i)->uuid());
		if (commonItem) {
			commonList.add(commonItem);
		} else {
			addedList.add(newList.itemAt(i));
		}
	}

	for (NPT_Ordinal i = 0; i < oldList.count(); i++) {
		deejay::DeviceDesc *commonItem = newList.find(oldList.itemAt(i)->uuid());
		if (!commonItem) {
			removedList.add(oldList.itemAt(i));
		}
	}
}

void DeviceDescList::swap(DeviceDescList& other)
{
	NPT_List<DeviceDesc*> *temp = m_ls;
	m_ls = other.m_ls;
	other.m_ls = temp;
}

} // namespace deejay
