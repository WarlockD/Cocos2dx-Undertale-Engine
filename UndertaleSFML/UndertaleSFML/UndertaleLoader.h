#pragma once

#include <SFML\Graphics.hpp>
#include <unordered_map>
#include <memory>


class UFont  : public std::enable_shared_from_this<UFont> {
	static std::unordered_map<size_t, std::weak_ptr<UFont>> _cache;
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
	uint16_t _index;
	uint16_t _frame;
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

namespace Global {
	bool LoadUndertaleDataWin(const std::string& filename);
	const sf::Texture& GetUndertaleTexture(size_t index);
	sf::Shader& LoadShader(const std::string& filename); // loads a shader, or from a cache
	void DestroyEveything();
};