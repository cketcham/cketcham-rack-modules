#include "rack.hpp"

using namespace rack;

struct CancelPasteItem : MenuItem {
  PianoRollWidget *widget = NULL;
  void onAction(EventAction &e) override {
    widget->state = COPYREADY;
  }
};
