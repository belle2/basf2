/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMHITBASE_H
#define SIMHITBASE_H

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
    /** Enum for background tags. */
    enum BG_TAG { bg_none         = 0, /**< No background.*/
                  bg_Coulomb_LER  = 1, /**< Coulomb LER.*/
                  bg_Coulomb_HER  = 2, /**< Coulomb HER.*/
                  bg_RBB_LER      = 3, /**< RBB LER.*/
                  bg_RBB_HER      = 4, /**< RBB_HER.*/
                  bg_Touschek_LER = 5, /**< Touschek LER.*/
                  bg_Touschek_HER = 6, /**< Touschek HER.*/
                  bg_twoPhoton    = 7, /**< 2-photon */
                  bg_other        = 99 /**< Other type of background.*/
                };
    /** Constructor */
    SimHitBase(): m_backgroundTag(bg_none) {}

    /** Set the background tag.
     * @param backgroundTag The desired setting of the background tag.
     */
    virtual void setBackgroundTag(unsigned int backgroundTag)
    { m_backgroundTag = static_cast<unsigned short>(backgroundTag); }

    /** Get the background tag.
     * @return The background tag value.
     */
    virtual unsigned short getBackgroundTag() const { return m_backgroundTag; }

    /** The method to shift the SimHit in time.
     * Re-implement for your subdetector's SimHit class if time-aware background
     * is needed.
     * @param delta The desired time shift.
     */
    virtual void shiftInTime(float /*delta*/) {}

  private:
    /** Background tag */
    unsigned short m_backgroundTag;

    ClassDef(SimHitBase, 2)
  };


} // end namespace Belle2

#endif
