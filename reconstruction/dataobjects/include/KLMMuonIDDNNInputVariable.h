/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * KLM MuonID DNN input variables datastore object to store
   * the input variables for retraining KLMMuonIDDNNExpertModule.
   */
  class KLMMuonIDDNNInputVariable : public RelationsObject {

  public:

    /** constructor */
    KLMMuonIDDNNInputVariable() {};

    /** destructor */
    virtual ~KLMMuonIDDNNInputVariable() {};

    /** set the DNN input variables. */
    void setKLMMuonIDDNNInputVariable(const std::vector<float>& inputdata)
    {
      m_dataset.assign(inputdata.begin(), inputdata.end());
    }

    /** Add additional input variables. Useful when you want to train with new input variables. */
    void addAdditionalInput(float input)
    {
      m_dataset.push_back(input);
    }

    /** get the DNN input variable with index iInput. */
    float getInputVariable(const unsigned int iInput) const {return m_dataset[iInput];}

    /** get the length of DNN input variables. */
    float getInputLength() const {return m_dataset.size();}

  private:

    /** vector to store the input array of KLMMuonIDDNNExpertModule. */
    std::vector<float> m_dataset;

    /** ClassDef. */
    ClassDef(KLMMuonIDDNNInputVariable, 1);
  };
}
