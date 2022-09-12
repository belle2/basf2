/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  * Different sets of scale factors are defined for tracks with/without a PXD hit.
  * For tracks with a PXD hit, in order to avoid severe underestimation of d0 and z0 errors,
  * lower limits (best resolution) can be set in a momentum-dependent form.
  * The module also accepts a V0 Kshort particle list as input and applies the error correction to its daughters.
  * Note the difference in impact parameter resolution between V0 daughters and tracks from IP,
  * as V0 daughters are free from multiple scattering through the beam pipe.
  *
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

    /** create a Particle with scaled errors */
    Particle* getChargedWithScaledError(const Particle* particle);

    /** create a TrackFitResult with scaled errors */
    const TrackFitResult* getTrackFitResultWithScaledError(const Particle* particle);

    /** get scale factors */
    std::vector<double> getScaleFactors(const Particle* particle, const TrackFitResult* trkfit);

  private:

    std::string m_inputListName;  /**< The name of input charged particle list */
    std::string m_outputListName;  /**< The name of output charged particle list */
    std::string m_outputAntiListName;   /**< output anti-particle list name */
    std::vector<double> m_scaleFactors_PXD;  /**< vector of five scale factors for helix parameter errors (for tracks with a PXD hit) */
    std::vector<double>
    m_scaleFactors_noPXD;  /**< vector of five scale factors for helix parameter errors (for tracks without a PXD hit) */
    std::vector<double> m_d0ResolPars; /**< parameters (a,b) to define d0 best resolution = a (+) b / (p*beta*sinTheta**1.5)  */
    std::vector<double> m_z0ResolPars; /**< parameters (a,b) to define z0 best resolution = a (+) b / (p*beta*sinTheta**2.5)  */
    double m_d0MomThr; /**< d0 best resolution is kept constant below this momentum. */
    double m_z0MomThr; /**< z0 best resolution is kept constant below this momentum. */
    int m_pdgCode;                /**< PDG code of the charged particle to be scaled */
    bool m_scaleKshort; /**< Whether the input particle list is Kshort or not */
    std::string m_decayString;   /**< Input DecayString specifying the input particle */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the charged particle */
    StoreObjPtr<ParticleList>  m_inputparticleList; /**< StoreObjptr for input charged particlelist */
    StoreObjPtr<ParticleList>  m_outputparticleList; /**< StoreObjptr for output particlelist */
    StoreObjPtr<ParticleList>  m_outputAntiparticleList; /**< StoreObjptr for output antiparticlelist */
    StoreArray<Particle> m_particles; /**< StoreArray of Particle objects */
    StoreArray<MCParticle> m_mcparticles; /**< StoreArray of MCParticle objects */
    StoreArray<PIDLikelihood> m_pidlikelihoods; /**< StoreArray of PIDLikelihood objects */
    StoreArray<TrackFitResult> m_trackfitresults; /**< StoreArray of TrackFitResult objects */
  };
}

