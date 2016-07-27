#include "obj_dialoguer.h"



void obj_face::setEmotion(int i) {
	if (i != _emotion) {
		_emotion = i; 
		updateEmotion();
	}
}
bool obj_face::init()  {
	if (Node::init() && (_faceSprite = USprite::create())) {
		CC_SAFE_RETAIN(_faceSprite);
		addChild(_faceSprite);
		updateEmotion();
		return true;
	}
	return false;
}
obj_face::~obj_face() {
	CC_SAFE_RELEASE_NULL(_faceSprite);
}
class obj_face_torieltalk : public obj_face {
	USprite* _glasses;
	USprite* _body;
protected:
	virtual void updateEmotion() override {
	//	if (_glasses) removeChild(_glasses);
	//	_glasses = nullptr;
		if (_emotion == 99) {
			_faceSprite->setUndertaleSprite(1986);
			setContentSize(_faceSprite->getContentSize());
			if (!_glasses) {
				addChild(_glasses = USprite::create(1989),100); // spr_face_torielglasses
				_glasses->setPosition(_glasses->getContentSize() / 2);
			}
			_emotion = 0;
		}
		else {
			switch (_emotion) {
			case 0: _faceSprite->setUndertaleSprite(1986); break;
			case 1: _faceSprite->setUndertaleSprite(2004); break;
			case 2: _faceSprite->setUndertaleSprite(1990); break;
			case 3: _faceSprite->setUndertaleSprite(1999); break;
			case 4: _faceSprite->setUndertaleSprite(2000); break;
			case 6: _faceSprite->setUndertaleSprite(1991); break;
			case 7: _faceSprite->setUndertaleSprite(1993); break;
			case 8: _faceSprite->setUndertaleSprite(1996); break;
			case 9: _faceSprite->setUndertaleSprite(1987); break;
			}
			
		}
		if (!_body) {
			addChild(_body = USprite::create(1985), -100); // "spr_face_torielbody" obj_torface.x + obj_torface.sprite_width / 2 - self.sprite_width / 2;
			Size contentSize(_body->getContentSize().width, _body->getContentSize().height + _faceSprite->getContentSize().height+9);
			setContentSize(contentSize);
			_faceSprite->setPositionY(_faceSprite->getPositionY() + 9);
			_body->setPositionY(_body->getPositionY() - 9);
	
			{// test box
				DrawNode * debug = DrawNode::create();
				Vec2 pos = getContentSize()/2;
				debug->drawRect(-pos, pos, Color4F::GREEN);
				addChild(debug, 1000);
			}
		}
	}
public:
	obj_face_torieltalk() : obj_face(), _glasses(nullptr), _body(nullptr) {}
	CREATE_FUNC(obj_face_torieltalk);
};

obj_face* obj_face::create(size_t face_index) {
	if(face_index == 1) return obj_face_torieltalk::create();
	else return nullptr;
	/*

	switch (face_index) {
	case 1: // obj_face_torieltalk
		
	case 2: // obj_face_floweytalk
	case 3: // obj_face_sans
	case 4: // obj_face_papyrus
	case 5: // obj_face_undyne
	case 6: // obj_face_alphys
	case 7: // obj_face_asgore
	case 8: // obj_face_mettaton
	case 9: //obj_face_asriel
	}
	return nullptr;
	*/
}


obj_dialoguer::obj_dialoguer() : UObject(), _face(nullptr), _writer(nullptr) {} 


obj_dialoguer::~obj_dialoguer()
{
	CC_SAFE_RELEASE_NULL(_face);
	CC_SAFE_RELEASE_NULL(_writer);
}

bool obj_dialoguer::init()  {
	if (UObject::init(obj_dialoguer::object_index)) {
		removeAllChildren();
		CC_SAFE_RELEASE_NULL(_writer);
		CC_SAFE_RELEASE_NULL(_face);
		_writer = obj_writer::create();
		CC_SAFE_RETAIN(_writer);
		_writer->setUndertaleFont(2); //generic dialog
		Size size(304 - 16, 80 - 5);

		setContentSize(size);
		DrawNode* box = DrawNode::create(3);
		box->drawSolidRect(Vec2(0, 0), size, Color4F::WHITE);
		box->drawSolidRect(Vec2(3, 3), size - Size(3, 3), Color4F::BLACK);
		//	box->drawRect(Vec2(0, 0), Vec2(304 - 16, 80 - 5), Color4F::WHITE);
		//box->setPosition(size / 2);
		box->setAnchorPoint(Vec2::ZERO);
		_writer->setAnchorPoint(Vec2::ZERO);
		addChild(box, -100);
		addChild(_writer, 100);
		_eventDispatcher->addCustomEventListener("obj_writer_halt", [this](EventCustom* e) {
			size_t halt = (size_t)e->getUserData();
		//	if (halt == 1) {
		//		this->nextDialogLine(); // key press
		//	}


		});
		_eventDispatcher->addCustomEventListener("face_change", [this](EventCustom* e) {
			size_t new_face = (size_t)e->getUserData();
			if (new_face != this->_face_index) {
				if (_face) removeChild(_face, true);
				addChild(_face = obj_face::create(new_face));
				_face->setEmotion(_emotion_index);
				reset();
				_face_index = new_face;
			}
		});
		_eventDispatcher->addCustomEventListener("emotion_change", [this](EventCustom* e) {
			size_t new_emotion = (size_t)e->getUserData();
			if (new_emotion != _emotion_index) {
				if (_face) _face->setEmotion((size_t)e->getUserData());
				_emotion_index = new_emotion;
			}
			
		});
		return true;
	}
	return false;
}

void obj_dialoguer::nextDialogLine() {
	_writer->clear();
	if (!_dialog.empty()) {
		_writer->setString(_dialog.front(),false);
		_dialog.pop();
		_writer->start();
	}
}
void obj_dialoguer::setFace(size_t index) {
	_eventDispatcher->dispatchCustomEvent("face_change", (void*)index);
}

void obj_dialoguer::setEmotion(size_t index) {
	_eventDispatcher->dispatchCustomEvent("emotion_change", (void*)index);
}
void obj_dialoguer::reset() {
	const Size& size = getContentSize();
	_writer->setAnchorPoint(Vec2(0, 1));
	if (_face) {
		_face->setAnchorPoint(Vec2(-0.5,-0.5));
		_face->setPosition(4, 8);
		_writer->setPosition(68, getContentSize().height / 2);
	}
	else {
		_writer->setPosition(3, getContentSize().height / 2);
	}
}
void obj_dialoguer::addString(const std::string& text) {
	_dialog.push(text);
}

void obj_dialoguer::startDialog() {
	if (_keyboardListener != nullptr) {
		_keyboardListener = EventListenerKeyboard::create();
		_keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
			this->nextDialogLine();
		};
		_eventDispatcher->addEventListenerWithSceneGraphPriority(_keyboardListener, this);
	}
	this->nextDialogLine();

}