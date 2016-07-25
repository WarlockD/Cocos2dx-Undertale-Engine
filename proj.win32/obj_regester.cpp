#include "UObject.h"
#include "obj_mainchara.h"
#include "obj_writer.h"

static std::unordered_map<size_t, std::function<UObject*(void)>> object_create_factory;

void RegesterObjectCreate(size_t obj_index, std::function<UObject*(void)> func) {
	object_create_factory[obj_index] = func;
}

UObject* CreateInstance(size_t object_index) {
	auto func = object_create_factory[object_index];
	if (func) return func();
	else return UObject::create(object_index);
}

struct RegesterAll {
	RegesterAll() {
		RegesterObjectCreate(obj_mainchara::object_index, &obj_mainchara::create);
		RegesterObjectCreate(obj_writer::object_index, &obj_writer::create);
	}
};

static RegesterAll s_regestered;