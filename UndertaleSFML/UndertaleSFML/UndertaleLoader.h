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
namespace Global {
	bool LoadUndertaleDataWin(const std::string& filename);
	const sf::Texture& GetUndertaleTexture(size_t index);
	sf::Shader& LoadShader(const std::string& filename); // loads a shader, or from a cache
	void DestroyEveything();
};