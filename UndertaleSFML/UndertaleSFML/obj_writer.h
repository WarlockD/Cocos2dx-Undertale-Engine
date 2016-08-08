#pragma once
#include "UndertaleLabel.h"

class Face {
public:
	struct FaceState {
		uint32_t face;
		uint32_t emotion;
		uint32_t flag;
		FaceState() : face(0), emotion(0), flag(0) {}
	};
protected:
	FaceState _state;
	virtual uint32_t changeFace(uint32_t newface)=0;
	virtual uint32_t changeEmotion(uint32_t newEmotion)=0;
	virtual uint32_t changeFlag(uint32_t newFlag) = 0;
public:
	Face() : _state() {}
	void setFace(uint32_t i) { if (i != _state.face) { _state.face= changeFace(i); } }
	void setEmotion(uint32_t i) { if (i != _state.emotion) { _state.emotion = changeEmotion(i); } }
	void setFlag(uint32_t i) { if (i != _state.flag) { _state.flag = changeFace(i); } }
	const FaceState& getState() const { return _state; }
	void setState(const FaceState& v) { _state = v; }
	virtual ~Face() {}
};

struct TEXTTYPE {
	int typer;
	int myfont;
	int mycolor;
	int writingx;
	int writingy;
	int writingxend;
	int shake;
	int textspeed;
	int txtsound;
	int spacing;
	int vspacing;
	// default generic
	TEXTTYPE() : typer(4), myfont(2), mycolor(16777215), writingx(20), writingy(20), writingxend(290), shake(0), textspeed(1), txtsound(101), spacing(8), vspacing(18) {}
};

class obj_writer : public UndertaleLabel {
protected:
	virtual void setFace(uint32_t i) { (void)i; }
	virtual void setEmotion(uint32_t i) { (void)i; }
	virtual void setFlag(uint32_t i) { (void)i; }
	std::vector<sf::Vertex> _vertCopy;
	UndertaleLib::UndertaleText _text; // find a way to keep UndertaleLib out of the header.
	UndertaleLib::UndertaleText::const_iterator _pos;
	bool _isTyping;
	sf::Clock _clock;
	TEXTTYPE _config;
	Face* _face;
	sf::Color _currentColor;
	size_t _nextLetterDelay;
	sf::Color _color;
	void do_typing();
	void setAllCharsVisible(bool visible) {
		for (size_t i = 0; i < _verts.size(); i++) {
			_verts[i].color = visible ? _vertCopy[i].color : sf::Color::Transparent;
		}
	}
	void setVisible(size_t char_index, bool visible) {
		size_t index = char_index * 6;
		sf::Color color = visible ? _vertCopy[index].color : sf::Color::Transparent;
		for (size_t i = 0; i < 6; i++) _verts[index + i].color = color;
	}
	void setLastVisible(bool visible) { setVisible(_verts.size() - 6, visible); }
public:
	obj_writer();
	void setConfig(const TEXTTYPE& config= TEXTTYPE());
	void start_typing() { _isTyping = true; }
	virtual void setText(const std::string& text) override;
	virtual void clear() override {
		_vertCopy.clear();
		_isTyping = false;
		_pos = _text.begin();
		_nextLetterDelay = 0;
		_currentColor = _color;
		UndertaleLabel::clear();
	}
	virtual void push_back(int a, const sf::Color& color = sf::Color::White) override {
		UndertaleLabel::push_back(a, color);
		_vertCopy.insert(_vertCopy.end(), _verts.end()-6, _verts.end());
	}
	virtual void pop_back() {
		UndertaleLabel::pop_back();
		_vertCopy.resize(_vertCopy.size() - 6);
	}
	virtual void update(float dt);
};