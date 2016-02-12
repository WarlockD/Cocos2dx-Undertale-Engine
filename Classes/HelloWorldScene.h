#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "LuaEngine.h"
class LuaSprite;
class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
	LuaSprite* testSprite;
    virtual bool init();
	int lua_startup(lua_State*L);
	int lua_step(lua_State*L);
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
	virtual void update(float dt) override;
};

#endif // __HELLOWORLD_SCENE_H__
