#include "UndertaleResources.h"

// This is a GREAT simple to use library
#include <mspack.h>
// Only problem is that it expects a raw cab file and we are opening an EXE with an embeded cab file.
// So we have to use custom rutinees.  Honstly we just need to override the open and seek, but there is no way 
// to just replace those pointers at run time
#include <fstream>


class my_file {
	static const uint32_t cabSig = 0x4643534D;
	std::fstream fs;
	std::streampos cabOffset;;
	size_t fileLen;
	mspack_system* _sys;
	std::string _filename;
	inline bool checkSignature(const char* data) const {
		return data[0] == 'M' && data[1] == 'S' && data[2] == 'C' && data[3] == 'F';
	}
	bool lookForCabFileStart() {
		// Since I know the undertell file is fairly close

		try {
			fs.seekg(0, fs.end);
			fileLen = fs.tellg();
			cabOffset = 0;

			fs.seekg(0, fs.beg);
			while (!fs.eof()) {
				union {
					uint32_t num;
					char buffer[sizeof(uint32_t)];
				} data;
				fs.read(data.buffer, sizeof(uint32_t));
				if (checkSignature(data.buffer)) {
					// found it
					fs.seekg(-static_cast<std::streamoff>(sizeof(uint32_t)), fs.cur); // backup
					cabOffset = fs.tellg();
					return true;
				}
			}
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("my_file::lookForCabFileStart: %s", e.what());
			return false;
		}
		close();
		return false;
	}

public:
	const char* filename() {
		if (fs.is_open() && fs.good())
			return _filename.c_str();
		else return nullptr;
	}
	my_file(mspack_system* sys) : cabOffset(0), fileLen(0), _sys(sys) {
		fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	}
	~my_file() {
		if (fs.is_open()) fs.close();
	}
	bool open(const char* filename, int mode) {
		switch (mode) {
		case MSPACK_SYS_OPEN_READ:   mode = std::ios::binary | std::ios::in;  break;
		case MSPACK_SYS_OPEN_WRITE:  mode = std::ios::binary | std::ios::out; break;
		case MSPACK_SYS_OPEN_UPDATE: mode = std::ios::binary | std::ios::out | std::ios::in; break;
		case MSPACK_SYS_OPEN_APPEND: mode = std::ios::binary | std::ios::app;  break;
		default:
			return false;
		}
		try {
			fs.open(filename, mode);
			_filename = filename;
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("my_file::open: %s file %s", e.what(), filename);
			return false;
		}
		return mode == (std::ios::binary | std::ios::in) ? lookForCabFileStart() : true;
	}
	void close() {
		_filename = "";
		fs.close();
	}
	int read(void* buffer, int bytes) {
		try {
			size_t currentOffset = fs.tellg();
			if (currentOffset + bytes >= fileLen) bytes = fileLen - currentOffset;
			fs.read(static_cast<char*>(buffer), bytes);
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("my_file::read: %s", e.what());
			return -1;
		}
		return bytes;
	}
	int write(void* buffer, int bytes) {
		try {
			fs.write(static_cast<char*>(buffer), bytes);
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("my_file::write: %s", e.what());
			return -1;
		}
		return bytes;
	}
	size_t tell() {
		try {
			return fs.tellg() - cabOffset;
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("my_file::tell: %s", e.what());
			return -1L;
		}
	}
	int seek(off_t offset, int mode) {
		switch (mode) {
		case MSPACK_SYS_SEEK_START: mode = std::fstream::beg; offset += cabOffset;  break;
		case MSPACK_SYS_SEEK_CUR:   mode = std::fstream::cur; break;
		case MSPACK_SYS_SEEK_END:   mode = std::fstream::end; break;
		default: return -1;
		}
		try {
			fs.seekg(offset, mode);
		}
		catch (std::ifstream::failure e) {
			CCLOGERROR("my_file::seek: %s", e.what());
			return -1;
		}
		return 0;
	}


};
static char msgbuf[128];
static void msg(const char* filename, const char *format, va_list ap) {
	vsnprintf_s(msgbuf, sizeof(msgbuf), format, ap);
	CCLOGERROR("%s: %s", filename, msgbuf);
}
static void msg(const char *format, va_list ap) {
	vsnprintf_s(msgbuf, sizeof(msgbuf), format, ap);
	CCLOGERROR(msgbuf);
}
static void msg(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	msg(format, ap);
	va_end(ap);
}
static mspack_file *map_open(struct mspack_system *self, const char *filename, int mode)
{
	my_file* fh = new my_file(self);
	if (fh && fh->open(filename, mode))
		return (mspack_file*)fh;
	delete fh;
	return nullptr;
}

static void map_close(struct mspack_file *file) {
	if (file == nullptr) return;
	((my_file*)file)->close();
}

static int map_read(struct mspack_file *file, void *buffer, int bytes) {
	if (file == nullptr) return -1;
	return ((my_file*)file)->read(buffer, bytes);
}

static int map_write(struct mspack_file *file, void *buffer, int bytes) {
	if (file == nullptr) return -1;
	return ((my_file*)file)->write(buffer, bytes);
}

static int map_seek(struct mspack_file *file, off_t offset, int mode) {
	if (file == nullptr) return -1;
	return ((my_file*)file)->seek(offset, mode);
}

static off_t map_tell(struct mspack_file *file) {
	if (file == nullptr) return -1;
	return ((my_file*)file)->tell();
}

static void map_msg(struct mspack_file *file, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	const char* filename = ((my_file*)file)->filename();
	if (filename)
		msg(filename, format, ap);
	else
		msg(format, ap);
	va_end(ap);
}




static void map_copy(void *src, void *dest, size_t bytes) {
	memcpy(dest, src, bytes);
}

static void *map_alloc(struct mspack_system *self, size_t bytes) { return static_cast<void*>(new char[bytes]); }
static void map_free(void *buffer) { delete buffer; }

static struct mspack_system my_system = {
	&map_open, &map_close, &map_read,  &map_write, &map_seek,
	&map_tell, &map_msg, &map_alloc, &map_free, &map_copy, nullptr
};


//mspack_system

bool testCabHeader(const char* data) {
	if (data[0] == 'M' && data[1] == 'S' && data[2] == 'C' && data[3] == 'F') return true;
	else return false;
	// The next unsigned int is reserved zero and the next one is the cab file size in bytes
	// this should be enough 
}
#define TEST_PTR(t, msg) if(!(t)) { CCLOGERROR(msg); break; }

static const char * UndertaleFilenameEXE = "UNDERTALE.EXE";
static const char * UndertaleFilnameDataWin = "data.win";


bool UndertaleResources::findUndertaleData() {
	std::string fullPath;
	mscab_decompressor* cabd = nullptr;
	mscabd_cabinet *cab = nullptr;
	mscabd_file *file = nullptr;
	bool failed = true;
	while (true) { 
		fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(UndertaleFilnameDataWin);
		if (fullPath.size() != 0) break;
		fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(UndertaleFilenameEXE);
		TEST_PTR(fullPath.size() != 0, "Cannot find data.win or UNDERTALE.EXE");

		int test;
		MSPACK_SYS_SELFTEST(test);
		TEST_PTR(test == MSPACK_ERR_OK, "MSPACK Test failed");

		cabd = mspack_create_cab_decompressor(&my_system);
		TEST_PTR(cabd, "Cannot create decompressor");
		cab = cabd->open(cabd, fullPath.c_str());
		TEST_PTR(cab, "Cannot open cab");
		for (file = cab->files; file; file = file->next) {
			if (strcmp(UndertaleFilnameDataWin, file->filename) == 0) {
				std::string path = cocos2d::FileUtils::getInstance()->getWritablePath();
				path += file->filename;
				TEST_PTR(cabd->extract(cabd, file, path.c_str()) == MSPACK_ERR_OK, "Could not extract data.win");
				CCLOG("TryToDoEXECAB: Saved %s", path.c_str());
				fullPath = path;
				failed = false;
				break;
			}
			//CCLOG("Filename: %s", file->filename);
		}
		break;
	}
	_data_win_path = fullPath;
	if (cab) cabd->close(cabd, cab);
	if (cabd) { mspack_destroy_cab_decompressor(cabd); cabd = nullptr; }
	return failed;
}
