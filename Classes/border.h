#pragma once
#include "cocos2d.h"
#include "LuaSprite.h"
namespace Undertale {

	/// Box border that is very prevelent in Undertaile
	class Border : public cocos2d::Sprite {
	protected:
		cocos2d::DrawNode* _box;// it uses spr_border, can we just use DrawNode?
		//LuaSprite* _sides[3]; // it uses spr_border, can we just use DrawNode?
		Border();
		virtual bool init(const int* r) ;
		int _ideal[3];
		void caculateBorders();
	public:
		~Border();
		static Border* create(const int r[4]); // mabye thickness?  its the same thickness eveywhere so mabye not
	};


}
