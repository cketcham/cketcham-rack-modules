#include "rack.hpp"
#include "window.hpp"
#include "../../include/PianoRoll/DragModes.hpp"
#include "../../include/PianoRoll/PatternWidget.hpp"
#include "../../include/PianoRoll/PianoRollWidget.hpp"
#include "../../include/PianoRoll/PianoRollModule.hpp"
#include "../../include/PianoRoll/MenuItems/CancelPasteItem.hpp"
#include "../../include/PianoRoll/MenuItems/ClearNotesItem.hpp"
#include "../../include/PianoRoll/MenuItems/ClockBufferItem.hpp"
#include "../../include/PianoRoll/MenuItems/CopyMeasureItem.hpp"
#include "../../include/PianoRoll/MenuItems/CopyPatternItem.hpp"
#include "../../include/PianoRoll/MenuItems/NotesToShowItem.hpp"
#include "../../include/PianoRoll/MenuItems/PasteMeasureItem.hpp"
#include "../../include/PianoRoll/MenuItems/PastePatternItem.hpp"

using namespace rack;

extern Plugin* plugin;

PianoRollWidget::PianoRollWidget(PianoRollModule *module) : ModuleWidget(module) {
  this->module = (PianoRollModule*)module;
  setPanel(SVG::load(assetPlugin(plugin, "res/PianoRoll.svg")));

  addInput(Port::create<PJ301MPort>(Vec(50.114, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::CLOCK_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(85.642, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RESET_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(121.170, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::PATTERN_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(156.697, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RUN_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(192.224, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RECORD_INPUT));

  addInput(Port::create<PJ301MPort>(Vec(421.394, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VOCT_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(456.921, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::GATE_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(492.448, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RETRIGGER_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(527.976, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VELOCITY_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::CLOCK_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RESET_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::PATTERN_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RUN_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RECORD_OUTPUT));

  addOutput(Port::create<PJ301MPort>(Vec(421.394, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VOCT_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(456.921, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::GATE_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(492.448, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RETRIGGER_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(527.976, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VELOCITY_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(563.503, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::END_OF_PATTERN_OUTPUT));

  PatternWidget* patternWidget = Widget::create<PatternWidget>(Vec(505.257, 380.f-224.259-125.586));
  patternWidget->module = module;
  patternWidget->widget = this;
  addChild(patternWidget);
}

void PianoRollWidget::appendContextMenu(Menu* menu) {

  menu->addChild(MenuLabel::create(""));
  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Copy / Paste"));

  CopyPatternItem *copyPatternItem = new CopyPatternItem();
  copyPatternItem->widget = this;
  copyPatternItem->module = module;
  copyPatternItem->text = "Copy Pattern";
  menu->addChild(copyPatternItem);

  CopyMeasureItem *copyMeasureItem = new CopyMeasureItem();
  copyMeasureItem->widget = this;
  copyMeasureItem->module = module;
  copyMeasureItem->text = "Copy Measure";
  menu->addChild(copyMeasureItem);

  switch(state) {
    case COPYREADY:
      break;
    case PATTERNLOADED:
      {
        PastePatternItem *pastePatternItem = new PastePatternItem();
        pastePatternItem->widget = this;
        pastePatternItem->module = module;
        pastePatternItem->text = "Paste Pattern";
        menu->addChild(pastePatternItem);
      }
      break;
    case MEASURELOADED:
      {
        PasteMeasureItem *pasteMeasureItem = new PasteMeasureItem();
        pasteMeasureItem->widget = this;
        pasteMeasureItem->module = module;
        pasteMeasureItem->text = "Paste Measure";
        menu->addChild(pasteMeasureItem);
      }
      break;
    default:
      state = COPYREADY;
      break;
  }

  menu->addChild(MenuLabel::create(""));
    menu->addChild(new ClearNotesItem(this->module));

  menu->addChild(MenuLabel::create(""));
  menu->addChild(MenuLabel::create("Notes to Show"));
    menu->addChild(new NotesToShowItem(this, 12));
    menu->addChild(new NotesToShowItem(this, 18));
    menu->addChild(new NotesToShowItem(this, 24));
    menu->addChild(new NotesToShowItem(this, 36));
    menu->addChild(new NotesToShowItem(this, 48));
    menu->addChild(new NotesToShowItem(this, 60));
  menu->addChild(MenuLabel::create(""));
  menu->addChild(MenuLabel::create("Clock Delay (samples)"));
    menu->addChild(new ClockBufferItem(module, 0));
    menu->addChild(new ClockBufferItem(module, 1));
    menu->addChild(new ClockBufferItem(module, 2));
    menu->addChild(new ClockBufferItem(module, 3));
    menu->addChild(new ClockBufferItem(module, 4));
    menu->addChild(new ClockBufferItem(module, 5));
    menu->addChild(new ClockBufferItem(module, 10));
}

Rect PianoRollWidget::getRollArea() {
  Rect roll;
  roll.pos.x = 15.f;
  roll.pos.y = 380-365.f;
  roll.size.x = 480.f;
  roll.size.y = 220.f;
  return roll;
}

std::vector<Key> PianoRollWidget::getKeys(const Rect& keysArea) {
  std::vector<Key> keys;

  int keyCount = (notesToShow) + 1;
  float keyHeight = keysArea.size.y / keyCount;

  int octave = lowestDisplayNote / 12;
  int offset = lowestDisplayNote % 12;

  for (int i = 0; i < keyCount; i++) {
    int n = (i+offset+12) % 12;
    keys.push_back(
      Key(
        Vec(keysArea.pos.x, (keysArea.pos.y + keysArea.size.y) - ( (1 + i) * keyHeight) ),
        Vec(keysArea.size.x, keyHeight ),
        n == 1 || n == 3 || n == 6 || n == 8 || n == 10,
        (i+offset) % 12,
        octave + ((i+offset) / 12)
      )
    );
  }

  return keys;
}

std::vector<BeatDiv> PianoRollWidget::getBeatDivs(const Rect &roll) {
  std::vector<BeatDiv> beatDivs;

  int totalDivisions = module->patternData.getStepsPerMeasure(module->transport.currentPattern());
  int divisionsPerBeat = module->patternData.getDivisionsPerBeat(module->transport.currentPattern());

  float divisionWidth = roll.size.x / totalDivisions;

  float top = roll.pos.y + topMargins;

  for (int i = 0; i < totalDivisions; i ++) {
    float x = roll.pos.x + (i * divisionWidth);

    BeatDiv beatDiv;
    beatDiv.pos.x = x;
    beatDiv.size.x = divisionWidth;
    beatDiv.pos.y = top;
    beatDiv.size.y = roll.size.y - (2 * topMargins);

    beatDiv.num = i;
    beatDiv.beat = (i % divisionsPerBeat == 0);

    beatDivs.push_back(beatDiv);
  }

  return beatDivs;
}

Rect PianoRollWidget::reserveKeysArea(Rect& roll) {
  Rect keysArea;
  keysArea.pos.x = roll.pos.x;
  keysArea.pos.y = roll.pos.y + topMargins;
  keysArea.size.x = 25.f;
  keysArea.size.y = roll.size.y - (2*topMargins);

  roll.pos.x = keysArea.pos.x + keysArea.size.x;
  roll.size.x = roll.size.x - keysArea.size.x;

  return keysArea;
}

std::tuple<bool, int> PianoRollWidget::findMeasure(Vec pos) {
  Rect roll = getRollArea();
  reserveKeysArea(roll);

  int numberOfMeasures = module->patternData.getMeasures(module->transport.currentPattern());
  float widthPerMeasure = roll.size.x / numberOfMeasures;
  float boxHeight = topMargins * 0.75;

  for (int i = 0; i < numberOfMeasures; i++) {
    if (Rect(Vec(roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - boxHeight), Vec(widthPerMeasure, boxHeight)).contains(pos)) {
      return std::make_tuple(true, i);
    }
  }

  return std::make_tuple(false, 0);
}

std::tuple<bool, bool> PianoRollWidget::findOctaveSwitch(Vec pos) {
  Rect roll = getRollArea();
  Rect keysArea = reserveKeysArea(roll);

  bool octaveUp = Rect(Vec(keysArea.pos.x, roll.pos.y), Vec(keysArea.size.x, keysArea.pos.y)).contains(pos);
  bool octaveDown = Rect(Vec(keysArea.pos.x, keysArea.pos.y + keysArea.size.y), Vec(keysArea.size.x, keysArea.pos.y)).contains(pos);
  
  return std::make_tuple(octaveUp, octaveDown);
}

std::tuple<bool, BeatDiv, Key> PianoRollWidget::findCell(Vec pos) {
  Rect roll = getRollArea();
  Rect keysArea = reserveKeysArea(roll);

  if (!roll.contains(pos)) {
    return std::make_tuple(false, BeatDiv(), Key());
  }

  auto keys = getKeys(keysArea);
  bool keyFound = false;
  Key cellKey;

  for (auto const& key: keys) {
    if (Rect(Vec(key.pos.x + key.size.x, key.pos.y), Vec(roll.size.x, key.size.y)).contains(pos)) {
      cellKey = key;
      keyFound = true;
      break;
    }
  }

  auto beatDivs = getBeatDivs(roll);
  bool beatDivFound = false;
  BeatDiv cellBeatDiv;

  for (auto const& beatDiv: beatDivs) {
    if (Rect(beatDiv.pos, beatDiv.size).contains(pos)) {
      cellBeatDiv = beatDiv;
      beatDivFound = true;
      break;
    }
  }

  return std::make_tuple(keyFound && beatDivFound, cellBeatDiv, cellKey);
}

void PianoRollWidget::drawKeys(NVGcontext *ctx, const std::vector<Key> &keys) {
  char buffer[100];

  for (auto const& key: keys) {
    nvgBeginPath(ctx);
    nvgStrokeWidth(ctx, 0.5f);
    nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));

    if (key.sharp) {
      nvgFillColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
    } else {
      nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.0));
    }
    nvgRect(ctx, key.pos.x, key.pos.y, key.size.x, key.size.y);

    nvgStroke(ctx);
    nvgFill(ctx);

    if (key.num == 0) {
      nvgBeginPath(ctx);
      snprintf(buffer, 10, "C%d", key.oct);
      nvgFontSize(ctx,key.size.y);
      nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
      nvgFillColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
      nvgText(ctx, key.pos.x + 4, key.pos.y + key.size.y - 2, buffer, NULL);
      nvgStroke(ctx);
      nvgFill(ctx);
    }
  }
}

void PianoRollWidget::drawSwimLanes(NVGcontext *ctx, const Rect &roll, const std::vector<Key> &keys) {

  for (auto const& key: keys) {

    if (key.sharp) {
      nvgBeginPath(ctx);
      nvgFillColor(ctx, nvgRGBAf(0.f, 0.0f, 0.0f, 0.25f));
      nvgRect(ctx, roll.pos.x, key.pos.y + 1, roll.size.x, key.size.y - 2);
      nvgFill(ctx);
    }

    nvgBeginPath(ctx);
    if (key.num == 11) {
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
      nvgStrokeWidth(ctx, 1.0f);
    } else {
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
      nvgStrokeWidth(ctx, 0.5f);
    }
    nvgMoveTo(ctx, roll.pos.x, key.pos.y);
    nvgLineTo(ctx, roll.pos.x + roll.size.x, key.pos.y);
    nvgStroke(ctx);
  }

  nvgBeginPath(ctx);
  nvgStrokeWidth(ctx, 1.f);
  nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
  nvgMoveTo(ctx, roll.pos.x, keys.back().pos.y);
  nvgLineTo(ctx, roll.pos.x + roll.size.x, keys.back().pos.y);
  nvgStroke(ctx);

  nvgBeginPath(ctx);
  nvgStrokeWidth(ctx, 1.f);
  nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
  nvgMoveTo(ctx, roll.pos.x, keys[0].pos.y + keys[0].size.y);
  nvgLineTo(ctx, roll.pos.x + roll.size.x, keys[0].pos.y + keys[0].size.y);
  nvgStroke(ctx);
}


void PianoRollWidget::drawBeats(NVGcontext *ctx, const std::vector<BeatDiv> &beatDivs) {
  bool first = true;
  for (const auto &beatDiv : beatDivs) {

    nvgBeginPath(ctx);

    if (beatDiv.beat && !first) {
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0));
      nvgStrokeWidth(ctx, 1.0f);
    } else if (beatDiv.triplet) {
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0));
      nvgStrokeWidth(ctx, 0.5f);
    } else {
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5));
      nvgStrokeWidth(ctx, 0.5f);
    }

    nvgMoveTo(ctx, beatDiv.pos.x, beatDiv.pos.y);
    nvgLineTo(ctx, beatDiv.pos.x, beatDiv.pos.y + beatDiv.size.y);

    nvgStroke(ctx);

    first = false;
  }
}

void PianoRollWidget::drawNotes(NVGcontext *ctx, const std::vector<Key> &keys, const std::vector<BeatDiv> &beatDivs) {
  int lowPitch = keys.front().num + (keys.front().oct * 12);
  int highPitch = keys.back().num + (keys.back().oct * 12);

  Rect roll = getRollArea();
  reserveKeysArea(roll);

  int pattern = module->transport.currentPattern();

  for (const auto &beatDiv : beatDivs) {
    if (module->patternData.isStepActive(pattern, currentMeasure, beatDiv.num) == false ) { continue; }
    int pitch = module->patternData.getStepPitch(pattern, currentMeasure, beatDiv.num);

    if (pitch < lowPitch) {
      nvgBeginPath(ctx);
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgStrokeWidth(ctx, 1.f);
      nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgRect(ctx, beatDiv.pos.x, roll.pos.y + roll.size.y - topMargins, beatDiv.size.x, 1);
      nvgStroke(ctx);
      nvgFill(ctx);
      continue;
    }

    if (pitch > highPitch) {
      nvgBeginPath(ctx);
      nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgStrokeWidth(ctx, 1.f);
      nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
      nvgRect(ctx, beatDiv.pos.x, roll.pos.y + topMargins -1, beatDiv.size.x, 1);
      nvgStroke(ctx);
      nvgFill(ctx);
      continue;
    }

    for (auto const& key: keys) {
      if (key.num + (key.oct * 12) == pitch) {

        float velocitySize = (module->patternData.getStepVelocity(pattern, currentMeasure, beatDiv.num) * key.size.y * 0.9f) + (key.size.y * 0.1f);

        nvgBeginPath(ctx);
        nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.25f));
        nvgStrokeWidth(ctx, 0.5f);
        nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.25f));
        nvgRect(ctx, beatDiv.pos.x, key.pos.y, beatDiv.size.x, (key.size.y - velocitySize));
        nvgStroke(ctx);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
        nvgStrokeWidth(ctx, 0.5f);
        nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
        nvgRect(ctx, beatDiv.pos.x, key.pos.y + (key.size.y - velocitySize), beatDiv.size.x, velocitySize);
        nvgStroke(ctx);
        nvgFill(ctx);


        if (module->patternData.isStepRetriggered(pattern, currentMeasure, beatDiv.num)) {
          nvgBeginPath(ctx);

          nvgStrokeWidth(ctx, 0.f);
          nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.f));

          nvgRect(ctx, beatDiv.pos.x, key.pos.y, beatDiv.size.x / 4.f, key.size.y);
          nvgStroke(ctx);
          nvgFill(ctx);
        }

        break;
      }
    }

  }
}

void PianoRollWidget::drawMeasures(NVGcontext *ctx) {
  Rect roll = getRollArea();
  reserveKeysArea(roll);

  int numberOfMeasures = module->patternData.getMeasures(module->transport.currentPattern());

  float widthPerMeasure = roll.size.x / numberOfMeasures;
  float boxHeight = topMargins * 0.75;

  for (int i = 0; i < numberOfMeasures; i++) {
    bool drawingCurrentMeasure = i == currentMeasure;
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 0.1f));
    nvgStrokeWidth(ctx, 1.f);
    nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, drawingCurrentMeasure ? 1.f : 0.25f));
    nvgRect(ctx, roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - boxHeight, widthPerMeasure, boxHeight);
    nvgStroke(ctx);
    nvgFill(ctx);

    if (drawingCurrentMeasure && measureLockPressTime > 0.5f) {
      float barHeight = boxHeight * rescale(clamp(measureLockPressTime, 0.f, 1.f), 0.5f, 1.f, 0.f, 1.f);
      nvgBeginPath(ctx);
      nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.f));
      nvgStrokeWidth(ctx, 0.f);
      nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.f));
      nvgRect(ctx, roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - barHeight, widthPerMeasure, barHeight);
      nvgStroke(ctx);
      nvgFill(ctx);
    }
  }

  if (module->transport.isLocked()) {
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.f));
    nvgStrokeWidth(ctx, 2.f);
    nvgRect(ctx, roll.pos.x, roll.pos.y + roll.size.y - boxHeight, roll.size.x, boxHeight);
    nvgStroke(ctx);
  }
}

void PianoRollWidget::drawPlayPosition(NVGcontext *ctx) {
  Rect roll = getRollArea();
  reserveKeysArea(roll);

  int divisionsPerMeasure = module->patternData.getStepsPerMeasure(module->transport.currentPattern());
  int playingMeasure = module->transport.currentMeasure();
  int noteInMeasure = module->transport.currentStepInMeasure();
  int numberOfMeasures = module->patternData.getMeasures(module->transport.currentPattern());

  if (noteInMeasure == -1) {
    return;
  }

  if (playingMeasure == currentMeasure) {

    float divisionWidth = roll.size.x / divisionsPerMeasure;
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 0.5f));
    nvgStrokeWidth(ctx, 0.5f);
    nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 0.2f));
    nvgRect(ctx, roll.pos.x + (noteInMeasure * divisionWidth), roll.pos.y, divisionWidth, roll.size.y - topMargins);
    nvgStroke(ctx);
    nvgFill(ctx);
  }

  float widthPerMeasure = roll.size.x / numberOfMeasures;
  float stepWidthInMeasure = widthPerMeasure / divisionsPerMeasure;	
  nvgBeginPath(ctx);
  nvgStrokeColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.f));
  nvgStrokeWidth(ctx, 1.f);
  nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 0.2f));
  nvgRect(ctx, roll.pos.x + (playingMeasure * widthPerMeasure) + (noteInMeasure * stepWidthInMeasure), roll.pos.y + roll.size.y - topMargins, stepWidthInMeasure, topMargins);
  nvgStroke(ctx);
  nvgFill(ctx);
}

void PianoRollWidget::drawVelocityInfo(NVGcontext *ctx) {
  char buffer[100];

  if (displayVelocityHigh > -1 || displayVelocityLow > -1) {
    float displayVelocity = max(displayVelocityHigh, displayVelocityLow);

    Rect roll = getRollArea();
    reserveKeysArea(roll);

    float posy;
    if (displayVelocityHigh > -1) {
      posy = roll.pos.y + ((roll.size.y * 0.25) * 1);
    } else {
      posy = roll.pos.y + ((roll.size.y * 0.25) * 3);				
    }

    nvgBeginPath(ctx);
    snprintf(buffer, 100, "Velocity: %06.3fV (Midi %03d)", displayVelocity * 10.f, (int)(127 * displayVelocity));

    nvgFontSize(ctx, roll.size.y / 12.f);
    float *bounds = new float[4];
    nvgTextBounds(ctx, roll.pos.x, posy, buffer, NULL, bounds);

    nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    nvgStrokeWidth(ctx, 5.f);
    nvgFillColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    nvgRect(ctx, roll.pos.x + (roll.size.x / 2.f) - ((bounds[2] - bounds[0]) / 2.f), bounds[1], bounds[2]-bounds[0], bounds[3]-bounds[1]);
    nvgStroke(ctx);
    nvgFill(ctx);

    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
    nvgText(ctx, roll.pos.x + (roll.size.x / 2.f) - ((bounds[2] - bounds[0]) / 2.f), posy, buffer, NULL);

    delete bounds;

    nvgStroke(ctx);
    nvgFill(ctx);
  }
}

void PianoRollWidget::drawBackgroundColour(NVGcontext* ctx) {
    nvgBeginPath(ctx);
    nvgFillColor(ctx, nvgHSL(backgroundHue, backgroundSaturation, backgroundLuminosity));
    nvgRect(ctx, 0, 0, box.size.x, box.size.y);
    nvgFill(ctx);
}

void PianoRollWidget::draw(NVGcontext* ctx) {
  drawBackgroundColour(ctx);

  ModuleWidget::draw(ctx);

  Rect roll = getRollArea();

  int measure = module->transport.currentMeasure();
  if (measure != currentMeasure && lastDrawnStep != module->transport.currentStepInPattern()) {
    currentMeasure = measure;
  }
  lastDrawnStep = module->transport.currentStepInPattern();

  Rect keysArea = reserveKeysArea(roll);
  auto keys = getKeys(keysArea);
  drawKeys(ctx, keys);
  drawSwimLanes(ctx, roll, keys);
  auto beatDivs = getBeatDivs(roll);
  drawBeats(ctx, beatDivs);
  drawNotes(ctx, keys, beatDivs);
  drawMeasures(ctx);
  drawPlayPosition(ctx);
  drawVelocityInfo(ctx);
}	

void PianoRollWidget::onMouseDown(EventMouseDown& e) {
  Vec pos = gRackWidget->lastMousePos.minus(box.pos);
  std::tuple<bool, bool> octaveSwitch = findOctaveSwitch(pos);
  std::tuple<bool, int> measureSwitch = findMeasure(pos);
  
  if (e.button == 1) {
    std::tuple<bool, BeatDiv, Key> cell = findCell(e.pos);
    if (!std::get<0>(cell)) { ModuleWidget::onMouseDown(e); return; }

    e.consumed = true;
    e.target = this;

    int currentPattern = module->transport.currentPattern();

    int beatDiv = std::get<1>(cell).num;

    module->patternData.toggleStepRetrigger(currentPattern, currentMeasure, beatDiv);
  } else if (e.button == 0 && std::get<0>(octaveSwitch)) {
    this->lowestDisplayNote = clamp(this->lowestDisplayNote + 12, -1 * 12, 8 * 12);
  } else if (e.button == 0 && std::get<1>(octaveSwitch)) {
    this->lowestDisplayNote = clamp(this->lowestDisplayNote - 12, -1 * 12, 8 * 12);
  } else if (e.button == 0 && std::get<0>(measureSwitch)) {
    this->currentMeasure = std::get<1>(measureSwitch);
    ModuleWidget::onMouseDown(e); // Allow drag operations to start on the measure
  } else {
    ModuleWidget::onMouseDown(e);
  }
}

void PianoRollWidget::onDragStart(EventDragStart& e) {
  Vec pos = gRackWidget->lastMousePos.minus(box.pos);
  std::tuple<bool, BeatDiv, Key> cell = findCell(pos);
  std::tuple<bool, int> measureSwitch = findMeasure(pos);

  Rect roll = getRollArea();
  Rect keysArea = reserveKeysArea(roll);
  bool inKeysArea = keysArea.contains(pos);
  bool inPianoRollLogo = Rect(Vec(506, 10), Vec(85, 13)).contains(pos);

  Rect playDragArea(Vec(roll.pos.x, roll.pos.y), Vec(roll.size.x, topMargins));

  if (std::get<0>(cell) && windowIsShiftPressed()) {
    currentDragType = new VelocityDragging(this, module, module->transport.currentPattern(), this->currentMeasure, std::get<1>(cell).num);
  } else if (std::get<0>(cell)) {
    currentDragType = new NotePaintDragging(this, module);
  } else if (inKeysArea) {
    currentDragType = new KeyboardDragging(this, module);
  } else if (playDragArea.contains(pos)) {
    currentDragType = new PlayPositionDragging(this, module);
  } else if (inPianoRollLogo && windowIsShiftPressed()) {
    currentDragType = new ColourDragging(this, module);
  } else if (std::get<0>(measureSwitch)) {
    currentDragType = new LockMeasureDragging(this, module);
  } else {
    currentDragType = new StandardModuleDragging(this, module);
  }

  ModuleWidget::onDragStart(e);
}

void PianoRollWidget::baseDragMove(EventDragMove& e) {
  ModuleWidget::onDragMove(e);
}

void PianoRollWidget::onDragMove(EventDragMove& e) {
  currentDragType->onDragMove(e);
}

void PianoRollWidget::onDragEnd(EventDragEnd& e) {
  delete currentDragType;
  currentDragType = NULL;

  ModuleWidget::onDragEnd(e);
}

json_t *PianoRollWidget::toJson() {
  json_t *rootJ = ModuleWidget::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  json_object_set_new(rootJ, "lowestDisplayNote", json_integer(this->lowestDisplayNote));
  json_object_set_new(rootJ, "notesToShow", json_integer(this->notesToShow));
  json_object_set_new(rootJ, "currentMeasure", json_integer(this->currentMeasure));
  json_object_set_new(rootJ, "backgroundHue", json_real(this->backgroundHue));
  json_object_set_new(rootJ, "backgroundSaturation", json_real(this->backgroundSaturation));
  json_object_set_new(rootJ, "backgroundLuminosity", json_real(this->backgroundLuminosity));
  return rootJ;
}

void PianoRollWidget::fromJson(json_t *rootJ) {
  ModuleWidget::fromJson(rootJ);

  json_t *lowestDisplayNoteJ = json_object_get(rootJ, "lowestDisplayNote");
  if (lowestDisplayNoteJ) {
    lowestDisplayNote = json_integer_value(lowestDisplayNoteJ);
  }

  json_t *notesToShowJ = json_object_get(rootJ, "notesToShow");
  if (notesToShowJ) {
    notesToShow = json_integer_value(notesToShowJ);
  }

  json_t *currentMeasureJ = json_object_get(rootJ, "currentMeasure");
  if (currentMeasureJ) {
    currentMeasure = json_integer_value(currentMeasureJ);
  }

  json_t *backgroundHueJ = json_object_get(rootJ, "backgroundHue");
  if (backgroundHueJ) {
    backgroundHue = json_real_value(backgroundHueJ);
  }

  json_t *backgroundSaturationJ = json_object_get(rootJ, "backgroundSaturation");
  if (backgroundSaturationJ) {
    backgroundSaturation = json_real_value(backgroundSaturationJ);
  }

  json_t *backgroundLuminosityJ = json_object_get(rootJ, "backgroundLuminosity");
  if (backgroundLuminosityJ) {
    backgroundLuminosity = json_real_value(backgroundLuminosityJ);
  }
}


