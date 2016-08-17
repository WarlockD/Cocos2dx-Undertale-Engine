#pragma once
class MessageDispatcher {
public:
	virtual void dispatchMessages(void) = 0;
	virtual void discardAllMessages(void) = 0;
	virtual void reset(void) = 0;
};

template < class T >
class MessageDispatcherImpl : public MessageDispatcher {
public:
	static MessageDispatcherImpl getInstance(void) {
		static MessageDispatcherImpl imp; // all live instances are reflected here
		return imp;
	}
public:
	virtual void dispatchMessages(void);

	void pushMessage(T *message);

	void registerHandler(MessageHandler< T > *handler);

	void unregisterHandler(MessageHandler< T > *handler);
};

class MessageQueue {
public:
	void registerDispatcher(MessageDispatcher *dispatcher);

public:
	void dispatchMessages(void);

	template< class T >
	void MessageQueue::pushMessage(T *message)
	{
		MessageDispatcherImpl::getInstance().pushMessage(message);
	}
};
