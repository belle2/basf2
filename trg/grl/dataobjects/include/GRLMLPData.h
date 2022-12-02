/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GRLMLPDATA_H
#define GRLMLPDATA_H

#include <TObject.h>

namespace Belle2 {
  /** Struct for training data of a single MLP for the neuro trigger */
  class GRLMLPData : public TObject {
  public:
    /** default constructor. */
    GRLMLPData(): m_inputSamples(), m_targetSamples() { }
    /** destructor, empty because we don't allocate memory anywhere. */
    ~GRLMLPData() { }

    /** add a pair of input and target */
    void addSample(const std::vector<float>& input, const std::vector<float>& target)
    {
      m_inputSamples.push_back(input);
      m_targetSamples.push_back(target);
    }

    /** get number of samples (same for input and target) */
    unsigned getNumberOfSamples() const { return m_targetSamples.size(); }
    /** get input vector of sample i */
    const std::vector<float>& getInput(unsigned i) const { return m_inputSamples[i]; }
    /** get target value of sample i */
    const std::vector<float>& getTarget(unsigned i) const { return m_targetSamples[i]; }

  private:
    /** list of input vectors for network training. */
    std::vector<std::vector<float>> m_inputSamples;
    /** list of target values for network training. */
    std::vector<std::vector<float>> m_targetSamples;

    //! Needed to make the ROOT object storable
    ClassDef(GRLMLPData, 1);
  };
}
#endif
