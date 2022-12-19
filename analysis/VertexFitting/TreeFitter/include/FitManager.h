/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/dataobjects/Particle.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>
#include <analysis/VertexFitting/TreeFitter/ConstraintConfiguration.h>

namespace TreeFitter {
  class DecayChain;
  class FitParams;
  class ParticleBase;
  class ConstraintConfiguration;

  /** this class */
  class FitManager {
  public:

    /** status flag of the fit-itereation (the step in the newton method) */
    enum VertexStatus { Success = 0, NonConverged, BadInput, Failed, UnFitted };

    /** constructor  */
    FitManager() : m_particle(0), m_decaychain(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_prec(0.01), m_updateDaugthers(false), m_ndf(0),
      m_fitparams(0), m_useReferencing(false)
    {}

    /** constructor  */
    FitManager(Belle2::Particle* particle,
               const ConstraintConfiguration& config,
               double prec = 0.01,
               bool updateDaughters = false,
               const bool useReferencing = false
              );

    /** use default copy constructor */
    FitManager(const FitManager& other) = delete;

    /** use default assignment op */
    FitManager& operator=(const FitManager& other) = delete;

    /** destructor does stuff */
    ~FitManager();

    /** main fit function that uses the kalman filter */
    bool fit();

    /** update particles parameters with the fit results */
    bool updateCand(Belle2::Particle& particle, const bool isTreeHead) const;

    /** locate particle base for a belle2 particle and update the particle with the values from particle base */
    void updateCand(const ParticleBase& pb, Belle2::Particle& cand, const bool isTreeHead) const;

    /** update the Belle2::Particles with the fit results  */
    void updateTree(Belle2::Particle& particle, const bool isTreeHead) const;

    /** extract cov from particle base */
    void getCovFromPB(const ParticleBase* pb, TMatrixFSym& returncov) const;

    /** get lifetime */
    std::tuple<double, double> getLifeTime(Belle2::Particle& cand) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(const ParticleBase* pb) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(const ParticleBase* pb, const FitParams& fitparams) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(Belle2::Particle& cand) const;

    /**  getter for the head of the tree*/
    Belle2::Particle* particle() { return m_particle; }

  private:
    /** head of the tree  */
    Belle2::Particle* m_particle;

    /**  the decay tree */
    DecayChain* m_decaychain;

    /** status of the current iteration */
    int m_status;

    /** chi2 of the current iteration */
    double m_chiSquare;

    /** precision that is needed for status:converged (delta chi2) */
    double m_prec;

    /** errorcode */
    ErrCode m_errCode;

    /** if this is set all daughters will be updated otherwise only the head of the tree */
    const bool m_updateDaugthers;

    /** number of degrees of freedom for this topology */
    int m_ndf;

    /** parameters to be fitted */
    FitParams* m_fitparams;

    /** use referencing */
    bool m_useReferencing;

    /** config container */
    const ConstraintConfiguration m_config;
  };
}
