#ifndef __DJDesc_h__
#define __DJDesc_h__

#include "DJUtils.h"

namespace deejay {

class DeviceDesc;
class ServiceDesc;
class IconDesc;
class ActionDesc;
class ArgumentDesc;
class StateVariableDesc;

class DeviceDescPrivate;
class ServiceDescPrivate;
class IconDescPrivate;
class ActionDescPrivate;
class ArgumentDescPrivate;
class StateVariableDescPrivate;

class DeviceDesc
{
public:
	void addRef();
	void release();

	const NPT_String& deviceType() const;
	const NPT_String& friendlyName() const;
	const NPT_String& manufacturer() const;
	const NPT_String& manufacturerURL() const;
	const NPT_String& modelDescription() const;
	const NPT_String& modelName() const;
	const NPT_String& modelNumber() const;
	const NPT_String& modelURL() const;
	const NPT_String& serialNumber() const;
	const NPT_String& upc() const;
	const NPT_String& presentationURL() const;
	const UUID& uuid() const;

	const NPT_String& rootDescURL() const;

	NPT_Cardinal childCount() const;
	DeviceDesc *childAt(NPT_Ordinal index) const;
	bool getChildAt(NPT_Ordinal index, DeviceDesc*& child) const;

	NPT_Cardinal serviceCount() const;
	ServiceDesc *serviceAt(NPT_Ordinal index) const;
	ServiceDesc *findServiceById(const NPT_String& serviceId) const;
	ServiceDesc *findServiceByType(const NPT_String& serviceType) const;
	bool hasServiceType(const NPT_String& serviceType) const;

	bool matchDeviceType(const NPT_String& deviceType) const;

	NPT_Cardinal iconCount() const;
	IconDesc *iconAt(NPT_Ordinal index) const;

private:
	friend class DeviceDescPrivate;

	DeviceDesc();
	~DeviceDesc();
	DeviceDesc(const DeviceDesc&);
	DeviceDesc& operator=(const DeviceDesc&);
	DeviceDescPrivate *m_private;
};

class ServiceDesc
{
public:
	const NPT_String& serviceId() const;
	const NPT_String& serviceType() const;
	const NPT_String& scpdURL() const;
	const NPT_String& eventSubURL() const;
	const NPT_String& controlURL() const;

	NPT_Cardinal actionCount() const;
	ActionDesc *actionAt(NPT_Ordinal index) const;
	ActionDesc *findAction(const NPT_String& name) const;
	bool hasAction(const NPT_String& name) const;

	NPT_Cardinal stateVariableCount() const;
	StateVariableDesc *stateVariableAt(NPT_Ordinal index) const;
	StateVariableDesc *findStateVariable(const NPT_String& name) const;
	bool hasStateVariable(const NPT_String name) const;

private:
	friend class ServiceDescPrivate;

	ServiceDesc();
	~ServiceDesc();
	ServiceDesc(const ServiceDesc&);
	ServiceDesc& operator=(const ServiceDesc&);
	ServiceDescPrivate *m_private;
};

class IconDesc
{
public:
	const NPT_String& mimeType() const;
	NPT_UInt32 width() const;
	NPT_UInt32 height() const;
	NPT_UInt32 depth() const;
	const NPT_String& url() const;
	const NPT_DataBuffer& iconData() const;

private:
	friend class IconDescPrivate;

	IconDesc();
	~IconDesc();
	IconDesc(const IconDesc&);
	IconDesc& operator=(const IconDesc&);
	IconDescPrivate *m_private;
};

class ActionDesc
{
public:
	const NPT_String& name() const;
	NPT_Cardinal argumentCount() const;
	NPT_Cardinal inputArgumentCount() const;
	NPT_Cardinal outputArgumentCount() const;
	ArgumentDesc *argumentAt(NPT_Ordinal index) const;

private:
	friend class ActionDescPrivate;

	ActionDesc();
	~ActionDesc();
	ActionDesc(const ActionDesc&);
	ActionDesc& operator=(const ActionDesc&);
	ActionDescPrivate *m_private;
};

class ArgumentDesc
{
public:
	const NPT_String& name() const;
	bool isInputArg() const;
	bool isOutputArg() const;
	const NPT_String& relatedStateVariableName() const;

private:
	friend class ArgumentDescPrivate;

	ArgumentDesc();
	~ArgumentDesc();
	ArgumentDesc(const ArgumentDesc&);
	ArgumentDesc& operator=(const ArgumentDesc&);
	ArgumentDescPrivate *m_private;
};

class StateVariableDesc
{
public:
	const NPT_String& name() const;
	const NPT_String& dataType() const;
	const NPT_String& defaultValue() const;
	NPT_Cardinal allowedValueCount() const;
	const NPT_String *allowedValueAt(NPT_Ordinal index) const;
	bool hasAllowedValueRange() const;
	bool hasAllowedValueRangeMinimum() const;
	bool hasAllowedValueRangeMaximum() const;
	bool hasAllowedValueRangeStep() const;
	const NPT_String *allowedValueRangeMinimum() const;
	const NPT_String *allowedValueRangeMaximum() const;
	const NPT_String *allowedValueRangeStep() const;

private:
	friend class StateVariableDescPrivate;

	StateVariableDesc();
	~StateVariableDesc();
	StateVariableDesc(const StateVariableDesc&);
	StateVariableDesc& operator=(const StateVariableDesc&);
	StateVariableDescPrivate *m_private;
};

class DeviceDescHolder
{
public:
	DeviceDescHolder(DeviceDesc *deviceDesc)
		: m_deviceDesc(deviceDesc)
	{
	}

	~DeviceDescHolder()
	{
		if (m_deviceDesc) {
			m_deviceDesc->release();
		}
	}

	DeviceDesc *m_deviceDesc;
};

class DeviceDescList
{
public:
	DeviceDescList();
	DeviceDescList(const DeviceDescList& other);
	DeviceDescList& operator=(const DeviceDescList& other);
	~DeviceDescList();

	void add(DeviceDesc *deviceDesc);

	NPT_Cardinal count() const;
	DeviceDesc *itemAt(NPT_Ordinal i) const;
	DeviceDesc *find(const UUID& uuid) const;

	static void differ(const DeviceDescList& oldList, const DeviceDescList& newList, DeviceDescList& removedList, DeviceDescList& addedList, DeviceDescList& commonList);

	void swap(DeviceDescList& other);
	void clear();
	void remove(const UUID& uuid);

private:
	NPT_List<DeviceDesc*> *m_ls;
};

} // namespace deejay

#endif // __DJDesc_h__
