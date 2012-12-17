#include "DJDescPriv.h"

namespace deejay {

DeviceDescPrivate::DeviceDescPrivate()
	: m_parent(NULL)
{
}

DeviceDescPrivate::~DeviceDescPrivate()
{
	for (NPT_Ordinal i = 0; i < m_serviceList.GetItemCount(); i++) {
		ServiceDesc *serviceDesc = *m_serviceList.GetItem(i);
		ServiceDescPrivate::deleteServiceDesc(serviceDesc);
	}

	for (NPT_Ordinal i = 0; i < m_iconList.GetItemCount(); i++) {
		IconDesc *iconDesc = *m_iconList.GetItem(i);
		IconDescPrivate::deleteIconDesc(iconDesc);
	}

	for (NPT_Ordinal i = 0; i < m_children.GetItemCount(); i++) {
		DeviceDesc *child = *m_children.GetItem(i);
		delete child;
	}
}

DeviceDesc *DeviceDescPrivate::newDeviceDesc()
{
	DeviceDesc *deviceDesc = new DeviceDesc();
	deviceDesc->addRef();
	return deviceDesc;
}

DeviceDescPrivate *DeviceDescPrivate::getPrivate(DeviceDesc *deviceDesc)
{
	return deviceDesc->m_private;
}

ServiceDescPrivate::ServiceDescPrivate()
{
}

ServiceDescPrivate::~ServiceDescPrivate()
{
	for (NPT_Ordinal i = 0; i < m_actionList.GetItemCount(); i++) {
		ActionDesc *actionDesc = *m_actionList.GetItem(i);
		ActionDescPrivate::deleteActionDesc(actionDesc);
	}

	for (NPT_Ordinal i = 0; i < m_stateVariableList.GetItemCount(); i++) {
		StateVariableDesc *stateVariableDesc = *m_stateVariableList.GetItem(i);
		StateVariableDescPrivate::deleteStateVariableDesc(stateVariableDesc);
	}
}

ServiceDesc *ServiceDescPrivate::newServiceDesc()
{
	return new ServiceDesc();
}

void ServiceDescPrivate::deleteServiceDesc(ServiceDesc *serviceDesc)
{
	delete serviceDesc;
}

ServiceDescPrivate *ServiceDescPrivate::getPrivate(ServiceDesc *serviceDesc)
{
	return serviceDesc->m_private;
}

IconDescPrivate::IconDescPrivate()
{
}

IconDescPrivate::~IconDescPrivate()
{
}

IconDesc *IconDescPrivate::newIconDesc()
{
	return new IconDesc();
}

void IconDescPrivate::deleteIconDesc(IconDesc *iconDesc)
{
	delete iconDesc;
}

IconDescPrivate *IconDescPrivate::getPrivate(IconDesc *iconDesc)
{
	return iconDesc->m_private;
}

ActionDescPrivate::ActionDescPrivate()
	: m_inputArgumentCount(0)
{
}

ActionDescPrivate::~ActionDescPrivate()
{
	for (NPT_Ordinal i = 0; i < m_argumentList.GetItemCount(); i++) {
		ArgumentDesc *argDesc = *m_argumentList.GetItem(i);
		ArgumentDescPrivate::deleteArgumentDesc(argDesc);
	}
}

ActionDesc *ActionDescPrivate::newActionDesc()
{
	return new ActionDesc();
}

void ActionDescPrivate::deleteActionDesc(ActionDesc *actionDesc)
{
	delete actionDesc;
}

ActionDescPrivate *ActionDescPrivate::getPrivate(ActionDesc *actionDesc)
{
	return actionDesc->m_private;
}

ArgumentDescPrivate::ArgumentDescPrivate()
{
}

ArgumentDescPrivate::~ArgumentDescPrivate()
{
}

ArgumentDesc *ArgumentDescPrivate::newArgumentDesc()
{
	return new ArgumentDesc();
}

void ArgumentDescPrivate::deleteArgumentDesc(ArgumentDesc *argumentDesc)
{
	delete argumentDesc;
}

ArgumentDescPrivate *ArgumentDescPrivate::getPrivate(ArgumentDesc *argumentDesc)
{
	return argumentDesc->m_private;
}

StateVariableDescPrivate::StateVariableDescPrivate()
	: m_hasAllowedMinimum(false)
	, m_hasAllowedMaximum(false)
	, m_hasAllowedStep(false)
{
}

StateVariableDescPrivate::~StateVariableDescPrivate()
{
}

StateVariableDesc *StateVariableDescPrivate::newStateVariableDesc()
{
	return new StateVariableDesc();
}

void StateVariableDescPrivate::deleteStateVariableDesc(StateVariableDesc *stateVariableDesc)
{
	delete stateVariableDesc;
}

StateVariableDescPrivate *StateVariableDescPrivate::getPrivate(StateVariableDesc *stateVariableDesc)
{
	return stateVariableDesc->m_private;
}

} // namespace deejay
