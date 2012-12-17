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

	void coreStopOnSynThread();
    bool wait(deejay::DLNACoreOp* op);

public:
    void startCore();//2012.2.17
    UnLockMode lockUI();
    void unLockUI(UnLockMode mode);

private:
    deejay::DLNACore        * m_core;
    //DLNACenter              * m_delegate;
    
    deejay::UUID            m_dmsCurrentUUID;
    deejay::UUID            m_dmrCurrentUUID;
    deejay::DeviceDesc      * m_currentServer;
    deejay::DeviceDesc      * m_currentRender;
    NPT_String              m_defaultRenderUUID;
    
private://for track
    deejay::DLNAQueryPositionInfoOp         *m_trackOp;
    unsigned int                            m_trackCount;
    
    
private://for wait
    bool                    m_lockFlag;
    UnLockMode              m_unLockMode;

    void loadConfig();
    void writeConfig();
    void WriteDLNAUserConfig();
    //NSString * ReadDLNAUserConfig();

public:
    void importPhotos();//2012.2.24
    void waitForImportPhotos();
    void finishImportPhotos();
    
    deejay::DLNACore * getCore() const;
    //void setDelegate(DLNACenter* delegate);
    //void clearDelegate();
    //DLNACenter * getDelegate() const;
    deejay::DeviceDescList getMediaRenderList() const;
    deejay::DeviceDescList getMediaServerList() const;
    deejay::UUID getCurrentServerUUID() const;
    deejay::UUID getCurrentRenderUUID() const;
    deejay::DeviceDesc* getCurrentServer() const;
    deejay::DeviceDesc* getCurrentRender() const;
    void traverseRenderList();
    
    const deejay::UUID& getServerUuidOfSelf();
    const deejay::UUID& getRenderUuidOfSelf();
    bool currentRenderIsSelf();
    
private:
    void setCurrentServer(deejay::DeviceDesc* dms);
    void setCurrentRender(deejay::DeviceDesc* dmr);
    
public:
    void restartUPNP();
    void refreshMediaSource();
    void startUPNPServer();
    void startUPNPRender();
    void startUPNPControlPoint();
    void stopUPNPServer();
    void stopUPNPRender();
    void stopUPNPControlpoint();
    
    void setProperty(const NPT_String& name, const NPT_String& value);
    void importFileSystemToMediaServer(const NPT_String& dir, const NPT_String& name);
    
    deejay::ServiceDesc *findServiceByType(const NPT_String& serviceType) const;
    deejay::ServiceDesc *findServiceById(const NPT_String& serviceType) const;
    NPT_Result queryStateVariables(const NPT_String& serviceId, const NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList);    
    
public:
    DLNAMediaObjType getMediaObjType(const deejay::DLNAObject* media);
    void browse(const NPT_String& serverOrDirTitle, const NPT_String& containerId);
    void refreshDevices(deejay::DLNACore::FlushMode flushMode);
    //void refreshDirectory(const NPT_String& directoryId, DLNAServerList* aController);
    void openMediaObj(const deejay::DLNAItem* mediaItem);
    void openMediaObj(const deejay::DLNAItem* mediaItem, unsigned int timeout);
    void setActiveServer(deejay::DeviceDesc& device);
    void setActiveRender(deejay::DeviceDesc& device);
    
    //control point
    void queryTrackInfo();
    int onQueryMediaPositionFinished();
    void stop();
    void pause();
    void play();
    void setVolume(int volume);
    void setProgress(int progress);
    void setMute(bool mute);
    
    //render
    void renderReportPlayBackState(DLNAMediaPlayBackState state);
    void renderReportProgress(long long playbackSeconds, long long durationSeconds);

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

private:
    void setThumbImg(NPT_String& url, DLNAMediaObjType mediaType);//2012.2.9//播放界面的缩略图    

};
#endif