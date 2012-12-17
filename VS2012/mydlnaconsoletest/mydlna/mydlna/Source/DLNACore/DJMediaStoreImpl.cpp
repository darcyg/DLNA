#include "DJMediaStoreImpl.h"

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("deejay.MediaStore")

namespace deejay {

struct MSMimeInfo
{
	const char *ext;
	const char *mimeType;
	MSMediaType mediaType;
	const char *extHint;
};

static const MSMimeInfo g_mimeInfo[] = {
	{ ".wmv",  "video/x-ms-wmv",  MSMediaType_Video, ".wmv" },
	{ ".avi",  "video/avi",       MSMediaType_Video, ".avi" },
	{ ".mpg",  "video/mpeg",      MSMediaType_Video, ".mpg" },
	{ ".mp4",  "video/mp4",       MSMediaType_Video, ".mp4" },
	{ ".m4v",  "video/x-m4v",     MSMediaType_Video, ".m4v" },
	{ ".mkv",  "video/x-matroska",MSMediaType_Video, ".mkv" },
	{ ".mov",  "video/quicktime", MSMediaType_Video, ".mov" },
	{ ".3gp",  "video/3gpp",      MSMediaType_Video, ".3gp" },
	{ ".wma",  "audio/x-ms-wma",  MSMediaType_Audio, ".wma" },
	{ ".mp3",  "audio/mpeg",      MSMediaType_Audio, ".mp3" },
	{ ".m4a",  "audio/x-m4a",     MSMediaType_Audio, ".m4a" },
	{ ".jpg",  "image/jpeg",      MSMediaType_Image, ".jpg" },
	{ ".jpe",  "image/jpeg",      MSMediaType_Image, ".jpg" },
	{ ".jpeg", "image/jpeg",      MSMediaType_Image, ".jpg" },
	{ ".png",  "image/png",       MSMediaType_Image, ".png" },
	{ ".bmp",  "image/bmp",       MSMediaType_Image, ".bmp" },
	{ ".gif",  "image/gif",       MSMediaType_Image, ".gif" },
	{ ".tif",  "image/tiff",      MSMediaType_Image, ".tif" },
	{ ".tiff", "image/tiff",      MSMediaType_Image, ".tif" },
};

NPT_Map<NPT_String, const MSMimeInfo*> g_mimeIndexByExt;

static NPT_UInt64 g_seq = 1000;
NPT_Mutex g_seqMutex;

static NPT_String generateObjectId()
{
	static const char ab[] = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const NPT_UInt32 ac = sizeof(ab) - 1;
	NPT_UInt64 seq;
	g_seqMutex.Lock();
	seq = g_seq++;
	g_seqMutex.Unlock();
	NPT_UInt64 q;
	NPT_UInt64 r;

	NPT_String result("M");

	for (;;) {
		q = seq / ac;
		r = seq - (q * ac);
		result.Append(ab + r, 1);
		if (q == 0) {
			break;
		}
		seq = q;
	}
	return result;
}

MSBaseObject::MSBaseObject()
	: m_objectId(generateObjectId()), m_parent(NULL)
{
}

MSBaseObject::MSBaseObject(const NPT_String& objectId)
	: m_objectId(objectId), m_parent(NULL)
{
}

MSBaseObject::~MSBaseObject()
{
}

bool MSBaseObject::hasProperty(const NPT_String& prop) const
{
	if (prop.Compare("@refID") == 0) {
		return false;
	}
	return false;
}

bool MSBaseObject::testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const
{
	return false;
}

bool MSBaseObject::eval(const NPT_String& v, const NPT_String& t, Operator op)
{
	switch (op) {
	case Op_Contains:
		return v.Find(t, 0, true) >= 0;
	case Op_DoesNotContain:
		return v.Find(t, 0, true) < 0;
	case Op_DerivedFrom:
		{
			if (v.Compare(t, true) == 0) {
				return true;
			}
			NPT_String t2 = t + ".";
			if (v.StartsWith(t2, true)) {
				return true;
			}
			return false;
		}
	case Op_EqualTo:
	case Op_NotEqualTo:
	case Op_LessThan:
	case Op_LessEqual:
	case Op_GreatThan:
	case Op_GreatEqual:
		return evalNum(v, t, op);
	default:
		return false;
	}
}

static bool parseInteger(const NPT_String& s, NPT_Int64& v)
{
	NPT_String s1 = s;
	s1.Trim();
	NPT_Cardinal chars_used;
	if (NPT_SUCCEEDED(NPT_ParseInteger64(s1, v, false, &chars_used)) && chars_used == s1.GetLength()) {
		return true;
	}
	return false;
}

bool MSBaseObject::evalNum(const NPT_String& v, const NPT_String& t, Operator op)
{
	NPT_Int64 n1, n2;
	if (parseInteger(v, n1) && parseInteger(v, n2)) {
		switch (op) {
		case Op_EqualTo:
			return n1 == n2;
		case Op_NotEqualTo:
			return n1 != n2;
		case Op_LessThan:
			return n1 < n2;
		case Op_LessEqual:
			return n1 <= n2;
		case Op_GreatThan:
			return n1 > n2;
		case Op_GreatEqual:
			return n1 >= n2;
		default:
			return false;
		}
	}

	int cc = v.Compare(t, true);
	switch (op) {
	case Op_EqualTo:
		return cc == 0;
	case Op_NotEqualTo:
		return cc != 0;
	case Op_LessThan:
		return cc < 0;
	case Op_LessEqual:
		return cc <= 0;
	case Op_GreatThan:
		return cc > 0;
	case Op_GreatEqual:
		return cc >= 0;
	default:
		return false;
	}
}

MSLink::MSLink()
	: m_target(NULL)
{
}

MSLink::~MSLink()
{
}

bool MSLink::hasProperty(const NPT_String& prop) const
{
	if (prop.Compare("@refID") == 0) {
		return true;
	}
	return m_target->hasProperty(prop);
}

bool MSLink::testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const
{
	return m_target->testProperty(prop, value, op);
}

MSObject::MSObject()
{
}

MSObject::MSObject(const NPT_String& objectId)
	: MSBaseObject(objectId)
{
}

MSObject::~MSObject()
{
}

bool MSObject::hasProperty(const NPT_String& prop) const
{
	return MSBaseObject::hasProperty(prop);
}

bool MSObject::testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const
{
	bool singleProp = false;
	NPT_String propValue;
	if (prop.Compare("upnp:class") == 0) {
		propValue = m_upnpClass;
		singleProp = true;
	} else if (prop.Compare("dc:creator") == 0) {
		propValue = m_creator;
		singleProp = true;
	} else if (prop.Compare("dc:title") == 0) {
		propValue = m_title;
		singleProp = true;
	} else if (prop.Compare("dc:date") == 0) {
		// TODO:
	} else if (prop.Compare("upnp:artist") == 0) {
		if (op == Op_EqualTo) {
			NPT_String ss(value);
			return m_artistList.Find(NPT_ObjectComparator<NPT_String>(ss));
		}
		return false;
	} else if (prop.Compare("upnp:genre") == 0) {
		if (op == Op_EqualTo) {
			NPT_String ss(value);
			return m_genreList.Find(NPT_ObjectComparator<NPT_String>(ss));
		}
		return false;
	} else if (prop.Compare("upnp:album") == 0) {
		if (op == Op_EqualTo) {
			NPT_String ss(value);
			return m_albumList.Find(NPT_ObjectComparator<NPT_String>(ss));
		}
		return false;
	}

	if (singleProp) {
		return eval(propValue, value, op);
	}

	return MSBaseObject::testProperty(prop, value, op);
}

MSContainer::MSContainer()
{
	m_upnpClass = "object.container";
}

MSContainer::MSContainer(const NPT_String& objectId)
	: MSObject(objectId)
{
	m_upnpClass = "object.container";
}

MSContainer::~MSContainer()
{
	NPT_List<MSBaseObject*> ls = m_children;
	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		MSBaseObject *obj = *ls.GetItem(i);
		delete obj;
	}
	m_children.Clear();
	m_searchClassList.Apply(NPT_ObjectDeleter<MSClassSpec>());
}

bool MSContainer::hasProperty(const NPT_String& prop) const
{
	if (prop.Compare("@childCount") == 0) {
		return true;
	}
	return MSObject::hasProperty(prop);
}

bool MSContainer::testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const
{
	if (prop.Compare("@childCount") == 0) {
		// TODO:
	}
	return MSObject::testProperty(prop, value, op);
}

void MSContainer::addChild(MSBaseObject *child)
{
	NPT_List<MSBaseObject*>::Iterator it = m_children.Find(NPT_ObjectComparator<MSBaseObject*>(child));
	if (!it) {
		m_children.Add(child);
		child->m_parent = this;
	}
}

MSBaseObject *MSContainer::childAt(NPT_Ordinal index) const
{
	NPT_List<MSBaseObject*>::Iterator it = m_children.GetItem(index);
	if (it) {
		return *it;
	}
	return NULL;
}

MSBaseObject *MSContainer::takeChildAt(NPT_Ordinal index)
{
	MSBaseObject *child = NULL;
	NPT_List<MSBaseObject*>::Iterator it = m_children.GetItem(index);
	if (it) {
		child = *it;
		child->m_parent = NULL;
		m_children.Erase(it);
	}
	return child;
}

NPT_Cardinal MSContainer::childCount() const
{
	return m_children.GetItemCount();
}

void MSContainer::addSearchClass(const NPT_String& text, bool includeDerived, const NPT_String& name)
{
	m_searchClassList.Add(new MSClassSpec(text, name, includeDerived));
}

const MSClassSpec *MSContainer::searchClassAt(NPT_Ordinal index) const
{
	return *m_searchClassList.GetItem(index);
}

NPT_Cardinal MSContainer::searchClassCount() const
{
	return m_searchClassList.GetItemCount();
}

MSFileItem::MSFileItem()
	: m_isIOSAssetItem(false)
{
}

MSFileItem::~MSFileItem()
{
}

bool MSFileItem::hasProperty(const NPT_String& prop) const
{
	return MSObject::hasProperty(prop);
}

bool MSFileItem::testProperty(const NPT_String& prop, const NPT_String& value, Operator op) const
{
	return MSObject::testProperty(prop, value, op);
}

IOSAssetItem::IOSAssetItem()
{
	m_isIOSAssetItem = true;
}

IOSAssetItem::~IOSAssetItem()
{
}

MSFolder::MSFolder()
{
	m_upnpClass = "object.container.storageFolder";
}

MSFolder::~MSFolder()
{
}

namespace sc {

Expr::Expr()
{
}

Expr::~Expr()
{
}

LogicalExpr::LogicalExpr(Expr *expr1, Expr *expr2)
	: m_expr1(expr1), m_expr2(expr2)
{
}

LogicalExpr::~LogicalExpr()
{
	delete m_expr1;
	delete m_expr2;
}

bool LogicalExpr::eval(MSBaseObject *obj)
{
	return eval(obj, m_expr1, m_expr2);
}

AndExpr::AndExpr(Expr *expr1, Expr *expr2)
	: LogicalExpr(expr1, expr2)
{
}

bool AndExpr::eval(MSBaseObject *obj, Expr *expr1, Expr *expr2)
{
	return expr1->eval(obj) && expr2->eval(obj);
}

OrExpr::OrExpr(Expr *expr1, Expr *expr2)
	: LogicalExpr(expr1, expr2)
{
}

bool OrExpr::eval(MSBaseObject *obj, Expr *expr1, Expr *expr2)
{
	return expr1->eval(obj) || expr2->eval(obj);
}

ExistsExpr::ExistsExpr(const NPT_String& prop, bool exists)
	: m_prop(prop), m_exists(exists)
{
}

bool ExistsExpr::eval(MSBaseObject *obj)
{
	return obj->hasProperty(m_prop) == m_exists;
}

PropExpr::PropExpr(const NPT_String& prop, const NPT_String& value, MSBaseObject::Operator op)
	: m_prop(prop), m_value(value), m_op(op)
{
}

bool PropExpr::eval(MSBaseObject *obj)
{
	return obj->testProperty(m_prop, m_value, m_op);
}

} // namespace sc

enum SCPState
{
	SCPState_NeedExpr,
	SCPState_NeedOp,
};

struct SCPContext
{
	SCPState m_state;
	NPT_List<sc::Expr*> m_expList;
	NPT_List<SCTokenizerLevel2::Token> m_opList;

	~SCPContext();
	sc::Expr *compile();

	sc::Expr *nextExpr();
	SCTokenizerLevel2::Token nextOp();
	void insertExpr(sc::Expr *expr);
	void insertOp(SCTokenizerLevel2::Token op);
};

SCPContext::~SCPContext()
{
	m_expList.Apply(NPT_ObjectDeleter<sc::Expr>());
	m_expList.Clear();
}

sc::Expr *SCPContext::nextExpr()
{
	NPT_List<sc::Expr*>::Iterator it = m_expList.GetFirstItem();
	sc::Expr *expr = *it;
	m_expList.Erase(it);
	return expr;
}

SCTokenizerLevel2::Token SCPContext::nextOp()
{
	NPT_List<SCTokenizerLevel2::Token>::Iterator it = m_opList.GetFirstItem();
	SCTokenizerLevel2::Token token = *it;
	m_opList.Erase(it);
	return token;
}

void SCPContext::insertExpr(sc::Expr *expr)
{
	m_expList.Insert(m_expList.GetFirstItem(), expr);
}

void SCPContext::insertOp(SCTokenizerLevel2::Token op)
{
	m_opList.Insert(m_opList.GetFirstItem(), op);
}

sc::Expr *SCPContext::compile()
{
	sc::Expr *result = NULL;
	if (m_expList.GetItemCount() == m_opList.GetItemCount() + 1) {
		for (;;) {
			NPT_Cardinal numExp = m_expList.GetItemCount();
			if (numExp == 1) {
				result = nextExpr();
				break;
			} else if (numExp == 2) {
				sc::Expr *expr1 = nextExpr();
				sc::Expr *expr2 = nextExpr();
				SCTokenizerLevel2::Token op = nextOp();
				if (op == SCTokenizerLevel2::Token_AND) {
					result = new sc::AndExpr(expr1, expr2);
				} else {
					result = new sc::OrExpr(expr1, expr2);
				}
				break;
			} else {
				sc::Expr *expr1 = nextExpr();
				sc::Expr *expr2 = nextExpr();
				SCTokenizerLevel2::Token op1 = nextOp();
				if (op1 == SCTokenizerLevel2::Token_AND) {
					insertExpr(new sc::AndExpr(expr1, expr2));
				} else {
					SCTokenizerLevel2::Token op2 = nextOp();
					if (op2 == SCTokenizerLevel2::Token_AND) {
						sc::Expr *expr3 = nextExpr();
						insertExpr(new sc::AndExpr(expr2, expr3));
						insertExpr(expr1);
						insertOp(op1);
					} else {
						insertExpr(new sc::OrExpr(expr1, expr2));
						insertOp(op2);
					}
				}
			}
		}
	}

	m_expList.Apply(NPT_ObjectDeleter<sc::Expr>());
	m_expList.Clear();
	m_opList.Clear();
	return result;
}

sc::Expr *compileSearchCriteria(const NPT_String& searchCriteria)
{
	SCTokenizerLevel2 tokenizer(searchCriteria);

	NPT_Stack<SCPContext*> stack;
	SCPContext *ctx = new SCPContext();
	ctx->m_state = SCPState_NeedExpr;
	stack.Push(ctx);

	sc::Expr *finalExpr = NULL;

	for (;;) {
		SCTokenizerLevel2::Expr expr;
		SCTokenizerLevel2::Token tk = tokenizer.nextToken(expr);
		NPT_LOG_FINEST_1("next token2 [%d]", tk);
		if (ctx->m_state == SCPState_NeedExpr) {
			if (tk == SCTokenizerLevel2::Token_LeftBracket) {
				NPT_LOG_FINEST("L2 Token_LeftBracket");
				ctx = new SCPContext();
				ctx->m_state = SCPState_NeedExpr;
				stack.Push(ctx);
			} else if (tk == SCTokenizerLevel2::Token_Expr) {
				sc::Expr *newExp;
				if (expr.op == MSBaseObject::Op_Exists) {
					NPT_LOG_FINEST("L2 Token_Expr (ExistsExpr)");
					newExp = new sc::ExistsExpr(expr.prop, expr.value.GetLength() > 0);
				} else {
					NPT_LOG_FINEST("L2 Token_Expr (PropExpr)");
					newExp = new sc::PropExpr(expr.prop, expr.value, expr.op);
				}
				ctx->m_expList.Add(newExp);
				ctx->m_state = SCPState_NeedOp;
			} else {
				NPT_LOG_FINEST("L2 Unexpected token pos1");
				break;
			}
		} else if (ctx->m_state == SCPState_NeedOp) {
			if (tk == SCTokenizerLevel2::Token_AND) {
				NPT_LOG_FINEST("L2 Token_AND");
				ctx->m_opList.Add(SCTokenizerLevel2::Token_AND);
				ctx->m_state = SCPState_NeedExpr;
			} else if (tk == SCTokenizerLevel2::Token_OR) {
				NPT_LOG_FINEST("L2 Token_OR");
				ctx->m_opList.Add(SCTokenizerLevel2::Token_OR);
				ctx->m_state = SCPState_NeedExpr;
			} else if (tk == SCTokenizerLevel2::Token_RightBracket) {
				NPT_LOG_FINEST("L2 Token_RightBracket");
				if (NPT_FAILED(stack.Pop(ctx))) {
					NPT_LOG_FINEST("L2 stack overflow!!!");
					break;
				}
				sc::Expr *subExpr = ctx->compile();
				delete ctx;
				if (!subExpr) {
					NPT_LOG_FINEST("L2 subExpr compile failed!!!");
					break;
				}
				if (NPT_FAILED(stack.Peek(ctx))) {
					NPT_LOG_FINEST("L2 stack overflow!!! 222");
					break;
				}
				ctx->m_expList.Add(subExpr);
				ctx->m_state = SCPState_NeedOp;
			} else if (tk == SCTokenizerLevel2::Token_EOS) {
				if (NPT_FAILED(stack.Pop(ctx))) {
					NPT_LOG_FINEST("L2 stack overflow!!! 333");
					break;
				}
				finalExpr = ctx->compile();
				delete ctx;
				break;
			} else {
				NPT_LOG_FINEST("L2 Unexpected token pos2");
				break;
			}
		}
	}

	while (NPT_SUCCEEDED(stack.Pop(ctx))) {
		delete ctx;
	}

	return finalExpr;
}

#ifdef DLNACORE_PLATFORM_IOS
void ios_photo_importer_init();
#endif // DLNACORE_PLATFORM_IOS

MediaStoreImpl::MediaStoreImpl(MediaStore::Callback *callback, MediaStore *intf)
	: m_root(NULL), m_callback(callback), m_intf(intf), m_systemUpdateId(0)
{
#ifdef DLNACORE_PLATFORM_IOS
	ios_photo_importer_init();
#endif // DLNACORE_PLATFORM_IOS

	if (g_mimeIndexByExt.GetEntryCount() == 0) {
		for (NPT_Ordinal i = 0; i < sizeof(g_mimeInfo) / sizeof(g_mimeInfo[0]); i++) {
			NPT_String ext(g_mimeInfo[i].ext);
			ext.MakeLowercase();
			g_mimeIndexByExt.Put(ext, g_mimeInfo + i);
		}
	}

	m_taskGroup = new TaskGroup();
	buildRootLocked();
/*
	const char *sc = "((upnp:class derivedfrom \"object.item.audioItem\" and upnp:class derivedfrom \"object.item.videoItem\" and @refID exists true) or upnp:class derivedfrom \"object.item.videoItem\" and @refID exists false or dc:title contains \"Christmas\")";
	SCTokenizerLevel2 tk(sc);

	for (;;) {
		SCTokenizerLevel2::Expr expr;
		SCTokenizerLevel2::Token token = tk.nextToken(expr);
		if (token == SCTokenizerLevel2::Token_EOS) {
			break;
		}

		if (token == SCTokenizerLevel2::Token_Error) {
			break;
		}
	}*/
}

MediaStoreImpl::~MediaStoreImpl()
{
	m_taskGroup->abort();
	m_taskGroup->wait();
	delete m_taskGroup;
	clearLocked();
}

void MediaStoreImpl::reset()
{
	WriteLocker locker(m_stateLock);
	clearLocked();
	buildRootLocked();
}

void MediaStoreImpl::buildRootLocked()
{
	m_root = new MSContainer("0");
	m_root->m_title = "Root";
	indexObjectLocked(m_root);

	m_dirStorage = new MSContainer("S2");
	m_dirStorage->m_title = "Browse Folders";
	m_dirStorage->m_upnpClass = "object.container.storageFolder";
	m_root->addChild(m_dirStorage);
	indexObjectLocked(m_dirStorage);

	MSContainer *dirMusic = new MSContainer("1");
	dirMusic->m_title = "Music";
	dirMusic->addSearchClass("object.item.audioItem", true);
	dirMusic->addSearchClass("object.container.playlistContainer", false);
	dirMusic->addSearchClass("object.container", false);
	dirMusic->addSearchClass("object.container.genre", true);
	dirMusic->addSearchClass("object.container.storageFolder", false);
	dirMusic->addSearchClass("object.container.genre.musicGenre", false);
	dirMusic->addSearchClass("object.item.audioItem.musicTrack", false);
	dirMusic->addSearchClass("object.container.album.musicAlbum", false);
	dirMusic->addSearchClass("object.container.album", true);
	dirMusic->addSearchClass("object.container.person.musicArtist", false);
	m_root->addChild(dirMusic);
	indexObjectLocked(dirMusic);

	m_dirMusicAlbum = new MSContainer("7");	
	m_dirMusicAlbum->m_title = "Album";
	dirMusic->addChild(m_dirMusicAlbum);
	indexObjectLocked(m_dirMusicAlbum);

	m_dirMusicAlbumUnknown = new MSContainer();
	m_dirMusicAlbumUnknown->m_title = "Unknown Album";
	m_dirMusicAlbumUnknown->m_upnpClass = "object.container.album.musicAlbum";
	m_dirMusicAlbumUnknown->m_artistList.Add("Unknown Artist");
	m_dirMusicAlbumUnknown->m_genreList.Add("Unknown Genre");
	m_dirMusicAlbumUnknown->m_albumList.Add("Unknown Album");
	m_dirMusicAlbum->addChild(m_dirMusicAlbumUnknown);
	indexObjectLocked(m_dirMusicAlbumUnknown);

	m_dirMusicArtist = new MSContainer("6");	
	m_dirMusicArtist->m_title = "Artist";
	dirMusic->addChild(m_dirMusicArtist);
	indexObjectLocked(m_dirMusicArtist);

	m_dirMusicArtistUnknown = new MSContainer();
	m_dirMusicArtistUnknown->m_title = "Unknown Artist";
	m_dirMusicArtistUnknown->m_upnpClass = "object.container.person.musicArtist";
	m_dirMusicArtistUnknown->m_artistList.Add("Unknown Artist");
	m_dirMusicArtistUnknown->m_genreList.Add("Unknown Genre");
	m_dirMusicArtistUnknown->m_albumList.Add("Unknown Album");
	m_dirMusicArtist->addChild(m_dirMusicArtistUnknown);
	indexObjectLocked(m_dirMusicArtistUnknown);

	m_dirMusicGenre = new MSContainer("5");	
	m_dirMusicGenre->m_title = "Genre";
	dirMusic->addChild(m_dirMusicGenre);
	indexObjectLocked(m_dirMusicGenre);

	m_dirMusicGenreUnknown = new MSContainer();
	m_dirMusicGenreUnknown->m_title = "Unknown Genre";
	m_dirMusicGenreUnknown->m_upnpClass = "object.container.genre.musicGenre";
	m_dirMusicGenreUnknown->m_artistList.Add("Unknown Artist");
	m_dirMusicGenreUnknown->m_genreList.Add("Unknown Genre");
	m_dirMusicGenreUnknown->m_albumList.Add("Unknown Album");
	m_dirMusicGenre->addChild(m_dirMusicGenreUnknown);
	indexObjectLocked(m_dirMusicGenreUnknown);

	m_dirMusicPlaylists = new MSContainer("F");	
	m_dirMusicPlaylists->m_title = "Playlists";
	dirMusic->addChild(m_dirMusicPlaylists);
	indexObjectLocked(m_dirMusicPlaylists);

	m_dirMusicAll = new MSContainer("4");
	m_dirMusicAll->m_title = "All Music";
	dirMusic->addChild(m_dirMusicAll);
	indexObjectLocked(m_dirMusicAll);

	m_dirMusicStorage = new MSContainer("14");
	m_dirMusicStorage->m_title = "Browse Folders";
	m_dirMusicStorage->m_upnpClass = "object.container.storageFolder";
	dirMusic->addChild(m_dirMusicStorage);
	indexObjectLocked(m_dirMusicStorage);

	MSContainer *dirVideo = new MSContainer("2");
	dirVideo->m_title = "Video";
	dirVideo->addSearchClass("object.container.playlistContainer", false);
	dirVideo->addSearchClass("object.container.person.movieActor", false);
	dirVideo->addSearchClass("object.container", false);
	dirVideo->addSearchClass("object.item.videoItem", true);
	dirVideo->addSearchClass("object.container.genre", true);
	dirVideo->addSearchClass("object.container.storageFolder", false);
	dirVideo->addSearchClass("object.item.videoItem.videoBroadcast", false);
	dirVideo->addSearchClass("object.container.album.videoAlbum", false);
	dirVideo->addSearchClass("object.container.genre.movieGenre", false);
	dirVideo->addSearchClass("object.container.album", true);
	m_root->addChild(dirVideo);
	indexObjectLocked(dirVideo);

	m_dirVideoAll = new MSContainer("8");
	m_dirVideoAll->m_title = "All Video";
	dirVideo->addChild(m_dirVideoAll);
	indexObjectLocked(m_dirVideoAll);

	m_dirVideoStorage = new MSContainer("15");
	m_dirVideoStorage->m_title = "Browse Folders";
	m_dirVideoStorage->m_upnpClass = "object.container.storageFolder";
	dirVideo->addChild(m_dirVideoStorage);
	indexObjectLocked(m_dirVideoStorage);

	MSContainer *dirPictures = new MSContainer("3");
	dirPictures->m_title = "Pictures";
	dirPictures->addSearchClass("object.container.playlistContainer", false);
	dirPictures->addSearchClass("object.item.imageItem.photo", false);
	dirPictures->addSearchClass("object.container.album.photoAlbum", false);
	dirPictures->addSearchClass("object.container", false);
	dirPictures->addSearchClass("object.container.storageFolder", false);
	dirPictures->addSearchClass("object.container.album.photoAlbum.dateTaken", false);
	dirPictures->addSearchClass("object.container.album", true);
	dirPictures->addSearchClass("object.item.imageItem", true);
	m_root->addChild(dirPictures);
	indexObjectLocked(dirPictures);

	m_dirPicAll = new MSContainer("B");
	m_dirPicAll->m_title = "All Pictures";
	dirPictures->addChild(m_dirPicAll);
	indexObjectLocked(m_dirPicAll);

	m_dirPicStorage = new MSContainer("16");
	m_dirPicStorage->m_title = "Browse Folders";
	m_dirPicStorage->m_upnpClass = "object.container.storageFolder";
	dirPictures->addChild(m_dirPicStorage);
	indexObjectLocked(m_dirPicStorage);
}

void MediaStoreImpl::clearLocked()
{
	if (m_root) {
		delete m_root;
		m_root = NULL;
	}
	m_objMap.Clear();
}

void MediaStoreImpl::load(NPT_InputStream *inputStream)
{
}

void MediaStoreImpl::save(NPT_OutputStream *outputStream) const
{
}

NPT_UInt32 MediaStoreImpl::systemUpdateId() const
{
	ReadLocker locker(m_stateLock);
	return m_systemUpdateId;
}

void MediaStoreImpl::setSystemUpdateId(NPT_UInt32 value)
{
	WriteLocker locker(m_stateLock);
	m_systemUpdateId = value;
}

void MediaStoreImpl::importFileSystem(const NPT_String& dir, const NPT_String& name, bool ignoreDot)
{
	NPT_List<NPT_String> dirs;
	dirs.Add(dir);
	NPT_List<NPT_String> names;
	names.Add(name);
	m_taskGroup->startTask(new MSImportTask(this, dirs, names, ignoreDot));
}

void MediaStoreImpl::importIOSPhotos(const NPT_String& name)
{
	m_taskGroup->startTask(new IOSAssetLibraryImportTask(this));
}

void MediaStoreImpl::onImportResult(const NPT_List<MSImportResult>& ls)
{
	WriteLocker locker(m_stateLock);
	NPT_List<MSBaseObject*> newItems;
	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		NPT_List<MSImportResult>::Iterator it = ls.GetItem(i);
		MSFolder *subFolder = it->m_folder;
		if (subFolder->childCount() > 0) {
			if (subFolder->m_title.GetLength() > 0) {
				m_dirStorage->addChild(subFolder);
				newItems.Add(subFolder);
			} else {
				while (subFolder->childCount() > 0) {
					MSBaseObject *child = subFolder->takeChildAt(0);
					m_dirStorage->addChild(child);
					newItems.Add(child);
				}
				delete subFolder;
			}
		} else {
			delete subFolder;
		}
	}

	buildCategoryForNewItemsLocked(newItems);

	if (m_systemUpdateId == 0xFFFFFFFF) {
		m_systemUpdateId = 0;
	} else {
		++m_systemUpdateId;
	}
	m_taskGroup->startTask(new NotifyTask(this));
}

void MediaStoreImpl::indexObjectLocked(MSBaseObject *obj)
{
	MSBaseObject **dummy;
	if (NPT_SUCCEEDED(m_objMap.Get(obj->m_objectId, dummy))) {
		// TODO:
		NPT_LOG_SEVERE_1("duplicated objectId [%s] !!!", obj->m_objectId.GetChars());
	}
	m_objMap.Put(obj->m_objectId, obj);
}

MSBaseObject *MediaStoreImpl::findObjectLocked(const NPT_String& objectId) const
{
	MSBaseObject **obj;
	if (NPT_SUCCEEDED(m_objMap.Get(objectId, obj))) {
		return *obj;
	}
	return NULL;
}

void MediaStoreImpl::buildCategoryForNewItemsLocked(const NPT_List<MSBaseObject*>& newItems)
{
	for (NPT_Ordinal i = 0; i < newItems.GetItemCount(); i++) {
		MSBaseObject *baseObj = *newItems.GetItem(i);
		if (MSFileItem *fileItem = baseObj->asFileItem()) {
			indexObjectLocked(fileItem);
			buildCategoryForFileItemLocked(m_dirMusicStorage, m_dirVideoStorage, m_dirPicStorage, fileItem);
		} else if (MSFolder *folder = baseObj->asFolder()) {
			indexObjectLocked(folder);
			MSFolder *dirMusic = new MSFolder();
			dirMusic->m_title = folder->m_title;

			MSFolder *dirVideo = new MSFolder();
			dirVideo->m_title = folder->m_title;

			MSFolder *dirPict = new MSFolder();
			dirPict->m_title = folder->m_title;

			buildCategoryLocked(dirMusic, dirVideo, dirPict, folder);

			if (dirMusic->childCount() > 0) {
				m_dirMusicStorage->addChild(dirMusic);
				indexObjectLocked(dirMusic);
			} else {
				delete dirMusic;
			}

			if (dirVideo->childCount() > 0) {
				m_dirVideoStorage->addChild(dirVideo);
				indexObjectLocked(dirVideo);
			} else {
				delete dirVideo;
			}

			if (dirPict->childCount() > 0) {
				m_dirPicStorage->addChild(dirPict);
				indexObjectLocked(dirPict);
			} else {
				delete dirPict;
			}
		}
	}
}

void MediaStoreImpl::buildCategoryLocked(MSFolder *dirMusicParent, MSFolder *dirVideoParent, MSFolder *dirPictParent, MSFolder *dir)
{
	for (NPT_Ordinal i = 0; i < dir->childCount(); i++) {
		MSBaseObject *baseObj = dir->childAt(i);
		if (MSFileItem *fileItem = baseObj->asFileItem()) {
			indexObjectLocked(fileItem);
			buildCategoryForFileItemLocked(dirMusicParent, dirVideoParent, dirPictParent, fileItem);
		} else if (MSFolder *folder = baseObj->asFolder()) {
			indexObjectLocked(folder);
			MSFolder *dirMusic = new MSFolder();
			dirMusic->m_title = folder->m_title;

			MSFolder *dirVideo = new MSFolder();
			dirVideo->m_title = folder->m_title;

			MSFolder *dirPict = new MSFolder();
			dirPict->m_title = folder->m_title;

			buildCategoryLocked(dirMusic, dirVideo, dirPict, folder);

			if (dirMusic->childCount() > 0) {
				dirMusicParent->addChild(dirMusic);
				indexObjectLocked(dirMusic);
			} else {
				delete dirMusic;
			}

			if (dirVideo->childCount() > 0) {
				dirVideoParent->addChild(dirVideo);
				indexObjectLocked(dirVideo);
			} else {
				delete dirVideo;
			}

			if (dirPict->childCount() > 0) {
				dirPictParent->addChild(dirPict);
				indexObjectLocked(dirPict);
			} else {
				delete dirPict;
			}
		}
	}
}

void MediaStoreImpl::buildCategoryForFileItemLocked(MSContainer *dirMusicParent, MSContainer *dirVideoParent, MSContainer *dirPictParent, MSFileItem *fileItem)
{
	switch (fileItem->m_mediaType) {
	case MSMediaType_Video:
		addLink(dirVideoParent, fileItem);
		addLink(m_dirVideoAll, fileItem);
		break;
	case MSMediaType_Audio:
		addLink(dirMusicParent, fileItem);
		addLink(m_dirMusicAll, fileItem);
		addLink(m_dirMusicAlbumUnknown, fileItem);
		addLink(m_dirMusicArtistUnknown, fileItem);
		addLink(m_dirMusicGenreUnknown, fileItem);
		break;
	case MSMediaType_Image:
		addLink(dirPictParent, fileItem);
		addLink(m_dirPicAll, fileItem);
		break;
	}
}

void MediaStoreImpl::addLink(MSContainer *container, MSFileItem *target)
{
	MSLink *link = new MSLink();
	link->m_target = target;
	container->addChild(link);
	indexObjectLocked(link);
}

// upnp:class
// dc:date
// dc:title
// upnp:originalTrackNumber

MSBasePropComparator::MSBasePropComparator(bool ascending)
	: m_ascending(ascending)
{
}

MSBasePropComparator::~MSBasePropComparator()
{
}

int MSBasePropComparator::compareN(const MSObject *obj1, const MSObject *obj2) const
{
	int r = compare(obj1, obj2);
	if (!m_ascending) {
		r *= -1;
	}
	return r;
}

MSUPnPClassComparator::MSUPnPClassComparator(bool ascending)
	: MSBasePropComparator(ascending)
{
}

int MSUPnPClassComparator::compare(const MSObject *obj1, const MSObject *obj2) const
{
	return NPT_String::Compare(obj1->m_upnpClass, obj2->m_upnpClass, true);
}

MSDateComparator::MSDateComparator(bool ascending)
	: MSBasePropComparator(ascending)
{
}

int MSDateComparator::compare(const MSObject *obj1, const MSObject *obj2) const
{
	if (obj1->m_date < obj2->m_date) {
		return -1;
	}

	if (obj1->m_date == obj2->m_date) {
		return 0;
	}

	return 1;
}

MSTitleComparator::MSTitleComparator(bool ascending)
	: MSBasePropComparator(ascending)
{
}

int MSTitleComparator::compare(const MSObject *obj1, const MSObject *obj2) const
{
	NPT_Int64 n1, n2;
	if (parseInteger(obj1->m_title, n1) && parseInteger(obj2->m_title, n2)) {
		if (n1 < n2) {
			return -1;
		}
		
		if (n1 == n2) {
			return 0;
		}

		return 1;
	}
	return NPT_String::Compare(obj1->m_title, obj2->m_title, true);
}

class MSSortFunc
{
public:
	MSSortFunc(const NPT_List<MSBasePropComparator*>& ls)
		: m_ls(ls)
	{
	}

	int operator()(const MSBaseObject *obj1, const MSBaseObject *obj2) const
	{
		const MSObject *realObj1;
		if (const MSLink *link1 = obj1->asLink()) {
			realObj1 = link1->m_target;
		} else {
			realObj1 = obj1->asObject();
		}

		const MSObject *realObj2;
		if (const MSLink *link2 = obj2->asLink()) {
			realObj2 = link2->m_target;
		} else {
			realObj2 = obj2->asObject();
		}

		int r = 0;
		for (NPT_Ordinal i = 0; i < m_ls.GetItemCount(); i++) {
			MSBasePropComparator *cmp = *m_ls.GetItem(i);
			r = cmp->compareN(realObj1, realObj2);
			if (r != 0) {
				break;
			}
		}
		return r;
	}

private:
	NPT_List<MSBasePropComparator*> m_ls;
};
/*
class TestSortFunc
{
public:
	int operator()(const MSBaseObject *obj1, const MSBaseObject *obj2) const
	{
		const MSObject *realObj1;
		if (const MSLink *link1 = obj1->asLink()) {
			realObj1 = link1->m_target;
		} else {
			realObj1 = obj1->asObject();
		}

		const MSObject *realObj2;
		if (const MSLink *link2 = obj2->asLink()) {
			realObj2 = link2->m_target;
		} else {
			realObj2 = obj2->asObject();
		}

		int c1 = NPT_String::Compare(realObj1->m_upnpClass.GetChars(), realObj2->m_upnpClass.GetChars());
		if (c1 == 0) {
			int c2 = NPT_String::Compare(realObj1->m_title.GetChars(), realObj2->m_title.GetChars());
			return c2 * -1;
		}
		return c1 * -1;
	}
};
*/
static void sortResult(NPT_List<MSBaseObject*>& ls, const NPT_String& sortCriteria)
{
	NPT_List<MSBasePropComparator*> cmpList;

	NPT_List<NPT_String> se = sortCriteria.Split(",");
	for (NPT_Ordinal i = 0; i < se.GetItemCount(); i++) {
		NPT_String sortCmd = *se.GetItem(i);
		sortCmd.Trim();
		if (!sortCmd.IsEmpty()) {
			bool ascending;
			switch (sortCmd[0]) {
			case '+':
				ascending = true;
				break;
			case '-':
				ascending = false;
				break;
			default:
				continue;
			}
			if (NPT_String::Compare(sortCmd.GetChars() + 1, "upnp:class") == 0) {
				cmpList.Add(new MSUPnPClassComparator(ascending));
			} else if (NPT_String::Compare(sortCmd.GetChars() + 1, "dc:title") == 0) {
				cmpList.Add(new MSTitleComparator(ascending));
			} else if (NPT_String::Compare(sortCmd.GetChars() + 1, "dc:date") == 0) {
				cmpList.Add(new MSDateComparator(ascending));
			}
		}
	}

	if (cmpList.GetItemCount() > 0) {
		ls.Sort(MSSortFunc(cmpList));
		cmpList.Apply(NPT_ObjectDeleter<MSBasePropComparator>());
	}
}

int MediaStoreImpl::browse(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& objectID, const NPT_String& browseFlag, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID)
{
	ReadLocker locker(m_stateLock);

	MSBaseObject *obj = findObjectLocked(objectID);
	if (!obj) {
		// No such object
		NPT_LOG_WARNING_1("Object [%s] not found!", objectID.GetChars());
		return 701;
	}

	if (browseFlag.Compare("BrowseMetadata") == 0) {
		// TODO:
		updateID = m_systemUpdateId;

		totalMatches = 1;
		numberReturned = 1;
		NPT_List<MSBaseObject*> ls;
		ls.Add(obj);
		result = formatDidlLocked(ls, urlFormatStr, NULL);
		return 0;
	}

	if (browseFlag.Compare("BrowseDirectChildren") == 0) {
		NPT_List<MSBaseObject*> ls;
		MSContainer *container = obj->asContainer();
		totalMatches = 0;
		if (container) {
			for (NPT_Ordinal i = 0; i < container->childCount(); i++) {
				MSBaseObject *childObj = container->childAt(i);
				{
					if (totalMatches >= startingIndex && ((requestedCount == 0) || (ls.GetItemCount() < requestedCount))) {
						ls.Add(childObj);
					}
					totalMatches++;
				}
			}

		}

		numberReturned = ls.GetItemCount();
		NPT_LOG_INFO_1("sortCriteria [%s]", sortCriteria.GetChars());
		sortResult(ls, sortCriteria);
		result = formatDidlLocked(ls, urlFormatStr, NULL);

		// TODO:
		updateID = m_systemUpdateId;
		return 0;
	}

	return 601;
}

int MediaStoreImpl::search(AbortableTask *task, const FrontEnd::InterfaceContext *ifctx, const FrontEnd::RequestContext& reqCtx, const NPT_String& urlFormatStr, const NPT_String& containerID, const NPT_String& searchCriteria, const NPT_String& filter, NPT_UInt32 startingIndex, NPT_UInt32 requestedCount, const NPT_String& sortCriteria, NPT_String& result, NPT_UInt32& numberReturned, NPT_UInt32& totalMatches, NPT_UInt32& updateID)
{
	ReadLocker locker(m_stateLock);

	MSBaseObject *obj = findObjectLocked(containerID);
	if (!obj) {
		// No such object
		NPT_LOG_WARNING_1("Container [%s] not found!", containerID.GetChars());
		return 701;
	}

	sc::Expr *matchExpr = NULL;

	NPT_String sc = searchCriteria;
	if (sc.Trim().Compare("*") == 0) {
		matchExpr = new sc::MatchAllExpr();
	} else {
		NPT_LOG_INFO_2("searching [%s] for [%s]", containerID.GetChars(), searchCriteria.GetChars());
		matchExpr = compileSearchCriteria(searchCriteria);
	}

	if (matchExpr) {
		NPT_LOG_INFO_2("search criteria compiled successfully [%s] [%p]", searchCriteria.GetChars(), matchExpr);
	} else {
		matchExpr = new sc::MatchNoneExpr();
		NPT_LOG_INFO_2("search criteria failed to compile, using default MatchNoneExpr!!! [%s] [%p]", searchCriteria.GetChars(), matchExpr);
	}

	updateID = m_systemUpdateId;

	NPT_List<MSBaseObject*> ls;
	NPT_List<MSBaseObject*> ls1;

	if (MSContainer *container = obj->asContainer()) {
		searchLockedR(container, matchExpr, ls1);
	}
	totalMatches = ls1.GetItemCount();
	for (NPT_UInt32 i = startingIndex; (i < ls1.GetItemCount()) && (requestedCount == 0 || ls.GetItemCount() < requestedCount); i++) {
		ls.Add(*ls1.GetItem(i));
	}

	numberReturned = ls.GetItemCount();
	NPT_LOG_INFO_1("sortCriteria [%s]", sortCriteria.GetChars());
	sortResult(ls, sortCriteria);
	result = formatDidlLocked(ls, urlFormatStr, m_dirStorage);

	delete matchExpr;

	return 0;
}

void MediaStoreImpl::searchLockedR(MSContainer *container, sc::Expr *matchExpr, NPT_List<MSBaseObject*>& ls)
{
	for (NPT_Ordinal i = 0; i < container->childCount(); i++) {
		MSBaseObject *childObj = container->childAt(i);
		if (matchExpr->eval(childObj)) {
			ls.Add(childObj);
		}
		if (MSContainer *childContainer = childObj->asContainer()) {
			searchLockedR(childContainer, matchExpr, ls);
		}
	}
}

void outputObject(NPT_XmlSerializer& xml, const MSObject *obj, const NPT_String& urlFormatStr)
{
	xml.StartElement("dc", "title");
	xml.Text(obj->m_title);
	xml.EndElement("dc", "title");

	if (!obj->m_creator.IsEmpty()) {
		xml.StartElement("dc", "creator");
		xml.Text(obj->m_creator);
		xml.EndElement("dc", "creator");
	}

	if (obj->m_date.ToNanos() != 0) {
		xml.StartElement("dc", "date");
		xml.Text(NPT_DateTime(obj->m_date).ToString(NPT_DateTime::FORMAT_W3C));
		xml.EndElement("dc", "date");
	}

	xml.StartElement("upnp", "class");
/*	if (!obj->classDesc().IsEmpty()) {
		xml.Attribute(NULL, "name", obj->classDesc());
	}*/
	xml.Text(obj->m_upnpClass);
	xml.EndElement("upnp", "class");

	for (NPT_Ordinal i = 0; i < obj->m_artistList.GetItemCount(); i++) {
		xml.StartElement("upnp", "artist");
		xml.Text(obj->m_artistList.GetItem(i)->GetChars());
		xml.EndElement("upnp", "artist");
	}

	for (NPT_Ordinal i = 0; i < obj->m_albumList.GetItemCount(); i++) {
		xml.StartElement("upnp", "album");
		xml.Text(obj->m_albumList.GetItem(i)->GetChars());
		xml.EndElement("upnp", "album");
	}

	for (NPT_Ordinal i = 0; i < obj->m_genreList.GetItemCount(); i++) {
		xml.StartElement("upnp", "genre");
		xml.Text(obj->m_genreList.GetItem(i)->GetChars());
		xml.EndElement("upnp", "genre");
	}
}

void outputFileItemCommon(NPT_XmlSerializer& xml, const MSFileItem *fileItem, const NPT_String& urlFormatStr)
{
	outputObject(xml, fileItem, urlFormatStr);

	xml.StartElement(NULL, "res");
	// VERY VERY VERY IMPORTANT!!!!!!
	// protocolInfo must be "http-get:*:[url]:*"
	//                  not "http-get:*:[url]:"
	// otherwise XBMC will not find any resource !!!
	xml.Attribute(NULL, "protocolInfo", NPT_String::Format("http-get:*:%s:*", fileItem->m_mimeType.GetChars()));
	xml.Attribute(NULL, "size", NPT_String::FromIntegerU(fileItem->m_size));

	NPT_String uu = NPT_String::Format(urlFormatStr, fileItem->m_objectId.GetChars());
	xml.Text(uu);
	xml.EndElement(NULL, "res");
}

void outputFileItem(NPT_XmlSerializer& xml, const MSFileItem *fileItem, const NPT_String& urlFormatStr, const MSContainer *topLevelFolder)
{
	xml.StartElement(NULL, "item");
	xml.Attribute(NULL, "id", fileItem->m_objectId);
	xml.Attribute(NULL, "parentID", fileItem->m_parent ? fileItem->m_parent->m_objectId : "-1");
	xml.Attribute(NULL, "restricted", "1");

	outputFileItemCommon(xml, fileItem, urlFormatStr);

	if (topLevelFolder) {
		NPT_String folderPath;
		MSContainer *container = fileItem->m_parent;
		while (container != topLevelFolder) {
			folderPath.Insert(container->m_title);
			container = container->m_parent;
			if (container != topLevelFolder) {
				folderPath.Insert("\\");
			}
		}

		//NPT_LOG_INFO_1("TTT [%s]", folderPath.GetChars());

		xml.StartElement(NULL, "desc");
		xml.Attribute(NULL, "id", "folderPath");
		xml.Attribute(NULL, "nameSpace", "urn:schemas-microsoft-com:WMPNSS-1-0/");
		xml.Attribute("xmlns", "microsoft", "urn:schemas-microsoft-com:WMPNSS-1-0/");
		xml.StartElement("microsoft", "folderPath");
		xml.Text(folderPath);
		xml.EndElement("microsoft", "folderPath");
		xml.EndElement(NULL, "desc");
	}

	xml.EndElement(NULL, "item");
}

void outputContainer(NPT_XmlSerializer& xml, const MSContainer *container, const NPT_String& urlFormatStr)
{
	xml.StartElement(NULL, "container");
	xml.Attribute(NULL, "id", container->m_objectId);
	xml.Attribute(NULL, "parentID", container->m_parent ? container->m_parent->m_objectId : "-1");
	xml.Attribute(NULL, "childCount", NPT_String::FromInteger(container->childCount()));
	xml.Attribute(NULL, "searchable", "1");
	xml.Attribute(NULL, "restricted", "1");

	outputObject(xml, container, urlFormatStr);

	for (NPT_Ordinal i = 0; i < container->searchClassCount(); i++) {
		const MSClassSpec *spec = container->searchClassAt(i);
		xml.StartElement("upnp", "searchClass");
		if (!spec->m_attrName.IsEmpty()) {
			xml.Attribute(NULL, "name", spec->m_attrName);
		}
		xml.Attribute(NULL, "includeDerived", spec->m_attrIncludeDerived ? "1" : "0");
		xml.Text(spec->m_text);
		xml.EndElement("upnp", "searchClass");
	}

	xml.EndElement(NULL, "container");
}

void outputLink(NPT_XmlSerializer& xml, const MSLink *link, const NPT_String& urlFormatStr)
{
	xml.StartElement(NULL, "item");
	xml.Attribute(NULL, "id", link->m_objectId);
	xml.Attribute(NULL, "parentID", link->m_parent ? link->m_parent->m_objectId : "-1");
	xml.Attribute(NULL, "restricted", "1");
	xml.Attribute(NULL, "refID", link->m_target->m_objectId);

	const MSFileItem *fileItem = link->m_target;
	outputFileItemCommon(xml, fileItem, urlFormatStr);

	xml.EndElement(NULL, "item");
}

NPT_String MediaStoreImpl::formatDidlLocked(const NPT_List<MSBaseObject*>& ls, const NPT_String& urlFormatStr, const MSContainer *topLevelFolder)
{
	NPT_StringOutputStream outputStream;
	NPT_XmlSerializer xml(&outputStream, 0, true, false);

	xml.StartDocument();
	xml.StartElement(NULL, "DIDL-Lite");
	xml.Attribute(NULL, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
	xml.Attribute("xmlns", "dc", "http://purl.org/dc/elements/1.1/");
	xml.Attribute("xmlns", "upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");

	for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
		const MSBaseObject *baseObj = *ls.GetItem(i);
		if (const MSFileItem *fileItem = baseObj->asFileItem()) {
			outputFileItem(xml, fileItem, urlFormatStr, topLevelFolder);
		} else if (const MSContainer *container = baseObj->asContainer()) {
			outputContainer(xml, container, urlFormatStr);
		} else if (const MSLink *link = baseObj->asLink()) {
			outputLink(xml, link, urlFormatStr);
		}
	}

	xml.EndElement(NULL, "DIDL-Lite");
	xml.EndDocument();
	return outputStream.GetString();
}

bool MediaStoreImpl::findFileDetail(const NPT_String& objectId, MediaStore::FileDetail& detail)
{
	ReadLocker locker(m_stateLock);
	MSBaseObject *baseObj = findObjectLocked(objectId);
	if (!baseObj) {
		return false;
	}

	const MSFileItem *fileItem = NULL;

	if (const MSLink *link = baseObj->asLink()) {
		baseObj = findObjectLocked(link->m_target->m_objectId);
		if (baseObj) {
			fileItem = baseObj->asFileItem();
		}
	}

	if (!fileItem) {
		fileItem = baseObj->asFileItem();
	}

	if (fileItem) {
		detail.m_mimeType = fileItem->m_mimeType;
		detail.m_modificationTime = fileItem->m_date;
		detail.m_path = fileItem->m_filePath;
		detail.m_size = fileItem->m_size;
		detail.m_type = fileItem->m_isIOSAssetItem ? MediaStore::FileDetail::ALAsset : MediaStore::FileDetail::PosixFile;
		return true;
	}

	return false;
}

bool MediaStoreImpl::buildDidlForFile(const NPT_String& path, const NPT_String& urlFormatStr, NPT_String& mimeType, NPT_String& metaData)
{
	NPT_FileInfo fileInfo;
	if (NPT_FAILED(NPT_File::GetInfo(path, &fileInfo))) {
		return false;
	}

	NPT_String ext = NPT_FilePath::FileExtension(path);
	ext.MakeLowercase();
	const MSMimeInfo **mimeInfo;
	if (NPT_FAILED(g_mimeIndexByExt.Get(ext, mimeInfo))) {
		return false;
	}
	MSFileItem *item = new MSFileItem();
	item->m_filePath = path;
	item->m_title = path;//NPT_FilePath::BaseName(path, false);
	item->m_mediaType = (*mimeInfo)->mediaType;
	item->m_size = fileInfo.m_Size;
	item->m_date = fileInfo.m_CreationTime;
	switch (item->m_mediaType) {
	case MSMediaType_Video:
		item->m_upnpClass = "object.item.videoItem";
		item->m_mimeType = (*mimeInfo)->mimeType;
		break;
	case MSMediaType_Audio:
		item->m_upnpClass = "object.item.audioItem";
		item->m_mimeType = (*mimeInfo)->mimeType;
		item->m_artistList.Add("Unknown Artist");
		item->m_genreList.Add("Unknown Genre");
		item->m_albumList.Add("Unknown Album");
		break;
	case MSMediaType_Image:
		item->m_upnpClass = "object.item.imageItem.photo";
		item->m_mimeType = (*mimeInfo)->mimeType;
		break;
	}
	item->m_objectId += ext;

	ReadLocker locker(m_stateLock);
	NPT_List<MSBaseObject*> ls;
	ls.Add(item);
	metaData = formatDidlLocked(ls, urlFormatStr, NULL);
	mimeType = item->m_mimeType;
	delete item;
	return true;
}

NPT_Result MediaStoreImpl::importPhotos(const NPT_String& name, DLNACoreOp **op)
{
	DLNAImportPhotosOp *impl = new DLNAImportPhotosOp();
	impl->addRef();
	DLNAImportPhotosTask *task = new DLNAImportPhotosTask(impl, this);
	NPT_Result nr = m_taskGroup->startTask(task);
	if (NPT_FAILED(nr)) {
		impl->release();
		return nr;
	}
	*op = impl;
	return NPT_SUCCESS;
}

MSImportTask::MSImportTask(MediaStoreImpl *owner, const NPT_List<NPT_String>& dirs, const NPT_List<NPT_String>& names, bool ignoreDot)
	: m_owner(owner), m_dirs(dirs), m_names(names), m_ignoreDot(ignoreDot)
{
}

void MSImportTask::exec()
{
	NPT_List<ImportContext*> ls;
	for (NPT_Ordinal i = 0; i < m_dirs.GetItemCount(); i++) {
		if (!aborted()) {
			ImportContext *impCtx = new ImportContext();
			impCtx->m_folder = new MSFolder();
			impCtx->m_folder->m_title = *m_names.GetItem(i);
			impCtx->rootPath = *m_dirs.GetItem(i);
			impCtx->folderName = *m_names.GetItem(i);
			NPT_FileInfo fileInfo;
			if (NPT_SUCCEEDED(NPT_File::GetInfo(impCtx->rootPath, &fileInfo))) {
				import(impCtx, impCtx->m_folder, impCtx->rootPath, fileInfo);
				if (impCtx->m_folder->childCount() > 0) {
					ls.Add(impCtx);
				} else {
					delete impCtx;
				}
			} else {
				delete impCtx;
			}
		} else {
			break;
		}
	}

	if (!aborted()) {
		NPT_List<MSImportResult> resultList;
		for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
			ImportContext *impCtx = *ls.GetItem(i);
			MSImportResult result;
			result.m_folder = impCtx->m_folder;
			impCtx->m_folder = NULL;
			resultList.Add(result);
		}
		m_owner->onImportResult(resultList);
	}

	ls.Apply(NPT_ObjectDeleter<ImportContext>());
}

void MSImportTask::doAbort()
{
}

void MSImportTask::import(ImportContext *impCtx, MSFolder *parentFolder, const NPT_String& dir, const NPT_FileInfo& dirInfo)
{
	NPT_List<NPT_String> ls;
	if (NPT_SUCCEEDED(NPT_File::ListDir(dir, ls))) {
		for (NPT_Ordinal i = 0; i < ls.GetItemCount(); i++) {
			if (!aborted()) {
				const NPT_String& fname = *ls.GetItem(i);
				NPT_String fullPath(dir);
				fullPath.Append(NPT_FilePath::Separator);
				fullPath.Append(fname);

				NPT_FileInfo fileInfo;
				if (NPT_SUCCEEDED(NPT_File::GetInfo(fullPath, &fileInfo))) {
					if (fileInfo.m_Type == NPT_FileInfo::FILE_TYPE_DIRECTORY && (!m_ignoreDot || !fname.StartsWith("."))) {
						MSFolder *subFolder = new MSFolder();
						subFolder->m_title = fname;
						import(impCtx, subFolder, fullPath, fileInfo);
						if (subFolder->childCount() > 0) {
							parentFolder->addChild(subFolder);
						} else {
							delete subFolder;
						}
					} else if (fileInfo.m_Type == NPT_FileInfo::FILE_TYPE_REGULAR && (!m_ignoreDot || !fname.StartsWith("."))) {
						importFile(impCtx, parentFolder, fullPath, fileInfo, dir);
					}
				}
			} else {
				break;
			}
		}
	}
}

void MSImportTask::importFile(ImportContext *impCtx, MSFolder *parentFolder, const NPT_String& path, const NPT_FileInfo& fileInfo, const NPT_String& dir)
{
	NPT_String ext = NPT_FilePath::FileExtension(path);
	ext.MakeLowercase();
	const MSMimeInfo **mimeInfo;
	if (NPT_SUCCEEDED(g_mimeIndexByExt.Get(ext, mimeInfo))) {
		NPT_LOG_FINER_1("importFile [%s]", path.GetChars());
		MSFileItem *item = new MSFileItem();
		item->m_filePath = path;
		item->m_title = NPT_FilePath::BaseName(path, false);
		item->m_mediaType = (*mimeInfo)->mediaType;
		item->m_size = fileInfo.m_Size;
		item->m_date = fileInfo.m_CreationTime;
		switch (item->m_mediaType) {
		case MSMediaType_Video:
			item->m_upnpClass = "object.item.videoItem";
			item->m_mimeType = (*mimeInfo)->mimeType;
			break;
		case MSMediaType_Audio:
			item->m_upnpClass = "object.item.audioItem";
			item->m_mimeType = (*mimeInfo)->mimeType;
			item->m_artistList.Add("Unknown Artist");
			item->m_genreList.Add("Unknown Genre");
			item->m_albumList.Add("Unknown Album");
			break;
		case MSMediaType_Image:
			item->m_upnpClass = "object.item.imageItem.photo";
			item->m_mimeType = (*mimeInfo)->mimeType;
			break;
		}
		item->m_objectId += ext;
		parentFolder->addChild(item);
	}
}

void iOSImportPhotos(MSFolder *folder);

#ifndef DLNACORE_PLATFORM_IOS

void iOSImportPhotos(MSFolder *folder)
{
}

#endif // DLNACORE_PLATFORM_IOS

IOSAssetLibraryImportTask::IOSAssetLibraryImportTask(MediaStoreImpl *owner)
	: m_owner(owner)
{
}

void IOSAssetLibraryImportTask::exec()
{
	if (!aborted()) {
		NPT_List<MSImportResult> resultList;
		MSImportResult result;
		result.m_folder = new MSFolder();
		resultList.Add(result);
		iOSImportPhotos(result.m_folder);
		m_owner->onImportResult(resultList);
	}
}

void IOSAssetLibraryImportTask::doAbort()
{
}

SCTokenizer::SCTokenizer(const NPT_String& searchCriteria)
	: m_sc(searchCriteria)
{
	m_pc = m_sc.GetChars();
	m_pe = m_pc + m_sc.GetLength();
}

bool SCTokenizer::nextChar(char& c)
{
	if (m_pc == m_pe) {
		return false;
	}
	c = *m_pc++;
	return true;
}

bool SCTokenizer::peekChar(char& c)
{
	if (m_pc == m_pe) {
		return false;
	}
	c = *m_pc;
	return true;
}

bool SCTokenizer::isWhitespace(char c)
{
	switch (c) {
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x20:
		return true;
	}
	return false;
}

bool SCTokenizer::isIdentChar(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ':' || c == '_' || c == '.' || c == '@' ||  (c >= '0' && c <= '9');
}

void SCTokenizer::skipWhitespaces()
{
	while (m_pc != m_pe) {
		if (isWhitespace(*m_pc)) {
			++m_pc;
		} else {
			break;
		}
	}
}

SCTokenizer::Token SCTokenizer::nextToken(NPT_String& val)
{
	val = NPT_String();

	skipWhitespaces();

	char c;
	if (!nextChar(c)) {
		return Token_EOS;
	}

	switch (c) {
	case '(':
		return Token_LeftBracket;
	case ')':
		return Token_RightBracket;
	case '=':
		return Token_EQ;
	case '!':
		if (nextChar(c) && c == '=') {
			return Token_NEQ;
		}
		return Token_Error;
	case '<':
		if (peekChar(c) && c == '=') {
			++m_pc;
			return Token_LE;
		}
		return Token_LT;
	case '>':
		if (peekChar(c) && c == '=') {
			++m_pc;
			return Token_GE;
		}
		return Token_GT;
	case '"':
		{
			NPT_String qstr;
			for (;;) {
				if (nextChar(c)) {
					if (c == '\\') {
						if (nextChar(c)) {
							if (c == '\\') {
								qstr.Append("\\");
							} else if (c == '"') {
								qstr.Append("\"");
							} else {
								return Token_Error;
							}
						} else {
							return Token_Error;
						}
					} else if (c == '"') {
						val = qstr;
						return Token_QuotedVal;
					} else {
						qstr.Append(&c, 1);
					}
				} else {
					return Token_Error;
				}
			}
		}
		break;
	default:
		{
			NPT_String str;
			if (isIdentChar(c)) {
				str.Append(&c, 1);
			} else {
				return Token_Error;
			}
			for (;;) {
				if (peekChar(c)) {
					if (isIdentChar(c)) {
						str.Append(&c, 1);
						m_pc++;
					} else {
						break;
					}
				} else {
					break;
				}
			}

			if (str.Compare("and") == 0) {
				return Token_AND;
			}

			if (str.Compare("or") == 0) {
				return Token_OR;
			}

			if (str.Compare("contains") == 0) {
				return Token_Contains;
			}
			
			if (str.Compare("doesNotContain") == 0) {
				return Token_DoesNotContain;
			}
			
			if (str.Compare("derivedfrom") == 0) {
				return Token_DerivedFrom;
			}
			
			if (str.Compare("exists") == 0) {
				return Token_Exists;
			}
			
			if (str.Compare("true") == 0) {
				return Token_True;
			}
			
			if (str.Compare("false") == 0) {
				return Token_False;
			}

			val = str;
			return Token_Property;
		}
		break;
	}

	return Token_EOS;
}

SCTokenizerLevel2::SCTokenizerLevel2(const NPT_String& searchCriteria)
	: m_tokenizer(searchCriteria)
{
}

SCTokenizerLevel2::Token SCTokenizerLevel2::nextToken(Expr& expr)
{
	Token retval;
	NPT_String val;
	SCTokenizer::Token token = m_tokenizer.nextToken(val);
	switch (token) {
	case SCTokenizer::Token_LeftBracket:
		NPT_LOG_FINEST("L1 token: Token_LeftBracket");
		retval = SCTokenizerLevel2::Token_LeftBracket;
		break;
	case SCTokenizer::Token_RightBracket:
		NPT_LOG_FINEST("L1 token: Token_RightBracket");
		retval = SCTokenizerLevel2::Token_RightBracket;
		break;
	case SCTokenizer::Token_AND:
		NPT_LOG_FINEST("L1 token: Token_AND");
		retval = SCTokenizerLevel2::Token_AND;
		break;
	case SCTokenizer::Token_OR:
		NPT_LOG_FINEST("L1 token: Token_OR");
		retval = SCTokenizerLevel2::Token_OR;
		break;
	case SCTokenizer::Token_EOS:
		NPT_LOG_FINEST("L1 token: Token_EOS");
		retval = SCTokenizerLevel2::Token_EOS;
		break;
	case SCTokenizer::Token_Error:
		NPT_LOG_FINEST("L1 token: Token_Error");
		retval = SCTokenizerLevel2::Token_Error;
		break;
	case SCTokenizer::Token_Property:
		{
			NPT_LOG_FINEST_1("L1 Token_Property [%s]", val.GetChars());

			NPT_String val1;
			SCTokenizer::Token token1 = m_tokenizer.nextToken(val1);
			NPT_String val2;
			SCTokenizer::Token token2 = m_tokenizer.nextToken(val2);
			if (token1 == SCTokenizer::Token_Exists) {
				if (token2 == SCTokenizer::Token_True) {
					expr.prop = val;
					expr.op = MSBaseObject::Op_Exists;
					expr.value = "1";
					NPT_LOG_FINEST("L1 token: Token_Expr (Exists True)");
					retval = Token_Expr;
				} else if (token2 == SCTokenizer::Token_False) {
					expr.prop = val;
					expr.op = MSBaseObject::Op_Exists;
					expr.value = "";
					NPT_LOG_FINEST("L1 token: Token_Expr (Exists False)");
					retval = Token_Expr;
				} else {
					NPT_LOG_FINEST("L1 token: Token_Error");
					retval = SCTokenizerLevel2::Token_Error;
				}
			} else if (token2 == SCTokenizer::Token_QuotedVal) {
				expr.prop = val;
				expr.value = val2;
				switch (token1) {
				case SCTokenizer::Token_EQ:
					NPT_LOG_FINEST("L1 token: Token_EQ");
					expr.op = MSBaseObject::Op_EqualTo;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_NEQ:
					NPT_LOG_FINEST("L1 token: Token_NEQ");
					expr.op = MSBaseObject::Op_NotEqualTo;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_LT:
					NPT_LOG_FINEST("L1 token: Token_LT");
					expr.op = MSBaseObject::Op_LessThan;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_LE:
					NPT_LOG_FINEST("L1 token: Token_LE");
					expr.op = MSBaseObject::Op_LessEqual;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_GT:
					NPT_LOG_FINEST("L1 token: Token_GT");
					expr.op = MSBaseObject::Op_GreatThan;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_GE:
					NPT_LOG_FINEST("L1 token: Token_GE");
					expr.op = MSBaseObject::Op_GreatEqual;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_Contains:
					NPT_LOG_FINEST("L1 token: Token_Contains");
					expr.op = MSBaseObject::Op_Contains;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_DoesNotContain:
					NPT_LOG_FINEST("L1 token: Token_DoesNotContain");
					expr.op = MSBaseObject::Op_DoesNotContain;
					retval = Token_Expr;
					break;
				case SCTokenizer::Token_DerivedFrom:
					NPT_LOG_FINEST("L1 token: Token_DerivedFrom");
					expr.op = MSBaseObject::Op_DerivedFrom;
					NPT_LOG_FINEST_1("L1 we are returning Token_Expr = %d", Token_Expr);
					retval = Token_Expr;
					break;
				default:
					NPT_LOG_FINEST_1("L1 token: Token_Error [%d]", token1);
					retval = Token_Error;
					break;
				}
			} else {
				NPT_LOG_FINEST_1("L1 token: Token_Error [%d]", token2);
				retval = Token_Error;
			}
		}
		break;
	default:
		NPT_LOG_FINEST_1("L1 token: Token_Error [%d]", token);
		retval = SCTokenizerLevel2::Token_Error;
		break;
	}
	return retval;
}

MediaStoreImpl::NotifyTask::NotifyTask(MediaStoreImpl *owner)
	: m_owner(owner)
{
}

void MediaStoreImpl::NotifyTask::exec()
{
	m_owner->m_callback->onMediaStoreUpdated(m_owner->m_intf);
}

//------------------------------------------------------------------------------
// DLNAImportPhotosOp
//------------------------------------------------------------------------------

DLNAImportPhotosOp::DLNAImportPhotosOp()
	: m_refCount(0), m_waitVar(0), m_abortFlag(false), m_finished(false), m_finishCallback(NULL)
	, m_succeeded(false), m_buddy(NULL)
{
}

DLNAImportPhotosOp::~DLNAImportPhotosOp()
{
}

int DLNAImportPhotosOp::addRef()
{
	return m_refCount.Increment();
}

int DLNAImportPhotosOp::release()
{
	int rc = m_refCount.Decrement();
	if (rc == 0) {
		delete this;
		return 0;
	}
	return rc;
}

void DLNAImportPhotosOp::abort()
{
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		m_abortFlag = true;
		if (m_buddy) {
			m_buddy->abort();
		}
	}
}

bool DLNAImportPhotosOp::aborted() const
{
	ReadLocker locker(m_stateLock);
	return m_abortFlag;
}

NPT_Result DLNAImportPhotosOp::wait(NPT_Timeout timeout)
{
	return m_waitVar.WaitWhileEquals(0, timeout);
}

bool DLNAImportPhotosOp::checkFinishedIfNotSetCallback(FinishCallback *callback)
{
	WriteLocker locker(m_stateLock);
	if (m_finished) {
		return true;
	}
	m_finishCallback = callback;
	return false;
}

bool DLNAImportPhotosOp::resetCallback()
{
	WriteLocker locker(m_stateLock);
	m_finishCallback = NULL;
	return m_finished;
}

bool DLNAImportPhotosOp::succeeded() const
{
	return m_succeeded;
}

void DLNAImportPhotosOp::setBuddy(DLNAImportPhotosTask *buddy)
{
	WriteLocker locker(m_stateLock);
	m_buddy = buddy;
}

void DLNAImportPhotosOp::notifyFinished()
{
	{
		WriteLocker locker(m_stateLock);
		if (m_finishCallback) {
			m_finishCallback->onDLNACoreOpFinished(this);
		}
		m_finished = true;
		m_waitVar.SetValue(1);
	}
}

//------------------------------------------------------------------------------
// DLNAImportPhotosTask
//------------------------------------------------------------------------------

DLNAImportPhotosTask::DLNAImportPhotosTask(DLNAImportPhotosOp *op, MediaStoreImpl *store)
	: m_op(op), m_store(store)
{
	m_op->addRef();
	m_op->setBuddy(this);
}

DLNAImportPhotosTask::~DLNAImportPhotosTask()
{
	m_op->setBuddy(NULL);
	m_op->release();
}

void DLNAImportPhotosTask::exec()
{
	NPT_List<MSImportResult> resultList;
	MSImportResult result;
	result.m_folder = new MSFolder();
	resultList.Add(result);
	iOSImportPhotos(result.m_folder);
	m_op->m_succeeded = true;
	m_store->onImportResult(resultList);
	m_op->notifyFinished();
}

} // namespace deejay
