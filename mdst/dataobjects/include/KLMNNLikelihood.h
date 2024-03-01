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
   * KLM muon likelihood (KLMLikelihood) datastore object to store results
   * from Neural Network based KLM muon likelihood calculations.
   */
  class KLMNNLikelihood : public RelationsObject {

  public:

    /** constructor */
    KLMNNLikelihood() {};

    /** destructor */
    virtual ~KLMNNLikelihood() {};

    /** set the NN KLM muon likelihood output  */
    void setKLMNNLikelihood(double muprob)
    {
      m_muprobnn = muprob;
    }

    /** get the NN KLM muon likelihood output  */
    double getKLMNNLikelihood() const {return m_muprobnn;}

  private:

    double m_muprobnn;

    /** ClassDef. */
    ClassDef(KLMNNLikelihood, 1);
  };
}
