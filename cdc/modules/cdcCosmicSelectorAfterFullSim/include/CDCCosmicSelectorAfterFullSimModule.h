/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCCOSMICSELECTORAFTERFULLSIM_H
#define CDCCOSMICSELECTORAFTERFULLSIM_H

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>

//C++/C standard lib elements.
//#include <string>

namespace Belle2 {

  /** The Class for.
   *
   */
  class CDCCosmicSelectorAfterFullSimModule : public Module {

  public:
    /** Constructor.*/
    CDCCosmicSelectorAfterFullSimModule();

    /** Initialize variables, print info, and start CPU clock. */
    void initialize();

    /** Actual digitization of all hits in the CDC.
     *
     *  The digitized hits are written into the DataStore.
     */
    void event();

  private:
    StoreArray<MCParticle> m_mcParticles; /**< array of MCParticle */
    StoreArray<CDCSimHit>  m_simHits;     /**< array of CDCSimHit */
    double m_xOfRegion;   /**< x-pos.      of region (cm) */
    double m_zOfRegion;   /**< z-pos.      of region (cm) */
    double m_wOfRegion;   /**< full-width  of region (cm) */
    double m_lOfRegion;   /**< full-length of region (cm) */
  };

} // end of Belle2 namespace

#endif // CDCCOSMICSELECTORAFTERFULLSIM_H
