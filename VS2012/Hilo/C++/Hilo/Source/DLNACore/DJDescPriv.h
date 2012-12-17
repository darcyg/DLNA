#ifndef __DJDescPriv_h__
#define __DJDescPriv_h__

#include "DJDesc.h"

namespace deejay {

class DeviceDescPrivate
{
public:
	DeviceDescPrivate();
	~DeviceDescPrivate();

	static DeviceDesc *newDeviceDesc();
	static DeviceDescPrivate *getPrivate(DeviceDesc *deviceDesc);

	NPT_AtomicVariable m_refCount;
	DeviceDesc *m_parent;

	NPT_String m_deviceType;
	NPT_String m_friendlyName;
	NPT_String m_manufacturer;
	NPT_String m_manufacturerURL;
	NPT_String m_modelDescription;
	NPT_String m_modelName;
	NPT_String m_modelNumber;
	NPT_String m_modelURL;
	NPT_String m_serialNumber;
	NPT_String m_upc;
	NPT_String m_presentationURL;
	UUID m_uuid;

	NPT_List<DeviceDesc*> m_children;
	NPT_List<ServiceDesc*> m_serviceList;
	NPT_List<IconDesc*> m_iconList;
	NPT_String m_rootDescURL;
};

class ServiceDescPrivate
{
public:
	ServiceDescPrivate();
	~ServiceDescPrivate();

	static ServiceDesc *newServiceDesc();
	static void deleteServiceDesc(ServiceDesc *serviceDesc);
	static ServiceDescPrivate *getPrivate(ServiceDesc *serviceDesc);

	NPT_String m_serviceId;
	NPT_String m_serviceType;
	NPT_String m_scpdURL;
	NPT_String m_controlURL;
	NPT_String m_eventSubURL;
	NPT_String m_scpdXML;

	NPT_List<ActionDesc*> m_actionList;
	NPT_List<StateVariableDesc*> m_stateVariableList;
};

class ServiceDescByIdFinder
{
public:
	ServiceDescByIdFinder(const NPT_String& serviceId)
		: m_serviceId(serviceId)
	{
	}

	bool operator()(const ServiceDesc *serviceDesc) const
	{
		return serviceDesc->serviceId().Compare(m_serviceId) == 0;
	}

private:
	NPT_String m_serviceId;
};

class ServiceDescByTypeFinder
{
public:
	ServiceDescByTypeFinder(const NPT_String& serviceType)
		: m_serviceType(serviceType)
	{
		if (m_serviceType.EndsWith(":*")) {
			m_matchAll = true;
			m_serviceType = m_serviceType.Left(m_serviceType.GetLength() - 2);
		}
	}

	bool operator()(const ServiceDesc *serviceDesc) const
	{
		if (m_matchAll) {
			NPT_String serviceType = serviceDesc->serviceType();
			int pos = serviceType.ReverseFind(":");
			if (pos > 0) {
				serviceType = serviceType.Left(pos);
				return m_serviceType.Compare(serviceType) == 0;
			}
			return false;
		} else {
			return m_serviceType.Compare(serviceDesc->serviceType()) == 0;
		}
	}

private:
	NPT_String m_serviceType;
	bool m_matchAll;
};

class IconDescPrivate
{
public:
	IconDescPrivate();
	~IconDescPrivate();

	static IconDesc *newIconDesc();
	static void deleteIconDesc(IconDesc *iconDesc);
	static IconDescPrivate *getPrivate(IconDesc *iconDesc);

	NPT_String m_mimeType;
	int m_width;
	int m_height;
	int m_depth;
	NPT_String m_url;
	NPT_DataBuffer m_iconData;
};

class ActionDescPrivate
{
public:
	ActionDescPrivate();
	~ActionDescPrivate();

	static ActionDesc *newActionDesc();
	static void deleteActionDesc(ActionDesc *actionDesc);
	static ActionDescPrivate *getPrivate(ActionDesc *actionDesc);

	NPT_String m_name;
	NPT_List<ArgumentDesc*> m_argumentList;
	int m_inputArgumentCount;
};

class ArgumentDescPrivate
{
public:
	ArgumentDescPrivate();
	~ArgumentDescPrivate();

	static ArgumentDesc *newArgumentDesc();
	static void deleteArgumentDesc(ArgumentDesc *argumentDesc);
	static ArgumentDescPrivate *getPrivate(ArgumentDesc *argumentDesc);

	NPT_String m_name;
	bool m_input;
	NPT_String m_relatedStateVariableName;
};

class StateVariableDescPrivate
{
public:
	StateVariableDescPrivate();
	~StateVariableDescPrivate();

	static StateVariableDesc *newStateVariableDesc();
	static void deleteStateVariableDesc(StateVariableDesc *stateVariableDesc);
	static StateVariableDescPrivate *getPrivate(StateVariableDesc *stateVariableDesc);

	NPT_String m_name;
	NPT_String m_dataType;
	NPT_String m_defaultValue;
	NPT_List<NPT_String> m_allowedValueList;
	NPT_String m_allowedMinimum;
	NPT_String m_allowedMaximum;
	NPT_String m_allowedStep;
	bool m_hasAllowedMinimum;
	bool m_hasAllowedMaximum;
	bool m_hasAllowedStep;
};

} // namespace deejay

#endif // __DJDescPriv_h__
