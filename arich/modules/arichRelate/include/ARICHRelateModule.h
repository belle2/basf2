/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dino Tahirovic                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRELATEMODULE_H
#define ARICHRELATEMODULE_H

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHAeroHit.h>

#include <framework/datastore/StoreArray.h>

#include <framework/core/Module.h>
#include <string>
#include <TFile.h>
#include <TNtuple.h>

namespace Belle2 {

  /**
   *
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
    StoreArray<MCParticle> m_mcParticles; /**< Required array of input MCParticles */
    StoreArray<Track> m_mdstTracks; /**< Required array of input Tracks */
    StoreArray<ARICHAeroHit> m_aeroHits; /**< Required array of input ARICHAeroHits */
    StoreArray<ExtHit> m_extHits; /**< Required array of input ExtHits */

  };

} // Belle2 namespace

#endif
