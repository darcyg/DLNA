#include "pch.h"
#include "DJUtils.h"

namespace deejay {

#if 1

class ReadWriteLockImpl
{
public:
	ReadWriteLockImpl();
	~ReadWriteLockImpl();

	void lockForRead();
	void lockForWrite();
	void unlockForRead();
	void unlockForWrite();

private:
	NPT_Mutex *m_rwlock;
};

ReadWriteLockImpl::ReadWriteLockImpl()
{
	m_rwlock = new NPT_Mutex();
}

ReadWriteLockImpl::~ReadWriteLockImpl()
{
	delete m_rwlock;
}

void ReadWriteLockImpl::lockForRead()
{
	m_rwlock->Lock();
}

void ReadWriteLockImpl::unlockForRead()
{
	m_rwlock->Unlock();
}

void ReadWriteLockImpl::lockForWrite()
{
	m_rwlock->Lock();
}

void ReadWriteLockImpl::unlockForWrite()
{
	m_rwlock->Unlock();
}

#else

class ReadWriteLockImpl
{
public:
	ReadWriteLockImpl();
	~ReadWriteLockImpl();

	void lockForRead();
	void lockForWrite();
	void unlockForRead();
	void unlockForWrite();

private:
	NPT_RWLock *m_rwlock;
};

ReadWriteLockImpl::ReadWriteLockImpl()
{
	m_rwlock = new NPT_RWLock();
}

ReadWriteLockImpl::~ReadWriteLockImpl()
{
	delete m_rwlock;
}

void ReadWriteLockImpl::lockForRead()
{
	m_rwlock->LockForRead();
}

void ReadWriteLockImpl::unlockForRead()
{
	m_rwlock->UnlockForRead();
}

void ReadWriteLockImpl::lockForWrite()
{
	m_rwlock->LockForWrite();
}

void ReadWriteLockImpl::unlockForWrite()
{
	m_rwlock->UnlockForWrite();
}

#endif

ReadWriteLock::ReadWriteLock()
{
	m_impl = new ReadWriteLockImpl();
}

ReadWriteLock::~ReadWriteLock()
{
	delete m_impl;
}

void ReadWriteLock::lockForRead() const
{
	m_impl->lockForRead();
}

void ReadWriteLock::lockForWrite() const
{
	m_impl->lockForWrite();
}

void ReadWriteLock::unlockForRead() const
{
	m_impl->unlockForRead();
}

void ReadWriteLock::unlockForWrite() const
{
	m_impl->unlockForWrite();
}

ReadLocker::ReadLocker(const ReadWriteLock *lock)
	: m_lock(lock)
{
	m_lock->lockForRead();
}

ReadLocker::ReadLocker(const ReadWriteLock& lock)
	: m_lock(&lock)
{
	m_lock->lockForRead();
}

ReadLocker::~ReadLocker()
{
	m_lock->unlockForRead();
}

WriteLocker::WriteLocker(const ReadWriteLock *lock)
	: m_lock(lock)
{
	m_lock->lockForWrite();
}

WriteLocker::WriteLocker(const ReadWriteLock& lock)
	: m_lock(&lock)
{
	m_lock->lockForWrite();
}

WriteLocker::~WriteLocker()
{
	m_lock->unlockForWrite();
}

UUID::UUID()
{
	memset(m_data, 0, sizeof(m_data));
}

UUID::UUID(const NPT_UInt8 data[16])
{
	memcpy(m_data, data, sizeof(m_data));
}

UUID::UUID(NPT_UInt64 mostSigBits, NPT_UInt64 leastSigBits)
{
	setData(mostSigBits, leastSigBits);
}

UUID::UUID(const UUID& other)
{
	memcpy(m_data, other.m_data, sizeof(m_data));
}

UUID& UUID::operator=(const UUID& other)
{
	memcpy(m_data, other.m_data, sizeof(m_data));
	return *this;
}

bool UUID::operator==(const UUID& other) const
{
	return memcmp(m_data, other.m_data, sizeof(m_data)) == 0;
}

bool UUID::operator!=(const UUID& other) const
{
	return memcmp(m_data, other.m_data, sizeof(m_data)) != 0;
}

static const UUID g_nullUUID;


UUID UUID::generate()
{
	UUID uuid;
	if (NPT_SUCCEEDED(NPT_System::GenerateUUID(uuid.m_data))) {
		return uuid;
	}
	return g_nullUUID;
}

const UUID& UUID::null()
{
	return g_nullUUID;
}

UUID UUID::fromString(const NPT_String& s)
{
	const char *cc = s.GetChars();
	if (s.GetLength() == 36 && cc[8] == '-' && cc[13] == '-' && cc[18] == '-' && cc[23] == '-') {
		UUID uuid;
		if (!parseHex(*reinterpret_cast<NPT_UInt32*>(uuid.m_data + 0), cc, 8)) return UUID();
		if (!parseHex(*reinterpret_cast<NPT_UInt16*>(uuid.m_data + 4), cc + 9, 4)) return UUID();
		if (!parseHex(*reinterpret_cast<NPT_UInt16*>(uuid.m_data + 6), cc + 14, 4)) return UUID();
		if (!parseHex(*reinterpret_cast<NPT_UInt8*>(uuid.m_data + 8), cc + 19, 2)) return UUID();
		if (!parseHex(*reinterpret_cast<NPT_UInt8*>(uuid.m_data + 9), cc + 21, 2)) return UUID();
		for (int i = 0; i < 6; i++) {
			if (!parseHex(*reinterpret_cast<NPT_UInt8*>(uuid.m_data + 10 + i), cc + 24 + i * 2, 2)) return UUID();
		}
		return uuid;
	}
	return UUID();
}

NPT_String UUID::toString() const
{
	return NPT_String::Format("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		*reinterpret_cast<const NPT_UInt32*>(m_data+0),
		*reinterpret_cast<const NPT_UInt16*>(m_data+4),
		*reinterpret_cast<const NPT_UInt16*>(m_data+6),
		*reinterpret_cast<const NPT_UInt8*>(m_data+8),
		*reinterpret_cast<const NPT_UInt8*>(m_data+9),
		*reinterpret_cast<const NPT_UInt8*>(m_data+10),
		*reinterpret_cast<const NPT_UInt8*>(m_data+11),
		*reinterpret_cast<const NPT_UInt8*>(m_data+12),
		*reinterpret_cast<const NPT_UInt8*>(m_data+13),
		*reinterpret_cast<const NPT_UInt8*>(m_data+14),
		*reinterpret_cast<const NPT_UInt8*>(m_data+15)
		);
}

bool UUID::isNull() const
{
	return *this == g_nullUUID;
}

void UUID::getData(NPT_UInt8 data[16]) const
{
	memcpy(data, m_data, 16);
}

void UUID::setData(const NPT_UInt8 data[16])
{
	memcpy(m_data, data, 16);
}

void UUID::getData(NPT_UInt64& mostSigBits, NPT_UInt64& leastSigBits) const
{
	mostSigBits = 0;
	mostSigBits |= *reinterpret_cast<const NPT_UInt32*>(m_data+0);
	mostSigBits <<= 16;
	mostSigBits |= *reinterpret_cast<const NPT_UInt16*>(m_data+4);
	mostSigBits <<= 16;
	mostSigBits |= *reinterpret_cast<const NPT_UInt16*>(m_data+6);

	leastSigBits = 0;
	for (int i = 0; i < 8; i++) {
		leastSigBits <<= 8;
		leastSigBits |= *reinterpret_cast<const NPT_UInt8*>(m_data+8+i);
	}
}

void UUID::setData(NPT_UInt64 mostSigBits, NPT_UInt64 leastSigBits)
{
	*reinterpret_cast<NPT_UInt32*>(m_data+0) = static_cast<NPT_UInt32>(mostSigBits >> 32);
	*reinterpret_cast<NPT_UInt16*>(m_data+4) = static_cast<NPT_UInt32>((mostSigBits & 0xFFFF0000) >> 16);
	*reinterpret_cast<NPT_UInt16*>(m_data+6) = static_cast<NPT_UInt32>((mostSigBits & 0xFFFF));
	*reinterpret_cast<NPT_UInt8*>(m_data+8) = static_cast<NPT_UInt8>(leastSigBits >> 56);
	*reinterpret_cast<NPT_UInt8*>(m_data+9) = static_cast<NPT_UInt8>(leastSigBits >> 48);
	*reinterpret_cast<NPT_UInt8*>(m_data+10) = static_cast<NPT_UInt8>(leastSigBits >> 40);
	*reinterpret_cast<NPT_UInt8*>(m_data+11) = static_cast<NPT_UInt8>(leastSigBits >> 32);
	*reinterpret_cast<NPT_UInt8*>(m_data+12) = static_cast<NPT_UInt8>(leastSigBits >> 24);
	*reinterpret_cast<NPT_UInt8*>(m_data+13) = static_cast<NPT_UInt8>(leastSigBits >> 16);
	*reinterpret_cast<NPT_UInt8*>(m_data+14) = static_cast<NPT_UInt8>(leastSigBits >> 8);
	*reinterpret_cast<NPT_UInt8*>(m_data+15) = static_cast<NPT_UInt8>(leastSigBits);
}

bool parseHex(NPT_UInt32& value, const char *s, int length)
{
	value = 0;
	for (int i = 0; i < length; i++) {
		value <<= 4;
		char c = s[i];
		if (c >= '0' && c <= '9') {
			value |= (c - '0');
		} else if (c >= 'a' && c <= 'f') {
			value |= 10 + (c - 'a');
		} else if (c >= 'A' && c <= 'F') {
			value |= 10 + (c - 'A');
		} else {
			return false;
		}
	}
	return true;
}

bool parseHex(NPT_UInt16& value, const char *s, int length)
{
	value = 0;
	for (int i = 0; i < length; i++) {
		value <<= 4;
		char c = s[i];
		if (c >= '0' && c <= '9') {
			value |= (c - '0');
		} else if (c >= 'a' && c <= 'f') {
			value |= 10 + (c - 'a');
		} else if (c >= 'A' && c <= 'F') {
			value |= 10 + (c - 'A');
		} else {
			return false;
		}
	}
	return true;
}

bool parseHex(NPT_UInt8& value, const char *s, int length)
{
	value = 0;
	for (int i = 0; i < length; i++) {
		value <<= 4;
		char c = s[i];
		if (c >= '0' && c <= '9') {
			value |= (c - '0');
		} else if (c >= 'a' && c <= 'f') {
			value |= 10 + (c - 'a');
		} else if (c >= 'A' && c <= 'F') {
			value |= 10 + (c - 'A');
		} else {
			return false;
		}
	}
	return true;
}

bool parseCacheControl(const NPT_String& text, int& timeout)
{
	if (text.StartsWith("max-age=", true) && NPT_SUCCEEDED(NPT_ParseInteger(text.GetChars() + 8, timeout))) {
		return true;
	}
	// Samsung XXX
	if (text.StartsWith("max-age", true)) {
		NPT_String p1 = text.SubString(7).Trim();
		if (p1.StartsWith("=")) {
			p1 = p1.SubString(1).Trim();
			if (NPT_SUCCEEDED(NPT_ParseInteger(p1, timeout))) {
				return true;
			}
		}
	}
	return false;
}

bool extractUuidFromUSN(const NPT_String& text, UUID& uuid)
{
	if (text.StartsWith("uuid:")) {
		uuid = UUID::fromString(text.SubString(5, 36));
		if (!uuid.isNull()) {
			return true;
		}
	}
	return false;
}

bool contentTypeIsUtf8Xml(const NPT_String& contentType)
{
	if (contentType.Compare("text/xml; charset=\"utf-8\"", true) == 0) {
		return true;
	}

	if (contentType.Compare("text/xml; charset=utf-8", true) == 0) {
		return true;
	}

	if (contentType.Compare("text/xml", true) == 0) {
		return true;
	}

	if (contentType.StartsWith("text/xml", true) && contentType.Find("charset=\"utf-8\"", 0, true) > 0) {
		return true;
	}

	return false;
}

bool parseTimeoutSecond(const NPT_String& text, int& timeout)
{
	if (text.Compare("Second-infinite", true) == 0) {
		timeout = -1;
		return true;
	}

	if (text.StartsWith("Second-", true) && NPT_SUCCEEDED(NPT_ParseInteger(text.GetChars() + 7, timeout))) {
		return true;
	}

	return false;
}

NPT_String getElementText(const NPT_XmlElementNode *el)
{
	const NPT_String *t = el->GetText();
	if (t) {
		return *t;
	}
	return NPT_String();
}

NPT_String getHttpStatusText(int statusCode)
{
	switch (statusCode) {
	case 200:
		return "OK";
	case 206:
		return "Partial Content";
	case 400:
		return "Bad Request";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 412:
		return "Precondition Failed";
	case 416:
		return "Requested Range Not Satisfiable";
	case 500:
		return "Internal Server Error";
	default:
		return NPT_String("WTF!!!");
	}
}

void setStatusCode(NPT_HttpResponse& resp, int statusCode)
{
	resp.SetStatus(statusCode, getHttpStatusText(statusCode), resp.GetProtocol());
}

bool matchNamespace(const NPT_XmlElementNode *el, const char *namespaceUri)
{
	const NPT_String *ns = el->GetNamespace();
	if (ns && ns->Compare(namespaceUri) == 0) {
		return true;
	}
	return false;
}

bool matchTagNamespace(const NPT_XmlElementNode *el, const char *tagName, const char *namespaceUri)
{
	if (el->GetTag().Compare(tagName) == 0) {
		return matchNamespace(el, namespaceUri);
	}
	return false;
}

bool parseBoolean(const NPT_String& text)
{
	if (text.Compare("true", true) == 0) {
		return true;
	}

	if (text.Compare("false", true) == 0) {
		return false;
	}

	if (text.Compare("0") == 0) {
		return false;
	}

	if (text.IsEmpty()) {
		return false;
	}

	return true;
}

bool parseResolution(const NPT_String& text, NPT_UInt32& x, NPT_UInt32& y)
{
	int pos = text.Find('x', 0, true);
	if (pos < 0) {
		return false;
	}

	if (NPT_FAILED(NPT_ParseInteger(text.Left(pos), x))) {
		return false;
	}

	if (NPT_FAILED(NPT_ParseInteger(text.SubString(pos+1), y))) {
		return false;
	}

	return true;
}

NPT_Result Helper::parseTrackDurationString(const NPT_String& text, NPT_UInt64& duration)
{
	NPT_List<NPT_String> parts = text.Split(":");
	if (parts.GetItemCount() != 3) {
		return NPT_ERROR_INVALID_FORMAT;
	}

	NPT_Result nr;
	unsigned int hh, mm, ss;

	nr = NPT_ParseInteger(parts.GetItem(0)->GetChars(), hh);
	if (NPT_FAILED(nr)) {
		return nr;
	}

	nr = NPT_ParseInteger(parts.GetItem(1)->GetChars(), mm);
	if (NPT_FAILED(nr)) {
		return nr;
	}

	NPT_List<NPT_String> parts2 = parts.GetItem(2)->Split(".");
	nr = NPT_ParseInteger(parts2.GetItem(0)->GetChars(), ss);
	if (NPT_FAILED(nr)) {
		return nr;
	}

	unsigned int ms = 0;
	if (parts2.GetItemCount() == 2) {
		if (parts2.GetItem(1)->Find('/') < 0) {
			nr = NPT_ParseInteger(parts2.GetItem(1)->GetChars(), ms);
			if (NPT_FAILED(nr)) {
				return nr;
			}
		}
	} else if (parts2.GetItemCount() != 1) {
		return NPT_ERROR_INVALID_FORMAT;
	}

	duration = hh;
	duration = duration * 60 + mm;
	duration = duration * 60 + ss;
	duration = duration * 1000 + ms;
	return NPT_SUCCESS;
}

NPT_String Helper::formatTrackDuration(NPT_UInt64 duration)
{
	NPT_UInt64 hh, mm, ss, ms;
	hh = duration / (60 * 60 * 1000);
	duration -= hh * (60 * 60 * 1000);
	mm = duration / (60 * 1000);
	duration -= mm * (60 * 1000);
	ss = duration / (1000);
	duration -= ss * 1000;
	ms = duration;
	//return NPT_String::Format("%s:%s:%s.%s", NPT_String::FromIntegerU(hh).GetChars(), NPT_String::FromIntegerU(mm).GetChars(), NPT_String::FromIntegerU(ss).GetChars(), NPT_String::FromIntegerU(ms).GetChars());
	return NPT_String::Format("%s:%s:%s", NPT_String::FromIntegerU(hh).GetChars(), NPT_String::FromIntegerU(mm).GetChars(), NPT_String::FromIntegerU(ss).GetChars());
}

static ReadWriteLock g_lock1;
NPT_String g_userAgentString;

void Helper::setUserAgentString(const NPT_String& userAgent)
{
	WriteLocker locker(g_lock1);
	g_userAgentString = userAgent;
}

void Helper::setupHttpRequest(NPT_HttpRequest& req)
{
	ReadLocker locker(g_lock1);
	if (!g_userAgentString.IsEmpty()) {
		req.GetHeaders().SetHeader(NPT_HTTP_HEADER_USER_AGENT, g_userAgentString, true);
	}
}

} // namespace deejay
