#import <AssetsLibrary/AssetsLibrary.h>
#import <MobileCoreServices/MobileCoreServices.h>
#import <MediaPlayer/MediaPlayer.h>
#import <AVFoundation/AVFoundation.h>
#include "DJMediaStoreImpl.h"

namespace deejay {

void iOSImportPhotos2(MSFolder *folder);
	
} // namespace deejay

struct ReadAssetParams
{
	void *m_buffer;
	NPT_Size m_bytesToRead;
	NPT_Size m_bytesRead;
	NPT_Result m_nr;
	NPT_Size m_offset;
	NPT_Size m_size;
	NPT_String m_path;
	NPT_SharedVariable m_waitVar;
};

@interface PtrWrapper : NSObject
{
	void *m_ptr;
}

- (id)initWithPtr:(void*)ptr;
- (void*)getPtr;

@end

@implementation PtrWrapper

- (id)initWithPtr:(void*)ptr
{
	if ((self = [super init])) {
		m_ptr = ptr;
	}
	return self;
}

- (void*)getPtr
{
	return m_ptr;
}

@end

@interface PhotoLib : NSObject
{
	ALAssetsLibrary *m_assetsLib;
};

- (id)init;
- (void)dealloc;
- (void)importPhotos:(id)arg;
- (void)readAsset:(id)arg;

@end

@implementation PhotoLib

- (id)init
{
	if ((self = [super init])) {
		m_assetsLib = [[ALAssetsLibrary alloc] init];
	}
	return self;
}

- (void)dealloc
{
	[m_assetsLib release];
	[super dealloc];
}

- (void)readAsset:(id)arg
{
	PtrWrapper *pw = (PtrWrapper*)arg;
	ReadAssetParams *rap = static_cast<ReadAssetParams*>([pw getPtr]);
	NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:rap->m_path.GetChars()]];

	[m_assetsLib assetForURL:url resultBlock:^(ALAsset *asset) {
		ALAssetRepresentation *repr = [asset defaultRepresentation];
		if (repr != nil) {
			if (rap->m_offset + rap->m_bytesToRead > rap->m_size) {
				rap->m_bytesToRead = rap->m_size - rap->m_offset;
			}
			if (rap->m_bytesToRead > 0) {
				[repr getBytes:(uint8_t*)rap->m_buffer fromOffset:rap->m_offset length:rap->m_bytesToRead error:NULL];
				rap->m_offset += rap->m_bytesToRead;
				rap->m_nr = NPT_SUCCESS;
			} else {
				rap->m_nr = NPT_ERROR_EOS;
			}
			
			rap->m_bytesRead = rap->m_bytesToRead;
		} else {
			rap->m_nr = NPT_FAILURE;
		}

		rap->m_waitVar.SetValue(1);
	} failureBlock:^(NSError *error) {
		rap->m_nr = NPT_FAILURE;
		
		rap->m_waitVar.SetValue(1);
	}];
}

- (void)importPhotos:(id)arg
{
	bool needLoop0 = true;
	bool hasLoop0 = false;
	bool *needLoop = &needLoop0;
	bool *hasLoop = &hasLoop0;
	PtrWrapper *pw = (PtrWrapper*)arg;
	deejay::MSFolder *folder = static_cast<deejay::MSFolder*>([pw getPtr]);
	[m_assetsLib enumerateGroupsWithTypes:ALAssetsGroupAll usingBlock:^(ALAssetsGroup *group, BOOL *stop) {
		if (group != nil) {
			deejay::MSFolder *subFolder = new deejay::MSFolder();
			NSString *groupName = [group valueForProperty:ALAssetsGroupPropertyName];
			subFolder->m_title = [groupName UTF8String];
			[group enumerateAssetsUsingBlock:^(ALAsset *result, NSUInteger index, BOOL *stop) {
				if (result != nil) {
					NPT_String sMimeType;
					NPT_String sUrl;
					ALAssetRepresentation *repr = [result defaultRepresentation];
					if (repr != nil) {
						NSString *mimeType = (NSString*)UTTypeCopyPreferredTagWithClass((CFStringRef)[repr UTI], kUTTagClassMIMEType);
						sMimeType = [mimeType UTF8String];
						[mimeType release];
						sUrl = [[[repr url] absoluteString] UTF8String];
					}
					if (!sMimeType.IsEmpty()) {
						NPT_UrlQuery query(NPT_Url(sUrl).GetQuery());	
						NPT_String sTitle;
						const char *fId = query.GetField("id");
						if (fId) {
							sTitle = fId;
							/*const char *fExt = query.GetField("ext");
							 if (fExt) {
							 sTitle = NPT_String::Format("%s_%s", sTitle.GetChars(), fExt);
							 }*/
						} else {
							sTitle = sUrl;
						}
						const char *a_upnpClass = NULL;
						const char *a_extHint;
						deejay::MSMediaType a_mediaType;
						if (sMimeType.Compare("image/jpeg", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".jpg";
							a_mediaType = deejay::MSMediaType_Image;
						} else if (sMimeType.Compare("image/png", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".png";
							a_mediaType = deejay::MSMediaType_Image;
						} else if (sMimeType.Compare("image/bmp", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".bmp";
							a_mediaType = deejay::MSMediaType_Image;
						} else if (sMimeType.Compare("image/gif", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".gif";
							a_mediaType = deejay::MSMediaType_Image;
						} else if (sMimeType.Compare("image/tiff", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".tif";
							a_mediaType = deejay::MSMediaType_Image;
						} else if (sMimeType.Compare("video/quicktime", true) == 0) {
							a_upnpClass = "object.item.videoItem";
							a_extHint = ".mov";
							a_mediaType = deejay::MSMediaType_Video;
						} else if (sMimeType.Compare("video/x-m4v", true) == 0) {
							a_upnpClass = "object.item.videoItem";
							a_extHint = ".m4v";
							a_mediaType = deejay::MSMediaType_Video;
						}
						if (a_upnpClass) {
							deejay::IOSAssetItem *item = new deejay::IOSAssetItem();
							item->m_filePath = sUrl;
							item->m_title = sTitle;
							item->m_mediaType = a_mediaType;
							item->m_size = [repr size];
							NSDate *assetDate = [result valueForProperty:ALAssetPropertyDate];
							item->m_date = NPT_TimeStamp([assetDate timeIntervalSince1970]);
							item->m_title = NPT_DateTime(item->m_date).ToString();
							item->m_upnpClass = a_upnpClass;
							item->m_mimeType = sMimeType;
							item->m_objectId += a_extHint;
							subFolder->addChild(item);
						}
						NSLog(@"[AL] %s [%s]", sUrl.GetChars(), sMimeType.GetChars());
					}
				}
			}];
			folder->addChild(subFolder);
		} else {
			//if (*hasLoop) {
				CFRunLoopStop(CFRunLoopGetCurrent());
			//} else {
			//	*needLoop = false;
			//}
//			waitVar->SetValue(1);
		}
	} failureBlock:^(NSError *error) {
		//if (*hasLoop) {
			CFRunLoopStop(CFRunLoopGetCurrent());
		//} else {
		//	*needLoop = false;
		//}
//		waitVar->SetValue(1);
	}];

	//if (*needLoop) {
	//	*hasLoop = true;
		CFRunLoopRun();
	//}
//	waitVar->WaitWhileEquals(0);
	
}

@end

namespace deejay {

PhotoLib *g_photoLib = NULL;
	
void ios_photo_importer_init()
{
	if (!g_photoLib) {
		g_photoLib = [[PhotoLib alloc] init];
	}
}

void import_iPod_library(MSFolder *folder);
void iOSImportPhotos2(MSFolder *folder);
	
void iOSImportPhotos(MSFolder *folder)
{
	PtrWrapper *pw = [[PtrWrapper alloc] initWithPtr:folder];
	[g_photoLib performSelectorOnMainThread:@selector(importPhotos:) withObject:pw waitUntilDone:YES];
	[pw release];
}
	
void iOSImportPhotos2(MSFolder *folder)
{
//    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
/*
	NSString *path = [NSHomeDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"Documents/TheRock/03 - Jade.mp3"]];
	AVURLAsset *asset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:path] options:nil];
	//AVURLAsset *asset = [AVURLAsset URLAssetWithURL:[NSURL URLWithString:@"assets-library://asset/asset.MOV?id=1000000006&ext=MOV"] options:nil];
	if (asset) {
		NSArray *tracks = [asset tracks];
		NSLog(@"track count: %d", [tracks count]);
		AVAssetReader *reader = [AVAssetReader assetReaderWithAsset:asset error:nil];
		if (reader) {
			[reader addOutput:[[[AVAssetReaderOutput alloc] init] autorelease]];
			NSLog(@"cc %d", [[reader outputs] count]);
		}
	}
*/
/*	NSString *path = [NSHomeDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"Documents/IMG_0208.mov"]];
	if (UIVideoAtPathIsCompatibleWithSavedPhotosAlbum(path)) {
		UISaveVideoAtPathToSavedPhotosAlbum(path, nil, nil, nil);
	}
*/	
	NPT_SharedVariable waitVar0(0);
	NPT_SharedVariable *waitVar = &waitVar0;
	
	ALAssetsLibrary *lib = [[ALAssetsLibrary alloc] init];
	[lib enumerateGroupsWithTypes:ALAssetsGroupAll usingBlock:^(ALAssetsGroup *group, BOOL *stop) {
		if (group != nil) {
			MSFolder *subFolder = new MSFolder();
			NSString *groupName = [group valueForProperty:ALAssetsGroupPropertyName];
			subFolder->m_title = [groupName UTF8String];
			[group enumerateAssetsUsingBlock:^(ALAsset *result, NSUInteger index, BOOL *stop) {
				if (result != nil) {
					NPT_String sMimeType;
					NPT_String sUrl;
					ALAssetRepresentation *repr = [result defaultRepresentation];
					if (repr != nil) {
						NSString *mimeType = (NSString*)UTTypeCopyPreferredTagWithClass((CFStringRef)[repr UTI], kUTTagClassMIMEType);
						sMimeType = [mimeType UTF8String];
						[mimeType release];
						sUrl = [[[repr url] absoluteString] UTF8String];
					}
					if (!sMimeType.IsEmpty()) {
						NPT_UrlQuery query(NPT_Url(sUrl).GetQuery());	
						NPT_String sTitle;
						const char *fId = query.GetField("id");
						if (fId) {
							sTitle = fId;
							/*const char *fExt = query.GetField("ext");
							if (fExt) {
								sTitle = NPT_String::Format("%s_%s", sTitle.GetChars(), fExt);
							}*/
						} else {
							sTitle = sUrl;
						}
						const char *a_upnpClass = NULL;
						const char *a_extHint;
						MSMediaType a_mediaType;
						if (sMimeType.Compare("image/jpeg", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".jpg";
							a_mediaType = MSMediaType_Image;
						} else if (sMimeType.Compare("image/png", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".png";
							a_mediaType = MSMediaType_Image;
						} else if (sMimeType.Compare("image/bmp", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".bmp";
							a_mediaType = MSMediaType_Image;
						} else if (sMimeType.Compare("image/gif", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".gif";
							a_mediaType = MSMediaType_Image;
						} else if (sMimeType.Compare("image/tiff", true) == 0) {
							a_upnpClass = "object.item.imageItem.photo";
							a_extHint = ".tif";
							a_mediaType = MSMediaType_Image;
						} else if (sMimeType.Compare("video/quicktime", true) == 0) {
							a_upnpClass = "object.item.videoItem";
							a_extHint = ".mov";
							a_mediaType = MSMediaType_Video;
						} else if (sMimeType.Compare("video/x-m4v", true) == 0) {
							a_upnpClass = "object.item.videoItem";
							a_extHint = ".m4v";
							a_mediaType = MSMediaType_Video;
						}
						if (a_upnpClass) {
							IOSAssetItem *item = new IOSAssetItem();
							item->m_filePath = sUrl;
							item->m_title = sTitle;
							item->m_mediaType = a_mediaType;
							item->m_size = [repr size];
							NSDate *assetDate = [result valueForProperty:ALAssetPropertyDate];
							item->m_date = NPT_TimeStamp([assetDate timeIntervalSince1970]);
							item->m_upnpClass = a_upnpClass;
							item->m_mimeType = sMimeType;
							item->m_objectId += a_extHint;
							subFolder->addChild(item);
						}
						NSLog(@"[AL] %s [%s]", sUrl.GetChars(), sMimeType.GetChars());
					}
				}
			}];
			folder->addChild(subFolder);
		} else {
			waitVar->SetValue(1);
		}
	} failureBlock:^(NSError *error) {
		waitVar->SetValue(1);
	}];

	waitVar->WaitWhileEquals(0);

	[lib release];

	//import_iPod_library(folder);

//    [pool release];
}

class ALAssetReader
	: public AdvStreamReader
{
public:
	ALAssetReader(ALAssetsLibrary *lib, const MediaStore::FileDetail& detail, NSURL *url);
	virtual NPT_Result seek(NPT_Size offset);
	virtual NPT_Result read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead);
	virtual void abort();
	
private:
	void onAssetFound(ALAsset *asset, void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead);
	void onAssetFailed(NSError *error);
	
private:
	ALAssetsLibrary *m_lib;
	MediaStore::FileDetail m_detail;
	NSURL *m_url;
	NPT_Size m_offset;
	NPT_SharedVariable m_waitVar;
	NPT_Result m_nr;
};
	
ALAssetReader::ALAssetReader(ALAssetsLibrary *lib, const MediaStore::FileDetail& detail, NSURL *url)
: m_lib(lib), m_detail(detail), m_url(url), m_offset(0)
{
}

NPT_Result ALAssetReader::seek(NPT_Size offset)
{
	NSLog(@"AL seek %p %d", this, offset);
	m_offset = offset;
	return NPT_SUCCESS;
}

NPT_Result ALAssetReader::read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead)
{
//	NSLog(@"AL read %p %d", this, bytesToRead);
	if (bytesRead) {
		*bytesRead = 0;
	}
/*	
	[m_lib assetForURL:m_url resultBlock:^(ALAsset *asset) {
		NSLog(@"succ");
	} failureBlock:^(NSError *error) {
		NSLog(@"error");
	}];
*/
	ALAssetsLibraryAssetForURLResultBlock resultBlock = ^(ALAsset *asset) {
		onAssetFound(asset, buffer, bytesToRead, bytesRead);
	};
	
	ALAssetsLibraryAccessFailureBlock failureBlock = ^(NSError *error) {
		onAssetFailed(error);
	};
	
	
	m_waitVar.SetValue(0);
	[m_lib assetForURL:m_url resultBlock:resultBlock failureBlock:failureBlock];
//	onAssetFound(nil, buffer, bytesToRead, bytesRead);
	m_waitVar.WaitWhileEquals(0);
	return m_nr;
}

void ALAssetReader::abort()
{
}

void ALAssetReader::onAssetFound(ALAsset *asset, void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead)
{
//*	
	ALAssetRepresentation *repr = [asset defaultRepresentation];
	if (repr != nil) {
		if (m_offset + bytesToRead > m_detail.m_size) {
			bytesToRead = m_detail.m_size - m_offset;
		}
//		NSLog(@"AL found %p c1 %d %d %d", this, m_offset, bytesToRead, (NPT_Size)m_detail.m_size);
		if (bytesToRead > 0) {
			[repr getBytes:(uint8_t*)buffer fromOffset:m_offset length:bytesToRead error:NULL];
			m_offset += bytesToRead;
			m_nr = NPT_SUCCESS;
		} else {
			m_nr = NPT_ERROR_EOS;
		}
		
		if (bytesRead) {
			*bytesRead = bytesToRead;
		}
	} else {
		m_nr = NPT_FAILURE;
	}
/*/
	if (m_offset + bytesToRead > m_detail.m_size) {
		bytesToRead = m_detail.m_size - m_offset;
	}
	//NSLog(@"AL found %p c1 %d %d %d", this, m_offset, bytesToRead, (NPT_Size)m_detail.m_size);
	if (bytesToRead > 0) {
		memset(buffer, 0, bytesToRead);
		//[repr getBytes:(uint8_t*)buffer fromOffset:m_offset length:bytesToRead error:NULL];
		m_offset += bytesToRead;
		m_nr = NPT_SUCCESS;
	} else {
		m_nr = NPT_ERROR_EOS;
	}
	
	if (bytesRead) {
		*bytesRead = bytesToRead;
	}
//*/
	m_waitVar.SetValue(1);
}

void ALAssetReader::onAssetFailed(NSError *error)
{
	m_nr = NPT_FAILURE;
	m_waitVar.SetValue(1);
}

class ALAssetReader2
: public AdvStreamReader
{
public:
	ALAssetReader2(const MediaStore::FileDetail& detail);
	virtual NPT_Result seek(NPT_Size offset);
	virtual NPT_Result read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead);
	virtual void abort();

private:
	void onAssetFound(ALAsset *asset, void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead);
	void onAssetFailed(NSError *error);

private:
	MediaStore::FileDetail m_detail;
	NPT_Size m_offset;
	NPT_SharedVariable m_waitVar;
	NPT_Result m_nr;
};

ALAssetReader2::ALAssetReader2(const MediaStore::FileDetail& detail)
: m_detail(detail), m_offset(0)
{
}

NPT_Result ALAssetReader2::seek(NPT_Size offset)
{
	//NSLog(@"AL seek %p %d", this, offset);
	m_offset = offset;
	return NPT_SUCCESS;
}

NPT_Result ALAssetReader2::read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead)
{
	if (bytesRead) {
		*bytesRead = 0;
	}

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	ALAssetsLibrary *lib = [[[ALAssetsLibrary alloc] init] autorelease];
	NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:m_detail.m_path]];

	ALAssetsLibraryAssetForURLResultBlock resultBlock = ^(ALAsset *asset) {
		onAssetFound(asset, buffer, bytesToRead, bytesRead);
	};
		
	ALAssetsLibraryAccessFailureBlock failureBlock = ^(NSError *error) {
		onAssetFailed(error);
	};
		
	//m_waitVar.SetValue(0);
	[lib assetForURL:url resultBlock:resultBlock failureBlock:failureBlock];
	//m_waitVar.WaitWhileEquals(0);
	
	[pool release];
	return m_nr;
}
	
void ALAssetReader2::abort()
{
}
	
void ALAssetReader2::onAssetFound(ALAsset *asset, void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead)
{
	ALAssetRepresentation *repr = [asset defaultRepresentation];
	if (repr != nil) {
		if (m_offset + bytesToRead > m_detail.m_size) {
			bytesToRead = m_detail.m_size - m_offset;
		}
		if (bytesToRead > 0) {
			[repr getBytes:(uint8_t*)buffer fromOffset:m_offset length:bytesToRead error:NULL];
			m_offset += bytesToRead;
			m_nr = NPT_SUCCESS;
		} else {
			m_nr = NPT_ERROR_EOS;
		}

		if (bytesRead) {
			*bytesRead = bytesToRead;
		}
	} else {
		m_nr = NPT_FAILURE;
	}
	m_waitVar.SetValue(1);
}

void ALAssetReader2::onAssetFailed(NSError *error)
{
	m_nr = NPT_FAILURE;
	m_waitVar.SetValue(1);
}

class ALAssetReader3
: public AdvStreamReader
{
public:
	ALAssetReader3(const MediaStore::FileDetail& detail);
	virtual NPT_Result seek(NPT_Size offset);
	virtual NPT_Result read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead);
	virtual void abort();

private:
	MediaStore::FileDetail m_detail;
	NPT_Size m_offset;
};

ALAssetReader3::ALAssetReader3(const MediaStore::FileDetail& detail)
: m_detail(detail), m_offset(0)
{
}

NPT_Result ALAssetReader3::seek(NPT_Size offset)
{
	//NSLog(@"AL seek %p %d", this, offset);
	m_offset = offset;
	return NPT_SUCCESS;
}

NPT_Result ALAssetReader3::read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead)
{
	if (bytesRead) {
		*bytesRead = 0;
	}

	ReadAssetParams rap;
	rap.m_size = m_detail.m_size;
	rap.m_buffer = buffer;
	rap.m_bytesToRead = bytesToRead;
	rap.m_offset = m_offset;
	rap.m_path = m_detail.m_path;
	rap.m_waitVar.SetValue(0);
	PtrWrapper *pw = [[PtrWrapper alloc] initWithPtr:&rap];
	[g_photoLib performSelectorOnMainThread:@selector(readAsset:) withObject:pw waitUntilDone:YES];
	rap.m_waitVar.WaitWhileEquals(0);

	m_offset = rap.m_offset;
	if (bytesRead) {
		*bytesRead = rap.m_bytesRead;
	}
	[pw release];
	return rap.m_nr;
}

void ALAssetReader3::abort()
{
}

void serveIOSAsset(AbortableTask *task, const MediaStore::FileDetail& detail, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput)
{
/*	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	ALAssetsLibrary *lib = [[ALAssetsLibrary alloc] init];
	NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:detail.m_path]];

	ALAssetReader reader(lib, detail, url);
	serveStreamAdv(task, detail, &reader, reqCtx, req, resp, httpOutput, 1024 * 1);

	[lib release];
	[pool release];*/
//	ALAssetReader2 reader(detail);
//	serveStreamAdv(task, detail, &reader, reqCtx, req, resp, httpOutput, 1024 * 64);
	ALAssetReader3 reader(detail);
	serveStreamAdv(task, detail, &reader, reqCtx, req, resp, httpOutput, 1024 * 64);
}

void import_iPod_library(MSFolder *folder)
{
//	NSSet *propSet = [NSSet setWithObjects:MPMediaItemPropertyAssetURL, MPMediaItemPropertyTitle, MPMediaItemPropertyAlbumTitle, MPMediaItemPropertyReleaseDate, nil];
	
	MPMediaQuery *q1 = [MPMediaQuery songsQuery];
	MPMediaQuery *q2 = [MPMediaQuery albumsQuery];
	MPMediaQuery *q3 = [MPMediaQuery artistsQuery];
	MPMediaQuery *q4 = [MPMediaQuery genresQuery];
	MPMediaQuery *q5 = [[[MPMediaQuery alloc] init] autorelease];
	
	NSLog(@"q1 cc=%d", [[q1 items] count]);
	NSLog(@"q2 cc=%d", [[q2 items] count]);
	NSLog(@"q3 cc=%d", [[q3 items] count]);
	NSLog(@"q4 cc=%d", [[q4 items] count]);
	NSLog(@"q5 cc=%d", [[q5 items] count]);
	
	
	
	//MPMediaQuery *q = [[[MPMediaQuery alloc] init] autorelease];
	//[q setGroupingType:MPMediaGroupingTitle];
	MPMediaQuery *q = [MPMediaQuery songsQuery];
	NSArray *items = [q items];
	if (items != nil) {
		NSUInteger itemCount = [items count];
		for (NSUInteger i = 0; i < itemCount; i++) {
			MPMediaItem *mediaItem = [items objectAtIndex:i];
			if (mediaItem != nil) {
/*			IOSAssetItem *item = new IOSAssetItem();	
			item->m_filePath = sUrl;
			item->m_title = sTitle;
			item->m_mediaType = a_mediaType;
			item->m_size = [repr size];
			NSDate *assetDate = [result valueForProperty:ALAssetPropertyDate];
			item->m_date = NPT_TimeStamp([assetDate timeIntervalSince1970]);
			item->m_upnpClass = a_upnpClass;
			item->m_mimeType = sMimeType;
			item->m_objectId += a_extHint;
			subFolder->addChild(item);
			NSLog(@"[AL] %s", sUrl.GetChars());*/
				IOSAssetItem *item = new IOSAssetItem();
				item->m_upnpClass = "object.item.audioItem";
				item->m_mediaType = MSMediaType_Audio;
				item->m_mimeType = "audio/mpeg";
				item->m_objectId += ".mp3";
				item->m_size = 0;
				NSURL *assetURL = (NSURL*)[mediaItem valueForProperty:MPMediaItemPropertyAssetURL];
				if (assetURL != nil) {
					item->m_filePath = [[assetURL absoluteString] UTF8String];
				}
				NSString *propTitle = (NSString*)[mediaItem valueForProperty:MPMediaItemPropertyTitle];
				if (propTitle != nil) {
					item->m_title = [propTitle UTF8String];
				}
				NSDate *propDate = (NSDate*)[mediaItem valueForProperty:MPMediaItemPropertyReleaseDate];
				if (propDate != nil) {
					item->m_date = NPT_TimeStamp([propDate timeIntervalSince1970]);
				}
/*				[mediaItem enumerateValuesForProperties:propSet usingBlock:^(NSString *property, id value, BOOL *stop) {
					if (value != nil) {
						if (property == MPMediaItemPropertyAssetURL) {
							item->m_filePath = [value UTF8String];
						} else if (property == MPMediaItemPropertyTitle) {
							item->m_title = [value UTF8String];
						} else if (property == MPMediaItemPropertyAlbumTitle) {
						} else if (property == MPMediaItemPropertyReleaseDate) {
							item->m_date = NPT_TimeStamp([value timeIntervalSince1970]);
						}
					}
				}];*/
				if (!item->m_filePath.IsEmpty()) {
					item->m_title = NPT_String::Format("[%s]%s", item->m_title.GetChars(), item->m_filePath.GetChars());
					folder->addChild(item);
				} else {
					delete item;
				}
			}
		}
	}
}

} // namespace deejay
