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
  class KLMMuonIDDNN : public RelationsObject {

  public:

    /** constructor */
    KLMMuonIDDNN() {};

    /** destructor */
    virtual ~KLMMuonIDDNN() {};

    /** set the NN KLM muon likelihood output  */
    void setKLMMuonIDDNN(double klmMuonIDDNN)
    {
      m_klmMuonIDDNN = klmMuonIDDNN;
    }

    /** get the DNN based KLM muonID output  */
    double getKLMMuonIDDNN() const {return m_klmMuonIDDNN;}

  private:

    double m_klmMuonIDDNN;

    /** ClassDef. */
    ClassDef(KLMMuonIDDNN, 1);
  };
}
