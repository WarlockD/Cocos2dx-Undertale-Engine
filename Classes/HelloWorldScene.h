#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "proj.win32/UndertaleResourceNode.h"

class HelloWorld : public cocos2d::Layer
{
protected:
	bool _dragingView;
	
	cocos2d::Vec2 _dragOffset;
	cocos2d::Label* _debugTouch;
	URoom* _currentRoom;
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__
