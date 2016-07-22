#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <functional>

#ifdef UNDERLIBRARY_DLL
#ifdef UNDERLIBRARY_EXPORTS
#define UNDERLIBRARY_API __declspec(dllexport) 
#else
#define UNDERLIBRARY_API __declspec(dllimport) 
#endif
#else
#define UNDERLIBRARY_API
#endif

namespace UndertaleLib {

	inline size_t simple_hash(const char *str)
	{
		size_t hash = 5381;
		int c;
		while (c = (unsigned char)(*str++)) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		return hash;
	}
	inline size_t simple_hash(const char *str, size_t len)
	{
		size_t hash = 5381;
		while (len > 0) {
			int c = *str++;
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
			len--;
		}
		return hash;
	}
#pragma pack(push, 1)
	struct UndertaleString {
		uint32_t length;
		const char u_str[1];
		UndertaleString& operator=(UndertaleString const &) = delete;  // no move constructor
		UndertaleString(UndertaleString const &) = delete;           // no move constructor
		bool operator==(const UndertaleString& other) const { return &u_str[0] == &other.u_str[0]; } // always equal
		bool operator!=(const UndertaleString& other) const { return !(*this == other); }
	};
#pragma pack(pop)
	// Container for strings simple pointer strings.  Used for comparing and matching
	// carful though
	class String {
		friend class StringManager;
		
		struct istring {
			size_t uses;
			size_t len;
			size_t hash;
			const char* str;
		};
		istring* _str;

		friend class UndertaleFile;
	public:
		String();
		String(const char* str);
		String(const char* str, size_t len);

		String& operator=(const char* str);
		String& operator=(const std::string& str);

		const char* c_str() const { return _str->str; }
		std::string string() const { return std::string(_str->str, _str->len); }
		size_t length() const { return _str->len; }
		size_t hash() const { return _str->hash; }
		bool operator==(const String& other) const { return _str == other._str; }
		bool operator!=(const String& other) const { return _str != other._str; }
		bool operator==(const std::string& other) const { return  length() == other.length() && std::memcmp(c_str(), other.c_str(), length()) == 0; }
		bool operator!=(const std::string& other) const { return !(*this == other); }
		bool operator==(const char* other) const { return _str->str == other || (std::strlen(other) == _str->len && strncmp(_str->str, other, _str->len) == 0); }
		bool operator!=(const char* other) const { return !(*this == other); }
		bool operator<(const char* other) const { return std::strcmp(_str->str, other) < 0; } // for sets
	};


	enum class ChunkType : unsigned int {
		BAD = 0,
		GEN8,
		TXTR,
		BGND,
		TPAG,
		SPRT,
		ROOM,
		AUDO,
		SOND,
		FONT,
		OBJT,
		PATH,
		SCPT,
		CODE,
		VARS,
		FUNC,
		STRG,
		_CMAX
	};
	class Resource {
	protected:
		static constexpr ChunkType ResType = ChunkType::BAD;
		String _name;
		int _index;
		friend class UndertaleFile;
	public:
		Resource() : _name(), _index(-1) {}
		~Resource() {}
		ChunkType type() const { return ResType; }
		virtual size_t hash() const { return _name.hash(); }
		virtual bool valid() const { return index() >= 0; }
		virtual int index() const { return _index; }
		virtual const String& name() const { return _name; }
		inline bool operator==(const Resource& other) const { return _name == other._name; } // its name will be unique cause the pointer is
	};
	template<class T> class OffsetVectorIt;
	template<class T> class OffsetVector {
		const uint8_t* _data;
		struct Record {
			uint32_t count;
			uint32_t offsets[1];
			Record(Record const &) = delete;           // undefined
			Record& operator=(Record const &) = delete;  // undefined
		};
		const Record* _rec;
	public:
		typedef OffsetVectorIt<T> const_iterator;
		typedef const_iterator iterator;
		typedef ptrdiff_t difference_type;
		typedef size_t size_type;
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;
		OffsetVector() :_data(nullptr), _rec(nullptr) {}
		OffsetVector(const uint8_t* data, size_t offset) : _data(data), _rec(reinterpret_cast<const Record*>(data + offset)) {}
		OffsetVector(const uint8_t* data, const uint8_t* rec) : _data(data), _rec(reinterpret_cast<const Record*>(rec)) {}
		const T& at(uint32_t i) const { return *reinterpret_cast<const T*>(_data + _rec->offsets[i]); }
		const T& operator[](uint32_t i) const { return at(i); }

		OffsetVectorIt<T> begin() const;
		OffsetVectorIt<T>  end() const;
		size_t size() const { return _rec->count; }
		typedef std::initializer_list<T> test_t;
	};
	template<class T> class OffsetVectorIt {
		const OffsetVector<T>& _vec;
		size_t _pos;
	public:
		OffsetVectorIt(const OffsetVector<T>& vec, size_t pos) : _vec(vec), _pos(pos) {}
		OffsetVectorIt(const OffsetVector<T>& vec) : _vec(vec), _pos(0) {}
		OffsetVectorIt& operator++() { _pos++; return *this; }
		OffsetVectorIt& operator--() { _ pos--; return *this; }
		const T& operator*() const { return _vec[_pos]; }
		bool operator==(const OffsetVectorIt& other) const { return _pos == other._pos; }
		bool operator!=(const OffsetVectorIt& other) const { return _pos != other._pos; }
	};


	template<class T> OffsetVectorIt<T> OffsetVector<T>::begin() const { return OffsetVectorIt<T>(*this, (size_t)0); }
	template<class T>  OffsetVectorIt<T> OffsetVector<T>::end() const { return OffsetVectorIt<T>(*this, size()); }
}

namespace std {
	template <>
	struct std::hash<UndertaleLib::Resource>
	{
		std::size_t operator()(const UndertaleLib::Resource& r) const { return r.hash(); }
	};
	template <>
	struct std::hash<UndertaleLib::String>
	{
		std::size_t operator()(const UndertaleLib::String& r) const { return UndertaleLib::simple_hash(r.c_str(), r.length()); }
	};
	template <>
	struct std::less<UndertaleLib::String>
	{
		std::size_t operator()(const UndertaleLib::String& l, const UndertaleLib::String& r) const {  return std::strcmp(l.c_str(), r.c_str()); }
	};
	template <>
	struct std::hash<UndertaleLib::UndertaleString>
	{
		std::size_t operator()(const UndertaleLib::UndertaleString& r) const { return UndertaleLib::simple_hash(r.u_str, r.length); }
	};
	template <>
	struct std::hash<UndertaleLib::UndertaleString*>
	{
		std::size_t operator()(const UndertaleLib::UndertaleString* r) const { return UndertaleLib::simple_hash(r->u_str, r->length); }
	};
	template<>
	struct std::hash<const UndertaleLib::Resource*> {
		std::size_t operator()(const UndertaleLib::Resource  *r) const { return r->hash(); }
	};
	template<>
	struct std::equal_to<UndertaleLib::Resource const *> {
		bool operator()(UndertaleLib::Resource const *l, UndertaleLib::Resource const *r) const { return *l == *r; }
	};
}
// We load up the entire datawin file once the main object is created
// and its used with pointers to all the data
// because of this, this library really dosn't work in Big eadin systems.
namespace UndertaleLib {
	// parser used to parse undertell text
	class UndertaleText {
	public:
		enum class Token : int {
			Bad = 0, // stop value = 0 means nothin after this
			Letter,
			Delay, // "^[0-9]" // 0 is default
			NewLine, // "&", also checks for \n|\r|\r\n|\n\r
			Color,
			Choicer,
			Face,
			Emotion,
			Typer,  // all follow a \  
			Flag, // Usally this was global.flag[20]  Used for animations and extra sprite movements
			Infinity, // its a shaking infinity sign for the asriel dremo fight when checking his stats  //z
			NextString, // "%"
			SelfDestroy, // "%%"
			Halt // all follow a /
		};
		class token_t;
		typedef std::vector<token_t> token_container;
		typedef std::vector<token_t>::iterator iterator;
		typedef std::vector<token_t>::const_iterator const_iterator;
		class token_t {
			Token _token;
			int _pos;
			int _value;	//	value of token 
		public:
			token_t() : _token(Token::Bad), _value(0), _pos(-1) {}
			token_t(Token token,int pos) : _token(token), _value(0), _pos(pos) {}
			token_t(Token token, int pos, int value) : _token(token), _value(value), _pos(pos) {}
			inline Token token() const { return _token; }
			inline int value() const { return _value; }
			inline int pos() const { return _pos; }
			bool operator==(const token_t& other) const { return _token == other._token; }
			bool operator!=(const token_t& other) const { return _token != other._token; }
			bool operator==(const Token& token) const { return _token == token; }
			bool operator!=(const Token& token) const { return _token != token; }
		};
	private:
		std::string _text;
		std::string _cleaned;
		std::vector<token_t> _parsed;
		size_t _charCount; // number of displable chars
		void parse(bool includeNewLinesInCleanedText);
	public:
		UndertaleText() {}
		UndertaleText(const std::string& text) : _text(text) { parse(true); }
		UndertaleText(const std::string& text, bool includeNewLinesInCleanedText) : _text(text) { parse(includeNewLinesInCleanedText); }
		const std::string& getText() const { return _text; }
		void setText(const std::string& text, bool includeNewLinesInCleanedText = true) { _text = text; parse(includeNewLinesInCleanedText); }
		const std::string& getCleanedText() const { return _cleaned; }
		const token_container& getTokens() const { return _parsed; }
		iterator begin() { return _parsed.begin(); }
		iterator end() { return _parsed.end(); }
		const_iterator begin() const { return _parsed.begin(); }
		const_iterator end() const { return _parsed.end(); }
		token_t operator[](size_t i) const { return i >= _parsed.size() ? token_t() : _parsed[i]; }
		size_t size() const { return _parsed.size(); }
		size_t charCount() const { return _charCount; }
	};
	// This structure is pointed to somewhere in the data win
	struct SpriteFrame {
		short x;
		short y;
		unsigned short width;
		unsigned short height;
		short offset_x;
		short offset_y;
		unsigned short crop_width;
		unsigned short crop_height;
		unsigned short original_width;
		unsigned short original_height;
		short texture_index;
		bool valid() const { return texture_index != -1; }
		SpriteFrame() : x(0), y(0), width(0), height(0), offset_x(0), offset_y(0), crop_width(0), crop_height(0), original_width(0), original_height(0), texture_index(-1) {}
	};
	class BitMask {
		int _width;
		int _height;
		const uint8_t* _raw;
		BitMask() : _width(0), _height(0), _raw(nullptr) {}
	public:
		int stride() const { return (_width + 7) / 8; }
		int width() const { return _width; }
		int height() const { return _height; }
		const uint8_t* raw() { return _raw; }
		const uint8_t* scaneline(int line) const { return _raw + (stride()*line); }
		// mainly here for a helper function, tells if a bit is set
		// its really here to describe how bitmaks are made
		bool isSet(int x, int y) const {
			uint8_t pixel = _raw[y * _width + x / 8];
			uint8_t bit = (7 - (x & 7) & 31);
			return ((pixel >> bit) & 1) != 0;
		}
		friend class UndertaleFile;
	};
	class Texture {
		const uint8_t* _data;
		size_t _len;
	public:
		Texture() : _data(nullptr), _len(0) {}
		Texture(const uint8_t* data, size_t len) :_data(data), _len(len) {}
		const uint8_t* data() const { return _data; }
		size_t len() const { return _len; }
	};
	class Background : public Resource {
		friend class UndertaleFile;
	protected:
		static constexpr ChunkType ResType = ChunkType::BGND;
		struct RawBackground {
			uint32_t name_offset;
			uint32_t trasparent;
			uint32_t smooth;
			uint32_t preload;
			uint32_t frame_offset;
		};
		const RawBackground* _raw;
		typedef RawBackground RawResourceType;
		const SpriteFrame* _frame;
	public:
		Background() : Resource(), _raw(nullptr), _frame(nullptr) {}
		bool trasparent() const { return _raw->trasparent != 0; }
		bool smooth() const { return _raw->smooth != 0; }
		bool preload() const { return _raw->preload != 0; }
		const SpriteFrame& frame() const { return *_frame; }
	};
	class Room : public Resource {
		friend class UndertaleFile;
	protected:
		static constexpr ChunkType ResType = ChunkType::ROOM;
	public:
		struct View {
			int visible;
			int x;
			int y;
			int width;
			int height;
			int port_x;
			int port_y;
			int port_width;
			int port_height;
			int border_x;
			int border_y;
			int speed_x;
			int speed_y;
			int view_index;
		};
		struct Background {
			int visible;// bool
			int foreground;// bool
			int background_index;// bool
			int x;
			int y;
			int tiled_x;
			int tiled_y;
			int speed_x;
			int speed_y;
			int strech; // bool
		};
		struct Object {
			int x;
			int y;
			int object_index;
			int id;
			int code_offset;
			float scale_x;
			float scale_y;
			int color;
			float rotation;
		};
		struct Tile {
			int x;
			int y;
			int background_index;
			int offset_x;
			int offset_y;
			int width;
			int height;
			int depth;
			int id;
			float scale_x;
			float scale_y;
			int blend; // color value
		};
	protected:
		struct RawRoom {
			int name_offset;
			int caption_offset;
			int width;
			int height;
			int speed;
			int persistent;
			int color;
			int show_color;
			int code_offset;
			int flags;
			int background_offset;
			int view_offset;
			int object_offset;
			int tiles_offset;
		};
		const RawRoom* _raw;
		typedef RawRoom RawResourceType;
		String _caption;
		OffsetVector<View> _views;
		OffsetVector<Background> _backgrounds;
		OffsetVector<Object> _objects;
		OffsetVector<Tile> _tiles;
	public:
		Room() : Resource(), _raw(nullptr) {}
		String caption() const { return _caption; }
		int width() const { return _raw->width; }
		int height() const { return _raw->height; }
		int speed() const { return _raw->speed; }
		bool persistent() const { return _raw->persistent != 0; }
		int color() const { return _raw->color; }
		bool show_color() const { return _raw->show_color != 0; }
		int code_offset() const { return _raw->code_offset; }
		bool enable_views() const { return (_raw->flags & 1) != 0; }
		bool view_clear_screen() const { return (_raw->flags & 2) != 0; }
		bool clear_display_buffer() const { return (_raw->flags & 14) != 0; }
		const OffsetVector<View>& views() const { return _views; }
		const OffsetVector<Background>& backgrounds() const { return _backgrounds; }
		const OffsetVector<Object>& objects() const { return _objects; }
		const OffsetVector<Tile>& tiles() const { return _tiles; }
	};
	class Sound : public Resource {
	protected:
		static constexpr ChunkType ResType = ChunkType::SOND;
	private:
		struct RawSound {
			int name_offset;
			int audio_type;
			int extension_offset;
			int filename_offset;
			int effects;
			float volume; 
			float pan;
			int other;
			int sound_index;
		};
		struct RawAudioData {
			const int size;
			const int data[1];
			RawAudioData(RawAudioData const &) = delete;           // undefined
			RawAudioData& operator=(RawAudioData const &) = delete;  // undefined
		};
		friend class UndertaleFile;
		typedef RawSound RawResourceType;
		const RawSound* _raw;
		const RawAudioData* _data;
		String _extension;
		String _filename;
	public:
		Sound() : Resource(), _raw(nullptr), _data(nullptr) {}
		int audio_type() const { return _raw->audio_type; }
		String extension() const { return _extension; }
		String filename() const { return _filename; }
		int effects() const { return _raw->effects; }
		float volume() const { return _raw->volume;}
		float pan() const { return _raw->pan; }
		int other() const { return _raw->other; }
		const RawAudioData* data() const { return _data; }
	};
	class Font : public Resource {
	protected:
		static constexpr ChunkType ResType = ChunkType::FONT;
	public:
		struct Kerning {
			short other;
			short amount;
		};
		struct Glyph {
			short ch;
			short x;
			short y;
			short width;
			short height;
			short shift;
			short offset;
			unsigned short kerning_count;
			Kerning kernings[1];
			//uint32_t count;
		//	uint32_t offsets[1];
			Glyph(Glyph const &) = delete;           // undefined
			Glyph& operator=(Glyph const &) = delete;  // undefined
		};
	private:
		struct RawFont {
			int name_offset;
			int description_offset;
			int size;
			int bold;
			int italic;
			int flags; // (antiAlias | CharSet | firstchar)
			int lastChar;
			uint32_t frame_offset;
			float scale_width;
			float scale_height;
		};
		friend class UndertaleFile;
		typedef RawFont RawResourceType;
		const RawFont* _raw;
		const SpriteFrame* _frame;
		String _description;
		OffsetVector<Glyph> _glyphs;
	public:
		Font() : Resource(), _raw(nullptr), _frame(nullptr), _description() {}
		int size() const { return _raw->size; }
		bool bold() const { return _raw->bold != 0; }
		bool italic() const { return _raw->italic != 0; }
		bool antiAlias() const { return ((_raw->flags >> 24) & 0xFF) != 0; }
		int charSet() const { return (_raw->flags >> 16) & 0xFF; }
		uint16_t firstChar() const { return (_raw->flags) & 0xFFFF; }
		uint16_t lastChar() const { return _raw->lastChar; }
		const SpriteFrame& frame() const { return *_frame; }
		float scaleWidth() const { return _raw->scale_width; }
		float scaleHeight() const { return _raw->scale_height; }
		const OffsetVector<Glyph>& glyphs() const { return _glyphs; }
	};
	class Object : public Resource {
		friend class UndertaleFile;
	protected:
		static constexpr ChunkType ResType = ChunkType::OBJT;
	private:
#pragma pack(1)
		struct RawObject {
			int name_offset;
			int sprite_index;
			int visible;
			int solid;
			int depth;
			int persistent;
			int parent_index;
			int mask;
			int physics_enabled;
		};
#pragma pack()
		typedef RawObject RawResourceType;
		const RawResourceType* _raw;
	public:
		Object() :Resource(), _raw(nullptr) {}
		int sprite_index() const { return _raw->sprite_index; }
		bool visible() const { return _raw->visible !=0; }
		bool solid() const { return _raw->solid!=0; }
		int depth() const { return _raw->depth; }
		bool persistent() const { return _raw->persistent != 0; }
		int parent_index() const { return _raw->parent_index; }
		bool mask() const { return _raw->mask != 0; }
		bool physics_enabled() const { return _raw->physics_enabled != 0; }
	};

	class Sprite : public Resource {
	protected:
		static constexpr ChunkType ResType = ChunkType::SPRT;
	private:
#pragma pack(1)
		struct RawSprite {
			const int name_offset;
			int width;
			int height;
			int left;
			int right;
			int bottom;
			int top;
			int trasparent;
			int smooth;
			int preload;
			int mode;
			int colcheck;
			int original_x;
			int original_y;
		};
#pragma pack()
		typedef RawSprite RawResourceType;
		const RawSprite* _raw;
		OffsetVector<SpriteFrame> _frames;
		std::vector<BitMask> _masks;
		friend class UndertaleFile;
	public:
		Sprite() :Resource(), _raw(nullptr) {}
		int width() const { return _raw->width; }
		int height() const { return _raw->height; }
		int left() const { return _raw->left; }
		int right() const { return _raw->right; }
		int bottom() const { return _raw->bottom; }
		int top() const { return _raw->top; }
		bool trasparent() const { return _raw->trasparent != 0; }
		bool smooth() const { return _raw->smooth != 0; }
		bool preload() const { return _raw->width != 0; }
		int mode() const { return _raw->mode; }
		int colcheck() const { return _raw->colcheck; }
		int origin_x() const { return _raw->original_x; }
		int origin_y() const { return _raw->original_y; }
		bool has_mask() const { return _masks.size() > 0; }
		const OffsetVector<SpriteFrame>& frames() const { return _frames; }
		const std::vector<BitMask>& masks() const { return _masks; }
	};
	class UndertaleFile {
		struct Chunk {
			union {
				char name[4];
				uint32_t iname;
			};
			uint32_t size;
			uint32_t count;
			uint32_t offsets[1];  // used for fast lookups
			Chunk(Chunk const &) = delete;           // undefined
			Chunk& operator=(Chunk const &) = delete;  // undefined
		};

		std::vector<const Chunk*> _chunks;
		std::vector<uint8_t> _data;
		class ResourceKey {
			int _index;
			ChunkType _type;
		public:
			ResourceKey() : _index(0), _type(ChunkType::BAD) {}
			ResourceKey(ChunkType t, int index) : _index(index), _type(t) {}
			inline int index() const { return _index; }
			inline ChunkType type() const { return _type; }
			bool operator==(const ResourceKey& k)  const { return type() == k.type() && index() == k.index(); }
		};
		struct ResourceKeyHasher
		{
			std::size_t operator()(const ResourceKey& r) const { return (uint8_t)r.type() << 24 || r.index(); }
		};
		// Contains the managed pointer
		// lookup by index
		// lookup for name
		std::unordered_map<String, ResourceKey> _nameCache;
		// all strings hash
		std::unordered_set<const UndertaleString*> _strings; // the strings are directly mapped to the loaded data.win


		bool internalParse();
		void internalReset();
		const Chunk* getChunk(ChunkType t) const { return _chunks[(uint32_t)t]; }
		template<class T> void addChunkToLookup();
	
		template<class T> const uint8_t* preCreateResorce(int index, T&res) const;


		template<class T> T createResource(int index) const;
		template<> Font createResource(int index) const;
		template<> Sprite createResource(int index) const;
		template<> Background createResource(int index) const;
		template<> Room createResource(int index) const;
		template<> Sound createResource(int index) const;

		String getUndertaleString(int offset) const;
		template<class C> void fillList(size_t offset, std::vector<const C*>& list) const;
		template<class C, class P> void fillList(size_t offset, std::vector<const C*>& list, P pred) const;

	public:

		UndertaleFile();
		bool isLoaded() const { return _data.size() > 0; }
		bool loadFromFilename(const std::string& filename);
		bool loadFromData(const std::vector<uint8_t>& data); // we want it, so its a move
															 //bool loadFromData(const std::vector<uint8_t*>& _data); // copy it instead
		bool loadFromData(std::vector<uint8_t>&& data);

		Sprite LookupSprite(int index) const;
		Room LookupRoom(int index) const;
		Background LookupBackground(int index) const;
		Font LookupFont(int index) const;
		/// Returns the raw location of the texture, its a png so hopefuly you have something that can
		/// read it
		Texture LookupTexture(int index) const;
		Object LookupObject(int index) const;
		Sound UndertaleFile::LookupSound(int index)  const;
		// Some times I love templates, some times I hate them
		template<class T> T LookupByName(const String& str) const {
			auto it = _nameCache.find(str);
			if (it != _nameCache.cend() && it->second.type() == T::ResType)
				return createResource<T>(it->second.index());
			return T();
		}
#if 0
		template<class T> T LookupByName(const std::string& str) const {
			return LookupByName(str.c_str());
		}
		template<class T> T LookupByName(const char str) const {
			return LookupByName(String(str));
		}
#endif
		std::vector<Font> ReadAllfonts();

	};
}