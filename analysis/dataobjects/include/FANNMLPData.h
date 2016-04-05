/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FANNMLPDATA_H
#define FANNMLPDATA_H

#include <framework/datastore/RelationsObject.h>
#include <analysis/dataobjects/Particle.h>

#include <vector>
#include <set>
#include <map>
#include <string>

namespace Belle2 {
  /** Struct for training data of a single FANN MLP */
  class FANNMLPData : public RelationsObject {
  public:
    /** Default constructor. */
    FANNMLPData(): m_inputSamples(), m_targetSamples() { }
    /** Destructor, empty because we don't allocate memory anywhere. */
    ~FANNMLPData() { }

    /** Add a pair of input and target */
    void addSample(std::vector<float> input, std::vector<float> target)
    {
      m_inputSamples.push_back(input);
      m_targetSamples.push_back(target);
    }

    /** Get number of samples (same for input and target) */
    unsigned nSamples() const { return m_targetSamples.size(); }
    /** Get input vector of sample i */
    std::vector<float> getInput(unsigned i) const { return m_inputSamples[i]; }
    /** Get target value of sample i */
    std::vector<float> getTarget(unsigned i) const { return m_targetSamples[i]; }

    /** Clears the input and target vectors*/
    void clear() { m_inputSamples.clear(); m_targetSamples.clear();}

    /** Gets two vectors containing the list of minimal and maximal input values to be used for scaling. */
    std::vector<std::vector<float>> getMinAndMaxInputSample();

    /** Gets two vectors containing the list of minimal and maximal target values to be used for scaling. */
    std::vector<std::vector<float>> getMinAndMaxTargetSample();

  private:
    /** List of input vectors for network training. */
    std::vector<std::vector<float>> m_inputSamples;
    /** List of target values for network training. */
    std::vector<std::vector<float>> m_targetSamples;

    //! Needed to make the ROOT object storable
    ClassDef(FANNMLPData, 1);
  };
}
#endif