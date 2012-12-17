#ifndef __DJMediaStore_h__
#define __DJMediaStore_h__

#include <Neptune.h>
#include "DJFrontEnd.h"

namespace deejay {

class MediaStoreImpl;
class DLNACoreOp;

class MediaStore
{
public:
	class Callback
	{
	public:
		virtual void onMediaStoreUpdated(MediaStore *mediaStore) = 0;
	};

	struct FileDetail
	{
		enum Type
		{
			PosixFile,
			ALAsset,
		};

		Type m_type;
		NPT_TimeStamp m_modificationTime;
		NPT_LargeSize m_size;
		NPT_String m_path;
		NPT_String m_mimeType;
	};

	MediaStore(Callback *callback);
	~MediaStore();

	NPT_UInt32 systemUpdateId() const;
	void setSystemUpdateId(NPT_UInt32 value);

	void reset();
	void load(NPT_InputStream *inputStream);
	void save(NPT_OutputStream *outputStream) const;

	void importFileSystem(const NPT_String& dir, const NPT_String& name, bool ignoreDot);
	void importIOSPhotos(const NPT_String& name);
	int browse(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& objectID, const NPT_String& browseFlag, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID);
	int search(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& containerID, const NPT_String& searchCriteria, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID);
	bool findFileDetail(const NPT_String& objectId, FileDetail& detail);
	bool buildDidlForFile(const NPT_String& path, const NPT_String& urlFormatStr, NPT_String& mimeType, NPT_String& metaData);

	NPT_Result importPhotos(const NPT_String& name, DLNACoreOp **op);

private:
	MediaStoreImpl *m_impl;
};

class AdvStreamReader
{
public:
	virtual NPT_Result seek(NPT_Size offset) = 0;
	virtual NPT_Result read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead) = 0;
	virtual void abort() = 0;
};

void serveStreamAdv(AbortableTask *task, const MediaStore::FileDetail& detail, AdvStreamReader *reader, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput, NPT_Size bufferSize);

} // namespace deejay

#endif // __DJMediaStore_h__
