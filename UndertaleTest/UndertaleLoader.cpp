#include "UndertaleLoader.h"
#include "UndertaleLib.h"
#include <cassert>

static UndertaleLib::UndertaleFile file;
static std::unordered_map<size_t, sf::Texture> global_textures;


UFont::UFont() : _texture(nullptr) , _fontSize(0) { }
const sf::Glyph& UFont::getGlyph(sf::Uint32 codePoint) const {
	GlyphTable::const_iterator it = _glyphTable.find(codePoint);
	assert(it != _glyphTable.end());
	return it->second;
}
bool UFont::loadUndertaleFont(size_t font_index) {
	auto ufont = file.LookupFont(font_index);
	assert(ufont.valid());
	_fontSize = ufont.size();
	auto& uframe = ufont.frame();
	_texture = &Global::GetUndertaleTexture(uframe.texture_index);
	for (auto& g : ufont.glyphs()) {
		sf::Glyph glyph;
		glyph.advance = g.shift;
		glyph.bounds = sf::FloatRect(g.offset, g.height/2, g.width, g.height);
		glyph.textureRect = sf::IntRect(uframe.offset_x+  g.x, uframe.offset_y + g.y, g.width, g.height);
	}
	return true;
}
namespace Global {
	
	bool LoadUndertaleDataWin(const std::string& filename) {
		return file.loadFromFilename(filename);
	}

	const sf::Texture& GetUndertaleTexture(size_t index) {
		sf::Texture& texture = global_textures[index];
		if (texture.getNativeHandle() != 0) return texture;
		auto utexture = file.LookupTexture(index);
		assert(utexture.len() != 0);
		assert(texture.loadFromMemory(utexture.data(), utexture.len()));
		return texture;
	}



	void DestroyEveything() {
		global_textures.clear();
	}
};