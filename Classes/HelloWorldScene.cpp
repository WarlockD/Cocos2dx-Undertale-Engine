#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "../proj.win32/UndertaleResourceNode.h"
#include "../proj.win32/UObject.h"
#include "../proj.win32/obj_dialoguer.h"
USING_NS_CC;

#ifdef _DEBUG
UndertaleLabel* s_debugText = nullptr;

#endif

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
   // auto scene = Scene::create();
	auto scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}
void debugWriter(obj_dialoguer* writer) {
	writer->setFace(1);
	writer->addString("\\E2* Greetings,^1 my child^2.&* Do not worry^1, I did&  not leave you./");
	writer->addString("\\E0* I was merely behind this&  pillar the whole time./");
	writer->addString("* Thank you for trusting&  me./");
	writer->addString("\\E2* However^1, there was an&  important reason for&  this exercise./");
	writer->addString("* ... to test your&  independence./");
	writer->addString("\\E1* I must attend to some&  business^1, and you must&  stay alone for a while./");
	writer->addString("\\E0* Please remain here^2.&*\\E1 It\'s dangerous to&  explore by yourself./");
	writer->addString("\\E0* I have an idea^2.&* I will give you a&  CELL PHONE./");
	writer->addString("* If you have a need for&  anything^1, just call./");
	writer->addString("\\E1* Be good^1, alright?/%");
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

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
  //  auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);

	

  
	static size_t ufont = 2;
	static float shake = 37.0f;
	auto debugtouch = _debugTouch = s_debugText = UndertaleLabel::create(ufont);

	//auto debugtouch = _debugTouch = s_debugText=Label::createWithTTF("empty", "fonts/8bitoperator_jve.ttf", 18);

	
	auto label = UndertaleLabel::create(ufont);
	_debugTouch->setPosition(Vec2(origin.x, visibleSize.height - label->getLineHeight() + origin.y));
	_debugTouch->setString("NoTouch");
	_debugTouch->setPosition(origin.x, visibleSize.height + origin.y);
	_debugTouch->setAnchorPoint(Vec2(0.0, 1.0));
	label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height - label->getLineHeight() + origin.y));
	this->addChild(label, 10);
	this->addChild(_debugTouch, 10);
	
//	label->setString("BOW TO \\YME!");
	// position the label on the center of the screen
	//label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - label->getContentSize().height));
	//label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	// add the label as a child to this layer
	//this->addChild(label, 1);

	URoom* uroom = URoom::create(35);
	_currentRoom = uroom;

	// obj_dialoguer::create();
	obj_dialoguer* writer = dynamic_cast<obj_dialoguer*>(uroom->instance_create(10, 10, obj_dialoguer::object_index));
	writer->setFace(1);
	debugWriter(writer);
	writer->startDialog();
	writer->setLocalZOrder(10000);
	writer->setPosition(0.0f, 400.0f);
	uroom->setPosition(0.0f, 0.0f);
	//uroom->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	this->addChild(uroom, -10);
	uroom->setScale(2.0f, 2.0f);

	label->setString(uroom->getDebugName());
	
	_dragingView = false;
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = [uroom](Touch* touch, Event* event) -> bool {
		return true;
	//	Vec2 pos = touch->getLocationInView() - touch->getPreviousLocationInView();
	//	pos.y *= -1.0f;
	//	pos += uroom->getPosition();
	//	uroom->setPosition(pos);
	};
	touchListener->onTouchMoved = [uroom](Touch* touch, Event* event) {
		Vec2 pos = touch->getLocationInView() - touch->getPreviousLocationInView();
		pos.y *= -1.0f;
		pos += uroom->getPosition();
		uroom->setPosition(pos);
	};
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);


	auto moseListener = EventListenerMouse::create();
	/*
	
	moseListener->onMouseMove = [uroom](EventMouse* event) {
		if (event->getMouseButton() == 0) {
			Vec2 pos = event->getDelta(); //event->getLocationInView() - event->getPreviousLocationInView();
			pos.y *= -1.0f;
		
			pos += uroom->getPosition();
			uroom->setPosition(pos);
		}
	};
	*/
	moseListener->onMouseScroll = [debugtouch,uroom](EventMouse* event) {
		float scale = uroom->getScale();
		float scroll = event->getScrollY();
		if (scroll > 0.0f) uroom->setScale(scale + 0.2f);
		else if(scroll < 0.0f) uroom->setScale(scale - 0.2f);
	};
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(moseListener, this);




	//sprite->setScale(4.0f);
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = [uroom,label](EventKeyboard::KeyCode keyCode, Event* event) {

		Vec2 loc = event->getCurrentTarget()->getPosition();
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			ufont++;
			s_debugText->setUndertaleFont(ufont);
		//	event->getCurrentTarget()->setPosition(--loc.x, loc.y);
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			ufont--;
			s_debugText->setUndertaleFont(ufont);
		//	event->getCurrentTarget()->setPosition(++loc.x, loc.y);
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
			//shake++;
			//label->setShake(shake);
			uroom->nextRoom();
			uroom->setPosition(0.0f, 0.0f);
			label->setString(uroom->getDebugName());
		//	event->getCurrentTarget()->setPosition(loc.x, ++loc.y);
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			//shake--;
			//label->setShake(shake);
			uroom->previousRoom();
			uroom->setPosition(0.0f, 0.0f);
			label->setString(uroom->getDebugName());
		//	event->getCurrentTarget()->setPosition(loc.x, --loc.y);
			break;
		case EventKeyboard::KeyCode::KEY_1:
			for (auto o : uroom->getObjects()) o->setDrawNonVisiableSprite(!o->getDrawNonVisiableSprite());
			break;
		case EventKeyboard::KeyCode::KEY_2:
			for (auto o : uroom->getObjects()) o->setDrawIndex(!o->getDrawIndex());
			break;
		case EventKeyboard::KeyCode::KEY_3:
			for (auto o : uroom->getObjects()) o->setDrawBox(!o->getDrawBox());
			break;

		}
	};

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);


    // add "HelloWorld" splash screen"
   // auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
  //  sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
   // this->addChild(sprite, 0);
    
    return true;
}
/*

bool HelloWorld::onTouchBegan(Touch* touch, Event* event) {
	HelloWorld* self = dynamic_cast<HelloWorld*>(event->getCurrentTarget());
	auto bounds = event->getCurrentTarget()->getBoundingBox();
	//if (!self && bounds.containsPoint(touch->getLocation())) {
		std::stringstream touchDetails;
		touchDetails << "Touched at OpenGL coordinates: " <<
			touch->getLocation().x << "," << touch->getLocation().y << std::endl <<
			"Touched at UI coordinate: " <<
			touch->getLocationInView().x << "," << touch->getLocationInView().y << std::endl <<
			"Touched at local coordinate:" <<
			event->getCurrentTarget()->convertToNodeSpace(touch->getLocation()).x << "," <<
			event->getCurrentTarget()->convertToNodeSpace(touch->getLocation()).y << std::endl <<
			"Touch moved by:" << touch->getDelta().x << "," << touch->getDelta().y;
		_debugTouch->setString(touchDetails.str().c_str());
	//}
	return true;

}
*/

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}
