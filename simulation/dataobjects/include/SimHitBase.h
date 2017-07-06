/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Marko staric                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

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
     * Enum for background tags.
     */
    enum BG_TAG { bg_none             = 0,  /**< No background */
                  bg_Coulomb_LER      = 1,  /**< Coulomb LER */
                  bg_Coulomb_HER      = 2,  /**< Coulomb HER */
                  bg_RBB_LER          = 3,  /**< Radiative Bhabha LER */
                  bg_RBB_HER          = 4,  /**< Radiative Bhabha HER */
                  bg_Touschek_LER     = 5,  /**< Touschek LER */
                  bg_Touschek_HER     = 6,  /**< Touschek HER */
                  bg_twoPhoton        = 7,  /**< 2-photon */
                  bg_RBB_gamma        = 8,  /**< Gammas from radiative Bhabha */
                  bg_RBB_LER_far      = 9,  /**< Radiative Bhabha far LER */
                  bg_RBB_HER_far      = 10, /**< Radiative Bhabha far HER */
                  bg_Touschek_LER_far = 11, /**< Touschek far LER */
                  bg_Touschek_HER_far = 12, /**< Touschek far HER */
                  bg_SynchRad_LER     = 13, /**< Synchrotron radiation LER */
                  bg_SynchRad_HER     = 14, /**< Synchrotron radiation HER */
                  bg_BHWide_LER       = 15, /**< Wide angle radiative Bhabha LER */
                  bg_BHWide_HER       = 16, /**< Wide angle radiative Bhabha HER */
                  bg_RBB              = 17, /**< Radiative Bhabha */
                  bg_BHWide           = 18, /**< Wide angle radiative Bhabha */
                  bg_BHWideLargeAngle = 19, /**< Large angle radiative Bhabha */
                  bg_InjectionLER     = 21, /**< injection background LER */
                  bg_InjectionHER     = 22, /**< injection background HER */
                  bg_other            = 99  /**< Other type of background */
                };
    /**
     * Constructor
     */
    SimHitBase(): m_backgroundTag(bg_none) {}

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

