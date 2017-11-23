#ifndef CDCTRIGGERMLPDATA_H
#define CDCTRIGGERMLPDATA_H

#include <TObject.h>

namespace Belle2 {
  /** Struct for training data of a single MLP for the neuro trigger */
  class CDCTriggerMLPData : public TObject {
  public:
    /** default constructor. */
    CDCTriggerMLPData(): inputSamples(), targetSamples(), hitCounters(), trackCounter(0) { }
    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerMLPData() { }

    /** add hit counters for a layer with nWires */
    void addCounters(unsigned nWires)
    {
      std::vector<unsigned short> counters;
      counters.assign(nWires, 0);
      hitCounters.push_back(counters);
    }
    /** increase counter for super layer and track segment number in super layer.
     *  track segment number can be negative.
     *  hits in the wrong hemisphere (not in [-nWires/4, nWires/4]) are skipped. */
    void addHit(unsigned iSL, int iTS);
    /** increase track counter */
    void countTrack() { ++trackCounter; }
    /** add a pair of input and target */
    void addSample(const std::vector<float>& input, const std::vector<float>& target)
    {
      inputSamples.push_back(input);
      targetSamples.push_back(target);
    }

    /** get track counter */
    short getTrackCounter() const { return trackCounter; }
    /** get hit counter for super layer and track segment number is super layer.
     *  track segment number can be negative.
     *  @return counter or 0 (for invalid input) */
    unsigned short getHitCounter(unsigned iSL, int iTS) const;
    /** get number of samples (same for input and target) */
    unsigned nSamples() const { return targetSamples.size(); }
    /** get input vector of sample i */
    const std::vector<float>& getInput(unsigned i) const { return inputSamples[i]; }
    /** get target value of sample i */
    const std::vector<float>& getTarget(unsigned i) const { return targetSamples[i]; }

  private:
    /** list of input vectors for network training. */
    std::vector<std::vector<float>> inputSamples;
    /** list of target values for network training. */
    std::vector<std::vector<float>> targetSamples;
    /** hit counter of active track segment IDs,
     *  used to determine the relevant id range for an MLP. */
    std::vector<std::vector<unsigned short>> hitCounters;
    /** number of tracks used for the hit counter. */
    short trackCounter;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerMLPData, 1);
  };
}
#endif
