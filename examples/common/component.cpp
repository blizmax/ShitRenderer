#include "component.h"
#include "gameobject.h"
#include "transform.h"

Transform *Component::getParentTransform()
{
	return _parent->getComponent<Transform>();
}

void Component::needUpdate(bool notifyChildren)
{
	if (_status == Status::UPDATED)
		_status = Status::PREPARED;
	_parent->needUpdate(true, notifyChildren);
}