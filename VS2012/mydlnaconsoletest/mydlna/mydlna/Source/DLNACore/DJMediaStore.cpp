#include "DJMediaStoreImpl.h"

namespace deejay {

MediaStore::MediaStore(Callback *callback)
{
	m_impl = new MediaStoreImpl(callback, this);
}

MediaStore::~MediaStore()
{
	delete m_impl;
}

void MediaStore::reset()
{
	m_impl->reset();
}

void MediaStore::load(NPT_InputStream *inputStream)
{
	m_impl->load(inputStream);
}

void MediaStore::save(NPT_OutputStream *outputStream) const
{
	m_impl->save(outputStream);
}

NPT_UInt32 MediaStore::systemUpdateId() const
{
	return m_impl->systemUpdateId();
}

void MediaStore::setSystemUpdateId(NPT_UInt32 value)
{
	m_impl->setSystemUpdateId(value);
}

void MediaStore::importFileSystem(const NPT_String& dir, const NPT_String& name, bool ignoreDot)
{
	m_impl->importFileSystem(dir, name, ignoreDot);
}

void MediaStore::importIOSPhotos(const NPT_String& name)
{
	m_impl->importIOSPhotos(name);
}

int MediaStore::browse(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& objectID, const NPT_String& browseFlag, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID)
{
	return m_impl->browse(task, ifctx, reqCtx, urlFormatStr, objectID, browseFlag, filter, startingIndex, requestedCount, sortCriteria, result, numberReturned, totalMatches, updateID);
}

int MediaStore::search(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& containerID, const NPT_String& searchCriteria, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID)
{
	return m_impl->search(task, ifctx, reqCtx, urlFormatStr, containerID, searchCriteria, filter, startingIndex, requestedCount, sortCriteria, result, numberReturned, totalMatches, updateID);
}

bool MediaStore::findFileDetail(const NPT_String& objectId, FileDetail& detail)
{
	return m_impl->findFileDetail(objectId, detail);
}

bool MediaStore::buildDidlForFile(const NPT_String& path, const NPT_String& urlFormatStr, NPT_String& mimeType, NPT_String& metaData)
{
	return m_impl->buildDidlForFile(path, urlFormatStr, mimeType, metaData);
}

NPT_Result MediaStore::importPhotos(const NPT_String& name, DLNACoreOp **op)
{
	return m_impl->importPhotos(name, op);
}

class ServeFileAbortCallback
	: public AbortableTask::Callback
{
public:
	ServeFileAbortCallback(bool *abortFlag, AdvStreamReader *reader)
		: m_abortFlag(abortFlag), m_reader(reader)
	{
	}

	virtual void onAborted(AbortableTask *task)
	{
		*m_abortFlag = true;
		if (m_reader) {
			m_reader->abort();
		}
	}

private:
	bool *m_abortFlag;
	AdvStreamReader *m_reader;
};

bool parseRangeHeader(const NPT_String& text, NPT_UInt64 totalLength, NPT_UInt64& offset, NPT_UInt64& length)
{
	if (!text.StartsWith("bytes=")) {
		return false;
	}

	if (text.Find(',') >= 0) {
		// multiple ranges, currently unsupported
		return false;
	}

	int sep = text.Find('-', 6);
	if (sep < 0) {
		return false;
	}

	NPT_String p1 = text.SubString(6, sep - 6);
	NPT_String p2 = text.SubString(sep + 1);
	NPT_UInt64 v1, v2;

	if (p1.IsEmpty()) {
		if (p2.IsEmpty()) {
			return false;
		} else {
			if (NPT_FAILED(NPT_ParseInteger64(p2, v2))) {
				return false;
			}
			if (v2 <= totalLength) {
				offset = totalLength - v2;
				length = v2;
			} else {
				offset = 0;
				length = totalLength;
			}
		}
	} else {
		if (NPT_FAILED(NPT_ParseInteger64(p1, v1))) {
			return false;
		}
		if (v1 >= totalLength) {
			return false;
		}
		if (p2.IsEmpty()) {
			offset = v1;
			length = totalLength - offset;
		} else {
			if (NPT_FAILED(NPT_ParseInteger64(p2, v2))) {
				return false;
			}
			if (v2 >= totalLength) {
				return false;
			}
			if (v2 < v1) {
				return false;
			}
			offset = v1;
			length = v2 - v1 + 1;
		}
	}

	return true;
}

void serveStream(AbortableTask *task, NPT_InputStream *inputStream, const NPT_TimeStamp& modificationTime, const NPT_String& mimeType, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput)
{
	bool isGetMethod = req->GetMethod().Compare("GET") == 0;
	bool isHeadMethod = req->GetMethod().Compare("HEAD") == 0;
	if (isGetMethod || isHeadMethod) {
		NPT_Result nr;
		NPT_UInt64 offset, length;
		NPT_LargeSize totalSize;
		inputStream->GetSize(totalSize);

		NPT_HttpHeader *hdrRange = req->GetHeaders().GetHeader("RANGE");
		if (hdrRange) {
			if (!parseRangeHeader(hdrRange->GetValue(), totalSize, offset, length)) {
				setStatusCode(resp, 416);
				httpOutput->writeResponseHeader(resp);
				return;
			}
			setStatusCode(resp, 206);
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_RANGE, NPT_String::Format("bytes %s-%s/%s", NPT_String::FromIntegerU(offset).GetChars(), NPT_String::FromIntegerU(offset + length - 1).GetChars(), NPT_String::FromIntegerU(totalSize).GetChars()));
			inputStream->Seek(offset);
		} else {
			offset = 0;
			length = totalSize;
			setStatusCode(resp, 200);
		}

		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, mimeType);
		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromIntegerU(length));
		resp.GetHeaders().SetHeader("Last-Modified", NPT_DateTime(modificationTime).ToString(NPT_DateTime::FORMAT_RFC_1123));
		resp.GetHeaders().SetHeader("Accept-Ranges", "bytes");

		resp.GetHeaders().SetHeader("EXT", "");
		if (reqCtx.transferMode != FrontEnd::TM_None) {
			const char *transferMode = "Streaming";
			switch (reqCtx.transferMode) {
			case FrontEnd::TM_Background:
				transferMode = "Background";
				break;
			case FrontEnd::TM_Interactive:
				transferMode = "Interactive";
				break;
			}
			resp.GetHeaders().SetHeader("transferMode.dlna.org", transferMode);
		}

		if (reqCtx.getcontentFeaturesReq) {
			NPT_String contentFeatures("DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000");
			resp.GetHeaders().SetHeader("contentFeatures.dlna.org", contentFeatures);
		}

		httpOutput->writeResponseHeader(resp);

		if (isGetMethod) {

			bool abortFlag = false;
			ServeFileAbortCallback abortCallback(&abortFlag, NULL);

			if (task->registerAbortCallback(&abortCallback)) {
				NPT_DataBuffer buffer(4096);

				NPT_UInt64 cbRemain = length;

				for (;;) {
					if (abortFlag) {
						break;
					}

					if (cbRemain == 0) {
						break;
					}

					NPT_Size cbRead;
					NPT_UInt64 cbToRead = cbRemain;
					if (cbToRead > buffer.GetBufferSize()) {
						cbToRead = buffer.GetBufferSize();
					}
					nr = inputStream->Read(buffer.UseData(), buffer.GetBufferSize(), &cbRead);
					if (NPT_FAILED(nr)) {
						/*if (nr == NPT_ERROR_EOS) {
						} else {
						}*/
						break;
					}

					if (abortFlag) {
						break;
					}

					if (cbRead > 0) {
						cbRemain -= cbRead;
						httpOutput->writeData(buffer.GetData(), cbRead);
					}
				}

				task->unregisterAbortCallback(&abortCallback);
			}
		}
	} else {
		setStatusCode(resp, 405);
		resp.GetHeaders().SetHeader("Allow", "GET, HEAD");
		httpOutput->writeResponseHeader(resp);
	}
}

void serveFile2(AbortableTask *task, const NPT_String& filePath, const NPT_String& mimeType, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput)
{
#if 1
	NPT_File f(filePath);
	NPT_FileInfo fileInfo;
	if (NPT_FAILED(f.GetInfo(fileInfo)) || fileInfo.m_Type != NPT_FileInfo::FILE_TYPE_REGULAR || NPT_FAILED(f.Open(NPT_FILE_OPEN_MODE_READ))) {
		setStatusCode(resp, 404);
		httpOutput->writeResponseHeader(resp);
		return;
	}

	NPT_InputStreamReference fileInput;
	f.GetInputStream(fileInput);
	serveStream(task, fileInput.AsPointer(), fileInfo.m_ModificationTime, mimeType, reqCtx, req, resp, httpOutput);
#else
	bool isGetMethod = req->GetMethod().Compare("GET") == 0;
	bool isHeadMethod = req->GetMethod().Compare("HEAD") == 0;
	if (isGetMethod || isHeadMethod) {
		NPT_Result nr;
		NPT_File f(filePath);
		NPT_FileInfo fileInfo;
		if (NPT_FAILED(f.GetInfo(fileInfo)) || fileInfo.m_Type != NPT_FileInfo::FILE_TYPE_REGULAR || NPT_FAILED(f.Open(NPT_FILE_OPEN_MODE_READ))) {
			setStatusCode(resp, 404);
			httpOutput->writeResponseHeader(resp);
			return;
		}

		NPT_InputStreamReference fileInput;
		f.GetInputStream(fileInput);

		NPT_UInt64 offset, length;

		NPT_HttpHeader *hdrRange = req->GetHeaders().GetHeader("RANGE");
		if (hdrRange) {
			if (!parseRangeHeader(hdrRange->GetValue(), fileInfo.m_Size, offset, length)) {
				setStatusCode(resp, 416);
				httpOutput->writeResponseHeader(resp);
				return;
			}
			setStatusCode(resp, 206);
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_RANGE, NPT_String::Format("bytes %s-%s/%s", NPT_String::FromIntegerU(offset).GetChars(), NPT_String::FromIntegerU(offset + length - 1).GetChars(), NPT_String::FromIntegerU(fileInfo.m_Size).GetChars()));
			fileInput->Seek(offset);
		} else {
			offset = 0;
			length = fileInfo.m_Size;
			setStatusCode(resp, 200);
		}

		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, mimeType);
		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromIntegerU(length));
		resp.GetHeaders().SetHeader("Last-Modified", NPT_DateTime(fileInfo.m_ModificationTime).ToString(NPT_DateTime::FORMAT_RFC_1123));
		resp.GetHeaders().SetHeader("Accept-Ranges", "bytes");

		resp.GetHeaders().SetHeader("EXT", "");
		if (reqCtx.transferMode != FrontEnd::TM_None) {
			const char *transferMode = "Streaming";
			switch (reqCtx.transferMode) {
			case FrontEnd::TM_Background:
				transferMode = "Background";
				break;
			case FrontEnd::TM_Interactive:
				transferMode = "Interactive";
				break;
			}
			resp.GetHeaders().SetHeader("transferMode.dlna.org", transferMode);
		}

		if (reqCtx.getcontentFeaturesReq) {
			NPT_String contentFeatures("DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000");
			resp.GetHeaders().SetHeader("contentFeatures.dlna.org", contentFeatures);
		}

		httpOutput->writeResponseHeader(resp);

		if (isGetMethod) {

			bool abortFlag = false;
			ServeFileAbortCallback abortCallback(&abortFlag);

			if (task->registerAbortCallback(&abortCallback)) {
				NPT_DataBuffer buffer(4096);

				NPT_UInt64 cbRemain = length;

				for (;;) {
					if (abortFlag) {
						break;
					}

					if (cbRemain == 0) {
						break;
					}

					NPT_Size cbRead;
					NPT_UInt64 cbToRead = cbRemain;
					if (cbToRead > buffer.GetBufferSize()) {
						cbToRead = buffer.GetBufferSize();
					}
					nr = fileInput->Read(buffer.UseData(), buffer.GetBufferSize(), &cbRead);
					if (NPT_FAILED(nr)) {
						/*if (nr == NPT_ERROR_EOS) {
						} else {
						}*/
						break;
					}

					if (abortFlag) {
						break;
					}

					if (cbRead > 0) {
						cbRemain -= cbRead;
						httpOutput->writeData(buffer.GetData(), cbRead);
					}
				}

				task->unregisterAbortCallback(&abortCallback);
			}
		}
	} else {
		setStatusCode(resp, 405);
		resp.GetHeaders().SetHeader("Allow", "GET, HEAD");
		httpOutput->writeResponseHeader(resp);
	}
#endif
}

class AdvFileReader
	: public AdvStreamReader
{
public:
	AdvFileReader(NPT_InputStream *inputStream)
		: m_inputStream(inputStream)
	{
	}

	virtual NPT_Result seek(NPT_Size offset)
	{
		return m_inputStream->Seek(offset);
	}

	virtual NPT_Result read(void *buffer, NPT_Size bytesToRead, NPT_Size *bytesRead)
	{
		return m_inputStream->Read(buffer, bytesToRead, bytesRead);
	}

	virtual void abort()
	{
	}

private:
	NPT_InputStream *m_inputStream;
};

void serveFile(AbortableTask *task, const NPT_String& filePath, const NPT_String& mimeType, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput)
{
	NPT_File f(filePath);
	NPT_FileInfo fileInfo;
	if (NPT_FAILED(f.GetInfo(fileInfo)) || fileInfo.m_Type != NPT_FileInfo::FILE_TYPE_REGULAR || NPT_FAILED(f.Open(NPT_FILE_OPEN_MODE_READ))) {
		setStatusCode(resp, 404);
		httpOutput->writeResponseHeader(resp);
		return;
	}

	MediaStore::FileDetail detail;
	detail.m_mimeType = mimeType;
	detail.m_path = filePath;
	detail.m_modificationTime = fileInfo.m_ModificationTime;
	detail.m_size = fileInfo.m_Size;
	detail.m_type = MediaStore::FileDetail::PosixFile;

	NPT_InputStreamReference fileInput;
	f.GetInputStream(fileInput);

	AdvFileReader reader(fileInput.AsPointer());
	serveStreamAdv(task, detail, &reader, reqCtx, req, resp, httpOutput, 1024 * 64);
}

void serveStreamAdv(AbortableTask *task, const MediaStore::FileDetail& detail, AdvStreamReader *reader, const FrontEnd::RequestContext& reqCtx, const NPT_HttpRequest *req, NPT_HttpResponse& resp, HttpOutput *httpOutput, NPT_Size bufferSize)
{
	bool isGetMethod = req->GetMethod().Compare("GET") == 0;
	bool isHeadMethod = req->GetMethod().Compare("HEAD") == 0;
	if (isGetMethod || isHeadMethod) {
		NPT_Result nr;
		NPT_UInt64 offset, length;

		NPT_HttpHeader *hdrRange = req->GetHeaders().GetHeader("RANGE");
		if (hdrRange) {
			if (!parseRangeHeader(hdrRange->GetValue(), detail.m_size, offset, length)) {
				setStatusCode(resp, 416);
				httpOutput->writeResponseHeader(resp);
				return;
			}
			setStatusCode(resp, 206);
			resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_RANGE, NPT_String::Format("bytes %s-%s/%s", NPT_String::FromIntegerU(offset).GetChars(), NPT_String::FromIntegerU(offset + length - 1).GetChars(), NPT_String::FromIntegerU(detail.m_size).GetChars()));
		} else {
			offset = 0;
			length = detail.m_size;
			setStatusCode(resp, 200);
		}

		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_TYPE, detail.m_mimeType);
		resp.GetHeaders().SetHeader(NPT_HTTP_HEADER_CONTENT_LENGTH, NPT_String::FromIntegerU(length));
		resp.GetHeaders().SetHeader("Last-Modified", NPT_DateTime(detail.m_modificationTime).ToString(NPT_DateTime::FORMAT_RFC_1123));
		resp.GetHeaders().SetHeader("Accept-Ranges", "bytes");

		resp.GetHeaders().SetHeader("EXT", "");
		if (reqCtx.transferMode != FrontEnd::TM_None) {
			const char *transferMode = "Streaming";
			switch (reqCtx.transferMode) {
			case FrontEnd::TM_Background:
				transferMode = "Background";
				break;
			case FrontEnd::TM_Interactive:
				transferMode = "Interactive";
				break;
			}
			resp.GetHeaders().SetHeader("transferMode.dlna.org", transferMode);
		}
		 
		if (reqCtx.getcontentFeaturesReq) {
			NPT_String contentFeatures("DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000");
			resp.GetHeaders().SetHeader("contentFeatures.dlna.org", contentFeatures);
		}

		httpOutput->writeResponseHeader(resp);

		if (isGetMethod) {

			bool abortFlag = false;
			ServeFileAbortCallback abortCallback(&abortFlag, reader);

			if (task->registerAbortCallback(&abortCallback)) {

				reader->seek(offset);

				NPT_DataBuffer buffer(bufferSize);

				NPT_UInt64 cbRemain = length;

				for (;;) {
					if (abortFlag) {
						break;
					}

					if (cbRemain == 0) {
						break;
					}

					NPT_Size cbRead;
					NPT_Size cbToRead = cbRemain;
					if (cbToRead > buffer.GetBufferSize()) {
						cbToRead = buffer.GetBufferSize();
					}
					nr = reader->read(buffer.UseData(), buffer.GetBufferSize(), &cbRead);
					if (NPT_FAILED(nr)) {
						/*if (nr == NPT_ERROR_EOS) {
						} else {
						}*/
						break;
					}

					if (abortFlag) {
						break;
					}

					if (cbRead > 0) {
						cbRemain -= cbRead;
						httpOutput->writeData(buffer.GetData(), cbRead);
					}
				}

				task->unregisterAbortCallback(&abortCallback);
			}
		}
	} else {
		setStatusCode(resp, 405);
		resp.GetHeaders().SetHeader("Allow", "GET, HEAD");
		httpOutput->writeResponseHeader(resp);
	}
}

} // namespace deejay
