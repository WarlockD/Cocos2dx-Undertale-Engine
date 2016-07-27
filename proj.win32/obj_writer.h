#pragma once
#include "UObject.h"
#include "UndertaleResourceNode.h"

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
class obj_writer : public UObject
{
private:
	friend class UObject;
	TEXTTYPE _config;
	CC_SYNTHESIZE_RETAIN(FontAtlas*, _fontAtlas, FontAtlas);
	UndertaleLib::UndertaleText _text;
	cocos2d::Vec2 _startWriting;
	cocos2d::Vec2 _writing;
	size_t _lineno;
	UndertaleLib::UndertaleText::const_iterator _current;
	int _frameDelay;
	cocos2d::Color3B _currentColor;
	cocos2d::Sprite* getletter(char16_t ch) const;
	void preFixWriting(char16_t ch);
	void newLine();
	void postFixWriting(char16_t ch);
	
public:
	bool init() override;
	static constexpr int object_index = 782;
	static constexpr char* object_name = "OBJ_WRITER";
	obj_writer();
	virtual ~obj_writer();
	void setString(const std::string& text);
	UndertaleLib::UndertaleText& getParser() { return _text; }
	const UndertaleLib::UndertaleText& getParser() const { return _text; }
	void reset();
	void start();
	void stop();
	void setType(const TEXTTYPE& type);
	virtual void update(float dt) override;
	CREATE_FUNC(obj_writer);
};

