#include "binaryReader.h"

#include <iostream>
#include <istream>
#include <streambuf>
#include <string>


// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
// REALLY awsome
// but HAD to change the vars and the way it was structured...  just my personal issues
class char_array_buffer : public std::streambuf
{
private:
	const char * const _begin;
	const char * const _end;
	const char * _current;
public:

	char_array_buffer(const char* begin, const char* end) : _begin(begin), _end(end), _current(begin) {}
	char_array_buffer(const char* begin, size_t length) : _begin(begin), _end(begin+ length), _current(begin) {}
	explicit char_array_buffer(const char* str) : char_array_buffer(str, strlen(str)) {}

private:
	// copy ctor and assignment not implemented;
	// copying not allowed
	char_array_buffer(const char_array_buffer &);
	char_array_buffer &operator= (const char_array_buffer &);
	int_type underflow() override
	{
		if (_current == _end) return traits_type::eof();
		return traits_type::to_int_type(*_current);
	}
	int_type uflow() override
	{
		if (_current == _end) return traits_type::eof();
		return traits_type::to_int_type(*_current++);
	}
	int_type pbackfail(int_type ch) override
	{
		if (_current == _begin || (ch != traits_type::eof() && ch != _current[-1])) return traits_type::eof();
		return traits_type::to_int_type(*--_current);
	}
	
	std::streamsize showmanyc() override
	{
		assert(std::less_equal<const char *>()(_current, _end));
		return _end - _current;
	}
};



BinaryReader::BinaryReader(std::istream& s, std::streamsize length) : s(&s), _length(length) { }
BinaryReader::BinaryReader(std::istream& s) : s(&s) {
	push();
	s.seekg(0, std::ios::end);
	_length = s.tellg();
	s.seekg(0, std::ios::beg);
	pop();
}


std::string BinaryReader::readString() { // read a zero terminated string
	std::string builder;
	int c;
	while ((c = s->get()) != -1 && c != 0) builder.push_back(c);
	return builder;
}
std::string BinaryReader::readFixedString(size_t len) { // read a fixed sized string, might not end in 0
	std::string builder;
	int c;
	while ((c = s->get()) != -1 && --len) builder.push_back(c);
	return builder;
}
std::string BinaryReader::readStringAtOffset(std::streamoff offset) { 	// read zero string at offset
	push(offset);
	std::string builder = readString();
	pop();
	return builder;
}
BinaryVectorReader::BinaryVectorReader(const std::vector<char>& data) : BinaryReader(dynamic_cast<std::istream*>(new char_array_buffer(data.data(),data.size())), data.size()) {}
BinaryVectorReader::BinaryVectorReader(const char* data, size_t length) : BinaryReader(dynamic_cast<std::istream*>(new char_array_buffer(data, length)), length) {}
BinaryVectorReader::~BinaryVectorReader(){
	if (s) { delete s; s = nullptr; }
};


BinaryFileReader::BinaryFileReader() : BinaryReader(new std::fstream, 0) {}
BinaryFileReader::~BinaryFileReader() { if (s) { delete s; s = nullptr; } }

bool BinaryFileReader::open(const std::string& filename) {
	std::fstream* fs = dynamic_cast<std::fstream*>(s);
//	fs->exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fs->close();
	_length = 0;
	try {
		
		fs->open(filename.c_str(), std::ios::binary | std::ios::in);
		fs->seekg(0, std::ios::end);
		_length = fs->tellg();
		fs->seekg(0, std::ios::beg);
	}
	catch (std::ifstream::failure e) {
		fs->exceptions(0);
		CCLOGERROR("BinaryReader::open: %s file %s", e.what(), filename);
		//throw e; // throw it back
		return false; // or return, one of the two
	}
	return true;
}
void BinaryFileReader::close()
{
	std::fstream* fs = dynamic_cast<std::fstream*>(s);
	fs->close();
}

