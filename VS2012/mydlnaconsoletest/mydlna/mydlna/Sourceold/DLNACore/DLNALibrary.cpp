#include "DLNALibraryImpl.h"

namespace deejay {

DLNALibrary::DLNALibrary()
{
	m_impl = new DLNALibraryImpl();
}

DLNALibrary::~DLNALibrary()
{
	delete m_impl;
}

DLNALibraryImpl *DLNALibrary::d_ptr() const
{
	return m_impl;
}

int DLNALibrary::addRef()
{
	return d_ptr()->m_refCount.Increment();
}

int DLNALibrary::release()
{
	int rc = d_ptr()->m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

URLLoader::URLLoader()
{
	m_impl = new URLLoaderImpl();
}

URLLoader::~URLLoader()
{
	delete m_impl;
}

NPT_Result URLLoader::start()
{
	return m_impl->start();
}

void URLLoader::shutdown()
{
	m_impl->shutdown();
}

void URLLoader::waitForDone()
{
	m_impl->waitForDone();
}

NPT_Result URLLoader::enqueue(const NPT_String& url, URLLoaderTask::FinishCallback *callback, URLLoaderTask **task)
{
	return m_impl->enqueue(url, callback, task);
}

} // namespace deejay
