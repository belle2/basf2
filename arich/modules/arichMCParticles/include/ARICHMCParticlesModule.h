/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHMCPARTICLESMODULE_H
#define ARICHMCPARTICLESMODULE_H

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <string>
#include <TFile.h>
#include <TNtuple.h>

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
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

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
