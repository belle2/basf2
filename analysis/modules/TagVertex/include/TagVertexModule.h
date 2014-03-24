/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TAGVERTEXMODULE_H
#define TAGVERTEXMODULE_H

#include <framework/core/Module.h>

// rave
#include <analysis/raveInterface/RaveSetup.h>
#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveKinematicVertexFitter.h>

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
  class TagVertexModule : public Module {

  public:

    /**
     * Constructor
     */
    TagVertexModule();

    /**
     * Destructor
     */
    virtual ~TagVertexModule();

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

    //std::string m_EventType;      /**< Btag decay type */
    std::string m_listName;       /**< Breco particle list name */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    std::vector<Belle2::Track*> tagTracks;  /**< tracks of the rest of the event */
    double m_fitPval;             /**< P value of the tag side fit result */

    /** central method for the tag side vertex fit */
    bool doVertexFit(Particle* Breco);

    /** calculate the constraint for the vertex fit on the tag side */
    bool findConstraint(Particle* Breco);

    /** get the vertex of the MC B particle associated to Btag. It works anly with signal MC */
    void BtagMCVertex(Particle* Breco);

    /** compare Breco with the two MC B particles */
    bool compBrecoBgen(Particle* Breco, MCParticle* Bgen);

    /** get the tracks list from the rest of event */
    bool getTagTracks(Particle* Breco);

    /** TO DO: tag side vertex fit in the case of semileptonic tag side decay */
    //bool makeSemileptonicFit(Particle *Breco);

    /**
     * make the vertex fit on the tag side:
     * RAVE AVF
     * tracks coming from Ks removed
     * all other tracks used
     */
    bool makeGeneralFit(Particle* Breco);

  };
}

#endif /* TAGVERTEXMODULE_H */
