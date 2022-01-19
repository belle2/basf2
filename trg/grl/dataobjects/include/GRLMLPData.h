#ifndef GRLMLPDATA_H
#define GRLMLPDATA_H

#include <TObject.h>

namespace Belle2 {
  /** Struct for training data of a single MLP for the neuro trigger */
  class GRLMLPData : public TObject {
  public:
    /** default constructor. */
    GRLMLPData(): inputSamples(), targetSamples() { }
    /** destructor, empty because we don't allocate memory anywhere. */
    ~GRLMLPData() { }

    /** add a pair of input and target */
    void addSample(const std::vector<float>& input, const std::vector<float>& target)
    {
      inputSamples.push_back(input);
      targetSamples.push_back(target);
    }

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

    //! Needed to make the ROOT object storable
    ClassDef(GRLMLPData, 1);
  };
}
#endif
