/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: H. Tanigawa                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <string>

namespace Belle2 {
  /**
  * scale the error of helix parameters
  *
  * Creates a new charged particle list whose helix errors are scaled by constant factors.
  * Lower bounds can be defined for the helix errors.

  */
  class HelixErrorScalerModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    HelixErrorScalerModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** loop over the input charged particles */
    virtual void event() override;

    /** create a TrackFitResult with scaled errors */
    TrackFitResult* getTrackFitResultWithScaledError(const TrackFitResult* trkfit);

  private:

    std::string m_inputListName;  /**< The name of input charged particle list */
    std::string m_outputListName;  /**< The name of output charged particle list */
    std::string m_outputAntiListName;   /**< output anti-particle list name */
    std::vector<double> m_scaleFactors;  /**< vector of five scale factors for helix parameter errors */
    std::vector<double> m_minErrors;  /**< vector of five scale lower bounds for helix parameter errors */
    int m_pdgCode;                /**< PDG code of the charged particle to be scaled */
    std::string m_decayString;   /**< Input DecayString specifying the input particle */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the charged particle */
    StoreObjPtr<ParticleList>  m_inputparticleList; /**<StoreObjptr for input charged particlelist */
    StoreObjPtr<ParticleList>  m_outputparticleList; /**<StoreObjptr for output particlelist */
    StoreObjPtr<ParticleList>  m_outputAntiparticleList; /**<StoreObjptr for output antiparticlelist */
    StoreArray<Particle> m_particles; /**< StoreArray of Particle objects */
    StoreArray<MCParticle> m_mcparticles; /**< StoreArray of MCParticle objects */
    StoreArray<PIDLikelihood> m_pidlikelihoods; /**< StoreArray of PIDLikelihood objects */
    StoreArray<TrackFitResult> m_trackfitresults; /**< StoreArray of TrackFitResult objects */
  };
}

