#ifndef __avcore1_h__
#define __avcore1_h__

#include "DJDeviceImpl.h"
#include "avcore4.h"
#include "DJMediaStore.h"

namespace deejay {

class SimpleDMS
	: public DeviceImpl
	, public MediaStore::Callback
{
public:
	SimpleDMS(const UUID& uuid);
	virtual ~SimpleDMS();

	void setUuid(const UUID& uuid);
	const UUID& uuid() const;
	void setFriendlyName(const NPT_String& friendlyName);
	void setPlatformName(const NPT_String& platformName);

	MediaStore *mediaStore() const;

protected:
	virtual void outputMoreDeviceDescription(NPT_XmlSerializer *xml);
	virtual int onAction(const ServiceDecl *serviceDecl, const ActionDecl *actionDecl, AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, const NPT_List<NPT_String>& inputArgNames, const NPT_List<NPT_String>& inputArgValues, NPT_List<NPT_String>& outputArgValues);
	virtual bool onHttpRequest(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const NPT_String& relPath, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, NPT_InputStream *inputStream, HttpOutput *httpOutput);
	virtual void onMediaStoreUpdated(MediaStore *mediaStore);

private:
	MediaStore *m_store;
};

} // namespace deejay

#endif // __avcore1_h__
