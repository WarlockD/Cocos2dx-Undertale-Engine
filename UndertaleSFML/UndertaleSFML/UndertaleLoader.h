#pragma once

#include "Global.h"
#include "Drawables.h"
#include "UndertaleLib.h"

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



class UndertaleSprite  : public SpriteFrameBase {
public:
	
	class UndertaleSpriteData : std::enable_shared_from_this<UndertaleSpriteData> {
	protected:
		sf::Vector2f _size;
		size_t _index;
		std::vector<sf::Vertex> _verts;
		const sf::Texture* _texture;
		UndertaleSpriteData() : _index(0), _texture(nullptr) {}
	public:
		typedef std::shared_ptr<UndertaleSpriteData> type;
		type ptr() { return shared_from_this(); }
		static type LoadSprite(size_t sprite_index);
		inline const sf::Vertex* frame(size_t i) const { return _verts.data() + (i * 6); }
		inline size_t frame_count() const { return _verts.size() / 6; }
		inline const sf::Texture* texture() const { return _texture; }
		inline size_t index() const { return _index; }
		inline const sf::Vector2f size() const { return _size; }
	};
	UndertaleSpriteData::type _sprite;
	std::unordered_map<size_t, UndertaleSpriteData::type> _cache; 
	// we use this so if we change this sprite, we don't lose the old one in case we need to switch fast
	size_t _image_index;
public:
	UndertaleSprite() : _image_index(0), _sprite(nullptr) {}
	
	explicit UndertaleSprite(size_t index) : _image_index(0), _sprite(UndertaleSpriteData::LoadSprite(index)), _cache({ std::make_pair(index, _sprite) }) {}
	// interface
	const sf::Texture* texture() const override final { return _sprite ? _sprite->texture() : nullptr; }
	const sf::Vertex* ptr() const override final { return _sprite ? _sprite->frame(_image_index) : nullptr; }
	sf::FloatRect bounds() const override final { return _sprite ? sf::FloatRect(sf::Vector2f(), _sprite->size()) : sf::FloatRect(); }
	// sprite stuff
	bool valid() const { return (bool)_sprite; }
	size_t sprite_index() const { return _sprite ? _sprite->index() : 0; }
	void sprite_index(size_t index) { 
		if (!_sprite || sprite_index() != index) {
			auto it = _cache.find(index);
			if (it != _cache.end()) _sprite = it->second;
			else _cache.emplace(std::make_pair(index, _sprite = UndertaleSpriteData::LoadSprite(index)));
			_image_index %=  _sprite->frame_count();
		}
	}
	void clear_cache() { _cache.clear(); }
	size_t image_index() const { return _image_index; }
	void image_index(size_t index) { _image_index = index % _sprite->frame_count(); }
	virtual bool next_frame() { image_index(_image_index + 1);  return true; }; // This interface just tells the Renderable to do next frame
	virtual bool prev_frame() { image_index(_image_index - 1); return true; }; // This interface just tells the Renderable to do prev frame
};

// font texture algorithm 
// http://gamedev.stackexchange.com/questions/2829/texture-packing-algorithm

class UndertaleRoom : std::enable_shared_from_this<UndertaleRoom> {

public:
	struct RoomObject  {
		UndertaleLib::Object obj;
		Body body;
		int depth;
	};
	struct RoomBackground {
		SpriteFrame frame;
		sf::Vector2f pos;
		sf::Vector2f speed;
		bool strech;
		bool tiled;
		bool forground;
		int depth;
		bool visible;
	};
	typedef std::shared_ptr<UndertaleRoom> type;
	type ptr() { return shared_from_this(); }
	static type LoadRoom(size_t sprite_index);
	
	const std::unordered_map<const sf::Texture*, TileMap> tiles() const { return _tiles; }
	const std::vector<RoomBackground>& backgrounds() const { return _backgrounds; }
	const std::vector<RoomObject>& objects() const { return _objects; }
	size_t index() const { return _index; }
protected:
	std::unordered_map<const sf::Texture*, TileMap> _tiles;
	std::vector<RoomBackground> _backgrounds;
	std::vector<RoomObject> _objects;
	size_t _index;
	UndertaleRoom() : _index(0) {}
};

namespace Global {
	UndertaleLib::Object LookupObject(size_t index);
	SpriteFrame LoadFrame(const UndertaleLib::SpriteFrame& uf);
	SpriteFrame LookupBackground(size_t index);
	SpriteFrame LoadFrame(const UndertaleLib::SpriteFrame& uf);
	bool LoadUndertaleDataWin(const std::string& filename);
	const sf::Texture& GetUndertaleTexture(size_t index);
	sf::Shader& LoadShader(const std::string& filename); // loads a shader, or from a cache
	void DestroyEveything();
};