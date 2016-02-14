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
	static inline float makeFrameTime(uint32_t value) { return (float)value * (1.0f / 30.0f); }
	enum class HaltType {
		Paused,
		Typing,
		WaitingOnKeyPress,
		DoneTyping
	};
	enum class TokenType {
		NotSet,
		Event,
		Sprite,
		Halt,
		Speed,
		Choise
	};
	// not alot of error checking in this class so be carful
	class LabelToken {
		union ValueUnion {
			struct EventStruct {
				cocos2d::EventCustom* event;
				void* userData;
			} event;
			struct SpriteStruct {
				char16_t ch;
				LuaLabelSprite* sprite;
			} sprite;
			HaltType halt;
			uint32_t speed;
			int choise; // for latter
			ValueUnion() : event({ 0, 0 }) {}
			ValueUnion(char16_t ch, LuaLabelSprite* sprite) : sprite({ ch, sprite }) {}
			ValueUnion(HaltType halt) : halt(halt) {}
			ValueUnion(uint32_t speed) : speed(speed) {}
			ValueUnion(void* userData, cocos2d::EventCustom* event) : event({ event, userData }) {}
		} _value;
		TokenType _type;
	public:
		LabelToken() : _value(), _type(TokenType::NotSet)  {}
	
		LabelToken(char16_t ch, LuaLabelSprite* sprite) : _value(ch, sprite), _type(TokenType::Sprite){}
		LabelToken(HaltType halt) : _value(halt), _type(TokenType::Halt) {}
		LabelToken(uint32_t speed) : _value(speed), _type(TokenType::Speed) {}
		LabelToken(void* userData, cocos2d::EventCustom* event) : _value(userData, event), _type(TokenType::Event) {}
		inline TokenType type() const { return _type; }
		inline uint32_t getSpeed() const { return _value.speed; }
		inline LuaLabelSprite* getSprite() const { return  _value.sprite.sprite; }
		inline void dispatchEvent(cocos2d::Node* node) const {
			_value.event.event->setUserData(_value.event.userData);
			node->getEventDispatcher()->dispatchEvent(_value.event.event);
		}
		inline HaltType getHalt() const { return  _value.halt; }
		inline int getChar() const { return  _value.sprite.ch; }
	};
	std::string _originalString;
	std::u16string _utf16Text;

	std::vector<std::vector<LabelToken>> _letters;
	cocos2d::Vector<LuaLabelSprite*> _letterCache;
	cocos2d::FontAtlas* _atlas;
	cocos2d::EventListenerKeyboard* _keyboardListener;
	int _currentTextPos;
	int _currentLine;
	uint32_t _textSpeed;
	float _currentTime;
	float _shakeTimer;
	float _currentShakeTimer;
	cocos2d::EventCustom _facemotion_event;
	cocos2d::EventCustom _facemovement_event;
	cocos2d::EventListenerKeyboard* _onKeyPressEvent;
	HaltType _halt;
	uint32_t _typesound;
	istring _typingSound;
	virtual void updateContent(bool hideLetters);
	bool _needsUpdate;
	void nextLine();
	LuaLabel();
	// set that each frame is 30 times a second
	inline void setCurrentTime(uint32_t speed) { _currentTime = makeFrameTime(speed); }
	void setHalt(HaltType halt);
	HaltType getHalt() const { return _halt; }
	uint32_t _hspacing;
	uint32_t _vspacing;
	float _end_x;
public:
	CC_SYNTHESIZE(float, _shake, Shake);
public:
	virtual ~LuaLabel();
	static LuaLabel* create(istring font, uint32_t speed, float shake);
	void clear();
	virtual bool init() { return Node::init(); }
	virtual bool init(istring font, cocos2d::Color3B color, float x, float y, float end_x, uint32_t shake, istring sound, uint32_t hspacing, uint32_t vspacing);
	inline virtual bool isPasued() const { return  HaltType::WaitingOnKeyPress == _halt || HaltType::Paused == _halt; }
	inline virtual bool isWaitingOnKeyPress() const { 
		return  _halt == HaltType::WaitingOnKeyPress ; 
	}
	virtual void restartTyping();
	virtual void setSpacing(uint32_t hspacing, uint32_t vspacing) {
		_hspacing = hspacing;  _vspacing = vspacing; _needsUpdate
			= true; updateContent(false);
	}
	virtual void setTypingSpeed(uint32_t speed);
	virtual void setTypingSound(istring soundFile);
	virtual void setString(const std::string& text, bool startTyping=true) ;
	virtual void update(float dt) override;
	
//	virtual void updateLabelLetters() override;
};