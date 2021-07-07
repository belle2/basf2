/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/BackgroundMetaData.h>
#include <framework/datastore/RelationsObject.h>


namespace Belle2 {

  /**
    * Class SimHitBase - A common base for subdetector SimHits.
    *
    * This is a base class from which other SimHit classes have to be derived in
    * to be usable for background mixing.
    * It implements a background type tag and a method to shift the SimHit in time.
    */
  class SimHitBase : public RelationsObject {
  public:
    /**
     * Constructor
     */
    SimHitBase(): m_backgroundTag(BackgroundMetaData::bg_none) {}

    /**
     * Set background tag.
     * @param backgroundTag The desired setting of the background tag.
     */
    virtual void setBackgroundTag(unsigned int backgroundTag)
    { m_backgroundTag = static_cast<unsigned short>(backgroundTag); }

    /**
     * Get background tag.
     * @return The background tag value.
     */
    virtual unsigned short getBackgroundTag() const { return m_backgroundTag; }

    /**
     * The method to shift the SimHit in time.
     * Re-implement for your subdetector's SimHit class if time-aware background
     * is needed.
     * @param delta The desired time shift.
     */
    virtual void shiftInTime(float /*delta*/) {}

    /**
     * Get time of the hit.
     * Re-implement for your subdetector's SimHit class.
     * @return global time
     */
    virtual float getGlobalTime() const { return 0; }


  private:

    unsigned short m_backgroundTag; /**< Background tag */

    ClassDef(SimHitBase, 3)
  };


} // end namespace Belle2

