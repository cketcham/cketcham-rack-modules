#include "ModuleDragType.hpp"

struct BaseWidget;

struct ColourDragging : public ModuleDragType {
  BaseWidget* widget;

	ColourDragging(BaseWidget* widget);
	virtual ~ColourDragging();

	void onDragMove(rack::EventDragMove& e) override;
};

