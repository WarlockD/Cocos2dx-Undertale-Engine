#pragma once
#include "cocos2d.h"
#include "LuaEngine.h"
// Keeping this to remind myself, but basicly cocos2d does not make it easy to subclass your own Font and get Label to work proerly with it

class LuaFontConfiguration;
class LuaFont : public cocos2d::Font {
	LuaFontConfiguration* _config;
	LuaFont();
public:
	static LuaFont* create(istring fontName);
	virtual cocos2d::FontAtlas* createFontAtlas() override;
	virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const override;
	virtual int getFontMaxHeight() const { return 0; }
};
class LuaLabelSprite;
class LuaLabel : public cocos2d::Node {
protected:
	std::string _originalString;
	std::u16string _utf16Text;

	std::vector<cocos2d::Vector<LuaLabelSprite*>> _letters;
	cocos2d::Vector<LuaLabelSprite*> _letterCache;
	cocos2d::FontAtlas* _atlas;
	cocos2d::EventListenerKeyboard* _keyboardListener;
	int _currentTextPos;
	int _currentLine;
	float _textSpeed;
	float _currentTime;
	float _shakeTimer;
	float _currentShakeTimer;
	int _halt;
	int _typesound;
	istring _typingSound;
	virtual void updateContent(bool hideLetters);
	bool _needsUpdate;
	void nextLine();
	CC_SYNTHESIZE(float, _shake, Shake);
public:
	static LuaLabel* create(istring font, float speed);
	virtual void setTypingSpeed(float speed);
	virtual void setTypingSound(istring soundFile);
	virtual void setString(const std::string& text) ;
	virtual void update(float dt) override;
	
//	virtual void updateLabelLetters() override;
};