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
 bool LuaSprite::init(cocos2d::Vector<cocos2d::SpriteFrame*>* frame) {
	 _frames = frame;
	 _speed = 0.0f;
	 _direction = 0.0f;
	 _image_index = 0;
	 _frameCount = frame->size();
	 return initWithSpriteFrame(frame->at(0));
}
LuaSprite * LuaSprite::create(cocos2d::Vector<cocos2d::SpriteFrame*>* frame)
{
	LuaSprite* pSprite = new LuaSprite();
	if (pSprite && pSprite->init(frame)) {
		pSprite->autorelease();
		return pSprite;
	}
	CC_SAFE_DELETE(pSprite);
	return nullptr;
}


void LuaSprite::update(float dt)
{
	if (stepBullet(dt)) return;
	if (!_movementVector.isZero()) {
		Vec2 current = this->getPosition();
		current += _movementVector;
		this->setPosition(current);
	}
	if ( _image_speed > 0) {
		_current_image_time += _image_speed;
		if (abs(_current_image_time) >= 1.0f) {
			if (_current_image_time > 0) {
				if ((++_image_index) >= _frameCount) _image_index = 0;
			}
			else {  // unsigned numbers don't go to -1 ugh
				if ((--_image_index) >= _frameCount) _image_index = _frameCount-1;
			}
			_current_image_time = 0.0f;
			setSpriteFrame(_frames->at(_image_index));
		}
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
static const char* luastring_image_index= "image_index";
static const char* luastring_image_speed = "image_speed";

static std::unordered_map<const char*, std::function<int(lua_State*,LuaSprite*)>> LuaSprite_setters;
static std::unordered_map<const char*, std::function<int(lua_State*, LuaSprite*)>> LuaSprite_getters;

static int LuaSprite__newindex(lua_State* L) {
	LuaSprite* sprite = LUA_SPRITE_TEST(L, 1);
	const char* index = luaL_checkstring(L, 2);
	//lua_Number value = luaL_checknumber(L, 3);
	auto it = LuaSprite_setters.find(index);
	if(it == LuaSprite_setters.end()) return luaL_error(L, "Invalid property '%'s' of LuaSprite",index);
	return it->second(L,sprite);
}
static int LuaSprite__index(lua_State* L) {
	LuaSprite* sprite = LUA_SPRITE_TEST(L, 1);
	const char* index = luaL_checkstring(L, 2);
	auto it = LuaSprite_getters.find(index);
	if (it == LuaSprite_getters.end()) return luaL_error(L, "Invalid property '%'s' of LuaSprite", index);
	return it->second(L, sprite);
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

	LuaSprite* sprite = LuaSprite::create(frames);
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
	luastring_image_index = istring(luastring_image_index).c_str();
	luastring_image_speed = istring(luastring_image_speed).c_str();

	LuaSprite_setters[luastring_direction] = [](lua_State* L, LuaSprite* sprite) { sprite->setDirection(luaL_checknumber(L,3));	return 0; };
	LuaSprite_setters[luastring_speed] = [](lua_State* L, LuaSprite* sprite) { sprite->setSpeed(luaL_checknumber(L, 3));	return 0; };
	LuaSprite_setters[luastring_x] = [](lua_State* L, LuaSprite* sprite) { sprite->setPositionX(luaL_checknumber(L, 3));	return 0; };
	LuaSprite_setters[luastring_y] = [](lua_State* L, LuaSprite* sprite) { sprite->setPositionY(luaL_checknumber(L, 3));	return 0; };
	LuaSprite_setters[luastring_image_index] = [](lua_State* L, LuaSprite* sprite) { sprite->setImageIndex((size_t)luaL_checkinteger(L,3)); 	return 0; };
	LuaSprite_setters[luastring_image_speed] = [](lua_State* L, LuaSprite* sprite) { sprite->setImageSpeed((size_t)luaL_checknumber(L, 3)); 	return 0; };

	LuaSprite_getters[luastring_direction] = [](lua_State* L, LuaSprite* sprite) {  lua_pushnumber(L,sprite->getDirection()); 	return 1; };
	LuaSprite_getters[luastring_speed] = [](lua_State* L, LuaSprite* sprite) {  lua_pushnumber(L, sprite->getSpeed()); return 1; };
	LuaSprite_getters[luastring_x] = [](lua_State* L, LuaSprite* sprite) {  lua_pushnumber(L, sprite->getPositionX()); return 1; };
	LuaSprite_getters[luastring_y] = [](lua_State* L, LuaSprite* sprite) {  lua_pushnumber(L, sprite->getPositionY()); return 1; };
	LuaSprite_getters[luastring_width] = [](lua_State* L, LuaSprite* sprite) {  lua_pushnumber(L, sprite->getContentSize().width); return 1; };
	LuaSprite_getters[luastring_height] = [](lua_State* L, LuaSprite* sprite) {  lua_pushnumber(L, sprite->getContentSize().height); return 1; };
	LuaSprite_setters[luastring_image_index] = [](lua_State* L, LuaSprite* sprite) { lua_pushinteger(L,sprite->getImageIndex()); return 1; };
	LuaSprite_setters[luastring_image_speed] = [](lua_State* L, LuaSprite* sprite) { lua_pushnumber(L, sprite->getImageSpeed()); return 1; };

	luaL_newmetatable(L, LuaEngineMetaTableNames::metaTableName<LuaSprite>());

	luaL_setfuncs(L, LuaSprite_functions, 0);
	lua_pushcfunction(L, LuaSprite__new);
	lua_setglobal(L, "Sprite");
}