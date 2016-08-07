#include "UndertaleLoader.h"
#include "UndertaleLib.h"
#include <cassert>

static UndertaleLib::UndertaleFile file;
static std::unordered_map<size_t, std::unique_ptr<sf::Texture>> global_textures;
static std::unordered_map<std::string, std::unique_ptr<sf::Shader>> global_shader_cache;
std::unordered_map<size_t, std::weak_ptr<UFont>> UFont::_cache;

UFont::UFont() : _texture(nullptr) , _fontSize(0) { }
const sf::Glyph& UFont::getGlyph(sf::Uint32 codePoint) const {
	GlyphTable::const_iterator it = _glyphTable.find(codePoint);
	assert(it != _glyphTable.end());
	return it->second;
}
	
std::shared_ptr<UFont> UFont::LoadUndertaleFont(size_t index) {
	auto& w_ptr = _cache[index];
	if (!w_ptr.expired()) return w_ptr.lock();
	else {
		auto ufont = file.LookupFont(index);
		if (!ufont.valid()) return std::shared_ptr<UFont>();
		UFont* f = new UFont;
		f->_fontSize = ufont.size();
		f->_index = index;
		auto& uframe = ufont.frame();
		f->_texture = &Global::GetUndertaleTexture(uframe.texture_index);
		for (auto& g : ufont.glyphs()) {
		
			sf::Glyph glyph;
			glyph.advance = g.shift;
			glyph.bounds = sf::FloatRect((float)g.offset, 0.0f, (float)g.width, (float)g.height);
			glyph.textureRect = sf::IntRect(uframe.x + g.x, uframe.y + g.y, g.width, g.height);
			f->_glyphTable[g.ch] = glyph;
		//	assert(g.ch == '*');
		}
		std::shared_ptr<UFont> st(f);
		_cache[index] = st;
		return st;
	}
}
float UFont::getKerning(sf::Uint32 first, sf::Uint32 second) const {
	return 0.0f;
}
float UFont::getLineSpacing() const {
	return 0.0f;
}
std::string replace_extension(const std::string& filename, const std::string& new_extension) {
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos) return filename;
	return filename.substr(0, lastdot) + "." + new_extension;
}
namespace Global {
	bool LoadUndertaleDataWin(const std::string& filename) {
		return file.loadFromFilename(filename);
	}

	const sf::Texture& GetUndertaleTexture(size_t index) {
		auto& lookup = global_textures[index];
		if (!lookup) {
			auto utexture = file.LookupTexture(index);
			assert(utexture.len() != 0);
			sf::Texture* texture = new sf::Texture;
			assert(texture->loadFromMemory(utexture.data(), utexture.len()));
			texture->setSmooth(false);
			lookup.reset(texture);
		}
		return *lookup.get();
	}
	sf::Shader& LoadShader(const std::string& filename) {
		auto& lookup = global_shader_cache[filename];
		if (!lookup) {
			sf::Shader* shader = new sf::Shader;
			assert(shader->loadFromFile(replace_extension(filename, "vert"), replace_extension(filename, "frag")));
			lookup.reset(shader);
		}
		return *lookup.get();
	}// loads a shader, or from a cache


	void DestroyEveything() {
		global_textures.clear();
		global_shader_cache.clear();
	}
};