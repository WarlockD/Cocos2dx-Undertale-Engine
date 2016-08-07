#pragma once

#include <SFML\Graphics.hpp>
#include <unordered_map>

class UFont  {
protected:
	size_t _fontSize;
	const sf::Texture* _texture;
	typedef std::unordered_map<sf::Uint32, sf::Glyph> GlyphTable;
	GlyphTable _glyphTable;

	

public:
	UFont();
	size_t getFontSize() const { return _fontSize; }
	bool loadUndertaleFont(size_t font_index);
	const sf::Glyph& getGlyph(sf::Uint32 codePoint) const;
	float getKerning(sf::Uint32 first, sf::Uint32 second, unsigned int characterSize) const;
	float getLineSpacing(unsigned int characterSize) const;
};
namespace Global {
	bool LoadUndertaleDataWin(const std::string& filename);
	const sf::Texture& GetUndertaleTexture(size_t index);

	void DestroyEveything();
};