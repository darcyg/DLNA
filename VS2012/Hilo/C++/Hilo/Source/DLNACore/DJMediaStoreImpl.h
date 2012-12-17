#ifndef __DJMediaStoreImpl_h__
#define __DJMediaStoreImpl_h__

#include "DJMediaStore.h"
#include "DJTaskGroup.h"
#include "DLNACoreOp.h"

namespace deejay {

class MSFileItem;
class MSContainer;
class MSLink;
class MSFolder;
class MSObject;

enum MSMediaType
{
	MSMediaType_Unknown,
	MSMediaType_Video,
	MSMediaType_Audio,
	MSMediaType_Image,
};

class MSBaseObject
{
public:
	virtual ~MSBaseObject();
	virtual MSObject *asObject() { return NULL; }
	virtual MSContainer *asContainer() { return NULL; }
	virtual MSFileItem *asFileItem() { return NULL; }
	virtual MSLink *asLink() { return NULL; }
	virtual MSFolder *asFolder() { return NULL; }

	virtual const MSObject *asObject() const { return NULL; }
	virtual const MSContainer *asContainer() const { return NULL; }
	virtual const MSFileItem *asFileItem() const { return NULL; }
	virtual const MSLink *asLink() const { return NULL; }
	virtual const MSFolder *asFolder() const { return NULL; }

	enum Operator {
		Op_Contains,
		Op_DoesNotContain,
		Op_DerivedFrom,
		Op_EqualTo,
		Op_NotEqualTo,
		Op_LessThan,
		Op_LessEqual,
		Op_GreatThan,
		Op_GreatEqual,
		Op_Exists,
	};

	virtual bool hasProperty(const NPT_String& prop) const;
	virtual bool testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const;

protected:
	MSBaseObject();
	MSBaseObject(const NPT_String& objectId);

	static bool eval(const NPT_String& v, const NPT_String& t, Operator op);
	static bool evalNum(const NPT_String& v, const NPT_String& t, Operator op);

public:
	NPT_String m_objectId;
	MSContainer *m_parent;
};

class MSLink
	: public MSBaseObject
{
public:
	MSLink();
	virtual ~MSLink();
	virtual MSLink *asLink() { return this; }
	virtual const MSLink *asLink() const { return this; }

	virtual bool hasProperty(const NPT_String& prop) const;
	virtual bool testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const;

public:
	MSFileItem *m_target;
};

class MSObject
	: public MSBaseObject
{
protected:
	MSObject();
	MSObject(const NPT_String& objectId);
	virtual ~MSObject();
	virtual MSObject *asObject() { return this; }
	virtual const MSObject *asObject() const { return this; }

	virtual bool hasProperty(const NPT_String& prop) const;
	virtual bool testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const;

public:
	NPT_String m_title;
	NPT_String m_creator;
	NPT_String m_upnpClass;
	NPT_TimeStamp m_date;

	NPT_List<NPT_String> m_artistList;
	NPT_List<NPT_String> m_genreList;
	NPT_List<NPT_String> m_albumList;
};

struct MSClassSpec
{
public:
	MSClassSpec(const NPT_String& text, const NPT_String& name, bool includeDerived)
		: m_text(text), m_attrName(name), m_attrIncludeDerived(includeDerived)
	{
	}

	NPT_String m_text;
	NPT_String m_attrName;
	bool m_attrIncludeDerived;
};

class MSContainer
	: public MSObject
{
public:
	MSContainer();
	MSContainer(const NPT_String& objectId);
	virtual ~MSContainer();
	virtual MSContainer* asContainer() { return this; }
	virtual const MSContainer* asContainer() const { return this; }

	void addChild(MSBaseObject *child);
	MSBaseObject *childAt(NPT_Ordinal index) const;
	MSBaseObject *takeChildAt(NPT_Ordinal index);
	NPT_Cardinal childCount() const;

	virtual bool hasProperty(const NPT_String& prop) const;
	virtual bool testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const;

	void addSearchClass(const NPT_String& text, bool includeDerived, const NPT_String& name = NPT_String());
	const MSClassSpec *searchClassAt(NPT_Ordinal index) const;
	NPT_Cardinal searchClassCount() const;

protected:
	NPT_List<MSBaseObject*> m_children;
	NPT_List<MSClassSpec*> m_searchClassList;
};

class MSFileItem
	: public MSObject
{
public:
	MSFileItem();
	virtual ~MSFileItem();
	virtual MSFileItem *asFileItem() { return this; }
	virtual const MSFileItem *asFileItem() const { return this; }

	virtual bool hasProperty(const NPT_String& prop) const;
	virtual bool testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const;

public:
	NPT_String m_filePath;
	NPT_String m_mimeType;
	MSMediaType m_mediaType;
	NPT_UInt64 m_size;
	bool m_isIOSAssetItem;
};

class IOSAssetItem
	: public MSFileItem
{
public:
	IOSAssetItem();
	virtual ~IOSAssetItem();

	enum AssetType {
		AssetType_Photo,
		AssetType_Video,
		AssetType_Unknown,
	};

public:
//	AssetType m_assetType;
//	NPT_List<NPT_String> m_UTIList;
//	NPT_List<NPT_String> m_URLList;
};

class MSFolder
	: public MSContainer
{
public:
	MSFolder();
	virtual ~MSFolder();
	virtual MSFolder *asFolder() { return this; }
	virtual const MSFolder *asFolder() const { return this; }
};

namespace sc {

class Expr
{
public:
	virtual ~Expr();
	virtual bool eval(MSBaseObject *obj) = 0;

protected:
	Expr();
};

class MatchAllExpr
	: public Expr
{
public:
	virtual bool eval(MSBaseObject *obj) { return true; }
};

class MatchNoneExpr
	: public Expr
{
public:
	virtual bool eval(MSBaseObject *obj) { return false; }
};

class LogicalExpr
	: public Expr
{
public:
	LogicalExpr(Expr *expr1, Expr *expr2);
	virtual ~LogicalExpr();
	virtual bool eval(MSBaseObject *obj);

protected:
	virtual bool eval(MSBaseObject *obj, Expr *expr1, Expr *expr2) = 0;

private:
	Expr *m_expr1;
	Expr *m_expr2;
};

class AndExpr
	: public LogicalExpr
{
public:
	AndExpr(Expr *expr1, Expr *expr2);

protected:
	virtual bool eval(MSBaseObject *obj, Expr *expr1, Expr *expr2);
};

class OrExpr
	: public LogicalExpr
{
public:
	OrExpr(Expr *expr1, Expr *expr2);

protected:
	virtual bool eval(MSBaseObject *obj, Expr *expr1, Expr *expr2);
};

class ExistsExpr
	: public Expr
{
public:
	ExistsExpr(const NPT_String& prop, bool exists);
	virtual bool eval(MSBaseObject *obj);

private:
	NPT_String m_prop;
	bool m_exists;
};

class PropExpr
	: public Expr
{
public:
	PropExpr(const NPT_String& prop, const NPT_String& value, MSBaseObject::Operator op);
	virtual bool eval(MSBaseObject *obj);

private:
	NPT_String m_prop;
	NPT_String m_value;
	MSBaseObject::Operator m_op;
};

} // namespace sc

class SCTokenizer
{
public:
	enum Token {
		Token_EOS,
		Token_Error,
		Token_QuotedVal,
		Token_LeftBracket,
		Token_RightBracket,
		Token_EQ,
		Token_NEQ,
		Token_LT,
		Token_LE,
		Token_GT,
		Token_GE,
		Token_Contains,
		Token_DoesNotContain,
		Token_DerivedFrom,
		Token_Exists,
		Token_AND,
		Token_OR,
		Token_True,
		Token_False,
		Token_Property
	};

	SCTokenizer(const NPT_String& searchCriteria);
	Token nextToken(NPT_String& val);

private:
	bool nextChar(char& c);
	bool peekChar(char& c);
	void skipWhitespaces();
	static bool isWhitespace(char c);
	static bool isIdentChar(char c);

	NPT_String m_sc;
	const char *m_pc;
	const char *m_pe;
};

class SCTokenizerLevel2
{
public:
	enum Token {
		Token_EOS,
		Token_Error,
		Token_LeftBracket,
		Token_RightBracket,
		Token_AND,
		Token_OR,
		Token_Expr
	};

	struct Expr
	{
		NPT_String prop;
		NPT_String value;
		MSBaseObject::Operator op;
	};

	SCTokenizerLevel2(const NPT_String& searchCriteria);
	Token nextToken(Expr& expr);

private:
	SCTokenizer m_tokenizer;
};

class MSBasePropComparator
{
public:
	MSBasePropComparator(bool ascending);
	virtual ~MSBasePropComparator();
	int compareN(const MSObject *obj1, const MSObject *obj2) const;

protected:
	virtual int compare(const MSObject *obj1, const MSObject *obj2) const = 0;

private:
	bool m_ascending;
};

class MSUPnPClassComparator
	: public MSBasePropComparator
{
public:
	MSUPnPClassComparator(bool ascending);

protected:
	virtual int compare(const MSObject *obj1, const MSObject *obj2) const;
};

class MSDateComparator
	: public MSBasePropComparator
{
public:
	MSDateComparator(bool ascending);

protected:
	virtual int compare(const MSObject *obj1, const MSObject *obj2) const;
};

class MSTitleComparator
	: public MSBasePropComparator
{
public:
	MSTitleComparator(bool ascending);

protected:
	virtual int compare(const MSObject *obj1, const MSObject *obj2) const;
};


struct MSImportResult
{
	MSFolder *m_folder;
};

class MediaStoreImpl
{
public:
	MediaStoreImpl(MediaStore::Callback *callback, MediaStore *intf);
	~MediaStoreImpl();

	NPT_UInt32 systemUpdateId() const;
	void setSystemUpdateId(NPT_UInt32 value);

	void reset();
	void load(NPT_InputStream *inputStream);
	void save(NPT_OutputStream *outputStream) const;

	void importFileSystem(const NPT_String& dir, const NPT_String& name, bool ignoreDot);
	void importIOSPhotos(const NPT_String& name);
	int browse(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& objectID, const NPT_String& browseFlag, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID);
	int search(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& containerID, const NPT_String& searchCriteria, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID);
	bool findFileDetail(const NPT_String& objectId, MediaStore::FileDetail& detail);

	void onImportResult(const NPT_List<MSImportResult>& ls);

	bool buildDidlForFile(const NPT_String& path, const NPT_String& urlFormatStr, NPT_String& mimeType, NPT_String& metaData);

	NPT_Result importPhotos(const NPT_String& name, DLNACoreOp **op);

private:
	void clearLocked();
	void buildRootLocked();
	void buildCategoryForNewItemsLocked(const NPT_List<MSBaseObject*>& newItems);
	void indexObjectLocked(MSBaseObject *obj);
	void buildCategoryLocked(MSFolder *dirMusicParent, MSFolder *dirVideoParent, MSFolder *dirPictParent, MSFolder *dir);
	void buildCategoryForFileItemLocked(MSContainer *dirMusicParent, MSContainer *dirVideoParent, MSContainer *dirPictParent, MSFileItem *fileItem);
	MSBaseObject *findObjectLocked(const NPT_String& objectId) const;
	NPT_String formatDidlLocked(const NPT_List<MSBaseObject*>& ls, const NPT_String& urlFormatStr, const MSContainer *topLevelFolder);
	void addLink(MSContainer *container, MSFileItem *target);
	void searchLockedR(MSContainer *container, sc::Expr *matchExpr, NPT_List<MSBaseObject*>& ls);

	class NotifyTask
		: public Task
	{
	public:
		NotifyTask(MediaStoreImpl *owner);

	protected:
		virtual void exec();

	private:
		MediaStoreImpl *m_owner;
	};

private:
	TaskGroup *m_taskGroup;
	ReadWriteLock m_stateLock;
	MSContainer *m_root;
	MSContainer *m_dirStorage;
	MSContainer *m_dirMusicAll;
	MSContainer *m_dirMusicStorage;
	MSContainer *m_dirMusicAlbum;
	MSContainer *m_dirMusicAlbumUnknown;
	MSContainer *m_dirMusicArtist;
	MSContainer *m_dirMusicArtistUnknown;
	MSContainer *m_dirMusicGenre;
	MSContainer *m_dirMusicGenreUnknown;
	MSContainer *m_dirMusicPlaylists;
	MSContainer *m_dirVideoAll;
	MSContainer *m_dirVideoStorage;
	MSContainer *m_dirPicAll;
	MSContainer *m_dirPicStorage;

	NPT_Map<NPT_String, MSBaseObject*> m_objMap;

	MediaStore::Callback *m_callback;
	MediaStore *m_intf;
	NPT_UInt32 m_systemUpdateId;
};

class MSImportTask
	: public Task
{
public:
	struct ImportContext
	{
		NPT_String folderName;
		NPT_String rootPath;
		MSFolder *m_folder;

		ImportContext()
			: m_folder(NULL)
		{
		}

		~ImportContext()
		{
			if (m_folder) {
				delete m_folder;
			}
		}
	};

	MSImportTask(MediaStoreImpl *owner, const NPT_List<NPT_String>& dirs, const NPT_List<NPT_String>& names, bool ignoreDot);

protected:
	virtual void exec();
	virtual void doAbort();

private:
	void import(ImportContext *impCtx, MSFolder *parentFolder, const NPT_String& dir, const NPT_FileInfo& dirInfo);
	void importFile(ImportContext *impCtx, MSFolder *parentFolder, const NPT_String& path, const NPT_FileInfo& fileInfo, const NPT_String& dir);

private:
	MediaStoreImpl *m_owner;
	NPT_List<NPT_String> m_dirs;
	NPT_List<NPT_String> m_names;
	bool m_ignoreDot;
};

class IOSAssetLibraryImportTask
	: public Task
{
public:
	IOSAssetLibraryImportTask(MediaStoreImpl *owner);

protected:
	virtual void exec();
	virtual void doAbort();

private:
	MediaStoreImpl *m_owner;
};

class DLNAImportPhotosTask;

class DLNAImportPhotosOp
	: public DLNACoreOp
{
public:
	DLNAImportPhotosOp();
	virtual ~DLNAImportPhotosOp();

	virtual int addRef();
	virtual int release();
	virtual void abort();
	virtual bool aborted() const;
	virtual NPT_Result wait(NPT_Timeout timeout);
	virtual bool checkFinishedIfNotSetCallback(FinishCallback *callback);
	virtual bool resetCallback();
	virtual bool succeeded() const;

	void setBuddy(DLNAImportPhotosTask *buddy);
	void notifyFinished();

	ReadWriteLock m_stateLock;
	NPT_AtomicVariable m_refCount;
	NPT_SharedVariable m_waitVar;
	bool m_abortFlag;
	bool m_finished;
	FinishCallback *m_finishCallback;
	bool m_succeeded;
	DLNAImportPhotosTask *m_buddy;
};

class DLNAImportPhotosTask
	: public Task
{
public:
	DLNAImportPhotosTask(DLNAImportPhotosOp *op, MediaStoreImpl *store);
	virtual ~DLNAImportPhotosTask();

	virtual void exec();

protected:
	DLNAImportPhotosOp *m_op;
	ReadWriteLock m_stateLock;
	MediaStoreImpl *m_store;
};

} // namespace deejay

#endif // __DJMediaStoreImpl_h__
