#include "TextWriter.h"

USING_NS_CC;
/*
if string_char_at(originalstring,stringpos)!=" "
if string_char_at(originalstring,stringpos)!="&"
if string_char_at(originalstring,stringpos)!="^"
if string_char_at(originalstring,stringpos-1)!="^"
if string_char_at(originalstring,stringpos)!="\"
if string_char_at(originalstring,stringpos-1)!="\"
if string_char_at(originalstring,stringpos)!="/"
if string_char_at(originalstring,stringpos)!="%"
{
sound_stop(txtsound)
sound_play(txtsound)
}
if string_char_at(originalstring,stringpos)="&" then stringpos+=1
if string_char_at(originalstring,stringpos)="\" then stringpos+=2
*/

// Sort of an all in one action for the text typing
class Letterhake : public Action {
	CC_SYNTHESIZE(int, _shake, Shake);
	CC_SYNTHESIZE(Color3B, _color, Color);
	CC_SYNTHESIZE(float, _delay, Delay);
private:
	Point _inital;;
	bool _delayDone;
	float _start_time;
public:
	Letterhake() : _shake(0), _inital(0, 0), _delayDone(false), _start_time(0.0f) {}
	bool init(Color3B color, int shake, float delay)
	{
		_color = color;
		_delay = delay;
		_shake = shake;
		_start_time = 0.0f;
		_delayDone = false;
		_inital = Point();
		return true;
	}
	static Letterhake* create(Color3B color, int shake=0,float delay=0.500f)
	{
		//MoveTo
		Letterhake *action = new Letterhake();
		if (action && action->init(color, shake, delay)) {
			action->autorelease();
			return action;
		}
		delete action;
		return (action = nullptr);
	}
	bool isDone() const override {
		return false;
	} // always shaking ALL the time
	void step(float dt) override {
		if (_target == nullptr) return; // we do nothing
		for (;;) { // sigh, did I just over optimize here?
			if (_delayDone) break;
			_start_time += dt;
			if (_start_time < _delay) return;
			_start_time = 0.0f;// we reuse this for update
			_delayDone = true;
			_inital = _target->getPosition();
	    	//_target->setVisible(true); // dosn't work cause Label makes eveything invisiable
			_target->setScale(1.0f);
			CCLOG("Displaying Sprite '%c' (%f,%f) (%f,%f)", this->getTag(), _inital.x, _inital.y, _target->getPositionX(), _target->getPositionY());
		}
		update(dt);
	}
	Letterhake* clone() const override  {
		// no copy constructor    
		auto a = new (std::nothrow) Letterhake();
		a->init(_color, _shake, _delay);
		a->autorelease();
		return a;
	}
	void update(float delta) override {
		if (_target == nullptr || !_delayDone) return; // we do nothing
		_start_time += delta;
		if (_start_time < 0.100) return; // 10 times a second?
		_start_time = 0.0f;
		Point p = _inital;
		if (_shake > 38) {
			switch (_shake) {
			case 39: // earthquake?
			case 40: // all rolls with movement
			case 41: // demosceen roll, up and down? direction 10
			case 42: // drifting text direction 20
			case 43: // singing
				p = Point((p.x *0.70),(p.y*0.70)); // direction 30
			}
		}
		else if (_shake > 0) {
			float rndx = RandomHelper::random_real(0.0f,(float)_shake) - (_shake / 2);
			float rndy = RandomHelper::random_real(0.0f,(float)_shake) - (_shake / 2);
			p += Point(rndx, rndy);
		}
		_target->setPosition(p);
	}
	void startWithTarget(Node* target) override {
		_start_time = 0.0f;
		_delayDone = false;
		_inital = target->getPosition();
		target->setColor(_color);
		target->setScale(0);
	    target->setVisible(false);
		Action::startWithTarget(target);
	}
	void stop() override {
		_target->setPosition(_inital);
		Action::stop();
	}
};
/* In this class I could either
	A: Rebuild the Label class and handle all the letter to text but with more single letter control
	B: Just put a label in here and modify it each frame
	B is just easyer:P
*/
inline int quickDidgit(char c) {
	if (!isdigit(c)) return -1;
	return '0' - c;
}



struct Token {
	int letter;
	Action* action;
};

void TextWriter::update(float f) {
	Label::update(f);
	
}
Action* makeAction(int shake, float delay) {
	auto hideAction = Hide::create();
	auto showAction = Show::create();
	auto delayAction = DelayTime::create(delay);
	auto sequence = Sequence::create(hideAction, delayAction, showAction, nullptr);
	return sequence;
}

void TextWriter::startTyping()
{
	const std::string& text = getString();
	size_t len = text.length();
	float delay = 0.500;
	float added_delay = delay;
	for (int i = 0; i < len; i++) {
		//const Token& t = tokens[i];
		Sprite* spr = getLetter(i);
		spr->runAction(makeAction(40, added_delay));
		added_delay += delay;
		spr->setTag(text[i]);
		//spr->runAction(makeAction(shake,(delay / label_text.length() - 1)*i));
	}
}

TextWriter * TextWriter::createWithBMFont(const std::string & bmfontFilePath, const std::string & text, int shake)
{
	float delay = 0.500;
	TextWriter* tw = new TextWriter();
	if (tw && tw->setBMFontFilePath(bmfontFilePath)) {
		tw->autorelease();
	
		//set all the characters scale to zero
	//	tw->setVisible(false);
		float current_delay = delay;
		size_t pos=0;
		auto gch = [text, &pos]() { return pos < text.length() ? text[pos++] : -1; };
		auto pch = [text, pos]() { return pos < text.length() ? text[pos] : -1; };
		std::string label_text;
		std::vector<Token> tokens;
		tokens.reserve(text.length());
		int speed = 2;
		label_text.reserve(text.length());
		bool done = false;
		Token t;
		Color3B color = Color3B::WHITE;
		while(!done) {
			t.letter = label_text.length();
			int c = gch();
			if (c == -1) break;

			switch (c) {
			case '^': // We are chaning the text speed in the next charater
				speed = quickDidgit(gch()) * 10;
				continue;
			case '&': // considered a newline
				c = '\n'; // just change it, label will handle  it
				break;
			case '\\':
				c = gch();
				switch (c) {
				case 'R': color = Color3B::RED; break;
				case 'W': color =  Color3B::WHITE; break;
				case 'Y': color = Color3B::YELLOW; break;
				case 'X': color = Color3B::BLACK; break;
				case 'B': color = Color3B::BLUE; break;
				case 'C': break; // choise see obj_choicer
				case 'E':  // change the face emotion
				{
				//	uint32_t frame = quickDidgit(gch());
				//	if (_facemotion && frame < _facemotionframes.size()) _facemotion->setSpriteFrame(_facemotionframes[frame]);
				}
				continue;
				case 'F':  // face change? have to look at some of the strings on this one
				{
					int frame = quickDidgit(gch());
					CCLOG("F Thing '%c'", frame);
					//	if (_facemotion && frame < _facemotionframes.size()) _facemotion->setSpriteFrame(_facemotionframes[frame]);
				}
				continue;
				case 'T': // same here, just for Toeil?
				{
					int frame = gch(); // charater
					CCLOG("T Thing '%c'", frame);
					//if (_facemotion && frame < _facemotionframes.size()) _facemotion->setSpriteFrame(_facemotionframes[frame]);
				}
				continue;
				default:
					CCLOG("Unkonwn \\ escape '%c'", c);
				
				}
				continue;
			case '/': // halt here
				c = gch();
				done = true;
				continue;
				/*
				if (c == '%') _halt = 2;
				else if (c == '^') _halt = 4;
				else _halt = 1;
				*/
				
			case '%':
				if (pch() == '%') { 
					done = true; 	
					continue;
				}  // self distruct? make invisiable?
				/*
				_pos = 0; // Ooooh new string in list
				_text = _textList[_textListPos++];
				_writing = Point();
				_lineno = 0;
				removeAllChildrenWithCleanup(true);
				*/
				done = true;
				continue;
			}	
			// put the charater on the screen
			label_text.push_back(c);
			t.action = Letterhake::create(color, shake, current_delay);
			current_delay += delay;
			tokens.push_back(t);
		}
		tw->setString(label_text);
		size_t len = label_text.length();
		for (int i = 0; i < len; i++) {
			const Token& t = tokens[i];
			Sprite* spr = tw->getLetter(t.letter);
			spr->setVisible(false);
			spr->runAction(t.action);
			spr->setTag(label_text[t.letter]);
			//spr->runAction(makeAction(shake,(delay / label_text.length() - 1)*i));
		}
		//tw->setVisible(true);
		return tw;
	}
	delete tw;
	return nullptr;
}

