#include "cocos2d.h"

#include "UndertaleResources.h"
#include <unordered_set>
#include <fstream>
#include "LuaEngine.h"
#include <cstdarg>
#include <csignal>
#include <new>

extern "C" {    // another way
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
	#include "lstring.h"
};

USING_NS_CC;

static std::string LuaEngineBuffer = "";
static lua_State *globalL = nullptr;
static const char *progname = nullptr;
extern "C" {
	/* print a string */
	void LuaEngine_lua_writestring(const char* s, size_t l) {
		LuaEngineBuffer.append(s, l);
	}
	void LuaEngine_lua_writeline() {
		LuaEngineBuffer.push_back('\n');
		CCLOGERROR("--------------------- LUA ERROR START -------------------------");
		CCLOGERROR(LuaEngineBuffer.c_str());
		CCLOGERROR("--------------------- LUA ERROR END   -------------------------\n");
		LuaEngineBuffer.clear();
	}
	void LuaEngine_lua_writestringerror(const char* fmt, ...) {
		char buffer[512];
		va_list va;
		va_start(va, fmt);
		vsprintf_s(buffer, fmt, va);
		va_end(va);
		LuaEngineBuffer.push_back('\n');
		CCLOGERROR("--------------------- LUA ERROR START -------------------------");
		CCLOGERROR(buffer);
		CCLOGERROR("--------------------- LUA ERROR END   -------------------------\n");
	}

	/*
	** Hook set by signal function to stop the interpreter.
	*/
	static void lstop(lua_State *L, lua_Debug *ar) {
		(void)ar;  /* unused arg. */
		lua_sethook(L, NULL, 0, 0);  /* reset hook */
		luaL_error(L, "interrupted!");
	}


	/*
	** Function to be called at a C signal. Because a C signal cannot
	** just change a Lua state (as there is no proper synchronization),
	** this function only sets a hook that, when called, will stop the
	** interpreter.
	*/
	static void laction(int i) {
		signal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
		lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
	}

	/*
	** Prints an error message, adding the program name in front of it
	** (if present)
	*/
	static void l_message(const char *pname, const char *msg) {
		if (pname) lua_writestringerror("%s: ", pname);
			lua_writestringerror("%s\n", msg);
	}

	/*
	** Check whether 'status' is not OK and, if so, prints the error
	** message on the top of the stack. It assumes that the error object
	** is a string, as it was either generated by Lua or by 'msghandler'.
	*/
	static int report(lua_State *L, int status) {
		if (status != LUA_OK) {
			const char *msg = lua_tostring(L, -1);
			l_message(progname, msg);
			lua_pop(L, 1);  /* remove message */
		}
		return status;
	}
	/*
	** Message handler used to run all chunks
	*/
	static int msghandler(lua_State *L) {
		const char *msg = lua_tostring(L, 1);
		if (msg == NULL) {  /* is error object not a string? */
			if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
				lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
				return 1;  /* that is the message */
			else
				msg = lua_pushfstring(L, "(error object is a %s value)",
					luaL_typename(L, 1));
		}
		luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
		return 1;  /* return the traceback */
	}
	/*
	** Interface to 'lua_pcall', which sets appropriate message function
	** and C-signal handler. Used to run all chunks.
	*/
}
/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print(lua_State *L) {
	int n = lua_gettop(L);
	if (n > 0) {  /* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK)
			l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
				lua_tostring(L, -1)));
	}
}


static int docall(lua_State *L, int narg, int nres) {
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, msghandler);  /* push message handler */
	lua_insert(L, base);  /* put it under function and args */
	globalL = L;  /* to be available to 'laction' */
	signal(SIGINT, laction);  /* set C-signal handler */
	status = lua_pcall(L, narg, nres, base);
	signal(SIGINT, SIG_DFL); /* reset C-signal handler */
	lua_remove(L, base);  /* remove message handler from the stack */
	return status;
}

/*
Above is the structure for strings in lua.  So we need to go back size_t for the len and unsigned int for the hash
This is DANGROUS but it saves time and space
The string is a buffer at the end of the TString var.
*/
//struct istring_internal { UTString utsr; }; // we don't really make this, we just dynamic cast it to UTString


struct hash_TString {
	inline size_t operator()(const TString *l) const{ return l->hash; }
};

// list of meta tables we have to regester
void lua_RegesterVec2(lua_State* L);
void lua_Regester_LuaSprite(lua_State* L);


class internal_luaEngine {
public:
	static internal_luaEngine* s_maker;
	static const char* emptyString;
	static cocos2d::Node * scene;
	static lua_State* L ;
	internal_luaEngine() {
		L = luaL_newstate();
		luaL_openlibs(L);
		emptyString = getStringInterned("");
	}

	inline const char* getEmptyString() const {
		return emptyString;
	}
	inline lua_State* getState() const { return L; }
	// some internal lua trickery, works for 5.3 but note sure for
	// others
	inline const TString* getTStringInterned(const char*s) const { // inlined as there are only a few times we use this
		luaC_checkGC(L); // check the GC!
		// side note, you WANT to use luaS_new.  Lua 5.3 caches the pointers for const char* so if its a a literal, it will look up much faster
		// I thought of doing the same here with a map, but why do it twice?
		TString*  ts = luaS_new(L, s);
		GCObject *o = obj2gco(ts);
		if (G(L)->allgc == o) // if created it
			luaC_fix(L, o); // fix it so it can't be collected
		return ts;
	}
	inline const char* getStringInterned(const char* s) const { return getstr(getTStringInterned(s)); }
public: // get instance and other public static functions
	inline static internal_luaEngine* getInstance() {
		if (!s_maker) {
			s_maker = new internal_luaEngine();
			lua_RegesterVec2(L);
			lua_Regester_LuaSprite(L);
		}
		return s_maker;
	}
};

internal_luaEngine* internal_luaEngine::s_maker = nullptr;
const char* internal_luaEngine::emptyString = nullptr;
lua_State* internal_luaEngine::L = nullptr;
cocos2d::Node * internal_luaEngine::scene = nullptr;

istring::istring() : _internal(internal_luaEngine::getInstance()->getEmptyString()) { }
istring::istring(const char * str) : _internal(internal_luaEngine::getInstance()->getStringInterned(str)) { }
istring::istring(const std::string & str) : istring(str.c_str()) {}
size_t istring::length() const { return tsslen((TString*)(_internal - sizeof(TString))); }
size_t istring::hash() const { return ((TString*)(_internal - sizeof(TString)))->hash; }

inline bool istring::isEmpty() const
{
	return _internal == internal_luaEngine::emptyString;
}

lua_State * LuaEngine::getLuaState()
{
	return internal_luaEngine::getInstance()->getState();
}

void LuaEngine::setLuaScene(cocos2d::Node * scene)
{
	internal_luaEngine::scene = scene;
}

cocos2d::Node * LuaEngine::getLuaScene()
{
	return internal_luaEngine::scene;
}

bool LuaEngine::RunGlobalUpdate(const char* global_name, float dt)
{
	lua_State* L = getLuaState();
	lua_getglobal(L, global_name);
	if (!lua_isfunction(L, -1)) {
		CCLOGERROR("RunGlobalUpdate: '%s' not a function", global_name);
		return false;
	}
	lua_pushnumber(L, dt);

	int status = docall(L, 1, 0);
	if (status == LUA_OK) return true;
	else return false;
}

bool LuaEngine::DoFile(const char * filename)
{
	lua_State* L = getLuaState();
	do {
		if (luaL_loadfile(L, filename) != LUA_OK) break;
		if (docall(L, 0, 0) != LUA_OK) break;
		
		return true;
	} while (false);
	int top = 0;
	while ((top = lua_gettop(L)) != 0) {
		// print all the stack?
		CCLOGERROR("Lua Error: %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}	
	return false;
}

// just to get me in the lua thing again, warping a Vec2 into a lua object
static const char* luastring_x = nullptr;
static const char* luastring_y = nullptr;


//template<typename T> inline const char* metaTableName<LuaSprite>() { return "LuaSpriteMT"; }

#define VEC2_METATABLENAME (LuaEngineMetaTableNames::metaTableName<cocos2d::Vec2>())
static const char* lua_vec2_metatable = "Vec2MT";
static int Vec2__newindex(lua_State* L) {
	Vec2* vec2 = static_cast<Vec2*>(luaL_checkudata(L, 1, VEC2_METATABLENAME));
	const char* index = luaL_checkstring(L, 2);
	if (index == luastring_x) vec2->x = luaL_checknumber(L, 3);
	else if(index == luastring_y) vec2->y = luaL_checknumber(L, 3);
	else return luaL_error(L, "Invalid property of Vec2");

	return 0;
}
static int Vec2__index(lua_State* L) {
	Vec2* vec2 = static_cast<Vec2*>(luaL_checkudata(L, 1, VEC2_METATABLENAME));
	const char* index = luaL_checkstring(L, 2);
	if (index == luastring_x) lua_pushnumber(L, vec2->x);
	else if (index == luastring_y) lua_pushnumber(L, vec2->y);
	else lua_pushnil(L);
	return 1;
}
static int Vec2__gc(lua_State* L) {
	Vec2* vec2 = static_cast<Vec2*>(luaL_checkudata(L, 1, VEC2_METATABLENAME));
	vec2->~Vec2(); // lua gets rid of this after
	return 0;
}
static int Vec2__tostring(lua_State* L) {
	Vec2* vec2 = static_cast<Vec2*>(luaL_checkudata(L, 1, VEC2_METATABLENAME));
	char buffer[128];
	int len = sprintf_s(buffer, "{ x=%.2f, y=%.2f }", vec2->x, vec2->y);
	lua_pushlstring(L, buffer, len);
	return 1;
}
static luaL_Reg Vec2_functions[] = {
	{ "__index",Vec2__index },
	{ "__newindex",Vec2__newindex },
	{ "__gc", Vec2__gc },
	{ "__tostring", Vec2__tostring },
	{  nullptr  ,nullptr},
};

void * operator new(size_t size, lua_State * L, const char* metaname)
{
	void* obj = lua_newuserdata(L , size);
	luaL_getmetatable(L, metaname);
	lua_setmetatable(L, -2);
	return obj;
}


static int Vec2__new(lua_State* L) {
	int args = lua_gettop(L);
	
	if (args != 2 && args != 0) return luaL_error(L, "Expected two args or no args for a Vec2 constructor");
	
	lua_Number x = 0;
	lua_Number y = 0;
	if (args == 2) {
		x = luaL_checknumber(L, 1);
		y = luaL_checknumber(L, 2);
	}
	Vec2* vec = LuaEngine::newUserData<cocos2d::Vec2>(L);

	Vec2* vec2 = new ((void*)vec) Vec2;
	return 1;
}
void lua_RegesterVec2(lua_State* L) {
	luastring_x = internal_luaEngine::getInstance()->getStringInterned("x");
	luastring_y = internal_luaEngine::getInstance()->getStringInterned("y");
	luaL_newmetatable(L, VEC2_METATABLENAME);
	
	luaL_setfuncs(L, Vec2_functions,0);
	CCLOG("Print Lua stack count: %i", lua_gettop(L));
	lua_pushcfunction(L, Vec2__new);
	lua_setglobal(L, "Vec2");
}