/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {

  /** The payload containing the cuts for 2 mva based filters
   *  to decide whether a RecoTrack should be flipped or not
   */
  class TrackFlippingCuts: public TObject {
  public:

    /** Default constructor */
    TrackFlippingCuts() {}
    /** Destructor */
    ~TrackFlippingCuts() {}

    /** Set the cuts for the two filters
     * @param cutFirst the cut for the 1st MVA
     * @param cutSecond the cut for the 2nd MVA
     */
    void setCuts(const float cutFirst, const float cutSecond)
    {
      m_cutFirst = cutFirst;
      m_cutSecond = cutSecond;
    }

    /** get the first cuts value */
    float getFirstCut() const
    {
      return m_cutFirst;
    }

    /** get the second cuts value */
    float getSecondCut() const
    {
      return m_cutSecond;
    }

  private:
    /** The cuts for the 1st filter*/
    float m_cutFirst;
    /** The cuts for the 2nd filter*/
    float m_cutSecond;

    ClassDef(TrackFlippingCuts, 1);  /**< ClassDef, necessary for ROOT */
  };
}
