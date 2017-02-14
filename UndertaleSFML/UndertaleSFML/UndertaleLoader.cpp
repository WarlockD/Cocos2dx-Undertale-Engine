#include "UndertaleLoader.h"
#include "UndertaleLib.h"
#include <cassert>

using namespace sf;
static UndertaleLib::UndertaleFile file;
static std::unordered_map<size_t, std::unique_ptr<sf::Texture>> global_textures;
static std::unordered_map<std::string, std::unique_ptr<sf::Shader>> global_shader_cache;

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


std::unordered_map<size_t, UndertaleSprite::UndertaleSpriteData::type> UndertaleSprite::_cache;
UndertaleSprite::UndertaleSpriteData::type  UndertaleSprite::UndertaleSpriteData::LoadSprite(size_t sprite_index) {
	auto& weak = _cache[sprite_index];
	if (weak) return weak;
	else {
		UndertaleLib::Sprite usprite = file.LookupSprite(sprite_index);
		UndertaleSpriteData* sprite = new UndertaleSpriteData;
		sprite->_size = sf::Vector2f(float(usprite.width()), float(usprite.height()));
		sprite->_index = sprite_index;
		sprite->_frames.reserve(usprite.frames().size());
		for (auto uframe : usprite.frames()) sprite->_frames.emplace_back(Global::LoadFrame(uframe));
		std::shared_ptr<UndertaleSpriteData> shared_ptr(sprite);
		_cache.emplace(std::make_pair(sprite_index, shared_ptr));
		return shared_ptr; // return ptr, should move
	}
}
void UndertaleSprite::sprite_index(size_t index) {
	if (!_sprite || sprite_index() != index) {
		auto it = _cache.find(index);
		if (it != _cache.end()) _sprite = it->second;
		else _cache.emplace(std::make_pair(index, _sprite = UndertaleSpriteData::LoadSprite(index)));
		_image_index %= _sprite->frame_count();
	}
}

UndertaleRoom::type
UndertaleRoom::LoadRoom(size_t room_index) {
	static std::unordered_map<size_t, std::weak_ptr<UndertaleRoom>> cache;
	auto& weak = cache[room_index];
	if (!weak.expired()) return weak.lock();
	else {
		UndertaleLib::Room uroom = file.LookupRoom(room_index);
		
		if (uroom.valid()) {
			UndertaleRoom* room = new UndertaleRoom;
			room->_index = room_index;
			std::unordered_map<size_t, SpriteFrame> _backgroundCache;
			for (auto& t : uroom.tiles()) {
				SpriteFrame& b = _backgroundCache[t.background_index];
				if (b.texture() == nullptr) b = Global::LookupBackground(t.background_index);
				auto& tile_mesh = room->_tiles[b.texture()];
				if (tile_mesh.texture() == nullptr) tile_mesh = TileMap(b.texture(),b.texRect());
				assert(t.blend == -1);
				assert(t.scale_x == 1 && t.scale_y == 1);
				tile_mesh.tile_create(sf::Vector2f(t.x, t.y), sf::IntRect(t.offset_x, t.offset_y, t.width, t.height));
			}
			for (auto& o : uroom.objects()) {
				RoomObject obj;
				obj.body.setPosition(o.x, o.y); 
				obj.body.setRotation(o.rotation);
				obj.body.setScale(o.scale_x, o.scale_y);
				obj.obj = file.LookupObject(o.object_index);
				obj.depth = obj.obj.depth();
				room->_objects.emplace_back(obj);
			}
			for (auto& b : uroom.backgrounds()) {
				if (b.background_index >= 0) {
					auto& frame = _backgroundCache[b.background_index];
					if (frame.texture() == nullptr) frame = Global::LookupBackground(b.background_index);
					RoomBackground bb;
					bb.frame = frame;
					bb.strech = b.strech != 0;
					bb.speed = sf::Vector2f(b.speed_x, b.speed_y);
					bb.forground = b.foreground != 0;
					bb.visible = b.visible != 0;
					bb.pos = sf::Vector2f((float)b.x, (float)b.y);
					room->_backgrounds.emplace_back(bb);
				}
			}
			std::shared_ptr<UndertaleRoom> shared_ptr(room);
			weak = shared_ptr; // save in cache
			return shared_ptr; // return ptr, should move
		}
		else return UndertaleRoom::type();
	}
}

namespace Global {
	UndertaleLib::Object LookupObject(size_t index) {
		return file.LookupObject(index);
	}
	SpriteFrame LoadFrame(const UndertaleLib::SpriteFrame& uf) {
		sf::FloatRect bounds(uf.offset_x, uf.offset_y, uf.width, uf.height);
		sf::IntRect texRect(uf.x, uf.y, uf.width, uf.height);
		const sf::Texture* texture = &Global::GetUndertaleTexture(uf.texture_index);
		return SpriteFrame::create(texture, texRect, sf::Vector2f(uf.offset_x, uf.offset_y));
	}
	SpriteFrame LookupBackground(size_t index) {
		auto& b = file.LookupBackground(index);
		if (b.valid()) return LoadFrame(b.frame());
		else return SpriteFrame();
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