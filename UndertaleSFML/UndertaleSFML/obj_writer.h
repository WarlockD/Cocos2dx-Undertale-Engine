#pragma once
#include "Global.h"
#include "UndertaleLabel.h"

class obj_face : public sf::Transformable, public sf::Drawable, public Component<774, obj_face> {
protected:
	uint32_t _face;
	uint32_t _emotion;
	std::vector<UndertaleSprite> _sprites;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	obj_face() : _face(0), _emotion(0) {}
public:
	static std::unique_ptr<obj_face> getFace(uint32_t index);
	size_t getFace() const { return _face; }
	size_t getEmotion() const { return _emotion; }
	virtual void setEmotion(uint32_t i) = 0;
	virtual ~obj_face() {}
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

class obj_writer : public UndertaleLabel, public sf::Transformable, public sf::Drawable,  public Component<782, obj_writer> {
protected:
	virtual void faceCallback(int v) {}
	virtual void emotionCallback(int v) {}
	virtual void flagCallback(int v) {}
	virtual void haltFCallback(int v) {}
	virtual void typerCallback(int v) {}

	std::vector<sf::Vertex> _vertCopy;
	UndertaleLib::UndertaleText _text; // find a way to keep UndertaleLib out of the header.
	UndertaleLib::UndertaleText::const_iterator _pos;
	bool _isTyping;
	sf::Time _frameTime;
	TEXTTYPE _config;
	sf::Color _currentColor;
	size_t _nextLetterDelay;
	sf::Color _color;
	void do_typing();
	void setAllCharsVisible(bool visible) {
		for (size_t i = 0; i < _textVerts.size(); i++) {
			_textVerts[i].color = visible ? _vertCopy[i].color : sf::Color::Transparent;
		}
	}
	void setVisible(size_t char_index, bool visible) {
		size_t index = char_index * 6;
		sf::Color color = visible ? _vertCopy[index].color : sf::Color::Transparent;
		for (size_t i = 0; i < 6; i++) _textVerts[index + i].color = color;
	}
	void setLastVisible(bool visible) { setVisible(_textVerts.size() - 6, visible); }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	//obj_writer(const obj_writer& copy) = default; // nothing special here, do default copy of eveything
public: 
	obj_writer();
	virtual void setConfig(const TEXTTYPE& config= TEXTTYPE());
	virtual void start_typing() { _isTyping = true; }
	virtual void stop_typing() { _isTyping = false; }
	virtual void setText(const std::string& text) ;
	virtual void clear()  {
		_vertCopy.clear();
		_isTyping = false;
		_pos = _text.begin();
		_nextLetterDelay = 0;
		_frameTime = sf::Time::Zero;
		_currentColor = _color;
		UndertaleLabel::clear();
	}
	virtual void push_back(int a, const sf::Color& color = sf::Color::White) override {
		UndertaleLabel::push_back(a, color);
		_vertCopy.insert(_vertCopy.end(), _textVerts.end()-6, _textVerts.end());
	}
	virtual void pop_back() {
		UndertaleLabel::pop_back();
		_vertCopy.resize(_vertCopy.size() - 6);
	}
	virtual void update(sf::Time dt) override;
	
};