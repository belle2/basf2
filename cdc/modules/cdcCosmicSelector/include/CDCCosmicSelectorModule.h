/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

//C++/C standard lib elements.
//#include <string>

namespace Belle2 {

  /** The Class for.
   *
   */
  class CDCCosmicSelectorModule : public Module {

  public:
    /** Constructor.*/
    CDCCosmicSelectorModule();

    /** Initialize variables, print info, and start CPU clock. */
    void initialize() override;

    /** Actual digitization of all hits in the CDC.
     *
     *  The digitized hits are written into the DataStore.
     */
    void event() override;

  private:
    StoreArray<MCParticle> m_mcParticles; /**< array of MCParticle */
    double m_xOfCounter;   /**< x-pos. of counter (cm) */
    double m_yOfCounter;   /**< y-pos. of counter (cm) */
    double m_zOfCounter;   /**< z-pos. of counter (cm) */
    double m_phiOfCounter; /**< phi-angle of counter (deg) */
    double m_wOfCounter;   /**< full-width  of counter (cm) */
    double m_lOfCounter;   /**< full-length of counter (cm) */
    double m_propSpeed;    /**< Light speed in counter (cm/ns) */
    int    m_tof;          /**< tof option */
    bool   m_cryGenerator; /**< cry or cosmics generator */
    bool    m_top;         /**< top option */
  };

} // end of Belle2 namespace
