/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Pablo Goldenzweig, Luigi Li Gioi      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FLAVORTAGGINGMODULE_H
#define FLAVORTAGGINGMODULE_H

#include <framework/core/Module.h>

#include <mdst/dataobjects/KLMCluster.h>
#include "mdst/dataobjects/ECLCluster.h"

// track objects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <string>


namespace Belle2 {

  class Particle;

  /**
   * Tag side Vertex Fitter module for modular analysis
   *
   * This module fits the Btag Vertex    *
   */
  class FlavorTaggingModule : public Module {

  public:

    /**
     * Constructor
     */
    FlavorTaggingModule();

    /**
     * Destructor
     */
    virtual ~FlavorTaggingModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things
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



  private:

    std::string m_listName;               /**< Breco particle list name */
    std::vector<Belle2::Track*> tagTracks;        /**< tracks of the rest of the event */
    std::vector<Belle2::ECLCluster* > tagECLClusters;     /**< ECLClusters of the rest of the event */
    std::vector<Belle2::KLMCluster* > tagKLMClusters;     /**< KLMClusters of the rest of the event */

    bool getTagObjects(Particle* Breco);  /** get the tracks, ECLClusters and KLMClusters on the tag side (rest of the event) */

    bool getMC_PDGcodes();      /** get the PDG Codes of the Daughters of B and B-Bar */
    bool Input_Values_Muon();   /** prepare input values for the NN specialized for Muons */



  };
}

#endif /* FLAVORTAGGINGMODULE_H */
