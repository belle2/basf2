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

//using namespace CLHEP;
//using namespace Belle2;

namespace TreeFitter {
  class DecayChain ;
  class FitParams ;
  class ParticleBase ;

  extern int vtxverbose ;
  extern std::vector<int> massConstraintList ;

  class Fitter {
  public:
    enum VertexType { None = -1, Geometric, Kinematic };
    enum VertexStatus { Success = 0, NonConverged, BadInput, Failed, UnFitted };

    bool printDaughters(Belle2::Particle* mother);

    Fitter() : m_particle(0), m_decaychain(0), m_fitparams(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_niter(-1), m_prec(0.01) {} //Default constructor

    Fitter(Belle2::Particle* particle, double prec = 0.01) ;
    ~Fitter() ;
    bool fit() ; //FT: had to be changed from void to bool
    void print() const ;
    void printConstraints(std::ostream& os = std::cout) const ;
    const CLHEP::HepSymMatrix& cov() const ;
    const CLHEP::HepVector& par() const ;
    CLHEP::HepSymMatrix cov(const std::vector<int>& indexVec) const ;
    CLHEP::HepVector par(const std::vector<int>& indexVec) const ;
    //const DecayChain* decayChain() const { return _decaychain; }
    int posIndex(Belle2::Particle* particle) const ;
    int momIndex(Belle2::Particle* particle) const ;
    int tauIndex(Belle2::Particle* particle) const ;

    double chiSquare() const { return m_chiSquare ; }
    double globalChiSquare() const ;
    int    nDof()      const ;
    int status() const { return m_status ; }
    //    int vertexType() const {return m_vtype;}//FT: not used yet
    int nIter() const { return m_niter ; }
    const ErrCode& errCode() { return m_errCode ; }

    // must be moved to derived class or so ...
    double add(Belle2::Particle& cand) ;
    double remove(Belle2::Particle& cand) ;
    void updateIndex() ;
    void fitOneStep() ;

    // interface to basf2
    TVector2 decayLength(Belle2::Particle& particle) const ;
    TVector2 lifeTime(Belle2::Particle& particle) const ;
    TVector2 decayLengthSum(Belle2::Particle&, Belle2::Particle&) const ;

    Belle2::Particle getFitted() ;
    Belle2::Particle getFitted(Belle2::Particle& particle);
    Belle2::Particle getFittedTree();
    //    Particle* fittedCand(Particle& particle, Particle* headoftree);

    //    BtaFitParams btaFitParams(Particle& particle) const ;//this passes out the set of particle parameters in BaBar, do the same for basf2
    CLHEP::HepSymMatrix extractCov7(const ParticleBase* pb) const;

    bool updateCand(Belle2::Particle& particle) const;
    void updateCand(const ParticleBase& pb, Belle2::Particle& cand) const ;
    void updateTree(Belle2::Particle& particle) const;

    static void setVerbose(int i) { vtxverbose = i ; }
    static void setMassConstraintList(std::vector<int> list) { massConstraintList = list; }

  public:
    //    BtaFitParams btaFitParams(const ParticleBase* pb) const ;//this passes out the set of particle parameters in BaBar, do the same for basf2
    TVector2 decayLength(const ParticleBase* pb) const ;
    TVector2 decayLengthSum(const ParticleBase*, const ParticleBase*) const ;

    DecayChain* decaychain() { return m_decaychain ; }
    FitParams* fitparams() { return m_fitparams ; }
    const DecayChain* decaychain() const { return m_decaychain ; }
    const FitParams* fitparams() const { return m_fitparams ; }
    Belle2::Particle* particle() { return m_particle; }
    static TVector2 decayLength(const ParticleBase* pb, const FitParams*) ;
  private:
    Belle2::Particle* m_particle ;
    DecayChain* m_decaychain ;
    FitParams* m_fitparams ;
    int m_status ;
    //    int m_vtype; //FT: not used yet
    double m_chiSquare ;
    int m_niter ;
    double m_prec ;
    ErrCode m_errCode ;
  } ;

}

#endif //FITTER_H
