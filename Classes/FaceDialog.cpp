#include "FaceDialog.h"

USING_NS_CC;

namespace Undertale {

	bool FaceDialog::init(istring font, cocos2d::Color3B color, float writingx, float writingy, float writingend, int shake, int textspeed, istring textsound)
	{
		if (Node::init()) {
			_font = font;
			_color = color;
			_writing = Vec2(writingx, writingy);
			_writingend = writingend;
			_shake = shake;
			_textspeed = textspeed;
			_textsound = textsound;
	
			_border = Border::create(288, 75); // dialogueBox 0 570, 135 
			addChild(_border, 0);
			_contentSize = _border->getContentSize();

			_textLabel = LuaLabel::create(font, textspeed, shake);
			_textLabel->setTypingSound(textsound);
			
			_textLabel->setPosition(-288/2+ writingx, 75/2- writingy);
			addChild(_textLabel, 1);
			
			_faceSprite = nullptr;
			return true;
		}
		return false;
	}

	FaceDialog * FaceDialog::create(istring font, cocos2d::Color3B color, float writingx, float writingy, float writingend, int shake, int textspeed, istring textsound)
	{
		FaceDialog* o = new FaceDialog();
		if (o && o->init(font, color, writingx, writingy, writingend, shake, textspeed, textsound)) {
			o->autorelease();
			return o;
		}
		return nullptr;
	}
	void FaceDialog::setString(const std::string& text) {
		_textLabel->setString(text);
	}
}