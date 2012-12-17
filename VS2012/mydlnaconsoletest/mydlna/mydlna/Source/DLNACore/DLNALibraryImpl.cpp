#include "DLNALibraryImpl.h"

namespace deejay {

DLNALibraryImpl::DLNALibraryImpl()
	: m_refCount(0)
{
}

DLNALibraryImpl::~DLNALibraryImpl()
{
}

DLNALibrary *DLNALibraryImpl::newDLNALibrary()
{
	return new DLNALibrary();
}

URLLoaderImpl::URLLoaderImpl()
	: m_state(State_Stopped)
{
}

URLLoaderImpl::~URLLoaderImpl()
{
	waitForDone();
}

NPT_Result URLLoaderImpl::start()
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Stopped) {
		return NPT_ERROR_INVALID_STATE;
	}

	m_state = State_Started;
	return NPT_SUCCESS;
}

void URLLoaderImpl::shutdown()
{
	WriteLocker locker(m_stateLock);
	if (m_state == State_Started) {
		m_state = State_Stopping;

		URLLoaderTaskImpl *taskImpl;
		while (NPT_SUCCEEDED(m_queue.PopHead(taskImpl))) {
			taskImpl->release();
		}

		NPT_Cardinal count = m_threadList.GetItemCount();
		for (NPT_Ordinal i = 0; i < count; ) {
			NPT_List<URLLoaderThread*>::Iterator it = m_threadList.GetItem(i);
			URLLoaderThread *thread = *it;
			if (NPT_SUCCEEDED(thread->Wait(0))) {
				m_threadList.Erase(it);
				delete thread;
				count--;
			} else {
				thread->abort();
				i++;
			}
		}
	}
}

void URLLoaderImpl::waitForDone()
{
	{
		ReadLocker locker(m_stateLock);
		if (m_state != State_Stopping) {
			return;
		}
	}

	for (NPT_Ordinal i = 0; i < m_threadList.GetItemCount(); i++) {
		URLLoaderThread *thread = *m_threadList.GetItem(i);
		thread->Wait();
	}

	WriteLocker locker(m_stateLock);
	if (m_state == State_Stopping) {
		m_threadList.Apply(NPT_ObjectDeleter<URLLoaderThread>());
		m_threadList.Clear();
		m_state = State_Stopped;
	}
}

NPT_Result URLLoaderImpl::enqueue(const NPT_String& url, URLLoaderTask::FinishCallback *callback, URLLoaderTask **task)
{
	WriteLocker locker(m_stateLock);
	if (m_state != State_Started) {
		return NPT_ERROR_INVALID_STATE;
	}

	URLLoaderTaskImpl *taskImpl = new URLLoaderTaskImpl(this, callback, url);
	taskImpl->addRef();

	taskImpl->addRef();
	*task = taskImpl;

	m_queue.Add(taskImpl);

	NPT_Cardinal count = m_threadList.GetItemCount();
	for (NPT_Ordinal i = 0; i < count; ) {
		NPT_List<URLLoaderThread*>::Iterator it = m_threadList.GetItem(i);
		URLLoaderThread *thread = *it;
		if (NPT_SUCCEEDED(thread->Wait(0))) {
			m_threadList.Erase(it);
			delete thread;
			count--;
		} else {
			i++;
		}
	}

	while (m_threadList.GetItemCount() < 2) {
		if (NPT_FAILED(startWorkerThreadLocked())) {
			break;
		}
	}

	wakeUpAllThreadsLocked();
	return NPT_SUCCESS;
}

NPT_Result URLLoaderImpl::startWorkerThreadLocked()
{
	URLLoaderThread *thread = new URLLoaderThread(this);
	m_threadList.Add(thread);
	NPT_Result nr = thread->Start();
	if (NPT_FAILED(nr)) {
		m_threadList.Remove(thread);
	}
	return nr;
}

void URLLoaderImpl::wakeUpAllThreadsLocked()
{
	NPT_Cardinal count = m_threadList.GetItemCount();
	for (NPT_Ordinal i = 0; i < count; ) {
		NPT_List<URLLoaderThread*>::Iterator it = m_threadList.GetItem(i);
		URLLoaderThread *thread = *it;
		if (NPT_SUCCEEDED(thread->Wait(0))) {
			m_threadList.Erase(it);
			delete thread;
			count--;
		} else {
			thread->wakeup();
			i++;
		}
	}
}

URLLoaderTaskImpl *URLLoaderImpl::dequeueTask()
{
	WriteLocker locker(m_stateLock);
	URLLoaderTaskImpl *taskImpl;
	if (NPT_SUCCEEDED(m_queue.PopHead(taskImpl))) {
		return taskImpl;
	}
	return NULL;
}

void URLLoaderImpl::elevateTask(URLLoaderTaskImpl *taskImpl)
{
	WriteLocker locker(m_stateLock);
	NPT_List<URLLoaderTaskImpl*>::Iterator it = m_queue.Find(NPT_ObjectComparator<URLLoaderTaskImpl*>(taskImpl));
	if (it) {
		m_queue.Erase(it);
		m_queue.Insert(m_queue.GetFirstItem(), taskImpl);
	}
}

URLLoaderThread::URLLoaderThread(URLLoaderImpl *loader)
	: m_loader(loader), m_abortFlag(false), m_buddy(NULL)
{
}

URLLoaderThread::~URLLoaderThread()
{
}

void URLLoaderThread::abort()
{
	WriteLocker locker(m_stateLock);
	if (m_buddy) {
		m_buddy->abort();
	}
	m_abortFlag = true;
	wakeup();
}

void URLLoaderThread::wakeup()
{
	m_waitVar.SetValue(1);
}

NPT_Result URLLoaderThread::wait(NPT_Timeout timeout)
{
	NPT_Result nr = m_waitVar.WaitWhileEquals(0, timeout);
	if (NPT_SUCCEEDED(nr)) {
		m_waitVar.SetValue(0);
	}
	return nr;
}

void URLLoaderThread::setBuddy(URLLoaderTaskImpl *buddy)
{
	WriteLocker locker(m_stateLock);
	m_buddy = buddy;
}

void URLLoaderThread::Run()
{
	NPT_Result nr;
	while (!m_abortFlag) {
		nr = wait(30000);
		if (nr == NPT_ERROR_TIMEOUT) {
			break;
		}

		while (!m_abortFlag) {
			URLLoaderTaskImpl *taskImpl;
			taskImpl = m_loader->dequeueTask();
			if (taskImpl) {
				setBuddy(taskImpl);
				taskImpl->exec();
				setBuddy(NULL);
				taskImpl->release();
			} else {
				break;
			}
		}
	}
}

URLLoaderTask::URLLoaderTask()
{
}

URLLoaderTask::~URLLoaderTask()
{
}

URLLoaderTaskImpl::URLLoaderTaskImpl(URLLoaderImpl *loader, FinishCallback *callback, const NPT_String& url)
	: m_loader(loader), m_callback(callback), m_refCount(0), m_url(url), m_httpClient(NULL)
{
}

URLLoaderTaskImpl::~URLLoaderTaskImpl()
{
}

int URLLoaderTaskImpl::addRef()
{
	return m_refCount.Increment();
}

int URLLoaderTaskImpl::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

void URLLoaderTaskImpl::elevate()
{
	if (m_loader) {
		m_loader->elevateTask(this);
	}
}

void URLLoaderTaskImpl::resetCallback()
{
	WriteLocker locker(m_stateLock);
	m_callback = NULL;
}

bool URLLoaderTaskImpl::succeeded() const
{
	return m_succeeded;
}

const NPT_DataBuffer& URLLoaderTaskImpl::data() const
{
	return m_data;
}

void URLLoaderTaskImpl::abort()
{
	ReadLocker locker(m_stateLock);
	if (m_httpClient) {
		m_httpClient->Abort();
	}
}

void URLLoaderTaskImpl::exec()
{
	NPT_Result nr;
	NPT_HttpRequest req(m_url, NPT_HTTP_METHOD_GET, NPT_HTTP_PROTOCOL_1_1);
	Helper::setupHttpRequest(req);
	NPT_HttpResponse *resp;
	NPT_HttpClient httpClient;
	m_succeeded = false;
	{
		WriteLocker locker(m_stateLock);
		m_httpClient = &httpClient;
	}
	nr = httpClient.SendRequest(req, resp);
	if (NPT_SUCCEEDED(nr)) {
		PtrHolder<NPT_HttpResponse> resp1(resp);
		NPT_HttpEntity *e = resp->GetEntity();
		if (e && e->GetContentLength() > 0) {
			e->Load(m_data);
			if (m_data.GetDataSize() == e->GetContentLength()) {
				m_succeeded = true;
			}
		}
	}
	{
		WriteLocker locker(m_stateLock);
		m_httpClient = NULL;
	}

	ReadLocker locker(m_stateLock);
	if (m_callback) {
		m_callback->onURLLoaderTaskFinished(this);
	}
}

} // namespace deejay
