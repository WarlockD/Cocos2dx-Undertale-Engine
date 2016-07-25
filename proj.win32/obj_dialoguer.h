#pragma once
#include "UObject.h"
#include "obj_writer.h"

class obj_dialoguer : public UObject
{

private:
	friend class UObject;
	USprite* _face;
	obj_writer* _writer;
	
public:
	static constexpr int object_index = 779;
	static constexpr char* object_name = "obj_dialoguer";
	static obj_dialoguer* create();
	void reset(); // set up the dialog boxes, align face, etc
	void setString(const std::string& text);

	obj_dialoguer();

	virtual ~obj_dialoguer();
};

