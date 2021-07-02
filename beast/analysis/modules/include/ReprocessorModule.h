/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef REPROCESSORMODULE_H
#define REPROCESSORMODULE_H

#include <mdst/dataobjects/MCParticle.h>
#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <beast/microtpc/dataobjects/TPCG4TrackInfo.h>
#include <beast/he3tube/dataobjects/HE3G4TrackInfo.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>

namespace Belle2 {
  //  namespace reprocessor {
  /**
   * Reprocessor Module
   *
   * Re-launch MC Particle
   *
   */
  class ReprocessorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ReprocessorModule();

    /**
     * Destructor
     */
    virtual ~ReprocessorModule();

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


  private:


    StoreObjPtr<EventMetaData> m_evtMetaData; /**< event meta data Object pointer */
    StoreArray<MCParticle> m_mcParticle; /**< mc Particle Array */

    /** Set PDG*/
    int m_input_TPC_PDG;
    /** Set N times */
    int m_input_TPC_Ntimes;
    /** Set PDG*/
    int m_input_HE3_PDG;
    /** Set N times */
    int m_input_HE3_Ntimes;


    StoreArray<HE3G4TrackInfo> mc_he3_parts; /**< Array of G4 particles crossing the He3 tubes */
    StoreArray<TPCG4TrackInfo> mc_tpc_parts; /**< Array of G4 particles crossing the uTPCs */
    StoreArray<SADMetaHit> MetaHits; /**< Array of SAD particles */



  };

  //  }
}

#endif /* REPROCESSORMODULE_H */
