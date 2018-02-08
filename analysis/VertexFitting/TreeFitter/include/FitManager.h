/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * right(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef FITTER_H
#define FITTER_H

#include <analysis/dataobjects/Particle.h>
#include <vector>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>
#include <analysis/VertexFitting/TreeFitter/EigenTypes.h>


namespace TreeFitter {
  class DecayChain;
  class FitParams;
  class ParticleBase;

  /**FIXME remove */
  extern int vtxverbose;

  /** list of pdg codes to mass constrain */
  extern std::vector<int> massConstraintList;

  /** this class */
  class FitManager {
  public:

    /** status flag of the fit-itereation (the step in the newton method) */
    enum VertexStatus { Success = 0, NonConverged, BadInput, Failed, UnFitted };

    /** empty constructor  */
    FitManager() : m_particle(0), m_decaychain(0), m_fitparams(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_niter(-1), m_prec(0.01) {} //Default constructor

    /** constructor  */
    FitManager(Belle2::Particle* particle, double prec = 0.01, int ipDimension = 0);

    /** destructor does stuff */
    ~FitManager();

    /** main fit function that uses the kalman filter */
    bool fit();

    /** update particles parameters with the fit results */
    bool updateCand(Belle2::Particle& particle) const;

    /** locate particle base for a belle2 particle and update the particle with the values from particle base */
    void updateCand(const ParticleBase& pb, Belle2::Particle& cand) const;

    /** update the Belle2::Particles with the fit results  */
    void updateTree(Belle2::Particle& particle) const;

    /** get covariance matrix */
    const EigenTypes::MatrixXd& getCovariance() const;

    /** extract cov from particle base */
    void getCovFromPB(const ParticleBase* pb, TMatrixFSym& returncov) const;

    /** get lifetime */
    std::tuple<double, double> getLifeTime(Belle2::Particle& cand) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(const ParticleBase* pb) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(const ParticleBase* pb, const FitParams* fitparams) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(Belle2::Particle& cand) const;

    /** get Statevector */
    const EigenTypes::ColVector& getStateVector() const;;

    /** getter for the index of the vertex position in the state vector */
    int posIndex(Belle2::Particle* particle) const;

    /** getter for the index of the momentum in the state vector */
    int momIndex(Belle2::Particle* particle) const;

    /** getter for the index of tau, the lifetime in the statevector*/
    int tauIndex(Belle2::Particle* particle) const;

    /** getter for chi2 of the newton iteration */
    double chiSquare() const { return m_chiSquare ; }

    /**  getter for the decay chains chi2 */
    double globalChiSquare() const;

    /** gettter for degrees of freedom of the fitparameters */
    int nDof() const;

    /** getter for the status of the newton iteration  */
    int status() const { return m_status; }

    /** getter for the current iteration number of the newton iteration  */
    int nIter() const { return m_niter; }

    /** getter for some errorcode flag  FIXME isnt this vovered by the statusflag?*/
    const ErrCode& errCode() { return m_errCode; }

    /** set mass cosntraint list */
    static void setMassConstraintList(std::vector<int> list) { massConstraintList = list; }

    /** get the decay chain FIXME unused */
    DecayChain* decaychain() { return m_decaychain; }

    /** get the entire statevector */
    FitParams* fitparams() { return m_fitparams; }

    /**  */
    const DecayChain* decaychain() const { return m_decaychain; }

    /** const getter for the statevector ???  */
    const FitParams* fitparams() const { return m_fitparams; }

    /**  getter for the head of the tree*/
    Belle2::Particle* particle() { return m_particle; }

    /** chi2 sum of all cosntraints  */
    double m_chi2sum;

    /** set the debug verbosity //JFK: FIXME remove debug stuff 2017-11-08   */
    static void setVerbose(int i) { vtxverbose = i; }


  private:
    /** head of the tree  */
    Belle2::Particle* m_particle;

    /**  the decay tree */
    DecayChain* m_decaychain;

    /**  the statevector */
    FitParams* m_fitparams;

    /** status of the current iteration (in the newton algorith) */
    int m_status;

    /** chi2 of the current newton iteration */
    double m_chiSquare;

    /** iteration index */
    int m_niter;

    /** precision that is needed for status:converged (delta chi2) */
    double m_prec;

    /** errorcode */
    ErrCode m_errCode;

  };
}
#endif
