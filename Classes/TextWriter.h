#pragma once

#include "cocos2d.h"

class TextWriter : public cocos2d::Label
{
	
protected:
	std::vector<const char*> _textList;
	uint32_t _textListPos;
	std::string _text; // original text
	std::string _processed; // the text currently in label
	cocos2d::Point _writing; // location of current letter
	cocos2d::Label* _label; // important, this is NOT a child, it does NOT get drawn
	uint32_t _lineno;	// current line number
	uint32_t _pos;		// position in text
	enum class State {
		NotWriting, // Not displaying anymore
		Writting, // we are typing line normal
		Paused, // paused typing, like waiting for a key
		Halted, // means we stopped and restarting clears the screen
	};
	State _state;
	float _start_time;
	float _speed;
	float _text_speed;
	float _current_text_speed;
	cocos2d::Sprite* _facemotion;
	std::vector<cocos2d::SpriteFrame*> _facemotionframes;
	float _direction;
	int _halt;
	// leet lexing skills here
	
	CC_SYNTHESIZE(int, _shake, Shake);
public:
	TextWriter():_textListPos(0),_text(""),_processed(""),_writing(0,0),_label(nullptr),_lineno(0),_pos(0),_state(State::NotWriting),_start_time(0.0f), _text_speed(500),_direction(2),_shake(10) {}
	virtual ~TextWriter() {}
	//CREATE_FUNC(TextWriter);
	//static TextWriter* createWithTTF(const std::string& text, const std::string& fontFile, float fontSize, const cocos2d::Size& dimensions /* = Size::ZERO */, cocos2d::TextHAlignment hAlignment /* = TextHAlignment::LEFT */, cocos2d::TextVAlignment vAlignment /* = TextVAlignment::TOP */);
//	static TextWriter* createWithBMFont(const std::string& bmfontFilePath, const std::string& text, const cocos2d::TextHAlignment& alignment /* = TextHAlignment::LEFT */, int maxLineWidth /* = 0 */, const cocos2d::Vec2& imageOffset /* = Vec2::ZERO */);
	//void update(float) override;
//	void start(); //

	inline void halt() { _state = State::Halted; }
	static TextWriter* createWithBMFont(const std::string& bmfontFilePath, const std::string& text, int shake);
	void update(float f) override;
	void startTyping();
	
private:
	cocos2d::Sprite* sprite;
};

