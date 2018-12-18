#include "rack.hpp"

using namespace rack;

struct PianoRollModule;
struct PianoRollWidget;

struct ModuleDragType {
	PianoRollWidget* widget;
	PianoRollModule* module;

	ModuleDragType(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~ModuleDragType();

	virtual void onDragMove(EventDragMove& e) = 0;
};

struct ColourDragging : public ModuleDragType {
	ColourDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~ColourDragging();

	void onDragMove(EventDragMove& e) override;
};

struct PlayPositionDragging : public ModuleDragType {
	PlayPositionDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~PlayPositionDragging();

	void setNote();
	void onDragMove(EventDragMove& e) override;
};

struct LockMeasureDragging : public ModuleDragType {
	std::chrono::_V2::system_clock::time_point longPressStart;
	LockMeasureDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~LockMeasureDragging();

	void onDragMove(EventDragMove& e) override;
};

struct KeyboardDragging : public ModuleDragType {
	float offset = 0;

  KeyboardDragging(PianoRollWidget* widget, PianoRollModule* module);
  virtual ~KeyboardDragging();

	void onDragMove(EventDragMove& e) override;
};

struct NotePaintDragging : public ModuleDragType {
	int lastDragBeatDiv = -1000;
	int lastDragPitch = -1000;
	bool makeStepsActive = true;

	NotePaintDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~NotePaintDragging();

	void onDragMove(EventDragMove& e) override;
};

struct VelocityDragging : public ModuleDragType {
  int pattern;
	int measure;
	int division;

	VelocityDragging(PianoRollWidget* widget, PianoRollModule* module, int pattern, int measure, int division);
	virtual  ~VelocityDragging();

	void onDragMove(EventDragMove& e) override;
};

struct StandardModuleDragging : public ModuleDragType {
	StandardModuleDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~StandardModuleDragging();

	void onDragMove(EventDragMove& e) override;
};

