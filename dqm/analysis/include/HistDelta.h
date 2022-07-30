/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TH1.h>
#include <queue>

namespace Belle2 {

  /**
   * Class to keep track of delta histograms
   */
  class HistDelta {
  private:
    int m_type;
    int m_parameter;
    int m_amountDeltas;
    TH1* m_lastHist;
    std::queue<TH1*> m_deltaHists;
  public:
    void update(TH1* hist);
    void reset(void);
  };

}
