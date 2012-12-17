//
//  DLNADelegation.cpp
//  Genie
//
//  Created by cs Siteview on 11-11-22.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#include "DLNADelegation.h"
//#import <CoreLocation/CoreLocation.h>

#define TRACK_TIMEOUT_COUNT 3
using namespace deejay;

DLNADelegation* DLNADelegation::s_instance = NULL;
bool DLNADelegation::s_photosImportedFlag = false;//2012.2.24

DLNADelegation* DLNADelegation::GetInstance()
{
    if (NULL == s_instance)
    {
        s_instance = new DLNADelegation();
    }
    return s_instance;
}

void DLNADelegation::ReleaseInstance()
{
    if(NULL != s_instance)
    {
        delete s_instance;
    }
}

bool DLNADelegation::isNull()
{
    if (NULL == s_instance)
    {
        return true;
    }
    return false;
}

DLNADelegation::DLNADelegation()
{
	m_core = new deejay::DLNACore(this);
    
    loadConfig();
    const char *dmrProtocolInfo =
    "http-get:*:image/png:*,"
    "http-get:*:image/jpeg:*,"
    "http-get:*:image/bmp:*,"
    "http-get:*:image/gif:*,"
    "http-get:*:audio/mpeg:*,"
    "http-get:*:audio/3gpp:*,"
    "http-get:*:audio/mp4:*,"
    "http-get:*:audio/x-ms-wma:*,"
    "http-get:*:audio/wav:*,"
    "http-get:*:video/mp4:*,"
    "http-get:*:video/mpeg:*,"
    "http-get:*:video/x-ms-wmv:*,"
    "http-get:*:video/x-ms-asf:*,"
    "http-get:*:video/3gpp:*,"
    "http-get:*:video/avi:*,"
    "http-get:*:video/quicktime:*";

	setProperty("PlatformName", "Win8");
	setProperty("OSVersion", "1.1");
	setProperty("DMRProtocolInfo", dmrProtocolInfo);
	setProperty("FriendlyName", "NetgareTest");

	importFileSystemToMediaServer("c:\\temp", "My Media");
	startCore();
}

void DLNADelegation::startCore()
{    
	m_core->start();
}

DLNADelegation::~DLNADelegation()
{
    if (m_currentServer)
    {
        m_currentServer->release();
        m_currentServer = NULL;
    }
    if (m_currentRender)
    {
        m_currentRender->release();
        m_currentRender = NULL;
    }
    if (m_trackOp) 
    {
		m_trackOp->release();
		m_trackOp = NULL;
	}
    //m_core->stop();
    coreStopOnSynThread();
    //[m_thumbImgArr release];
    //m_thumbImgArr = nil;
    delete m_core;
}

//void DLNADelegation::setDelegate(DLNACenter *delegate)
//{
//    m_delegate = delegate;
//}
//
//void DLNADelegation::clearDelegate()
//{
//    m_delegate = nil;
//}

//DLNACenter* DLNADelegation::getDelegate() const
//{
//    return m_delegate;
//}

///////////////////////////call back
void DLNADelegation::onMediaRendererListChanged()
{
    //[m_delegate performSelectorOnMainThread:@selector(notifyMediaRenderListChanged) withObject:nil waitUntilDone:NO];
}


void DLNADelegation::onMediaServerListChanged()
{
    //[m_delegate performSelectorOnMainThread:@selector(notifyMediaServerListChanged) withObject:nil waitUntilDone:NO];
}


void DLNADelegation::onMediaServerStateVariablesChanged(deejay::DeviceDesc *deviceDesc, deejay::ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
{
  /*  if (!m_dmsCurrentUUID.isNull() && m_dmsCurrentUUID == deviceDesc->uuid())
    {
        [m_delegate performSelectorOnMainThread:@selector(notifyMediaServerStatusChanged) withObject:nil waitUntilDone:NO];
    }*/
}

void DLNADelegation::onMediaRendererStateVariablesChanged(deejay::DeviceDesc *deviceDesc, deejay::ServiceDesc *serviceDesc, const NPT_List<NPT_String>& nameList, const NPT_List<NPT_String>& valueList)
{
    /*if (!m_dmrCurrentUUID.isNull() && m_dmrCurrentUUID == deviceDesc->uuid())
    {
        [m_delegate performSelectorOnMainThread:@selector(notifyMediaRenderStatusChanged) withObject:nil waitUntilDone:NO];
    }*/
}

////////////////

void DLNADelegation::dmrOpen(const NPT_String& url, const NPT_String& mimeType, const NPT_String& metaData)
{
    //NSAutoreleasePool * tmpPool = [[NSAutoreleasePool alloc] init];
    //NSMutableArray * info = [[[NSMutableArray alloc] init] autorelease];
    //[info addObject:[NSString stringWithUTF8String:url.GetChars()]];
    //[info addObject:[NSString stringWithUTF8String:mimeType.GetChars()]];
    //[info addObject:[NSString stringWithUTF8String:metaData.GetChars()]];
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderOpen:) withObject:info waitUntilDone:NO];
    //[tmpPool release];
}
void DLNADelegation::dmrPlay()
{
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderPlay) withObject:nil waitUntilDone:NO];
}
void DLNADelegation::dmrPause()
{
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderPause) withObject:nil waitUntilDone:NO];
}
void DLNADelegation::dmrStop()
{
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderStop) withObject:nil waitUntilDone:NO];
}
void DLNADelegation::dmrSeekTo(NPT_Int64 timeInMillis)
{
    //NSAutoreleasePool * tmpPool = [[NSAutoreleasePool alloc] init];
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderSeekTo:) withObject:[NSNumber numberWithLongLong:timeInMillis] waitUntilDone:NO];
    //[tmpPool release];
}
void DLNADelegation::dmrSetMute(bool mute)
{
    //NSAutoreleasePool * tmpPool = [[NSAutoreleasePool alloc] init];
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderSetMute:) withObject:[NSNumber numberWithBool:mute] waitUntilDone:NO];
    //[tmpPool release];
}
void DLNADelegation::dmrSetVolume(int volume)
{
    //NSAutoreleasePool * tmpPool = [[NSAutoreleasePool alloc] init];
    //[m_delegate performSelectorOnMainThread:@selector(OnMediaRenderSetVolume:) withObject:[NSNumber numberWithInt:volume] waitUntilDone:NO];
    //[tmpPool release];
}

//////////////

void DLNADelegation::onDLNACoreOpFinished(deejay::DLNACoreOp *op)
{
    deejay::DLNAQueryPositionInfoOp * queryOp;
    if ((queryOp = dynamic_cast<deejay::DLNAQueryPositionInfoOp*>(op)))
    {
        //[m_delegate  performSelectorOnMainThread:@selector(notifyTrackOpFinish) withObject:nil waitUntilDone:NO];
        return;
    }
    
    //deejay::DLNAProgressiveBrowseOp * browseOp;
    //if ((browseOp = dynamic_cast<deejay::DLNAProgressiveBrowseOp*>(op)))
    //{
    //    //op->release();
    //    //BrowseWaitPopup * browseWaitPopup = m_browseOpMap[browseOp];
    //    //dispatch_async(dispatch_get_main_queue(), ^{
    //    //    m_browseOpMap.erase(browseOp);
    //    //    delete browseWaitPopup;
    //    //    browseOp->release();//2012.2.8
    //    //});
    //    return;
    //}
    //
    ////for import photos callback
    //else
    //{
    //    //dispatch_async(dispatch_get_main_queue(), ^{finishImportPhotos();});
    //}
}

void DLNADelegation::onDLNAProgressiveBrowseOpResult(deejay::DLNAProgressiveBrowseOp *op, NPT_UInt32 startingIndex, NPT_UInt32 numberReturned, NPT_UInt32 totalMatches, const deejay::DLNAObjectList& ls)
{
    //deejay::DLNAObjectList * objList = new deejay::DLNAObjectList(ls);
    //dispatch_async(dispatch_get_main_queue(), ^{
    //    //NSLog(@"now progressive is %d  total is %d",(startingIndex+numberReturned),totalMatches);
    //    //progressiveBrowseResult(op, startingIndex, numberReturned, totalMatches, *objList);
    //});
}

void DLNADelegation::coreStopOnSynThread()
{

}

bool DLNADelegation::wait(deejay::DLNACoreOp* op)
{
    if (NPT_SUCCEEDED(op->wait(500)))
    {
        return true;
    }
    //WaitPopup  waitPopup;
    //if (op->checkFinishedIfNotSetCallback(&waitPopup))
    //{
    //    return true;
    //}
    //else
    //{
    //    waitPopup.showWaitDialog();
    //    UnLockMode mode = lockUI();
    //    op->resetCallback();
    //    if (UnLockModeManual == mode)
    //    {
    //        op->abort();
    //        return false;
    //    }
    //    return true;
    //}
	return true;
}

UnLockMode DLNADelegation::lockUI()
{
    m_lockFlag = true;

    return m_unLockMode;
}

void DLNADelegation::unLockUI(UnLockMode mode)
{
    m_unLockMode = mode;
    m_lockFlag = false;
}

void DLNADelegation::loadConfig()
{
 //   NSData * data = [NSData dataWithContentsOfFile:filePath];
 //   NPT_MemoryStream strm([data bytes], [data length]);
	//m_core->loadConfig(&strm);
}

void DLNADelegation::writeConfig()
{
    //NPT_MemoryStream cfgStrm;
    //m_core->saveConfig(&cfgStrm);
}

//
void DLNADelegation::WriteDLNAUserConfig()
{

}

//NSString * DLNADelegation::ReadDLNAUserConfig()
//{
//
//}

void DLNADelegation::importPhotos()
{    
    if (s_photosImportedFlag)
    {
        return;
    }
    deejay::DLNACoreOp* op;
    if (NPT_SUCCEEDED(m_core->importPhotos("", &op)))
    {
        waitForImportPhotos();
        op->checkFinishedIfNotSetCallback(this);
        op->release();
        s_photosImportedFlag = true;
    }
}

void DLNADelegation::waitForImportPhotos()
{

}

void DLNADelegation::finishImportPhotos()
{

}

deejay::DLNACore* DLNADelegation::getCore()  const
{
    return m_core;
}

//void DLNADelegation::setDelegate(DLNACenter *delegate)
//{
//    m_delegate = delegate;
//}
//
//void DLNADelegation::clearDelegate()
//{
//    m_delegate = nil;
//}
//
//DLNACenter* DLNADelegation::getDelegate() const
//{
//    return m_delegate;
//}

deejay::DeviceDescList DLNADelegation::getMediaRenderList() const
{
    return m_core->snapshotMediaRendererList();
}

deejay::DeviceDescList DLNADelegation::getMediaServerList() const
{
    return m_core->snapshotMediaServerList();
}

void DLNADelegation::setCurrentServer(deejay::DeviceDesc* mds)
{
    if (m_currentServer)
    {
        m_currentServer->release();
        m_currentServer = NULL;
    }
    m_currentServer = mds;
    m_currentServer->addRef();
    m_dmsCurrentUUID = mds->uuid();
}

void DLNADelegation::setCurrentRender(deejay::DeviceDesc* mdr)
{
    if (m_currentRender)
    {
        m_currentRender->release();
        m_currentRender = NULL;
        m_dmrCurrentUUID = deejay::UUID::null();
    }
    if (!mdr)
    {
        return;
    }
    m_currentRender = mdr;
    m_currentRender->addRef();
    m_dmrCurrentUUID = mdr->uuid();
    m_defaultRenderUUID = m_dmrCurrentUUID.toString();
}

deejay::UUID DLNADelegation::getCurrentServerUUID() const
{
    return m_dmsCurrentUUID;
}

deejay::UUID DLNADelegation::getCurrentRenderUUID() const
{
    return m_dmrCurrentUUID;
}

deejay::DeviceDesc* DLNADelegation::getCurrentServer() const
{
    return m_currentServer;
}

deejay::DeviceDesc* DLNADelegation::getCurrentRender() const
{
    return m_currentRender;
}

void DLNADelegation::traverseRenderList()
{
    const deejay::DeviceDescList& dmrList = getMediaRenderList();
    deejay::UUID defaultRenderUUID = deejay::UUID::fromString(m_defaultRenderUUID);
    deejay::DeviceDesc * defaultRender = dmrList.find(defaultRenderUUID);
    //setActiveRender(*defaultRender);
}

const deejay::UUID& DLNADelegation::getServerUuidOfSelf()
{
    return m_core->getMediaServerUuid();
}
const deejay::UUID& DLNADelegation::getRenderUuidOfSelf()
{
    return m_core->getMediaRendererUuid();
}

bool DLNADelegation::currentRenderIsSelf()
{
    if(m_defaultRenderUUID == getRenderUuidOfSelf().toString())
    {
        return true;
    }
    else
    {
        return false;
    }
}

DLNAMediaObjType DLNADelegation::getMediaObjType(const deejay::DLNAObject* media)
{
    DLNAMediaObjType type = DLNAMediaObjTypeVideo;
    if (media)
    {
        const NPT_String contentType = media->upnpClass().Left(21);
        if (contentType == "object.item.imageItem" ) 
        {
            type = DLNAMediaObjTypeImage;
        } 
        else if (contentType == "object.item.videoItem") 
        {
            type = DLNAMediaObjTypeVideo;
        } 
        else if (contentType == "object.item.audioItem")
        {
            type = DLNAMediaObjTypeAudio;
        }
    }
    return type;
}

/////////////////////
void DLNADelegation::restartUPNP()
{
        coreStopOnSynThread();
        m_core->start();
}

void DLNADelegation::refreshMediaSource()
{
    m_core->clearMediaServerContent();
 //   NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	//NSString *documentsDirectory = [paths objectAtIndex:0];
    //importFileSystemToMediaServer([documentsDirectory UTF8String],"");
    m_core->mediaStore()->importIOSPhotos("");
}

void DLNADelegation::startUPNPRender()
{
    m_core->enableFunction(deejay::DLNACore::Function_MediaRenderer, true);
}

void DLNADelegation::startUPNPServer()
{
    m_core->enableFunction(deejay::DLNACore::Function_MediaServer, true);
}

void DLNADelegation::startUPNPControlPoint()
{
    m_core->enableFunction(deejay::DLNACore::Function_ControlPoint, true);
}

void DLNADelegation::stopUPNPRender()
{
    m_core->enableFunction(deejay::DLNACore::Function_MediaRenderer, false);
}

void DLNADelegation::stopUPNPServer()
{
    m_core->enableFunction(deejay::DLNACore::Function_MediaServer, false);
}

void DLNADelegation::stopUPNPControlpoint()
{
    m_core->enableFunction(deejay::DLNACore::Function_ControlPoint, false);
}

void DLNADelegation::setProperty(const NPT_String& name, const NPT_String& value)
{
    m_core->setProperty(name,value);
}
void DLNADelegation::importFileSystemToMediaServer(const NPT_String& dir, const NPT_String& name)
{
    m_core->importFileSystemToMediaServer(dir,name);
}

deejay::ServiceDesc* DLNADelegation::findServiceByType(const NPT_String& serviceType) const
{
    return m_currentRender->findServiceByType(serviceType);
}

deejay::ServiceDesc* DLNADelegation::findServiceById(const NPT_String& serviceType) const
{
    return m_currentRender->findServiceById(serviceType);
}

NPT_Result DLNADelegation::queryStateVariables(const NPT_String& serviceId, const NPT_List<NPT_String>& nameList, NPT_List<NPT_String>& valueList)
{
    return m_core->queryStateVariables(m_dmrCurrentUUID, serviceId, nameList, valueList);
}

///////////////////UPNP
void DLNADelegation::browse(const NPT_String& serverOrDirTitle, const NPT_String& containerId)
{
    //*
    deejay::DLNABrowseOp *op;
	if (NPT_SUCCEEDED(m_core->browseMediaServer(m_dmsCurrentUUID, containerId, false, &op))) 
    {
        if (wait(op))
        {
            if (op->succeeded()) 
            {
                //[m_delegate addMediaObjectDir:serverOrDirTitle withList:op->objectList()];
                printf("borwse op  successed");
                printf("\n");
            }
            else
            {
                printf("borwse op un successed");
                printf("\n");
            }
        }
		else
        {
            printf("wait wrong-->click cancle btn");
            printf("\n");
        }
		op->release();
	}
    else
    {
        
    }
}

void DLNADelegation::refreshDevices(deejay::DLNACore::FlushMode flushMode)
{
    m_core->flushDeviceList(flushMode);
	m_core->searchDevices(30);
}

//void DLNADelegation::refreshDirectory(const NPT_String& containerId, DLNAServerList* aController)
//{
//    //*
//    deejay::DLNABrowseOp *op;
//	if (NPT_SUCCEEDED(m_core->browseMediaServer(m_dmsCurrentUUID, containerId, false, &op))) 
//    {
//        if (wait(op))
//        {
//            if (op->succeeded()) 
//            {
//                //[m_delegate reloadCurrentMediaObjectDir:op->objectList()];
//            }
//            else
//            {
//                printf("fresh borwse op un successed");
//                printf("\n");
//            }
//        }
//		else
//        {
//            printf("wait wrong-->click cancle btn");
//            printf("\n");
//        }
//		op->release();
//	}
//    else
//    {
//        
//    }
//}


void DLNADelegation::setThumbImg(NPT_String& url, DLNAMediaObjType mediaType)//2012.2.9
{

}

//void DLNADelegation::onThumbImgLoadingFinished(DLNAThumbImg * thumbImg)
//{
//    //refresh control ui
//    //[m_delegate setThumbImg:thumbImg.thumbImgData withType:thumbImg.type];
//    //for (DLNAThumbImg * elem in m_thumbImgArr)
//    //{
//        //if ([elem isEqual:thumbImg])
//        //{
//        //    [m_thumbImgArr removeObject:thumbImg];
//        //}
//    //}
//}

void DLNADelegation::openMediaObj(const deejay::DLNAItem* mediaItem)
{
    deejay::UUID renderUuid = getCurrentRenderUUID();
	if (!renderUuid.isNull()) 
    {
        DLNAMediaObjType type = getMediaObjType(mediaItem);
        NPT_String iconUrl;
        if (((deejay::DLNAObject*)mediaItem)->findThumbnailURL(200, 200, NULL, iconUrl))
        {
            //setThumbImg(iconUrl, type);
        }
        
		deejay::DLNACoreOp *op;
		if (NPT_SUCCEEDED(m_core->playMedia(renderUuid, mediaItem, &op))) 
        {
            if (wait(op))
            {
                if (op->succeeded()) 
                {
                    //[m_delegate showControlPointPageWithMediaType:type];
                }
                else
                {
                    printf("play op un successed");
                    printf("\n");
                }
            }
			else
            {
                printf("wait wrong-->click cancle btn");
                printf("\n");
            }
			op->release();
		}
        else
        {
            //NSLog(@"create Op error");
        }
	}
    else
    {
        //[m_delegate shouldSelectRenderForMedia:mediaItem];
    }
}

void DLNADelegation::openMediaObj(const deejay::DLNAItem* mediaItem, unsigned int timeout)
{
    deejay::UUID renderUuid = getCurrentRenderUUID();
	if (!renderUuid.isNull()) 
    {
        DLNAMediaObjType type = getMediaObjType(mediaItem);
        NPT_String iconUrl;
        if (((deejay::DLNAObject*)mediaItem)->findThumbnailURL(200, 200, NULL, iconUrl))
        {
            setThumbImg(iconUrl, type);
        }
        
		deejay::DLNACoreOp *op;
		if (NPT_SUCCEEDED(m_core->playMedia(renderUuid, mediaItem, &op))) 
        {
            /*if (NPT_SUCCEEDED(op->wait(timeout*1000)) && op->succeeded())
             {
             [m_delegate showControlPointPageWithMediaType:type];
             
             }
             /*/
            if (wait(op))
            {
                if (op->succeeded())
                {
                    //[m_delegate showControlPointPageWithMediaType:type];
                }
            }
            //*/
			else
            {
                //[m_delegate autoPlaybackTimeOut];
            }
			op->release();
		}
        else
        {
            
        }
	}
    else
    {
        //[m_delegate shouldSelectRenderForMedia:mediaItem];
    }
}

void DLNADelegation::setActiveServer(deejay::DeviceDesc& device)
{
    setCurrentServer(&device);
}

void DLNADelegation::setActiveRender(deejay::DeviceDesc& device)
{
    setCurrentRender(&device);
    //[m_delegate reloadMeidaRenderList:getMediaRenderList() withCurrentRenderID:m_defaultRenderUUID];
}



void DLNADelegation::queryTrackInfo()
{
    if (m_trackCount>TRACK_TIMEOUT_COUNT)
    {
        m_trackOp->release();
		m_trackOp = NULL;
    }

    if (!m_trackOp)
    {
		if (NPT_SUCCEEDED(m_core->queryMediaPositionInfo(m_dmrCurrentUUID, &m_trackOp))) 
        {
			m_trackOp->checkFinishedIfNotSetCallback(this);
		}
	}
    else
    {
        printf("-------------track no return\n");
        m_trackCount++;
    }
}

int DLNADelegation::onQueryMediaPositionFinished()
{
    int track = 0;
	if (m_trackOp) 
    {
		if (m_trackOp->succeeded()) 
        {
			track = m_trackOp->trackTime();
		}

        printf("-------------%d\n",track);
		m_trackOp->release();
		m_trackOp = NULL;
	}
    m_trackCount = 0;//if track successed  reset  count
    return track;
}

void DLNADelegation::stop()
{
    if (m_dmrCurrentUUID.isNull())
    {
        //[delegate() notAviableRender];
        return;
    }
    
	deejay::DLNACoreOp *op;
    if (NPT_SUCCEEDED(m_core->stopMedia(m_dmrCurrentUUID, &op)))
    {
        op->release();
    }
}

void DLNADelegation::pause()
{
    if (m_dmrCurrentUUID.isNull())
    {
        //[delegate() notAviableRender];
        return;
    }
    
	deejay::DLNACoreOp *op;
    if (NPT_SUCCEEDED(m_core->pauseMedia(m_dmrCurrentUUID, &op)))
    {
        op->release();
    }
}

void DLNADelegation::play()
{
    if (m_dmrCurrentUUID.isNull())
    {
        //[delegate() notAviableRender];
        return;
    }
    
	deejay::DLNACoreOp *op;
    if (NPT_SUCCEEDED(m_core->playMedia(m_dmrCurrentUUID, NULL, &op)))
    {
        op->release();
    }
}


void DLNADelegation::setVolume(int volume)
{
    if (m_dmrCurrentUUID.isNull()) 
    {
        //[delegate() notAviableRender];
		return;
	}
	
	deejay::DLNACoreOp *op;
    if (NPT_SUCCEEDED(m_core->changeMediaVolume(m_dmrCurrentUUID, volume, &op)))
    {
        op->release();
    }
}

void DLNADelegation::setProgress(int progress)
{
    if (m_dmrCurrentUUID.isNull())
    {
        //[delegate() notAviableRender];
        return;
    }
    
	deejay::DLNACoreOp *op;
    if (NPT_SUCCEEDED(m_core->seekMedia(m_dmrCurrentUUID, progress, &op)))
    {
        op->release();
    }
}


void DLNADelegation::setMute(bool mute)
{
    if (m_dmrCurrentUUID.isNull())
    {
        //[delegate() notAviableRender];
        return;
    }

    deejay::DLNACoreOp *op;
    if (NPT_SUCCEEDED(m_core->muteMedia(m_dmrCurrentUUID, mute, &op)))
    {
        op->release();
    }
}

void DLNADelegation::renderReportPlayBackState(DLNAMediaPlayBackState state)
{
    if (m_core)
    {
		switch (state) 
        {
            case DLNAMediaPlayBackStatePlaying:
                m_core->dmrReportState(deejay::DLNACore::DMRState_Playing);
                break;
            case DLNAMediaPlayBackStatePause:
                m_core->dmrReportState(deejay::DLNACore::DMRState_Paused);
                break;
            case DLNAMediaPlayBackStateStop:
                m_core->dmrReportState(deejay::DLNACore::DMRState_Stopped);
                break;
            case DLNAMediaPlayBackStateErr:
                m_core->dmrReportErrorStatus(true);
                break;
		}
	}
}

void DLNADelegation::renderReportProgress(long long playbackSeconds, long long durationSeconds)
{
    if (m_core) 
    {
		m_core->dmrReportProgress(playbackSeconds*1000, durationSeconds*1000);
	}
}