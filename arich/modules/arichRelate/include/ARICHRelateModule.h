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
  class ARICHAeroHit;

  /**
   * Creates relations between ARICHAeroHits and ExtHits. Allows to store simulation output without MCParticles
   */
  class ARICHRelateModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHRelateModule();

    /**
     * Destructor
     */
    virtual ~ARICHRelateModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    // required input
    StoreArray<MCParticle> m_mcParticles; /**< Required array of input MCParticles */
    StoreArray<Track> m_mdstTracks; /**< Required array of input Tracks */
    StoreArray<ARICHAeroHit> m_aeroHits; /**< Required array of input ARICHAeroHits */
    StoreArray<ExtHit> m_extHits; /**< Required array of input ExtHits */

  };

} // Belle2 namespace
