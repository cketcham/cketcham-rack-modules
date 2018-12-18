namespace SongRoll {

  class SongRollData;

  class Transport {
  public:
    Transport(SongRollData* data);
    void reset();

  private:
    SongRollData* data;
  };

}
