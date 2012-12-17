#ifndef __DLNALibrary_h__
#define __DLNALibrary_h__

#include <Neptune.h>

namespace deejay {

class DLNALibraryImpl;

class DLNALibrary
{
public:
	int addRef();
	int release();

	DLNALibraryImpl *d_ptr() const;

private:
	friend class DLNALibraryImpl;

	DLNALibrary();
	~DLNALibrary();
	DLNALibrary(const DLNALibrary&);
	DLNALibrary& operator=(const DLNALibrary&);

	DLNALibraryImpl *m_impl;
};

class URLLoader;
class URLLoaderTask;

class URLLoaderImpl;

class URLLoaderTask
{
public:
	class FinishCallback
	{
	public:
		virtual void onURLLoaderTaskFinished(URLLoaderTask *task) = 0;
	};

	virtual int addRef() = 0;
	virtual int release() = 0;
	virtual void elevate() = 0;
	virtual void resetCallback() = 0;
	virtual bool succeeded() const = 0;
	virtual const NPT_DataBuffer& data() const = 0;

protected:
	URLLoaderTask();
	virtual ~URLLoaderTask();
};

class URLLoader
{
public:
	URLLoader();
	~URLLoader();

	NPT_Result start();
	void shutdown();
	void waitForDone();

	NPT_Result enqueue(const NPT_String& url, URLLoaderTask::FinishCallback *callback, URLLoaderTask **task);

private:
	URLLoaderImpl *m_impl;
};

} // namespace deejay

#endif // __DLNALibrary_h__
