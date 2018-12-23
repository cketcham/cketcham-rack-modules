#include "rack.hpp"

struct ModuleDragType {
	ModuleDragType();
	virtual ~ModuleDragType();

	virtual void onDragMove(rack::EventDragMove& e) = 0;
};
