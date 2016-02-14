#include "HelloWorldScene.h"
#include "LuaEngine.h"
#include "UndertaleResources.h"
#include "lua.hpp"
#include "LuaFont.h"
#include "AudioEngine.h"
#include "border.h"
#include "obj_gasterblaster.h"
#include "FaceDialog.h"

USING_NS_CC;
using namespace experimental;
static int tags = 1001;
static HelloWorld* lua_hack = nullptr;
/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int lua_report(lua_State *L, int status) {
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		CCLOGERROR("%s\n", msg);
		lua_pop(L, 1);  /* remove message */
	}
	return status;
}

static int lua_setPos(lua_State*L) {
	int tag = luaL_checkinteger(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_hack->getChildByTag(tag)->setPosition(x, y);
	return 0;
}
static int lua_movePos(lua_State*L) {
	int tag = luaL_checkinteger(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	Node* node = lua_hack->getChildByTag(tag);

	Vec2 pos = node->getPosition() + Vec2(x, y);
	node->setPosition(pos);
	return 0;
}

static int lua_addsprite(lua_State* L) {
	const char* spriteName =  luaL_checkstring(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	int frame = luaL_checkinteger(L, 4);
	//Size visibleSize = Director::getInstance()->getVisibleSize();
	//Vec2 origin = Director::getInstance()->getVisibleOrigin();
	UndertaleResources* res = UndertaleResources::getInstance();
	Sprite* sprite = res->getInstance()->createSprite(spriteName, frame);
	sprite->setPosition(x, y);
	sprite->setTag(tags);
	lua_pushinteger(L, tags++);
	lua_hack->addChild(sprite, 1);
	return 1;
}
Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	UndertaleResources* res = UndertaleResources::getInstance();
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
 //   auto closeItem = MenuItemImage::create(
  //                                         "CloseNormal.png",
   //                                        "CloseSelected.png",
    //                                       CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
	bool mute = false;
	int musicID;
	if (AudioEngine::lazyInit()) {

	//	auto musicID = AudioEngine::play2d("music\\mus_zz_megalovania.ogg", true);
		//AudioEngine::setVolume(musicID, 0.25f);
	}
	/*

	LuaSprite* test2 = LuaSprite::create("spr_border", 200, 200);
	addChild(test2, 1);
	*/

	//auto box = Undertale::Border::create(32,602,250,385); // dialogueBox 0 570, 135
	//auto box = Undertale::Border::create(570,135); // dialogueBox 0 570, 135
	//auto box = Undertale::Border::create(217, 417, 180, 385); // standard box 1
	//auto box = Undertale::Border::create(217, 417, 125, 385); // tower box 2
	//auto box = Undertale::Border::create(237, 397, 250, 385); // // small box 3
	//auto box = Undertale::Border::create(267, 367, 295, 385); // // claustrophobic box 4
	//auto box = Undertale::Border::create(192, 442, 250, 385); // //  wide-small box 5
	//auto box = Undertale::Border::create(227, 407, 250, 385); // //  slightly bigger box PREP 6
	//auto box = Undertale::Border::create(227, 407, 200, 385); // //  slightly bigger box TALL 7
	//auto box = Undertale::Border::create(202, 432, 290, 385); // //  short box 8
//	addChild(box,1);
//	box->setPosition(640/2,385);
	//box->setAnchorPoint(Vec2(0, 0));
#if 0
	

//	auto closeItem = MenuItemImage::create("CloseNormal.png","CloseSelected.png", [](Ref* r) {  Director::getInstance()->end(); });
	auto muteItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png", [this,test2](Ref* r) {
		test2->setSpeed(test2->getSpeed()+1);
	});
	
	//closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,  origin.y + closeItem->getContentSize().height/2));
	muteItem->setPosition(Vec2(origin.x + visibleSize.width - muteItem->getContentSize().width / 2,
		origin.y + muteItem->getContentSize().height / 2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(muteItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
#endif
	

//	auto label = LuaLabel::create("fnt_main.fnt", 2,0.5f);
	
	const char* test_str = "\\E2* If you truly wish to&  leave the RUINS.../* I will not stop you./\\E2* However^1, when you&  leave.../\\E1* Please do not come&  back./\\E2* I hope you understand./%%";
	const char* test_str2 = "\XLa, la.^3 &Time to wake&up and\\R smell\\X &the^4 pain./* Though^2.^4.^6.^8.&It's still a&little shaky./* Though^2.^4.^6.^8.&It's still a&little shaky./fhuehfuehfuehfuheufhe / %";
	Undertale::FaceDialog* face = Undertale::FaceDialog::create("fnt_main.fnt", Color3B::WHITE, 20, 20, 290, 0, 1, "snd_txtasr.wav");
	face->setPosition(640 / 2, 385);
	addChild(face);
	face->setString(test_str);
		/*
	label->setPosition(200, 200);
		label->setTypingSound("snd_txtasr.wav");
		this->addChild(label, 1);
		label->setString(test_str2); // "facemotionEvent"), _facemovement_event("facemovementEvent")
		EventListenerCustom* listener = EventListenerCustom::create("facemotionEvent", [=](EventCustom* event) {
			CCLOG("FaceMotionEvent : %i", (int)(event->getUserData()));
		});

		_eventDispatcher->addEventListenerWithFixedPriority(listener, 1);
		listener = EventListenerCustom::create("facemovementEvent", [=](EventCustom* event) {
			CCLOG("FaceMovementEvent : %i", (int)(event->getUserData()));
		});
		_eventDispatcher->addEventListenerWithFixedPriority(listener, 1);
		label->restartTyping();
		*/

//	LuaEngine::setLuaScene(this);
//	lua_State* L = LuaEngine::getLuaState();

//	int ret = luaL_loadfile(L, "D:\\luascripts\\startup.lua");
//	if (LuaEngine::DoFile("D:\\luascripts\\startup.lua")) {
		//int ret = luaL_loadstring(L, "local tag = UndertaleSprite(\"spr_doglick\",200,200,4)\n\nfunction loop(dt)\n MoveSprite(tag,1,1)\n end\n ");
		//	assert(ret == 0);
		//	lua_pcall(L, 0, 1, 0);
	//	this->scheduleUpdate();

	

	//mus_zz_megalovania.ogg

	return true;
}

int HelloWorld::lua_startup(lua_State * L)
{
	return 0;
}

int HelloWorld::lua_step(lua_State * L)
{
	return 0;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void HelloWorld::update(float dt)
{
//	lua_State* L = LuaEngine::getLuaState();
//	lua_getglobal(L, "loop");
//	if(!LuaEngine::RunGlobalUpdate("loop",dt)) this->unscheduleUpdate();
//	lua_pushnumber(L, dt);
	//int status = lua_pcall(L, 1, 0, 0);
//	if (lua_report(L, status) != LUA_OK) this->unscheduleUpdate();
	
	//	lua_call(L, 1, 0);
}
