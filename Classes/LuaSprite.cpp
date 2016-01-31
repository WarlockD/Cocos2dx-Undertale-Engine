#include "LuaSprite.h"
#include "LuaEngine.h"
#include "UndertaleResources.h"
USING_NS_CC;

LuaSprite::LuaSprite()
{
}

LuaSprite::~LuaSprite()
{
}

LuaSprite * LuaSprite::create(const cocos2d::Vector<cocos2d::SpriteFrame*>& frame, int start, int size)
{
	LuaSprite* pSprite = new LuaSprite();
	if (pSprite && pSprite->initWithSpriteFrame(frame.at(0))) {
		pSprite->autorelease();
		pSprite->initOptions();
		pSprite->addEvents();
		pSprite->_speed = 0.0f;
		pSprite->_direction = 0.0f;
		
		return pSprite;
	}
	CC_SAFE_DELETE(pSprite);
	return nullptr;
}
LuaSprite * LuaSprite::create(const cocos2d::Vector<cocos2d::SpriteFrame*>& frame) {
	return create(frame, 0, frame.size());
}

void LuaSprite::initOptions()
{

}

void LuaSprite::addEvents()
{
}

void LuaSprite::update(float dt)
{
	if (!_movementVector.isZero()) {
		Vec2 current = this->getPosition();
		current += _movementVector;
		this->setPosition(current);
	}
}

#define LUA_SPRITE_METATABLE (LuaEngineMetaTableNames::metaTableName<LuaSprite>())
#define LUA_SPRITE_TEST(L,arg) (*static_cast<LuaSprite**>(luaL_checkudata(L, (arg), LUA_SPRITE_METATABLE)))
// just to get me in the lua thing again, warping a Vec2 into a lua object
static const char*  luastring_direction = "direction" ;
static const char* luastring_speed = "speed" ;
static const char*  luastring_x = "x";
static const char* luastring_y = "y";
static const char* luastring_width = "width";
static const char* luastring_height = "height";

static std::unordered_map<const char*, std::function<void(LuaSprite*sprite, lua_Number)>> LuaSprite_setters;
static std::unordered_map<const char*, std::function<lua_Number(LuaSprite*sprite)>> LuaSprite_getters;

static int LuaSprite__newindex(lua_State* L) {
	LuaSprite* sprite = LUA_SPRITE_TEST(L, 1);
	const char* index = luaL_checkstring(L, 2);
	lua_Number value = luaL_checknumber(L, 3);
	auto it = LuaSprite_setters.find(index);
	if(it == LuaSprite_setters.end()) return luaL_error(L, "Invalid property '%'s' of LuaSprite",index);
	it->second(sprite, value);

	return 0;
}
static int LuaSprite__index(lua_State* L) {
	LuaSprite* sprite = LUA_SPRITE_TEST(L, 1);
	const char* index = luaL_checkstring(L, 2);
	auto it = LuaSprite_getters.find(index);
	if (it == LuaSprite_getters.end()) return luaL_error(L, "Invalid property '%'s' of LuaSprite", index);
	lua_pushnumber(L, it->second(sprite));
	return 1;
}
static int LuaSprite__gc(lua_State* L) {
	LuaSprite* sprite = LUA_SPRITE_TEST(L, 1);
	LuaEngine::getLuaScene()->removeChild(sprite, true);
	return 0;
}
static int LuaSprite__tostring(lua_State* L) {
	LuaSprite* sprite = LUA_SPRITE_TEST(L, 1);
	char buffer[128];
	int len = sprintf_s(buffer, "{  We are a sprite! }");
	lua_pushlstring(L, buffer, len);
	return 1;
}
static luaL_Reg LuaSprite_functions[] = {
	{ "__index",LuaSprite__index },
	{ "__newindex",LuaSprite__newindex },
	{ "__gc", LuaSprite__gc },
	{ "__tostring", LuaSprite__tostring },
	{ nullptr  ,nullptr },
};




static int LuaSprite__new(lua_State* L) {
	int args = lua_gettop(L);

	if (args == 0) return luaL_error(L, "Must atleast have the name of the sprite");
	const char* spriteName = luaL_checkstring(L, 1);
	
	auto frames = UndertaleResources::getInstance()->getSpriteFrames(spriteName);
	if (!frames) return luaL_error(L, "Unkonwn sprite name '%s'",spriteName);

	LuaSprite* sprite = LuaSprite::create(*frames);
	if (!sprite) return luaL_error(L, "Could not create LuaSprite name '%s'", spriteName);
	LuaEngine::getLuaScene()->addChild(sprite, 1);
	sprite->scheduleUpdate();
	auto userdata = LuaEngine::newUserData(L, sprite);
	return 1;
}
void lua_Regester_LuaSprite(lua_State* L) {
	luastring_direction = istring(luastring_direction).c_str();
	luastring_speed = istring(luastring_speed).c_str();
	luastring_x = istring(luastring_x).c_str();
	luastring_y = istring(luastring_y).c_str();
	luastring_width = istring(luastring_width).c_str();
	luastring_height = istring(luastring_height).c_str();
	LuaSprite_setters[luastring_direction] = [](LuaSprite* sprite, lua_Number value) { sprite->setDirection(value); };
	LuaSprite_setters[luastring_speed] = [](LuaSprite* sprite, lua_Number value) { sprite->setSpeed(value); };
	LuaSprite_setters[luastring_x] = [](LuaSprite* sprite, lua_Number value) { sprite->setPositionX(value); };
	LuaSprite_setters[luastring_y] = [](LuaSprite* sprite, lua_Number value) { sprite->setPositionY(value); };
	

	LuaSprite_getters[luastring_direction] = [](LuaSprite* sprite) {  return sprite->getDirection(); };      
	LuaSprite_getters[luastring_speed] = [](LuaSprite* sprite) {  return sprite->getSpeed(); };
	LuaSprite_getters[luastring_x] = [](LuaSprite* sprite) {  return sprite->getPositionX(); };
	LuaSprite_getters[luastring_y] = [](LuaSprite* sprite) {  return sprite->getPositionY(); };
	LuaSprite_getters[luastring_width] = [](LuaSprite* sprite) {  return sprite->getContentSize().width; };
	LuaSprite_getters[luastring_height] = [](LuaSprite* sprite) {  return sprite->getContentSize().height; };

	luaL_newmetatable(L, LuaEngineMetaTableNames::metaTableName<LuaSprite>());

	luaL_setfuncs(L, LuaSprite_functions, 0);
	lua_pushcfunction(L, LuaSprite__new);
	lua_setglobal(L, "Sprite");
}