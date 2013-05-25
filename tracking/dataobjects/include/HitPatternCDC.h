/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Martin Heck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once
#include <framework/logging/Logger.h>
#include <bitset>
#include <algorithm>

namespace Belle2 {
  /** @addtogroup Tracking_dataobjects
   *  @ingroup dataobjects
   *  @{
   *  Track
   *  @}
   */
  /** Hit pattern of CDC hits within a track and efficient getters.
   */
  class HitPatternCDC {
  public:
    /** Create empty pattern.
     */
    HitPatternCDC() {}

    /** Initialize the pattern with some long int.
     */
    HitPatternCDC(const long& initValue) : m_pattern(initValue)
    {}

    /** Get the Total Number of CDC hits in the fit.
     */
    short getNHits() {
      return static_cast<short>(m_pattern.count());
    }

    /** Setter for single wire.
     *
     *  This function may throw an out-of-range exception.
     */
    void setLayer(short layer) {
      m_pattern.set(layer);
    }

    /** Getter for single wire.
     */
    bool getLayer(short layer) {
      return m_pattern[layer];
    }

    /** Getter for Super-Layer match.
     */
    bool getSLayer(short sLayer);

  private:
    std::bitset<56> m_pattern;
//    static const std::bitset<56> s_sLayerMasks[];

  };
}
