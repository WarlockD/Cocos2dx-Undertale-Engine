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
	FontAtlas* _fontAtlas;
	UndertaleLib::UndertaleText _text;
	cocos2d::Sprite* getletter(char16_t ch) ;
	cocos2d::Vector<cocos2d::Sprite*> _letterCache;
	size_t _currentCachePosition;
	size_t _frameDelay;
	size_t _typeingPosition;

	void updateLetters(bool visable);
	size_t _face;
	size_t _emotion;

	bool _instant;
	void changeFace(size_t face);
	void changeEmotion(size_t emotion);
public:
	size_t getFace() const { return _face; }
	size_t getEmotion() const { return _emotion; }
	bool init() override;

	void setFontAtlas(FontAtlas* atlas);
	void setUndertaleFont(size_t font_index);
	static constexpr int object_index = 782;
	static constexpr char* object_name = "OBJ_WRITER";
	obj_writer();
	virtual ~obj_writer();
	void setString(const std::string& text,bool instant=false);
	UndertaleLib::UndertaleText& getParser() { return _text; }
	const UndertaleLib::UndertaleText& getParser() const { return _text; }
	void clear();
	void reset();
	void start();
	void stop();
	void setType(const TEXTTYPE& type);
	virtual void update(float dt) override;
	CREATE_FUNC(obj_writer);
};

