#ifndef __DJUtils_h__
#define __DJUtils_h__

#include <Neptune.h>

namespace deejay {

class ReadWriteLockImpl;

class ReadWriteLock
{
public:
	ReadWriteLock();
	~ReadWriteLock();

	void lockForRead() const;
	void lockForWrite() const;
	void unlockForRead() const;
	void unlockForWrite() const;

private:
	ReadWriteLockImpl *m_impl;
};

class ReadLocker
{
public:
	ReadLocker(const ReadWriteLock *lock);
	ReadLocker(const ReadWriteLock& lock);
	~ReadLocker();

private:
	const ReadWriteLock *m_lock;
};

class WriteLocker
{
public:
	WriteLocker(const ReadWriteLock *lock);
	WriteLocker(const ReadWriteLock& lock);
	~WriteLocker();

private:
	const ReadWriteLock *m_lock;
};

class UUID
{
public:
	UUID();
	UUID(const NPT_UInt8 data[16]);
	UUID(NPT_UInt64 mostSigBits, NPT_UInt64 leastSigBits);
	UUID(const UUID& other);

	UUID& operator=(const UUID& other);
	bool operator==(const UUID& other) const;
	bool operator!=(const UUID& other) const;

	NPT_String toString() const;
	bool isNull() const;
	void getData(NPT_UInt8 data[16]) const;
	void setData(const NPT_UInt8 data[16]);
	void getData(NPT_UInt64& mostSigBits, NPT_UInt64& leastSigBits) const;
	void setData(NPT_UInt64 mostSigBits, NPT_UInt64 leastSigBits);

	static UUID fromString(const NPT_String& s);
	static UUID generate();
	static const UUID& null();

private:
	NPT_UInt8 m_data[16];
};

template <class T>
class DefaultPtrDeletePolicy
{
public:
	static void Delete(T *ptr)
	{
		delete ptr;
	}
};

template <class T, class DeletePolicy = DefaultPtrDeletePolicy<T> >
class PtrHolder
{
public:
	PtrHolder(T *ptr)
		: m_ptr(ptr)
	{
	}

	~PtrHolder()
	{
		if (m_ptr) {
			DeletePolicy::Delete(m_ptr);
		}
	}

	T *detach()
	{
		T *p = m_ptr;
		m_ptr = NULL;
		return p;
	}

	T *m_ptr;
};

bool parseHex(NPT_UInt32& value, const char *s, int length = 8);
bool parseHex(NPT_UInt16& value, const char *s, int length = 4);
bool parseHex(NPT_UInt8& value, const char *s, int length = 2);

bool parseCacheControl(const NPT_String& text, int& timeout);
bool extractUuidFromUSN(const NPT_String& text, UUID& uuid);
bool contentTypeIsUtf8Xml(const NPT_String& contentType);
bool parseTimeoutSecond(const NPT_String& text, int& timeout);
NPT_String getElementText(const NPT_XmlElementNode *el);
NPT_String getHttpStatusText(int statusCode);
void setStatusCode(NPT_HttpResponse& resp, int statusCode);
bool matchNamespace(const NPT_XmlElementNode *el, const char *namespaceUri);
bool matchTagNamespace(const NPT_XmlElementNode *el, const char *tagName, const char *namespaceUri);
bool parseBoolean(const NPT_String& text);
bool parseResolution(const NPT_String& text, NPT_UInt32& x, NPT_UInt32& y);

class Helper
{
public:
	static NPT_Result parseTrackDurationString(const NPT_String& text, NPT_UInt64& duration);
	static NPT_String formatTrackDuration(NPT_UInt64 duration);
	static void setUserAgentString(const NPT_String& userAgent);
	static void setupHttpRequest(NPT_HttpRequest& req);
};

} // namespace deejay

#endif // __DJUtils_h__
