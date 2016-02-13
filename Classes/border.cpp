#include "border.h"
#include "UndertaleResources.h"

USING_NS_CC;
Undertale::Border::Border() : _box(nullptr) {} //({ nullptr }, { nullptr }, { nullptr }, { nullptr }), Node() {}

bool Undertale::Border::init(const int* ideal)
{
	int i;
	if (Node::init()) do {
		//UndertaleResources* res = UndertaleResources::getInstance();
		//Vector<SpriteFrame*>* frames = res->getSpriteFrames("spr_border");
		//if (!(_sides[0] = LuaSprite::create(frames)) break;
	//	if (!(_sides[1] = LuaSprite::create(frames)) break;
		//if (!(_sides[2] = LuaSprite::create(frames)) break;
		//if (!(_sides[3] = LuaSprite::create(frames)) break;
		_box = DrawNode::create();
		_box->setColor(Color3B::WHITE);
		memcpy(_ideal, ideal, sizeof(int) * 4);
		Vec2 points[5];
		points[0] = Vec2(_ideal[0], _ideal[3]);
		points[1] = Vec2(_ideal[0], _ideal[2]);
		points[2] = Vec2(_ideal[1], _ideal[2]);
		points[3] = Vec2(_ideal[1], _ideal[3]);
		//r[4] = Vec2(_ideal[0], _ideal[3]);
		_box->drawPolygon(points, 4, Color4F::BLACK, 5, Color4F::WHITE);
		_contentSize = Size(_ideal[2] - _ideal[0], _ideal[1] - _ideal[3]);
		addChild(_box);
		return true;
	} while (false);
	return false;
}
void Undertale::Border::caculateBorders(){
	
}

Undertale::Border::~Border()
{
}

Undertale::Border * Undertale::Border::create(const int r[4]) {
	Border* o = new Border();
	if (o && o->init((const int*)r)) {
		o->autorelease();
		return o;
	}
	CC_SAFE_DELETE(o);
	return nullptr;
}
/*

Undertale::Border * Undertale::Border::create(const int* r)
{
	Border* o = new Border();
	if (o && o->init(r)) {
		o->autorelease();
		return o;
	}
	CC_SAFE_DELETE(o);
	return nullptr;
}
*/