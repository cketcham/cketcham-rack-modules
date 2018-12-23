#include "rack.hpp"
#include "../ModuleDragType.hpp"

using namespace rack;

struct PianoRollModule;
struct PianoRollWidget;

struct PianoRollDragType : ModuleDragType {
	PianoRollWidget* widget;
	PianoRollModule* module;

	PianoRollDragType(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~PianoRollDragType();
};


struct PlayPositionDragging : public PianoRollDragType {
	PlayPositionDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~PlayPositionDragging();

	void setNote();
	void onDragMove(EventDragMove& e) override;
};

struct LockMeasureDragging : public PianoRollDragType {
	std::chrono::_V2::system_clock::time_point longPressStart;
	LockMeasureDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~LockMeasureDragging();

	void onDragMove(EventDragMove& e) override;
};

struct KeyboardDragging : public PianoRollDragType {
	float offset = 0;

  KeyboardDragging(PianoRollWidget* widget, PianoRollModule* module);
  virtual ~KeyboardDragging();

	void onDragMove(EventDragMove& e) override;
};

struct NotePaintDragging : public PianoRollDragType {
	int lastDragBeatDiv = -1000;
	int lastDragPitch = -1000;
	bool makeStepsActive = true;

	NotePaintDragging(PianoRollWidget* widget, PianoRollModule* module);
	virtual ~NotePaintDragging();

	void onDragMove(EventDragMove& e) override;
};

struct VelocityDragging : public PianoRollDragType {
  int pattern;
	int measure;
	int division;

	VelocityDragging(PianoRollWidget* widget, PianoRollModule* module, int pattern, int measure, int division);
	virtual  ~VelocityDragging();

	void onDragMove(EventDragMove& e) override;
};


