#include "obj_gasterblaster.h"
#include "UndertaleResources.h"

USING_NS_CC;

namespace Undertale {
	obj_gasterblaster::obj_gasterblaster() : LuaSprite() {
		con = 1;
		ideal = Vec2(200, 200);
		idealrot = 90;

		skip = 1;
		pause = 8;
		col_o = 0;
		bt = 0;
		btimer = 0;
		fade = 1;
		terminal = 10;
		bb = 0;
		bbsiner = 0;
		innate_karma = 10;
		for (int i = 0; i < 3; i++) {
			blaster[i] = DrawNode::create();
			addChild(blaster[i], -1);
		}

	}
	obj_gasterblaster* obj_gasterblaster::create(float x, float y) { return create(Vec2(x, y)); }

	obj_gasterblaster * obj_gasterblaster::create(Vec2 pos)
	{
		obj_gasterblaster* obj = new obj_gasterblaster();
		if (obj && obj->init("spr_gasterblaster")) {
			obj->autorelease();
			obj->setScale(1, 1);
			obj->setPosition(pos);
			return obj;
		}
		return nullptr;
	}

	void obj_gasterblaster::setupBullet(Vec2 pos)
	{
		LuaSprite::setupBullet(getPosition());
		ideal = pos;
		con = 1;
		idealrot = 90;
		skip = 1;
		pause = 8;
		col_o = 0;
		bt = 0;
		btimer = 0;
		fade = 1;
		terminal = 10;
		bb = 0;
		bbsiner = 0;
		innate_karma = 10;
		alarm = -1;
		for (int i = 0; i < 3; i++) blaster[i]->clear();
	}

	bool obj_gasterblaster::stepBullet(float dt)
	{
		if (alarm > 0) { alarm--; return true; }
		else if (alarm == 0) { con++; alarm = -1; }


		if (con == 1 && skip == 0) {
			Vec2 pos = this->getPosition();
			float x = pos.x + std::floorf((ideal.x - pos.x) / 3.0f);
			float y = pos.y + std::floorf((ideal.y - pos.y) / 3.0f);
			if (x < ideal.x) x += 1;
			if (y < ideal.y) y += 1;
			if (x > ideal.x) x -= 1;
			if (y > ideal.y) y -= 1;
			if (std::abs(x - ideal.x) < 3.0f) x = ideal.x;
			if (std::abs(y - ideal.y) < 3.0f) y = ideal.y;
			if (std::abs(x - ideal.x) < 0.1f || (std::abs(y - ideal.y) < 0.1f)) {
				con = 2; alarm = 2;
			}
			setPosition(x, y);
		}
		if (con == 1 && skip == 1) {
			Vec2 pos = this->getPosition();
			float angle = getRotation();
			float x = pos.x + std::floorf((ideal.x - pos.x) / 3.0f);
			float y = pos.y + std::floorf((ideal.y - pos.y) / 3.0f);
			if (x < ideal.x) x += 1;
			if (y < ideal.y) y += 1;
			if (x > ideal.x) x -= 1;
			if (y > ideal.y) y -= 1;
			if (std::abs(x - ideal.x) < 3.0f) x = ideal.x;
			if (std::abs(y - ideal.y) < 3.0f) y = ideal.y;
			angle += std::floorf(((float)idealrot - angle) / 3.0f);
			if (angle < idealrot) angle += 1;
			if (angle > idealrot) angle -= 1;
			if (std::abs(angle - idealrot) < 3) angle = idealrot;
			if (((std::abs(pos.x - ideal.x) < 0.1) && (std::abs(pos.y - ideal.y) < 0.1)) && (std::abs(idealrot - angle) < 0.01)) {
				con = 4; alarm = pause;
			}
			setPosition(x, y);
			setRotation(angle);
		}
		if (con == 3) {
			float angle = getRotation();
			angle += std::floorf((idealrot - angle) / 3.0f);
			if (angle < idealrot) angle += 1;
			if (angle > idealrot) angle -= 1;
			if (std::abs(angle - idealrot) < 3) angle = idealrot;
			if (std::abs(angle - idealrot) < 0.01) {
				con = 4; alarm = pause;
			}
			setRotation(angle);
		}
		if (con == 5) {
			con = 6;
			alarm = 4;
		}
		if (con == 6) setImageIndex(getImageIndex() + 1);
		if (con == 7) {
			if (getImageIndex() == 4)  setImageIndex(5);
			else if (getImageIndex() == 5) setImageIndex(4);
			//setDirection(idealrot + 90);
			//   if( btimer == 0 ) with(obj_sansb) p_beam = 1;
			if (btimer == 0 && getScaleX() >= 2) {
				//sh = instance_create("obj_sans_shaker", 0, 0);
				//sh.intensity = 5;
			}
			btimer++;
			if (btimer < 5) {
				setSpeed(getSpeed() + 1);
				bt = bt + std::floorf((35 * getScaleX()) / 4.0f);
			}
			else setSpeed(getSpeed() + 4);

			if (btimer > (5 + terminal)) {
				bt *= 0.8f;
				fade -= 0.1f;
				int nfade = (int)(fade / 1.0f * 256);
				setOpacity(nfade);
				if (bt <= 2) con = 8;// void instance_destroy();
			}
			Vec2 pos = getPosition();
			float angle = getRotation();
			float xscale = getScaleX();
			Rect rect = this->getBoundingBox();
			Rect room = this->getParent()->getBoundingBox();
#if 0
			// Not sure what this does
			if (rect.origin.x < -rect.size.width) setSpeed(0);
			if (rect.origin.x > (room.size.width + rect.size.width)) setSpeed(0);
			if (rect.origin.y < -rect.size.height) setSpeed(0);
			if (rect.origin.y > (room.size.height + rect.size.height)) setSpeed(0);
#endif
			// lets fire the laser!
			// make lasers visiable
			bbsiner = bbsiner + 1;
			float bb = (std::sinf(bbsiner / 1.5) * bt) / 4.0f;
			float xx = lengthdir_x(70, (angle - 90)) * (xscale / 2.0f);
			float yy = lengthdir_y(70, (angle - 90)) * (xscale / 2.0f);
			float rr = random(0.0f, 2.0f) - random(0.0f, 2.0f);
			float rr2 = random(0.0f, 2.0f) - random(0.0f, 2.0f);
			float xxx = lengthdir_x(1000, (angle - 90));
			float yyy = lengthdir_y(1000, (angle - 90));
			Vec2 from(pos.x + xx + rr, pos.y + yy + rr2);
			blaster[0]->clear();
			blaster[0]->setLineWidth(bt + bb);
			blaster[0]->setRotation(getRotation());
			blaster[0]->setVisible(true);
			blaster[0]->drawLine(from, Vec2(pos.x + xxx + rr, pos.y + yyy + rr2), Color4F::WHITE);

			float xxa = lengthdir_x(50, (angle - 90)) * (xscale / 2.0f);
			float yya = lengthdir_y(50, (angle - 90)) * (xscale / 2.0f);
			float xxb = lengthdir_x(60, (angle - 90)) * (xscale / 2.0f);
			float yyb = lengthdir_y(60, (angle - 90)) * (xscale / 2.0f);

			blaster[1]->clear();
			blaster[1]->setLineWidth((bt / 2) + bb);
			blaster[1]->setRotation(getRotation());
			blaster[1]->setVisible(true);
			blaster[1]->drawLine(from, Vec2(pos.x + xxa + rr, pos.y + yya + yy + rr2), Color4F::WHITE);

			blaster[2]->clear();
			blaster[2]->setLineWidth((bt / 1.25) + bb);
			blaster[2]->setRotation(getRotation());
			blaster[2]->setVisible(true);
			blaster[2]->drawLine(from, Vec2(pos.x + xxb + rr, pos.y + yyb + yy + rr2), Color4F::WHITE);

			float nx_factor = lengthdir_x(1, angle);
			float ny_factor = lengthdir_y(1, angle);
#if 0
			if (col_o == 0 && fade > 0.8) { // I see, we only check the line once we draw the original laser

				for (int cl = 0; cl < 3; cl++) {
					/// check for line colision
				//	if !collision_line(((self.x + self.xx) - (((self.nx_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))), ((self.y + self.yy) - (((self.ny_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))), ((self.x + self.xxx) - (((self.nx_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))), ((self.y + self.yyy) - (((self.ny_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))), 743, 0, 1) then goto Label_38

				}
				for (int cl = 0; cl < 3; cl++) {
					/// check for line colision
					//	  if !collision_line(((self.x + self.xx) + (((self.nx_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))),((self.y + self.yy) + (((self.ny_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))),((self.x + self.xxx) + (((self.nx_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))),((self.y + self.yyy) + (((self.ny_factor * self.bt) / (Double)(2)) * (self.cl / (Double)(4)))),743,0,1) then goto Label_40

				}
			}
			if (col_o == 0) col_o = 1;
#endif
		}
		return false;
	}
	
}