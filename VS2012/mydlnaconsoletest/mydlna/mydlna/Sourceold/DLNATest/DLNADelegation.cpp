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
//bool DLNADelegation::s_photosImportedFlag = false;//2012.2.24

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

}

void DLNADelegation::startCore()
{
    //m_core->start();
}

DLNADelegation::~DLNADelegation()
{

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