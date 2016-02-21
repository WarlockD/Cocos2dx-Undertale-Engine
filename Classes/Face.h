#pragma once
#include "cocos2d.h"
#include "LuaSprite.h"
#include "LuaFont.h"

namespace Undertale {
	enum class UndertaleFaces {
		Sans,
		Undyne,
		Alphys,
		Asgore,
		TorielTalk,
		TorielBlink, // do we really need this?  Is she the only charater that blinks?
		Mettaton,
		Papyrus,
		
	};

	/// Face mechanics.  Blink etc
	class Face : public LuaSprite {
	protected:
		cocos2d::Vector<cocos2d::SpriteFrame*> _faceEmotionFrames[10];
		int _faceEmotion;
		UndertaleFaces _currentFace;
		Face();
		bool init(UndertaleFaces face, int faceEmotion);
		void blink();
	public:
		static Face* create(UndertaleFaces face, int faceEmotion);
		int getFaceEmotion() const { return _faceEmotion; }
		UndertaleFaces getFace() const { return _currentFace; }
		void setFaceEmotion(int value);
		void setFace(UndertaleFaces face);
		void setFace(int faceEmotion, UndertaleFaces face);

	};
}