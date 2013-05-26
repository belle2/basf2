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
   *  @ingroup Dataobjects
   *  @{
   *  Track
   *  @}
   */
  /** Hit pattern of CDC hits within a track and efficient getters.
   *
   *  The pattern is stored using a std::bitset. This allows to use some stuff from the
   *  STL, which most likely is very efficiently programmed rather than implementing the
   *  stuff oneself with integer types.
   *  For each layer there is one bit. In addition there is a bit for the superLayers 2-8,
   *  which can be set to indicate, that at least one of the layers has two hits for the track.
   *  This is the idea at the moment. Perhaps we will later decide to switch this to some
   *  better description of the outermost super-layer...
   */
  class HitPatternCDC {
  public:
    /** Create empty pattern.
     */
    HitPatternCDC() {}

    /** Initialize the pattern with some long int.
     */
    HitPatternCDC(const unsigned long& initValue) : m_pattern(initValue)
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
    void setLayer(unsigned short layer) {
      m_pattern.set(layer);
    }

    /** Getter for single wire.
     */
    bool getLayer(unsigned short layer) {
      return m_pattern[layer];
    }

    /** Getter for Super-Layer match.
     */
    bool getSLayer(unsigned short sLayer) {
      return ((m_pattern & s_sLayerMasks[sLayer]).any());
    }

    /** Getter for the approximate number of hits in one super-layer.
     *
     *  In case of multiple layers with two or more hits or
     *  any layers with more than two hits leads to under-counting.
     */
    unsigned short getNSLayer(unsigned short sLayer) {
      return static_cast<unsigned short>((m_pattern & s_sLayerMasks[sLayer]).count());
    }

  private:
    std::bitset<64> m_pattern;                     /**<  Saves the actual pattern.*/
    static const std::bitset<64> s_sLayerMasks[9]; /**<  Masks to zero out all bits from other layers.*/

  };
}
