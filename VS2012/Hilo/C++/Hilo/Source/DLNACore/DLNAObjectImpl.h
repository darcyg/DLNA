#ifndef __DLNAObjectImpl_h__
#define __DLNAObjectImpl_h__

#include "DLNAObject.h"

namespace deejay {

class DLNAResourceImpl
{
public:
	DLNAResourceImpl();
	~DLNAResourceImpl();

	NPT_String m_protocolInfoStr;
	NPT_String m_durationStr;
	NPT_String m_url;

	NPT_UInt64 m_size;
	NPT_UInt32 m_bitrate;
	NPT_UInt32 m_sampleFrequency;
	NPT_UInt32 m_bitsPerSample;
	NPT_UInt32 m_nrAudioChannels;
	NPT_UInt32 m_colorDepth;
	NPT_UInt32 m_resolutionX;
	NPT_UInt32 m_resolutionY;

	NPT_String m_piProtocol;
	NPT_String m_piNetwork;
	NPT_String m_piContentFormat;
	NPT_String m_piAdditionalInfo;
	NPT_String m_mimeType;
};

class DLNAObjectImpl
{
public:
	DLNAObjectImpl();
	virtual ~DLNAObjectImpl();

	NPT_AtomicVariable m_refCount;
	NPT_String m_objectId;
	NPT_String m_parentId;
	NPT_String m_title;
	NPT_String m_creator;
	NPT_String m_upnpClass;
	NPT_String m_upnpClassName;
	NPT_String m_writeStatus;
	bool m_restricted;
	NPT_String m_dateStr;

	NPT_String m_channelName;
	NPT_String m_scheduledStartTimeStr;
	NPT_String m_longDescription;
	NPT_String m_description;
	NPT_String m_rating;

	int m_originalTrackNumber;

	NPT_List<NPT_String> m_genreList;
	NPT_List<NPT_String> m_albumList;
	NPT_List<DLNAPeopleInvolved> m_artistList;
	NPT_List<DLNAPeopleInvolved> m_actorList;
	NPT_List<DLNAPeopleInvolved> m_authorList;
	NPT_List<NPT_String> m_albumArtURIList;
	NPT_List<NPT_String> m_publisherList;
	NPT_List<NPT_String> m_rightsList;

	NPT_List<DLNAResource*> m_resList;

	NPT_String m_didlText;

	NPT_Int64 m_resourceSize;
};

class DLNAItemImpl
	: public DLNAObjectImpl
{
public:
	DLNAItemImpl();
	virtual ~DLNAItemImpl();
	static DLNAItem *newDLNAItem();

	NPT_String m_refId;
};

class DLNAContainerImpl
	: public DLNAObjectImpl
{
public:
	DLNAContainerImpl();
	virtual ~DLNAContainerImpl();
	static DLNAContainer *newDLNAContainer();

	int m_childCount;
	bool m_searchable;
	NPT_Int64 m_storageUsed;
};

} // namespace deejay

#endif // __DLNAMediaObjectImpl_h__
