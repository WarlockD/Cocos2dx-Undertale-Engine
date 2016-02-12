#pragma once

#include "cocos2d.h"
#include "LuaSprite.h"
namespace Undertale {


	class obj_gasterblaster : public LuaSprite {
		int con;
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
		static obj_gasterblaster* create();
		virtual void  setupBullet(cocos2d::Vec2 pos) override;
		virtual void fireBullet()  override { LuaSprite::fireBullet(); }
		virtual bool stepBullet(float dt)  override;// this step is run at the start of update, on true return, update dosn't run this frame.
	//	virtual void stopBullet()  override;
	};


}