#pragma once
#include "UObject.h"


class obj_mainchara : public UObject {
public:
	static constexpr int object_index = 1570; // = 1570
	static constexpr char* object_name = "obj_mainchara";
private:
	friend class UObject;
public:
	static obj_mainchara* create();
	virtual void update(float dt) override;
};