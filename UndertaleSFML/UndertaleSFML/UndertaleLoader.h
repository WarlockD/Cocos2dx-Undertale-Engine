#pragma once

#include "Global.h"
#include "Drawables.h"


class UFont  : public std::enable_shared_from_this<UFont> {
protected:
	size_t _fontSize;
	const sf::Texture* _texture;
	typedef std::unordered_map<sf::Uint32, sf::Glyph> GlyphTable;
	GlyphTable _glyphTable;
	int _index;
	UFont();
public:
	static std::shared_ptr<UFont> LoadUndertaleFont(size_t font_index);
	int getIndex() const { return _index; }
	const sf::Texture& getTexture() const { return *_texture; }
	size_t getFontSize() const { return _fontSize; }
	const sf::Glyph& getGlyph(sf::Uint32 codePoint) const;
	float getKerning(sf::Uint32 first, sf::Uint32 second) const;
	float getLineSpacing() const;
};

class UndertaleSprite  : public sf::Drawable, public sf::Transformable {
	
protected:
	sf::Vector2f _size;
	size_t _index;
	size_t _frame;
	sf::Color _color;
	std::vector<sf::Vertex> _verts;
	std::shared_ptr<sf::Texture> _texture;
public:
	void loadFrame(size_t index, size_t frame = 0);
	UndertaleSprite() : _index(0), _frame(0) {}
	UndertaleSprite(size_t index, size_t frame = 0) { loadFrame(index, frame); }
	const sf::Color& getColor() const { return _color; }
	void setColor(const sf::Color& color);
	size_t getSpriteIndex() const { return _index;}
	size_t getImageIndex() const { return _frame/6; }
	size_t getFrameCount() const { return _verts.size()/6; }
	void setImageIndex(size_t frame) { _frame = frame*6; }
	const sf::Vector2f& getSize() const { return _size; }
	const sf::Texture& getTexture() const { return *_texture.get(); }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class SpriteEnity : public ex::Entity {
	
	size_t _sprite_index;
	Body* _body;
	SpriteEnity(ex::EntityManager *manager, Entity::Id id) : ex::Entity(manager, id) { }
public:
	virtual ~SpriteEnity();
	static SpriteEnity create(size_t sprite_index);
	void setSpriteIndex(size_t sprite_index);
	size_t getSpriteIndex() const { return _sprite_index; }
	Body& operator*() const { return *_body; }
	Body* operator->()  { return _body; } 
};
namespace Global {
	SpriteFrameCollection LoadSprite(size_t sprite_index);
	

	bool LoadUndertaleDataWin(const std::string& filename);
	const sf::Texture& GetUndertaleTexture(size_t index);
	sf::Shader& LoadShader(const std::string& filename); // loads a shader, or from a cache
	void DestroyEveything();
};