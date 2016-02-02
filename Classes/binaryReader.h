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
protected:
	std::istream* s;
	std::streamsize _length;
	std::stack<std::streamoff> _tellStack;
	BinaryReader() : s(nullptr), _length(0) {}
	BinaryReader(std::istream* s, size_t length) : s(s), _length(length) {}
public:
	BinaryReader(std::istream& s);
	BinaryReader(std::istream& s, std::streamsize length);
	BinaryReader(const BinaryReader& o) = delete;
	BinaryReader& operator=(const BinaryReader& o) = delete;
	BinaryReader(BinaryReader&& o) : s(o.s), _length(o._length) { o.s = nullptr; o._length = 0; }
	BinaryReader& operator=(BinaryReader&& o) { *this = BinaryReader(std::move(o)); return *this; }
	virtual ~BinaryReader() {}
	std::string readString(); // read a zero terminated string
	std::string readFixedString(size_t len); // read a fixed sized string, might not end in 0
	std::string readStringAtOffset(std::streamoff offset); // read zero string at offset but return back

	std::streamsize length() const { return _length;  }
	bool is_open() const { return s && s->good(); }
	// Some push routines.  Be carful as there isn't any stack checking
	inline void push() { _tellStack.push(s->tellg()); }
	inline void pop() { s->seekg(_tellStack.top()); _tellStack.pop(); }
	inline void push(std::streamoff pos) { push();  s->seekg(pos); }
	template<typename T> inline void read(T& num) const {
		s->read(reinterpret_cast<char*>(&num), sizeof(char) * sizeof(T));
	}
	template<typename T> inline void read(T* num) const {
		s->read(reinterpret_cast<char*>(num), sizeof(char) * sizeof(T));
	}
	template<typename T> inline void read(T& num, size_t count) const {
		s->read(reinterpret_cast<char*>(&num), count*sizeof(T));
	}
	template<typename T> inline void read(T* num, size_t count) const {
		s->read(reinterpret_cast<char*>(num), count*sizeof(T));
	}
	template<typename T> inline T read() const {
		T num;
		read(num);
		return num;
	}
	inline int readInt() const { return read<int32_t>(); }
	inline short readShort() const { return read<int16_t>(); }
	inline bool readBool() const {
		int b = readInt();
		assert(b == 0 || b == 1);
		return b != 0;
	}
	inline float readSingle() const { return read<float>(); }
	inline void seek(std::streamoff pos) { s->seekg(pos); }
	inline std::streamoff tell() const { return s->tellg(); }
	inline bool eof() const { return s->bad() || s->eof(); }

	template<typename T> inline void foreward() { s->seekg(sizeof(T), std::ios::cur); }
	template<typename T> inline void foreward(size_t count) { s->seekg(sizeof(T)*count, std::ios::cur); }
	inline void foreward(size_t count) { s->seekg(count, std::ios::cur); }

	template<typename T> inline void backward() { s->seekg(-((std::streamoff)sizeof(T)), std::ios::cur); }
	template<typename T> inline void backward(size_t count) { s->seekg(sizeof(T)*(-((std::streamoff)count)), std::ios::cur); }
	inline void backward(size_t count) { s->seekg(-((std::streamoff)count), std::ios::cur); }
};

class BinaryFileReader : public BinaryReader {	
public:
	BinaryFileReader();
	~BinaryFileReader();
	bool open(const std::string& filename);
	void close();
};

class BinaryVectorReader : public BinaryReader {
public:
	BinaryVectorReader(const std::vector<char>& data);
	BinaryVectorReader(const char* data, size_t length);

	~BinaryVectorReader();
};