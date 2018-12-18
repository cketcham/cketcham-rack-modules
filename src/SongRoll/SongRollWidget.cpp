#include "rack.hpp"
#include "window.hpp"
#include "../../include/SongRoll/SongRollWidget.hpp"
#include "../../include/SongRoll/SongRollModule.hpp"
#include "../../include/SongRoll/DragModes.hpp"

using namespace rack;
using namespace SongRoll;

extern Plugin* plugin;

SongRollWidget::SongRollWidget(SongRollModule *module) : ModuleWidget(module) {
  this->module = (SongRollModule*)module;
  setPanel(SVG::load(assetPlugin(plugin, "res/SongRoll.svg")));

  // addInput(Port::create<PJ301MPort>(Vec(50.114, 380.f-91-23.6), Port::INPUT, module, SongRollModule::CLOCK_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(85.642, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RESET_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(121.170, 380.f-91-23.6), Port::INPUT, module, SongRollModule::PATTERN_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(156.697, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RUN_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(192.224, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RECORD_INPUT));

  // addInput(Port::create<PJ301MPort>(Vec(421.394, 380.f-91-23.6), Port::INPUT, module, SongRollModule::VOCT_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(456.921, 380.f-91-23.6), Port::INPUT, module, SongRollModule::GATE_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(492.448, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RETRIGGER_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(527.976, 380.f-91-23.6), Port::INPUT, module, SongRollModule::VELOCITY_INPUT));

  // addOutput(Port::create<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::CLOCK_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::RESET_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::PATTERN_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::RUN_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::RECORD_OUTPUT));

  // addOutput(Port::create<PJ301MPort>(Vec(421.394, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::VOCT_OUTPUT));
  // addOutput(Port::create<PJ301MPo  json_t *lowestDisplayNoteJ = json_object_get(rootJ, "lowestDisplayNote");

  // patternWidget->widget = this;
  // addChild(patternWidget);
}

void SongRollWidget::appendContextMenu(Menu* menu) {

}

Rect SongRollWidget::getRollArea() {
  return Rect();
}

void SongRollWidget::drawBackgroundColour(NVGcontext* ctx) {
    nvgBeginPath(ctx);
    nvgFillColor(ctx, nvgHSL(backgroundHue, backgroundSaturation, backgroundLuminosity));
    nvgRect(ctx, 0, 0, box.size.x, box.size.y);
    nvgFill(ctx);
}

void SongRollWidget::draw(NVGcontext* ctx) {
  drawBackgroundColour(ctx);

  ModuleWidget::draw(ctx);
}

void SongRollWidget::onMouseDown(EventMouseDown& e) {
  Vec pos = gRackWidget->lastMousePos.minus(box.pos);

  ModuleWidget::onMouseDown(e);
}

void SongRollWidget::onDragStart(EventDragStart& e) {
  Vec pos = gRackWidget->lastMousePos.minus(box.pos);

  ModuleWidget::onDragStart(e);
}

void SongRollWidget::baseDragMove(EventDragMove& e) {
  ModuleWidget::onDragMove(e);
}

void SongRollWidget::onDragMove(EventDragMove& e) {
  if (currentDragType == NULL) { ModuleWidget::onDragMove(e); return; }

  currentDragType->onDragMove(e);
}

void SongRollWidget::onDragEnd(EventDragEnd& e) {
  if (currentDragType) {
    delete currentDragType;
    currentDragType = NULL;
  }

  ModuleWidget::onDragEnd(e);
}

json_t *SongRollWidget::toJson() {
  json_t *rootJ = ModuleWidget::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  return rootJ;
}

void SongRollWidget::fromJson(json_t *rootJ) {
  ModuleWidget::fromJson(rootJ);

}


