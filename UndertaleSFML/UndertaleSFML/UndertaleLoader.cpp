#include "UndertaleLoader.h"
#include "UndertaleLib.h"
#include <cassert>

using namespace sf;
static UndertaleLib::UndertaleFile file;
static std::unordered_map<size_t, std::unique_ptr<sf::Texture>> global_textures;
static std::unordered_map<std::string, std::unique_ptr<sf::Shader>> global_shader_cache;




void UndertaleSprite::setColor(const sf::Color& color) { 
	if (_color != color) {
		for (auto& s : _verts) s.color = color;
		_color = color;
	}
}

void UndertaleSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	states.texture = _texture.get();
	target.draw(_verts.data() + _frame , 6, sf::PrimitiveType::Triangles, states);
}
struct UndertaleSpriteCache {
	size_t index;
	size_t texture_index;
	UndertaleLib::Sprite usprite;
	std::vector<sf::Vertex> verts;
	sf::Vector2f size;
	UndertaleSpriteCache() : index(0) {}
	const std::vector<sf::Vertex>& getVerts() const { return verts; }
};
static std::unordered_map<size_t, UndertaleSpriteCache> s_spriteCache;

static const UndertaleSpriteCache& LookupSpriteCache(size_t index) {
	auto& it = s_spriteCache[index];
	if (it.index != index) {
		it.usprite = file.LookupSprite(index);
		it.size = sf::Vector2f(float(it.usprite.width()), float(it.usprite.height()));
		it.index = index;
		it.texture_index = it.usprite.frames()[0].texture_index;
		for (auto uframe : it.usprite.frames()) {
			assert(it.texture_index == uframe.texture_index); // should be the same for all
			float left = static_cast<float>(uframe.offset_x);
			float top = static_cast<float>(uframe.offset_y);
			float right = static_cast<float>(uframe.offset_x + uframe.width);
			float bottom = static_cast<float>(uframe.offset_y + uframe.height);

			float u1 = static_cast<float>(uframe.x);
			float v1 = static_cast<float>(uframe.y);
			float u2 = static_cast<float>(uframe.x + uframe.width);
			float v2 = static_cast<float>(uframe.y + uframe.height);

			// Add a quad for the current character
			it.verts.push_back(Vertex(Vector2f(left, top), sf::Color::White, Vector2f(u1, v1)));
			it.verts.push_back(Vertex(Vector2f(right, top), sf::Color::White, Vector2f(u2, v1)));
			it.verts.push_back(Vertex(Vector2f(left, bottom), sf::Color::White, Vector2f(u1, v2)));
			it.verts.push_back(Vertex(Vector2f(left, bottom), sf::Color::White, Vector2f(u1, v2)));
			it.verts.push_back(Vertex(Vector2f(right, top), sf::Color::White, Vector2f(u2, v1)));
			it.verts.push_back(Vertex(Vector2f(right, bottom), sf::Color::White, Vector2f(u2, v2)));
		}
	}
	return it;
}
void UndertaleSprite::loadFrame(size_t index, size_t frame) {
	if (index != _index) {
		auto& it = LookupSpriteCache(index);
		_verts = it.verts;
		_size = it.size;
		_index = it.index;
	}
	setImageIndex(frame);
}
UFont::UFont() : _texture(nullptr) , _fontSize(0) { }
const sf::Glyph& UFont::getGlyph(sf::Uint32 codePoint) const {
	GlyphTable::const_iterator it = _glyphTable.find(codePoint);
	assert(it != _glyphTable.end());
	return it->second;
}

std::shared_ptr<UFont> UFont::LoadUndertaleFont(size_t index) {
	static std::unordered_map<size_t, std::weak_ptr<UFont>> _cache;
	auto w_ptr = _cache[index];
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

SpriteEnity::~SpriteEnity() { destroy(); }
SpriteEnity SpriteEnity::create(size_t sprite_index) {
	ex::Entity sprite_id = global::getEntities().create();
	// don't want to invalidate the sprite entry
	SpriteEnity sprite(&global::getEntities(), sprite_id.id());
	sprite.setSpriteIndex(sprite_index);
	sprite._sprite_index = 0;
	sprite._body = sprite.assign<Body>().get();
	return sprite;
}
void SpriteEnity::setSpriteIndex(size_t sprite_index) {
	if (_sprite_index != sprite_index) {
		auto cache = LookupSpriteCache(sprite_index);
		assign<SpriteFrameCollection>(&Global::GetUndertaleTexture(cache.texture_index), cache.getVerts(), cache.size);
	}
}

namespace Global {
	SpriteFrameCollection LoadSprite(size_t sprite_index) {
		auto cache = LookupSpriteCache(sprite_index);
		return SpriteFrameCollection(&Global::GetUndertaleTexture(cache.texture_index), cache.getVerts(), cache.size);
		/*
		auto usprite = file.LookupSprite(sprite_index);
		sf::Vector2f size = sf::Vector2f(float(usprite.width()), float(usprite.height()));
		const sf::Texture* texture = &Global::GetUndertaleTexture(usprite.frames().at(0).texture_index);
	
	
		std::vector<SpriteFrame> frames;
		for (auto uframe : usprite.frames()) {
			frames.emplace_back(
				&Global::GetUndertaleTexture(uframe.texture_index),
				sf::IntRect(uframe.x, uframe.y, uframe.width, uframe.height),
				sf::FloatRect(uframe.offset_x, uframe.offset_y, uframe.width, uframe.height)
			);
		}
		*/
	}

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