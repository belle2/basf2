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
#include <bitset>
#include <algorithm>

namespace Belle2 {
  /** Hit pattern of the VXD with efficient setters and getters.
   *
   *  @TODO A lot, the interface has to be written soon.
   *
   *  This class was developed after and is similar to
   *  @sa HitPatternCDC
   *  We only save hits from the outgoing arm.
   */
  class HitPatternVXD {
  public:
    /** Create empty pattern.*/
    HitPatternVXD();

    /** Initialize the pattern with some short.*/
    HitPatternVXD(const unsigned short& initValue) : m_pattern(initValue)
    {}

    /** */
    bool hasLayer(const unsigned short&) {
      return 1;
    }

  private:
    /** Storing of actual hit pattern.
     *
     *  The two PXD layers get 4 bits each,
     *  the SVD layers have 2 bits each.
     *  In the PXD there is an indication if the hit was from the rescue mechanism
     *  and if it was taken during gated mode running.
     *  1-4 = normal hit, 5-8 = rescue hit, 9-12 = normal gated mode, 13-16 = rescue during gated mode;
     */
    std::bitset<16> m_pattern;
  };

}
