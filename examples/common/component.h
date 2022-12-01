#pragma once
#include "prerequisites.h"
#include "idObject.h"
#include "common.h"

class Transform;
class Component : public IdObject<Component>
{
protected:
	GameObject *_parent;
	std::string _name;

	virtual void prepareImpl() {}
	virtual void updateImpl() {}

public:
	enum class Event
	{
		CREATE,
		PREPARE,
		UPDATE,
		UPDATE_FRAME,
		DESTROY
	};
	enum class Status
	{
		CREATED,
		PREPARED,
		UPDATED,
	};

	Component(GameObject *parent) : _parent(parent)
	{
		_signal(this, Event::CREATE);
	}
	Component(GameObject *parent, std::string_view name)
		: _parent(parent), _name(name)
	{
		_signal(this, Event::CREATE);
	}
	virtual ~Component()
	{
	}

	constexpr std::string_view getName() const { return _name; }
	constexpr GameObject *getParent() const { return _parent; }

	Transform *getParentTransform();

	void prepare()
	{
		if (_status != Status::CREATED)
			return;
		_status = Status::PREPARED;
		prepareImpl();
		_signal(this, Event::PREPARE);
	}
	virtual void update()
	{
		if (_status == Status::UPDATED)
			return;
		else if (_status == Status::CREATED)
			prepare();
		_status = Status::UPDATED;
		updateImpl();
		_signal(this, Event::UPDATE);
	}

	void addListener(Shit::Slot<void(Component const *, Event)> slot)
	{
		_signal.Connect(slot);
	}
	void removeListener(Shit::Slot<void(Component const *, Event)> slot)
	{
		_signal.Disconnect(slot);
	}
	constexpr Status getStatus() const { return _status; }

	void needUpdate(bool notifyChildren = false);

protected:
	Shit::Signal<void(Component const *, Event)> _signal;
	Status _status{Status::CREATED};
	friend class GameObject;
};

class Behaviour : public Component
{
	virtual void updatePerFrameImpl(float frameDtMs) {}
	bool _pause = false;

public:
	Behaviour(GameObject *parent) : Component(parent) {}
	void updatePerFrame(float frameDtMs)
	{
		if (_pause)
			return;
		updatePerFrameImpl(frameDtMs);
		_signal(this, Event::UPDATE_FRAME);
	}
	constexpr bool isRunning() const { return !_pause; }
	constexpr void pause() { _pause = true; }
	constexpr void run() { _pause = false; }
};