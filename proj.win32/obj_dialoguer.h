#pragma once
#include "UObject.h"
#include "obj_writer.h"

class obj_face : public Node {
protected:
	size_t _emotion;
	size_t _face;
	USprite* _faceSprite;
	virtual void updateEmotion() = 0;
public:
	bool init() override;
	obj_face() : _emotion(0), _face(0), _faceSprite(nullptr) {}
	virtual ~obj_face();
	static obj_face* create(size_t face_index);
	void setEmotion(int i);
	int getEmotion() const { _emotion; }
	int getFace() const { return _face; }
	virtual void startAnimation() {}
	virtual void stopanimation() {}
};


class obj_dialoguer : public UObject
{

private:
	friend class UObject;
	obj_face* _face;
	obj_writer* _writer;
	std::queue<std::string> _dialog;
	cocos2d::EventListenerKeyboard* _keyboardListener;
	
public:
	void nextDialogLine(); // key press
	bool init() override;
	static constexpr int object_index = 779;
	static constexpr char* object_name = "obj_dialoguer";
	void reset(); // set up the dialog boxes, align face, etc
	void addString(const std::string& text);
	void setFace(size_t index);
	obj_dialoguer();
	void startDialog();

	virtual ~obj_dialoguer();
	CREATE_FUNC(obj_dialoguer);
};

