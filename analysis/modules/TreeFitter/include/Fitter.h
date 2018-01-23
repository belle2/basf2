/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FITTER_H
#define FITTER_H

#include <analysis/dataobjects/Particle.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Vector.h>

#include <vector>
#include <analysis/modules/TreeFitter/ErrCode.h>
#include <analysis/modules/TreeFitter/EigenTypes.h>

//using namespace CLHEP;
//using namespace Belle2;

namespace TreeFitter {
  class DecayChain;
  class FitParams;
  class ParticleBase;

  /**FIXME remove??  */
  extern int vtxverbose;

  /**FIXME what todo with this?  */
  extern std::vector<int> massConstraintList;

  class Fitter {
  public:

    /** FIXME unused  remove?*/
    enum VertexType { None = -1, Geometric, Kinematic };

    /** status flag of the fit-itereation (the step in the newton method) */
    enum VertexStatus { Success = 0, NonConverged, BadInput, Failed, UnFitted };


    /** empty constructor  */
    Fitter() : m_particle(0), m_decaychain(0), m_fitparams(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_niter(-1), m_prec(0.01) {} //Default constructor

    /** constructor  */
    Fitter(Belle2::Particle* particle, double prec = 0.01, int ipDimension = 0);

    /** destructor */
    ~Fitter();

    /** main fit function that uses the kalman filter */
    bool fitUseEigen();
    /** update particles parameters with the fit results */
    bool updateCandCopy(Belle2::Particle& particle) const;
    /** locate particle base for a belle2 particle and update the particle with the values from particle base */
    void updateCandCopy(const ParticleBase& pb, Belle2::Particle& cand) const;
    /** update the Belle2::Particles with the fit results  */
    void updateTreeCopy(Belle2::Particle& particle) const;

    /** get covariance matrix */
    const EigenTypes::MatrixXd& getCovariance() const;
    /** get parameters */
    //const EigenTypes::ColVector& par() const;
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



    /** covariance matrix of the state vector FIXME describe ordering */
    const CLHEP::HepSymMatrix& cov() const;

    /** the state vector, contains everything FIXME descripe ordering  */
    const CLHEP::HepVector& par() const;

    /** sub block of the covariance matrix of the statevector  */
    CLHEP::HepSymMatrix cov(const std::vector<int>& indexVec) const;

    /** sub vector of the statevector  */
    CLHEP::HepVector par(const std::vector<int>& indexVec) const;

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

    /** FIXME unused */
    void updateIndex();

    /** get decy length and error  */
    TVector2 decayLength(Belle2::Particle& particle) const;

    /** get life time and error  */
    TVector2 lifeTime(Belle2::Particle& particle) const;

    /** FIXME unused */
    TVector2 decayLengthSum(Belle2::Particle&, Belle2::Particle&) const;

    /** FIXME unused  */
    Belle2::Particle getFitted();

    /** FIXME unused */
    Belle2::Particle getFitted(Belle2::Particle& particle);

    /** FIXME unused */
    Belle2::Particle getFittedTree();

    /** get covariance of this particle  */
    CLHEP::HepSymMatrix extractCov7(const ParticleBase* pb) const;

    /** update particles parameters with the fit results */
    bool updateCand(Belle2::Particle& particle) const;

    /** locate particle base for a belle2 particle and update the particle with the values from particle base */
    void updateCand(const ParticleBase& pb, Belle2::Particle& cand) const;

    /** update the Belle2::Particles with the fit results  */
    void updateTree(Belle2::Particle& particle) const;

    /** FIXME delte?   */
    static void setVerbose(int i) { vtxverbose = i; }

    /** FIXME delte? */
    static void setMassConstraintList(std::vector<int> list) { massConstraintList = list; }

    /**  get decaylength and error for the particle base*/
    TVector2 decayLength(const ParticleBase* pb) const;

    /**  decaylength sum FIXME unused */
    TVector2 decayLengthSum(const ParticleBase*, const ParticleBase*) const;

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

    /** getter for decaylength from particle base  */
    static TVector2 decayLength(const ParticleBase* pb, const FitParams*);

    /** */
    double m_chi2sum;

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

#endif //FITTER_H
