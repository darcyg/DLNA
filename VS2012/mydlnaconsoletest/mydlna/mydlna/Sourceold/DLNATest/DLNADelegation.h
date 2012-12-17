//
//  DLNADelegation.h
//  Genie
//
//  Created by cs Siteview on 11-11-22.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//
#ifndef _DLNADELEGATION_HEAD_
#define _DLNADELEGATION_HEAD_

#include "DLNACore.h"
//#include "DLNACenter.h"
#include "DLNAGolbal.h"

//#include "map.h"
//#import "DLNAServerList.h"
//#import "DLNAThumbImg.h"
using namespace deejay;
class DLNADelegation : public deejay::DLNACoreDelegate
                     , public deejay::DLNACoreOp::FinishCallback
                     , public deejay::DLNAProgressiveBrowseOp::ResultCallback
{
public:
    static DLNADelegation* s_instance;
    static DLNADelegation *  GetInstance();
	static void ReleaseInstance();
    //2012.2.23
    static bool isNull();
    static bool s_photosImportedFlag;//2012.2.24照片库是否已经被导入
    
private:
    DLNADelegation();
    ~DLNADelegation();
    
public:
    void startCore();//2012.2.17
public:
    
protected:
    virtual void onMediaServerListChanged();
	virtual void onMediaRendererListChanged();
	virtual void onMediaServerStateVariablesChanged(deejay::DeviceDesc *deviceDesc, deejay::ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList);
	virtual void onMediaRendererStateVariablesChanged(deejay::DeviceDesc *deviceDesc, deejay::ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList);
    
    virtual void dmrOpen(const NPT_String& url, const NPT_String& mimeType, const NPT_String& metaData);
	virtual void dmrPlay();
	virtual void dmrPause();
	virtual void dmrStop();
	virtual void dmrSeekTo(NPT_Int64 timeInMillis);
	virtual void dmrSetMute(bool mute);
	virtual void dmrSetVolume(int volume);
    
    //call back
    virtual void onDLNACoreOpFinished(deejay::DLNACoreOp *op);
    virtual void onDLNAProgressiveBrowseOpResult(deejay::DLNAProgressiveBrowseOp *op, NPT_UInt32 startingIndex, NPT_UInt32 numberReturned, NPT_UInt32 totalMatches, const deejay::DLNAObjectList& ls);
    
};
#endif