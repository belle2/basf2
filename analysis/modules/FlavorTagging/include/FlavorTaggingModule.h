/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Moritz Gelb, Pablo Goldenzweig,       *
 *               Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FLAVORTAGGINGMODULE_H
#define FLAVORTAGGINGMODULE_H

#include <analysis/TMVAInterface/Teacher.h>
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
   * Flavor Tagging module for modular analysis
   *
   * This module finds the flavour of the non reconstructed B *
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


    int m_mode;       /**< Mode of use of the Flavor Tagging Module 0 for Teacher, 1 for Expert*/
    std::string m_listName;               /**< Breco particle list name */
    std::vector<Belle2::Track*> m_tagTracks;        /**< tracks of the rest of the event */
    //std::vector<Belle2::ECLCluster* > m_tagECLClusters;     /**< ECLClusters of the rest of the event */
    //std::vector<Belle2::KLMCluster* > m_tagKLMClusters;     /**< KLMClusters of the rest of the event */

    bool getTagObjects(Particle* Breco);  /** get the tracks, ECLClusters and KLMClusters on the tag side (rest of the event) */

    bool getMC_PDGcodes();      /** get the PDG Codes of the Daughters of B and B-Bar */


    bool Muon_Cathegory();   /** prepare input values for the NN specialized for Muons */

    bool Kaon_Category();   /** prepare input values for the NN specialized for Muons */

    TMVAInterface::Teacher* teacher; //

    /**
     * Variables needed for the Muon Cathegory
     */
    double Charge(Track* track); /** Charge of the fitted Track */
    double p_cms(Track* track); /**  Momentum of the fitted Track in the center of mass system */
    double Theta_Lab(Track* track); /**  Slope of the fitted Track in the r-z plane in the center of mass system */
    double PID_MonteCarlo(Track* track); /**  Flavor of Btag from the Monte Carlo (0 for B0bar, 1 for B0)*/
    double PID_Likelihood(Track* track); /**  Return the combined likelihood probability for a particle being a muon and not a pion.  */



  };
}

#endif /* FLAVORTAGGINGMODULE_H */
