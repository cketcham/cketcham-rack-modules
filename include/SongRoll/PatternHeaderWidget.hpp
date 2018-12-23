#include "rack.hpp"

using namespace rack;

namespace SongRoll {

  class PatternHeaderWidget : public Widget {
  public:
    int repeats;
    int repeats_completed;
    int pattern;
    bool active;

    PatternHeaderWidget(int repeats, int repeats_completed, int pattern);

    void draw(NVGcontext* ctx) override;
  };

}
