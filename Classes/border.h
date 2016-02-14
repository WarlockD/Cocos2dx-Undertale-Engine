#pragma once
#include "cocos2d.h"
#include "LuaSprite.h"
namespace Undertale {

	/// Box border that is very prevelent in Undertaile
	class Border : public cocos2d::Node {
	protected:
		cocos2d::DrawNode* _box;// it uses spr_border, can we just use DrawNode?
		//LuaSprite* _sides[3]; // it uses spr_border, can we just use DrawNode?
		Border();
		virtual bool init(int x0, int x1, int y0, int y1) ;
		virtual bool init(int width, int height);
		cocos2d::Rect _borderRect;
		void caculateBorders();
	public:
		~Border();
		static Border* create(int x0, int x1, int y0, int y1); // mabye thickness?  its the same thickness eveywhere so mabye not
		static Border* create(int width,int height); // mabye thickness?  its the same thickness eveywhere so mabye not
	};


}
