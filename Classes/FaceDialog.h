#pragma once
#include "cocos2d.h"
#include "LuaSprite.h"
#include "border.h"
#include "LuaFont.h"

namespace Undertale {

	/// Box border that is very prevelent in Undertaile
	class FaceDialog : public cocos2d::Node {
	protected:
	//	FaceDialog();
		LuaSprite* _faceSprite;
		LuaLabel* _textLabel;
		Border* _border;
		istring _font;
		cocos2d::Color3B _color;
		cocos2d::Vec2 _writing;
		float _writingend;
		int _shake;
		int _textspeed;
		istring _textsound;
		virtual bool init(istring font, cocos2d::Color3B color, float writingx, float writingy, float writingend, int shake, int textspeed, istring textsound);
	public:
	//	~FaceDialog();
		static FaceDialog* create(istring font, cocos2d::Color3B color, float writingx, float writingy, float writingend, int shake, int textspeed,istring textsound); // mabye thickness?  its the same thickness eveywhere so mabye not
		void setString(const std::string& text);
	};


}
