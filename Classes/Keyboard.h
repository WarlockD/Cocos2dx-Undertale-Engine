#pragma once
#include "cocos2d.h"
#include "LuaSprite.h"
#include "LuaFont.h"
#include <unordered_set>

namespace Undertale {
	// this class is just makes it easyer to check if multipul keys are pressed for moving around
	// works just not great preformance
	class KeyboardPoller : public cocos2d::Node {
		std::unordered_set<cocos2d::EventKeyboard::KeyCode> _keysDown;
		std::unordered_set<cocos2d::EventKeyboard::KeyCode> _keysToCatch; // tags for latter
		cocos2d::EventListenerKeyboard* _keyboardListener;
		
		bool init();
		KeyboardPoller() : _propergate(false), _keyboardListener(nullptr) {}
		CC_SYNTHESIZE(bool, _propergate, PropergateKeys);
	public:
		virtual ~KeyboardPoller() { CC_SAFE_RELEASE_NULL(_keyboardListener); }
		CC_DISALLOW_COPY_AND_ASSIGN(KeyboardPoller);
		
		bool keyDown(cocos2d::EventKeyboard::KeyCode key) const { return _keysDown.find(key) != _keysDown.cend(); }
		bool keyDown(cocos2d::EventKeyboard::KeyCode key1, cocos2d::EventKeyboard::KeyCode key2) const { 
			if (_keysDown.find(key1) != _keysDown.cend() && _keysDown.find(key2) != _keysDown.cend())
				return true;

			return false;
		}
		static KeyboardPoller* create();
		void setupKeyboardListener();
		void addKeyToCatch(cocos2d::EventKeyboard::KeyCode key) { _keysToCatch.insert(key); }
		void addKeyToCatch(cocos2d::EventKeyboard::KeyCode key1, cocos2d::EventKeyboard::KeyCode key2) { _keysToCatch.insert(key1); _keysToCatch.insert(key2); }
		void removeKeyToCatch(cocos2d::EventKeyboard::KeyCode key) { _keysToCatch.erase(key); }
	};






}
