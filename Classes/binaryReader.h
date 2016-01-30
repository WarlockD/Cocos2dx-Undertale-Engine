#pragma once
// rough implmentation of a binaryReader simiar to c# with some helpers
#include "cocos2d.h"
#include "LuaEngine.h"
#include <fstream>
#include <vector>
#include <stack>
#include <string>
#include <memory>



// similar to how std::unique_lock works, you run this in brackets and it will restore
// the stream back to state on deconstruction
template<class _BinaryStream> class StreamLock {
	_BinaryStream& s;
public:
	StreamLock(_BinaryStream& stream) : s(stream) { s.push(); }
	StreamLock(_BinaryStream& stream, std::streamoff starting) : s(stream) { s.push(starting); }
	~StreamLock() { s.pop(); }
};


class BinaryReader {
	std::unique_ptr<std::istream> s;
	std::streamsize _length;
	std::stack<std::streampos> _tellStack;
public:
	BinaryReader(const std::string& filename) {
		std::fstream* fs = new std::fstream();
		fs->exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			fs->open(filename, std::ios::binary | std::ios::in);
			fs->seekg(0, std::ios::end);
			_length = fs->tellg();
			fs->seekg(0, std::ios::beg);
			s = std::unique_ptr<std::istream>(static_cast<std::istream*>(fs));
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("BinaryReader::open: %s file %s", e.what(), filename);
			delete fs;
			throw e; // throw it back
		}
	}
	inline std::streamsize length() const { return _length; }
	// Some push routines.  Be carful as there isn't any stack checking
	inline void push() { _tellStack.push(s->tellg()); }
	inline void pop() { s->seekg(_tellStack.top()); _tellStack.pop(); }
	inline void push(std::streamoff pos) { push();  s->seekg(pos); }
	template<typename T> inline void read(T& num) const {
		s->read(reinterpret_cast<char*>(&num), sizeof(T));
	}
	template<typename T> inline void read(T* num) const {
		s->read(reinterpret_cast<char*>(num), sizeof(T));
	}
	template<typename T> inline void read(T& num, size_t count) const {
		s->read(reinterpret_cast<char*>(&num), count);
	}
	template<typename T> inline void read(T* num, size_t count) const {
		s->read(reinterpret_cast<char*>(num), count);
	}
	template<typename T> inline T read() const {
		T num;
		read(num);
		return num;
	}
	int readInt() const { return read<int32_t>(); }
	short readShort() const { return read<int16_t>(); }
	inline void seek(std::streamoff pos) { s->seekg(pos); }
	inline std::streamoff tell() const { return s->tellg(); }
	inline bool eof() const { return s->eof(); }
};

class Chunk {
	istring _name; // there are ALOT of text here here, so we want fast equals, searches and save memory
	size_t _begin;
	size_t _end;
	size_t _size;
public:
	Chunk() : _name(), _begin(0), _end(0), _size(0) {}
	Chunk(size_t begin, size_t limit) : _name(), _begin(begin), _end(limit), _size(limit-begin) {}
	Chunk(const char* name, size_t begin, size_t size) : _name(name), _begin(begin), _end(begin+size), _size(size) {}
	Chunk(istring name, size_t begin, size_t size) : _name(name), _begin(begin), _end(begin + size), _size(size) {}
	inline size_t begin() const { return _begin; }
	inline size_t end() const { return _end; }
	inline size_t size() const { return _size; }
	inline istring name() const { return _name; }
};
class ChunkReader {
	BinaryReader r;
	std::unordered_map<istring, Chunk> _chunks;

	std::vector<uint32_t> getOffsetEntries() {
		std::vector<uint32_t> entries;
		uint32_t count = r.read<uint32_t>();
		while (count > 0) {
			uint32_t offset = r.read<uint32_t>();
			entries.emplace_back(offset);
			count--;
		}
		return entries;

	}
	std::vector<uint32_t> getOffsetEntries(uint32_t start) {
		r.push(start);
		std::vector<uint32_t> vec(std::move(getOffsetEntries()));
		r.pop();
		return vec;
	}


// STRING DATA
private:
	// On strings, some of the data uses the offset to the string thats located while others use the index
	// in how the strings are read.  We keep both
	std::vector<istring> _stringIndex;
	std::unordered_map<uint32_t, istring> _stringOffsetMap;

	void doStringss() {
		r.seek(_chunks["STRG"].begin());
		std::vector<char> stringBuffer;
		stringBuffer.resize(200);
		//ChunkEntries entries(r, _chunks["STRG"]);
		for (uint32_t offset : getOffsetEntries()) {
			r.seek(offset);
			int stringSize = r.readInt();
			stringBuffer.resize(stringSize);
			r.read(stringBuffer.data(), stringSize);
			stringBuffer.push_back(0); // just on the safe size, but it should be in the file
			//std::string nstring(stringBuffer.data(), stringSize);
			istring nstring(stringBuffer.data());
			_stringIndex.push_back(nstring);
			_stringOffsetMap[offset + 4] = nstring;
		}
	}
public:
	istring stringByIndex(uint32_t i) const {
		if (i < _stringIndex.size()) return _stringIndex[i];
		else return istring();
	}
	istring stringByOffset(uint32_t i) const {
		auto it = _stringOffsetMap.find(i);
		if (it != _stringOffsetMap.end()) return it->second;
		else return istring();
	}
private: /// TEXTURES
	std::vector<istring> textureFiles;
	// textures
	void doTXRT()
	{
		textureFiles.clear();
		const Chunk& txrtChunk = _chunks["TXTR"];
		r.seek(txrtChunk.begin());
		auto textureOffsets = getOffsetEntries();
		std::vector<char> fileBuffer;
		fileBuffer.resize(100000);
		for (uint32_t i = 0; i < textureOffsets.size(); i++) {
			uint32_t offset = textureOffsets[i];
			uint32_t next_offset = (i + 1) < textureOffsets.size() ? textureOffsets[i + 1] : txrtChunk.end();
			uint32_t size = next_offset = offset;
			r.seek(offset);
			int dummy = r.readInt(); // always a 1
			uint32_t new_offset = r.readInt();
			r.seek(new_offset);
			std::string path = cocos2d::FileUtils::getInstance()->getWritablePath();
			path += "UndertaleTexture_" + std::to_string(textureFiles.size()) + ".png";
			fileBuffer.resize(size);
			r.read(fileBuffer.data(), size);
			std::fstream fs;
			fs.open(path.c_str(), std::fstream::out | std::fstream::binary);
			fs.write(fileBuffer.data(), size);
			fs.close();
			CCLOG("Saved Texture %s", path.c_str());
			textureFiles.push_back(path);
		}
	}
private: // SPRITE POS DATA
#pragma pack( push )
#pragma pack( 1 )
	struct SpriteInfo {
		short x, y, width, height, renderX, renderY, width0, height0, width1, height1, texture_id;
	};
#pragma pack( pop )
	std::vector<SpriteInfo> _spriteInfo;
	uint32_t _spritInfoOffset;
	void doTPANG() { // bulk load of the sprite data
		const Chunk& tpagChunk = _chunks["TPAG"];
		r.seek(tpagChunk.begin());
		uint32_t count = r.readInt();
		_spritInfoOffset = r.tell() + count*sizeof(int);
		r.seek(_spritInfoOffset);

		//auto tpagOffsets = getOffsetEntries(); // we don't really need these 

		_spriteInfo.resize(count);
		r.read(_spriteInfo.data(), _spriteInfo.size()); // This works only because I am 100% sure all this data is in tpang
 	}
	const SpriteInfo& lookupSpriteInfo(uint32_t fileOffset) const {
		fileOffset -= _spritInfoOffset;
		fileOffset /= sizeof(SpriteInfo);
		assert(fileOffset % sizeof(SpriteInfo));
		return _spriteInfo[fileOffset];
	}
public:
	ChunkReader(const std::string& filename) : r(filename) {
		readChunks();
	}
	void readChunks() {
		r.seek(0); // got to start
		std::streamsize full_size = r.length();
		while (r.tell() < full_size) {
			char chunkNameBuffer[5]; r.read(chunkNameBuffer, 4); chunkNameBuffer[4] = 0;
			istring chunkName = chunkNameBuffer;
			uint32_t chunkSize = r.read<uint32_t>();
			uint32_t chunkStart = r.tell();
			_chunks.emplace(std::make_pair(chunkName, Chunk(chunkName, chunkStart, chunkSize)));
			if (chunkName == "FORM") full_size = chunkSize; // special case
			else r.seek(chunkStart + chunkSize);
		}
		// This takes 2.5 seconds.  Humm.  Mabey only read the strings we NEED to read hah
		// I strings is the one thats taking up the most.  Humm.  I wonder if it would be better if we used
		// std::strings
		// Huh.  Same amount of time except the std::unodred offsetmap now takes up most of the time.  Meh
		//doStringss(); 
		//doTXRT(); // roughly 2 seconds here
		doTPANG();
	}

};