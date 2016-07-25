#pragma once
#include "cocos2d.h"
#include "UndertaleLib.h"

USING_NS_CC;



/*
class UndertaleLabel : : public Node, public LabelProtocol {
	std::string _text;

	UndertaleLib::Font _configuration;
	float _fontSize;
	float _scale; /// since the font size is fixed, we have to addust the scaling
	std::unordered_map<size_t, int> _kernings; // cheap kerning lookup
	UndertaleFont(UndertaleLib::Font configuration) : _configuration(configuration) {}
public:
	static UndertaleLabel* create(size_t font_index, onst std::string& text);
	virtual ~UndertaleFont();
	static UndertaleFont* create(size_t index);
	virtual FontAtlas* createFontAtlas();
	virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const override;
	int getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const;
	void setFontSize(float fontSize);
};
*/

class UndertaleLabel : public Label {
protected:
	UndertaleLib::UndertaleText _parser;
	std::vector<Color4B> _leterColors;
	std::vector<V3F_C4B_T2F_Quad> _quadOrigin;
	float _shake;
	float _direction;
	float _speed;
	virtual bool init() override;
public:
	UndertaleLabel();
	static UndertaleLabel* create(size_t font_index);
	void setUndertaleFont(size_t font_index);
	virtual void setString(const std::string& text) override;
	virtual void updateColor() override;
	virtual void setShake(float shake);
	float getShake() const { return _shake; }
	virtual void update(float delta) override;
};

class UObject;

class URoom : public LayerColor {
	UndertaleLib::Room _room;
	Layer* _tileLayer; // weak pointer
	Layer* _objectLayer; // weak pointer

	std::unordered_multiset<size_t, UObject*> _objectLookup;// weak pointer
	std::list<UObject*> _objects;
	Layer* _backgroundLayer; // weak pointer

	void addUObject(UObject* object);
	void removeObject(UObject* object);
	bool init() override;
public:
	const std::list<UObject*> getObjects() const { return _objects; }
	UObject* containsObject(std::function<bool(UObject*)> pred) const;
	UObject* instance_exists(size_t object_index) const;
	void instance_destroy(size_t object_index);
	UObject* instance_create(float x, float y, size_t object_index);


	URoom() : _tileLayer(nullptr), LayerColor() {}
	void setUndertaleRoom(const UndertaleLib::Room& room);
	void setUndertaleRoom(size_t room_index);
	const UndertaleLib::Room& getUndertaleRoom() const { return _room; }
	static URoom* create(const UndertaleLib::Room& room);
	static URoom* create(size_t room_index);
	static URoom* create(const std::string& name);

	void nextRoom();
	void previousRoom();
	std::string getDebugName() const;
};
class Undertale : public Ref {
	Map<size_t, Image*> _images;
	Map<size_t, FontAtlas*> _fontAtlases;
public:
	static bool loadDataWin(std::vector<uint8_t>&& data);
	static bool loadDataWin(const std::string& filename);
	static Undertale* getSingleton();
	static UndertaleLib::UndertaleFile* getFile();
	Texture2D* LookupTexture(size_t index);
	Image* LookupImage(size_t index);
	// Usally for backgrounds

	Image* ImageFromBackground(const UndertaleLib::Background& b);
	Image* ImageFromBackground(size_t index);
	Image* ImageFromBackground(const std::string& name);

	Texture2D* TextureFromBackground(const UndertaleLib::Background& b);
	Texture2D* TextureFromBackground(size_t index);
	Texture2D* TextureFromBackground(const std::string& name);

	FontAtlas* LookupFontAtlas(const UndertaleLib::Font& f);
	FontAtlas* LookupFontAtlas(size_t index);
	FontAtlas* LookupFontAtlas(const std::string& name);

	cocos2d::SpriteFrame* createSpriteFrame(const UndertaleLib::SpriteFrame& frame) const;
};