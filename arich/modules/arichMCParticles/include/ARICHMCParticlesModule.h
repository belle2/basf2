/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

namespace Belle2 {
  class Track;
  class ExtHit;
  class MCParticle;

  /**
   * Module to match ARICH hits to MCParticles
   */
  class ARICHMCParticlesModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHMCParticlesModule();

    /**
     * Destructor
     */
    virtual ~ARICHMCParticlesModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    // required input
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */
    StoreArray<ExtHit> m_extHits; /**< Required input array of ExtHits */
    StoreArray<MCParticle> m_arichMCPs; /**< Required input array of MCParticles */

  };

} // Belle2 namespace