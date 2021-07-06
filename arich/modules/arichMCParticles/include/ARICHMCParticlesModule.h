/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHMCPARTICLESMODULE_H
#define ARICHMCPARTICLESMODULE_H

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   *
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
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */
    StoreArray<ExtHit> m_extHits; /**< Required input array of ExtHits */
    StoreArray<MCParticle> m_arichMCPs; /**< Required input array of MCParticles */

  };

} // Belle2 namespace

#endif
