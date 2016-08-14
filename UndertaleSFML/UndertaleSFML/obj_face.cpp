#include "obj_writer.h"

void obj_face::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	for (auto& s : _sprites) target.draw(s, states);
}
class obj_animation {
	size_t _current;
	size_t _frame_time;
public:

};
class obj_face_torielblink : public Component<767, obj_face_torielblink, obj_face> {
	size_t _current;
	size_t _frame_time;
	size_t _blink_delay;
protected:
	obj_face_torielblink() : _frame_time(0), _current(0), _blink_delay(2000) {}
	void update(sf::Time dt) override {
		if(dt.asMilliseconds())
	}
	void setEmotion(uint32_t i) override {
		if (_emotion != i) {
			_emotion = i;
			_sprites.clear();
			switch (_emotion) {
			case 99:
			case 0:
				_sprites.emplace_back(1986);
				if (_emotion == 99) {
					_sprites.emplace_back(1989); 
					_emotion = 0;
				} 
				break;
			case 1:
				_sprites.emplace_back(2004);
				break;
			case 2:
				_sprites.emplace_back(1990);
				break;
			case 3:
				_sprites.emplace_back(1999);
				break;
			case 4:
				_sprites.emplace_back(2000);
				break;
			case 5:
				_sprites.emplace_back(1997);
				break;
			case 6:
				_sprites.emplace_back(1991);
				break;
			case 7:
				_sprites.emplace_back(1993);
				break;
			case 8:
				_sprites.emplace_back(1996);
				break;
			case 9:
				_sprites.emplace_back(1987);
				break;
			}
		}
	}
};

std::unique_ptr<obj_face> obj_face::getFace(size_t index) {
	std::unique_ptr<obj_face> obj;
//	if (obj_face_torielblink::ID == index) 
		obj = obj_face_torielblink::create();
		
	if (obj) {
		obj->_emotion = 10000;
		obj->setEmotion(0); // force the switch
	}
	return obj;
}