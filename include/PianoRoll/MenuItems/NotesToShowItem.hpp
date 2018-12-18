#include "rack.hpp"

using namespace rack;

struct NotesToShowItem : MenuItem {
  char buffer[100];
  PianoRollWidget* module;
  int value;
  NotesToShowItem(PianoRollWidget* module, int value) {
    this->module = module;
    this->value = value;

    snprintf(buffer, 10, "%d", value);
    text = buffer;
    if (value == module->notesToShow) {
      rightText = "✓";
    }
  }
  void onAction(EventAction &e) override {
    module->notesToShow = value;
  }
};
