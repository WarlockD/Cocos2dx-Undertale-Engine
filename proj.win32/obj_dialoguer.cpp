#include "obj_dialoguer.h"



obj_dialoguer::obj_dialoguer() : UObject(), _face(nullptr), _writer(nullptr) {} 


obj_dialoguer::~obj_dialoguer()
{
}
obj_dialoguer* obj_dialoguer::create() {
	obj_dialoguer* obj = new obj_dialoguer;
	if (obj && obj->init(obj_dialoguer::object_index)) {
		obj->setAnchorPoint(Vec2(0, 1));
		obj->autorelease();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}
void obj_dialoguer::reset() {

	/*

	if (instance_exists(1570)) {
		if (obj_mainchara.y > self.yy + 130) {
			self.side = 0;
			if (global.facechoice != 0) {
				self.writer = instance_create(self.xx + 68, self.yy - 5, 782/* OBJ_WRITER );
			//	script_execute(144 scr_facechoice );
			}
			else  self.writer = instance_create(self.xx + 10, self.yy - 5, 782/* OBJ_WRITER );
		}
		else {
			self.side = 1;
			if (global.facechoice != 0) {
				self.writer = instance_create(self.xx + 68, self.yy + 150, 782/* OBJ_WRITER );
				//script_execute(144 scr_facechoice );
			}
			else  self.writer = instance_create(self.xx + 10, self.yy + 150, 782/* OBJ_WRITER );
		}
	}
	*/
}
void obj_dialoguer::setString(const std::string& text) {
	removeAllChildrenWithCleanup(true);
	Size size(304 - 16, 80 - 5);

	setContentSize(size);
	DrawNode* box = DrawNode::create(3);
	box->drawSolidRect(Vec2(0, 0), Vec2(304-16, 80-5), Color4F::WHITE);
	box->drawSolidRect(Vec2(3, 3), Vec2(301 - 19, 77 - 8), Color4F::BLACK);
	
	//box->setPosition(size / 2);
	addChild(box, -1);
	addChild(_writer = obj_writer::create());
	//_writer->setAnchorPoint(Vec2(0, 1));
//	_writer->setPosition(10, 10);

	/*

	UObject* mainchara = _room->instance_exists(1570);
	if (mainchara != nullptr) { // obj_mainchara
		if (mainchara->getPosition().y > 130)
			_writer->setPosition(10, -5);
		else
			_writer->setPosition(10, 150);
	}
		*/
	_writer->setString(text);
	reset();
	_writer->start();
	/*

	if (instance_exists(self.writer) && self.writer.writingy > self.yy + 80)
		self.writer.writingy -= 155;
	if (instance_exists(774/* obj_face ) && obj_face.y > self.yy + 80)
		obj_face.y -= 155;
	*/
}