#include "rack.hpp"

using namespace rack;

namespace SongRoll {

  class SongRollData;

  class RollArea : public Widget {
  public:
    SongRollData& data;
    RollArea(Rect box, SongRollData& data);
  };
}
