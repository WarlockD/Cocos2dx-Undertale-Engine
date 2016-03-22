#pragma once

#include "cocos2d.h"
#include "LuaSprite.h"
namespace Undertale {


	class obj_gasterblaster : public cocos2d::Node {
		int con;
		MovementAction* _runAwayAction;
		const cocos2d::Vector<cocos2d::SpriteFrame*>* _gasterSpriteFrames;
		int _image_index;
		cocos2d::Sprite* _gasterSprite;
		cocos2d::DrawNode *blaster[3];
		cocos2d::Vec2 ideal;
		int alarm;
		int p_beam;
		int idealrot;

			int skip;
			int pause;
			int col_o;
		float  bt;
			int btimer;
			float fade;
			int terminal;
			int bb;
			int bbsiner;
			int innate_karma;
		//	519.self.p_power = 1
			obj_gasterblaster();
	public:
		void setSkip(bool value) { skip = value ? 1 : 0; }
		virtual bool init() override;
		static obj_gasterblaster* create(float x, float y);
		static obj_gasterblaster* create(const cocos2d::Vec2 pos = cocos2d::Vec2 ::ZERO);
		void fireBullet(cocos2d::Vec2 pos, float angle);
		virtual void update(float dt)  override;// this step is run at the start of update, on true return, update dosn't run this frame.
	//	virtual void stopBullet()  override;
	};


}