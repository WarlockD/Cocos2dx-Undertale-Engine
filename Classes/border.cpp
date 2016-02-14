#include "border.h"
#include "UndertaleResources.h"

USING_NS_CC;
Undertale::Border::Border() : _box(nullptr) {} //({ nullptr }, { nullptr }, { nullptr }, { nullptr }), Node() {}
bool Undertale::Border::init(int width, int height)
{
	int i;
	if (Node::init()) do {
		_box = DrawNode::create();
		_box->setColor(Color3B::WHITE);
		Rect r(0, 0, width, height);
		_borderRect = r;
		float x = (width) / 2;
		float y = (height) / 2;
		setAnchorPoint(Vec2(1, 0));
		Vec2 points[4];
		points[0] = Vec2(x, y);
		points[1] = Vec2(x, -y);
		points[2] = Vec2(-x, -y);
		points[3] = Vec2(-x, y);



		//	Vec2 points[4];
		//	points[0] = Vec2(x0, y0);
		//	points[1] = Vec2(x1, y0);
		//	points[2] = Vec2(x1, y1);
		//	points[3] = Vec2(x0, y1);

		//r[4] = Vec2(_ideal[0], _ideal[3]);
		_box->drawPolygon(points, 4, Color4F::BLACK, 3, Color4F::WHITE);
		_contentSize = _borderRect.size;
		addChild(_box);
		return true;
	} while (false);
	return false;
}
bool Undertale::Border::init(int x0, int x1, int y0, int y1)
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
		Rect r(x0, y0, x1 - x0, y1 - y0);
		_borderRect = r;
		float x = (x1 - x0) / 2;
		float y = (y1 - y0) / 2;

		Vec2 points[4];
		points[0] = Vec2(x, y);
		points[1] = Vec2(x, -y);
		points[2] = Vec2(-x, -y);
		points[3] = Vec2(-x, y);
	


	//	Vec2 points[4];
	//	points[0] = Vec2(x0, y0);
	//	points[1] = Vec2(x1, y0);
	//	points[2] = Vec2(x1, y1);
	//	points[3] = Vec2(x0, y1);

		//r[4] = Vec2(_ideal[0], _ideal[3]);
		_box->drawPolygon(points, 4, Color4F::BLACK,3, Color4F::WHITE);
		_contentSize = _borderRect.size;
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

Undertale::Border * Undertale::Border::create(int x0, int x1, int y0, int y1) {
	Border* o = new Border();
	if (o && o->init(x0,x1,y0,y1)) {
		o->autorelease();
		return o;
	}
	CC_SAFE_DELETE(o);
	return nullptr;
}
Undertale::Border* Undertale::Border::create(int width, int height) { // mabye thickness?  its the same thickness eveywhere so mabye not
	Border* o = new Border();
	if (o && o->init(width,height)) {
		o->autorelease();
		return o;
	}
	CC_SAFE_DELETE(o);
	return nullptr;

}