/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric . Luigi Li Gioi                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEVERTEXFITTERMODULE_H
#define PARTICLEVERTEXFITTERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// kfitter
#include <analysis/KFit/MassFitKFit.h>
#include <analysis/KFit/MassVertexFitKFit.h>
#include <analysis/KFit/VertexFitKFit.h>
#include <analysis/KFit/MakeMotherKFit.h>

// rave
#include <analysis/raveInterface/RaveSetup.h>
#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveKinematicVertexFitter.h>

namespace Belle2 {

  class Particle;

  /**
   * Vertex fitter module
   */
  class ParticleVertexFitterModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleVertexFitterModule();

    /**
     * Destructor
     */
    virtual ~ParticleVertexFitterModule();

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

    std::string m_listName;       /**< particle list name */
    double m_confidenceLevel;     /**< required fit confidence level */
    double m_Bfield;              /**< magnetic field from data base */
    std::string m_vertexFitter;   /**< Vertex Fitter name */
    std::string m_fitType;        /**< type of the kinematic fit */
    std::string m_withConstraint; /**< additional constraint on vertex */
    std::string m_decayString   ; /**< daughter particles selection */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */

    /**
     * Main steering routine
     * @param p pointer to particle
     * @return true for successfull fit and prob(chi^2,ndf) > m_confidenceLevel
     */
    bool doVertexFit(Particle* p);

    /**
     * Unconstrained fit using Kfitter
     * @param p pointer to particle
     * @return true for successfull fit
     */
    bool doKvFit(Particle* p);

    /**
     * Make mother using Kfitter
     * @param kv reference to Kfitter object
     * @param p pointer to particle
     * @return true for successfull construction of mother
     */
    bool makeKvMother(analysis::VertexFitKFit& kv, Particle* p);

    /**
     * Fit using Rave
     * @param p pointer to particle
     * @return true for successfull fit and update of mother
     */
    bool doRaveFit(Particle* mother);

    /**  check if all the Daughters (o grand-daugthers) are selected for the vertex fit*/
    bool allSelectedDaughters(const Particle* mother, std::vector<const Particle*> tracksVertex);

  };

} // Belle2 namespace

#endif
