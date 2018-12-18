#include "window.hpp"
#include "../../include/PianoRoll/DragModes.hpp"
#include "../../include/PianoRoll/PianoRollModule.hpp"
#include "../../include/PianoRoll/PianoRollWidget.hpp"

static const float VELOCITY_SENSITIVITY = 0.0015f;
static const float KEYBOARDDRAG_SENSITIVITY = 0.1f;
static const float COLOURDRAG_SENSITIVITY = 0.0015f;

ModuleDragType::ModuleDragType(PianoRollWidget* widget, PianoRollModule* module) : widget(widget), module(module) {}

ModuleDragType::~ModuleDragType() {}

ColourDragging::ColourDragging(PianoRollWidget* widget, PianoRollModule* module) : ModuleDragType(widget, module) {
  windowCursorLock();
}

ColourDragging::~ColourDragging() {
	windowCursorUnlock();
}

void ColourDragging::onDragMove(EventDragMove& e) {
	float speed = 1.f;
	float range = 1.f;

	float delta = COLOURDRAG_SENSITIVITY * e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	widget->backgroundHue = clamp(widget->backgroundHue + delta, 0.f, 1.f);
}

PlayPositionDragging::PlayPositionDragging(PianoRollWidget* widget, PianoRollModule* module): ModuleDragType(widget, module) {
	setNote();
}

PlayPositionDragging::~PlayPositionDragging() {
	module->auditioner.stop();
}

void PlayPositionDragging::onDragMove(EventDragMove& e) {
	setNote();
}

void PlayPositionDragging::setNote() {
  Rect roll = widget->getRollArea();
  widget->reserveKeysArea(roll);

  Vec pos = gRackWidget->lastMousePos.minus(widget->box.pos);

  if (!roll.contains(pos)) {
		module->auditioner.stop();
    return;
  }

  auto beatDivs = widget->getBeatDivs(roll);
  bool beatDivFound = false;
  BeatDiv cellBeatDiv;

  for (auto const& beatDiv: beatDivs) {
    if (Rect(Vec(beatDiv.pos.x, 0), Vec(beatDiv.size.x, roll.size.y)).contains(pos)) {
      cellBeatDiv = beatDiv;
      beatDivFound = true;
      break;
    }
  }

  if (beatDivFound) {
    module->transport.setMeasure(widget->currentMeasure);
    module->transport.setStepInMeasure(cellBeatDiv.num);
    module->auditioner.start(module->transport.currentStepInPattern());
  } else {
		module->auditioner.stop();
  }
}

KeyboardDragging::KeyboardDragging(PianoRollWidget* widget, PianoRollModule* module) : ModuleDragType(widget, module) {
  windowCursorLock();
}

KeyboardDragging::~KeyboardDragging() {
  windowCursorUnlock();
}

LockMeasureDragging::LockMeasureDragging(PianoRollWidget* widget, PianoRollModule* module): ModuleDragType(widget, module) {
	longPressStart = std::chrono::high_resolution_clock::now();
	widget->measureLockPressTime = 0.f;
}

LockMeasureDragging::~LockMeasureDragging() {
	widget->measureLockPressTime = 0.f;
}

void LockMeasureDragging::onDragMove(EventDragMove &e) {
	auto currTime = std::chrono::high_resolution_clock::now();
	double pressTime = std::chrono::duration<double>(currTime - longPressStart).count();
	widget->measureLockPressTime = clamp(pressTime, 0.f, 1.f);
	if (pressTime >= 1.f) {

		if (!module->transport.isLocked() || (module->transport.currentMeasure() != widget->currentMeasure)) {
			module->transport.lockMeasure();

			if (module->transport.currentMeasure() != widget->currentMeasure) {
				// We just locked the measure, but the play point is outside the selected measure - move the play point into the last note of the current measure
				module->transport.setMeasure(widget->currentMeasure);
			}
		} else {
			module->transport.unlockMeasure();
		}

		longPressStart = std::chrono::high_resolution_clock::now();
	}
}

void KeyboardDragging::onDragMove(EventDragMove& e) {
	float speed = 1.f;
	float range = 1.f;

	float delta = KEYBOARDDRAG_SENSITIVITY * e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	offset += delta;

	while (offset >= 1.f) {
		widget->lowestDisplayNote = clamp(widget->lowestDisplayNote + 1, -1 * 12, 8 * 12);
		offset -= 1;
	}

	while (offset <= -1.f) {
		widget->lowestDisplayNote = clamp(widget->lowestDisplayNote - 1, -1 * 12, 8 * 12);
		offset += 1;
	}
}

NotePaintDragging::NotePaintDragging(PianoRollWidget* widget, PianoRollModule* module) : ModuleDragType(widget, module) {
	Vec pos = gRackWidget->lastMousePos.minus(widget->box.pos);

	std::tuple<bool, BeatDiv, Key> cell = widget->findCell(pos);
	if (!std::get<0>(cell)) {
		return;
	}

	int beatDiv = std::get<1>(cell).num;
	int pitch = std::get<2>(cell).pitch();

	if (pitch == module->patternData.getStepPitch(module->transport.currentPattern(), widget->currentMeasure, beatDiv)) {
		makeStepsActive = !module->patternData.isStepActive(module->transport.currentPattern(), widget->currentMeasure, beatDiv);
	} else {
		makeStepsActive = true;
	}
}

NotePaintDragging::~NotePaintDragging() {
	if (makeStepsActive) {
		module->auditioner.stop();
	}
}

void NotePaintDragging::onDragMove(EventDragMove& e) {
	Vec pos = gRackWidget->lastMousePos.minus(widget->box.pos);

	std::tuple<bool, BeatDiv, Key> cell = widget->findCell(pos);
	if (!std::get<0>(cell)) {
		module->auditioner.stop();
		return;
	}

	int beatDiv = std::get<1>(cell).num;
	int pitch = std::get<2>(cell).pitch();

	if (lastDragBeatDiv != beatDiv || lastDragPitch != pitch) {
		lastDragBeatDiv = beatDiv;
		lastDragPitch = pitch;
		if (makeStepsActive) {
			bool wasAlreadyActive = module->patternData.isStepActive(module->transport.currentPattern(), widget->currentMeasure, beatDiv);
			module->patternData.setStepActive(module->transport.currentPattern(), widget->currentMeasure, beatDiv, true);
			module->patternData.setStepPitch(module->transport.currentPattern(), widget->currentMeasure, beatDiv, pitch);
			
			if (!wasAlreadyActive) {
				module->patternData.setStepVelocity(module->transport.currentPattern(), widget->currentMeasure, beatDiv, 0.75);
			}

			module->patternData.adjustVelocity(module->transport.currentPattern(), widget->currentMeasure, beatDiv, 0.f);

			module->auditioner.start(beatDiv + (module->patternData.getStepsPerMeasure(module->transport.currentPattern()) * widget->currentMeasure));
			module->auditioner.retrigger();
		} else {
			module->patternData.setStepActive(module->transport.currentPattern(), widget->currentMeasure, beatDiv, false);
	    module->patternData.setStepRetrigger(module->transport.currentPattern(), widget->currentMeasure, beatDiv, false);
		}
	};
}

VelocityDragging::VelocityDragging(PianoRollWidget* widget, PianoRollModule* module, int pattern, int measure, int division) 
  :ModuleDragType(widget, module), 
    pattern(pattern),
    measure(measure),
    division(division) {
  windowCursorLock();
}

VelocityDragging::~VelocityDragging() {
	windowCursorUnlock();
	widget->displayVelocityHigh = -1;
	widget->displayVelocityLow = -1;
}

void VelocityDragging::onDragMove(EventDragMove& e) {
	float speed = 1.f;
	float range = 1.f;
	float delta = VELOCITY_SENSITIVITY * -e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	float newVelocity = module->patternData.adjustVelocity(module->transport.currentPattern(), measure, division, delta);

	Rect roll = widget->getRollArea();
	roll.size.y = roll.size.y / 2.f;
	if (roll.contains(widget->dragPos)) {
		widget->displayVelocityHigh = -1;
		widget->displayVelocityLow = newVelocity;
	} else {
		widget->displayVelocityHigh = newVelocity;
		widget->displayVelocityLow = -1;
	}
}

StandardModuleDragging::StandardModuleDragging(PianoRollWidget* widget, PianoRollModule* module) : ModuleDragType(widget, module) {}
StandardModuleDragging::~StandardModuleDragging() {}

void StandardModuleDragging::onDragMove(EventDragMove& e) {
	widget->baseDragMove(e);
}
