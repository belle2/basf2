/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>

using namespace Belle2;

void
CDCTriggerMLPData::addHit(unsigned iSL, int iTS)
{
  if (iSL < m_hitCounters.size()) {
    int N = m_hitCounters[iSL].size();
    // shift negative indices
    if (iTS < 0) iTS += N;
    // check the range
    //   0, N:        hit is on the track
    //   [0, N/4]:    hit is in quadrant left of the track
    //   [N/4, 3N/4]: hit is in hemisphere opposite of the track (skipped)
    //   [3N/4, N]:   hit is in quadrant right of the track
    if ((0 <= iTS && iTS <= N / 4) || (3 * N / 4 <= iTS && iTS < N)) {
      ++m_hitCounters[iSL][iTS];
    }
  }
}

unsigned short
CDCTriggerMLPData::getHitCounter(unsigned iSL, int iTS) const
{
  if (iSL < m_hitCounters.size()) {
    if (iTS < 0) iTS += m_hitCounters[iSL].size();
    if (0 <= iTS && (unsigned)iTS < m_hitCounters[iSL].size()) {
      return m_hitCounters[iSL][iTS];
    }
  }
  return 0;
}
