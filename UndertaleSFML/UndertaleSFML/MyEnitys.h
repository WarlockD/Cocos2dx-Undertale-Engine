#pragma once
#include "Global.h"
#include "UndertaleLabel.h"

struct UpdateDelegate {

};
enum class  Event { CREATE, DESTROY, UPDATE };

class Subject
{
	// http://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
	// really nice idea, works well too
	template<typename T>
	struct HasUpdateMethod
	{
		template<typename U, void(U::*)(float)> struct SFINAE {};
		template<typename U> static char Test(SFINAE<U, &U::update>*);
		template<typename U> static int Test(...);
		static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
	};
	template<typename Entity>
	void RegesterUpdateMethod(Entity& entity, std::true_type)
	{
		registerObserver(Event::UPDATE, std::bind(&Entity::update, &entity, std::placeholders::_1);
	}
	template<typename Entity> void RegesterUpdateMethod(Entity& entity, std::false_type) { }
public:
	template <typename Entity>
	void regesterMemberFunctions(Entity& entity)
	{
		RegesterUpdateMethod(entity,std::integral_constant<bool, HasUpdateMethod<Entity>::Has>());
	}
	template <typename Observer>
	void registerObserver(const E& event, Observer&& observer)
	{
		observers_[event].push_back(std::forward<Observer>(observer));
	}
	/*
	
	template <typename Observer>
	void registerObserver(E&& event, Observer&& observer)
	{
		observers_[std::move(event)].push_back(std::forward<Observer>(observer));
	}
	*/
	void notify(const Event& event) const
	{
		for (const auto& obs : observers_[event]) obs();
	}
	// other methods as before
private:
	std::unordered_map<Event, std::vector<std::function<void()>>> observers_;
};

namespace Component {
	extern kult::component<'anim', StopWatch<float>> animation;
	extern kult::component<'vel', sf::Vector2f> velocity;
	extern kult::component<'pos', sf::Vector2f> position;
	extern kult::component<'body', Body> body;
	extern kult::component<'fcol', SpriteFrameCollection> frames;
	extern kult::component<'fsng', SpriteFrame> frame;
	extern kult::component<'text', UndertaleLabelBuilder> text;
};





class USprite : kult::entity {
	Body& _body;
	SpriteFrameCollection& _frames;
	size_t _sprite_index;
public:
	USprite() : _body((*this)[Component::body]), _frames((*this)[Component::frames]) , _sprite_index(-1) {}
	void setSprite(size_t sprite_index) {
		if(_sprite_index!= sprite_index)
			_frames = Global::LoadSprite(_sprite_index = sprite_index);
	}
	size_t getSpriteIndex() const { return _sprite_index; }
	SpriteFrameCollection& getFrames() { return _frames; }
	const SpriteFrameCollection& getFrames() const { return _frames; }
	Body& getBody() { return _body; }
	const Body& getBody() const { return _body; }
};

class Update : kult::entity {