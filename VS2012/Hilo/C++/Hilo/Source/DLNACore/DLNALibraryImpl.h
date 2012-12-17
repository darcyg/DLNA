#ifndef __DLNALibraryImpl_h__
#define __DLNALibraryImpl_h__

#include "DLNALibrary.h"
#include "DJUtils.h"

namespace deejay {

class DLNALibraryImpl
{
public:
	DLNALibraryImpl();
	~DLNALibraryImpl();

	static DLNALibrary *newDLNALibrary();

	NPT_AtomicVariable m_refCount;
};

class URLLoaderTaskImpl;
class URLLoaderThread;

class URLLoaderImpl
{
public:
	URLLoaderImpl();
	~URLLoaderImpl();

	NPT_Result start();
	void shutdown();
	void waitForDone();

	NPT_Result enqueue(const NPT_String& url, URLLoaderTask::FinishCallback *callback, URLLoaderTask **task);

	NPT_Result startWorkerThreadLocked();
	void wakeUpAllThreadsLocked();
	URLLoaderTaskImpl *dequeueTask();

	void elevateTask(URLLoaderTaskImpl *taskImpl);

	enum State {
		State_Stopped,
		State_Stopping,
		State_Started,
	};

	ReadWriteLock m_stateLock;
	State m_state;

	NPT_List<URLLoaderThread*> m_threadList;

	NPT_List<URLLoaderTaskImpl*> m_queue;
};

class URLLoaderThread
	: public NPT_Thread
{
public:
	URLLoaderThread(URLLoaderImpl *loader);
	~URLLoaderThread();

	virtual void Run();
	void abort();
	void wakeup();
	NPT_Result wait(NPT_Timeout timeout);
	void setBuddy(URLLoaderTaskImpl *buddy);

private:
	URLLoaderImpl *m_loader;
	NPT_SharedVariable m_waitVar;
	ReadWriteLock m_stateLock;
	URLLoaderTaskImpl *m_buddy;
	bool m_abortFlag;
};

class URLLoaderTaskImpl
	: public URLLoaderTask
{
public:
	URLLoaderTaskImpl(URLLoaderImpl *loader, FinishCallback *callback, const NPT_String& url);
	virtual ~URLLoaderTaskImpl();
	virtual int addRef();
	virtual int release();
	virtual void elevate();
	virtual void resetCallback();
	virtual bool succeeded() const;
	virtual const NPT_DataBuffer& data() const;

	void abort();
	void exec();

	URLLoaderImpl *m_loader;
	NPT_AtomicVariable m_refCount;
	NPT_String m_url;
	NPT_HttpClient *m_httpClient;
	FinishCallback *m_callback;
	ReadWriteLock m_stateLock;
	bool m_succeeded;
	NPT_DataBuffer m_data;
};

} // namespace deejay

#endif // __DLNALibraryImpl_h__
