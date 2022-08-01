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
#include <vector>

namespace Belle2 {

  /**
   * Class to keep track of delta histograms
   */
  class HistDelta {
  public:
    int m_type;
    int m_parameter;
    unsigned int m_amountDeltas;
    TH1* m_lastHist;
    std::vector<TH1*> m_deltaHists;
  public:
    HistDelta(int t = 0, int p = 0, unsigned int a = 0);
    void set(int t, int p, unsigned int a);
    void update(TH1* hist);
    void reset(void);
    TH1* getDelta(unsigned int n = 0);
  };

}
