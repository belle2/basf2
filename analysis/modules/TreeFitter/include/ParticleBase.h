/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEBASE_H
#define PARTICLEBASE_H

//#include <string>
//#include <vector>
#include <analysis/modules/TreeFitter/Constraint.h>
#include <analysis/modules/TreeFitter/Projection.h>
//#include "DecayTreeFitter/ChiSquare.h"
#include <analysis/modules/TreeFitter/ErrCode.h>

// Magnetic field
//#include <framework/geometry/BFieldManager.h>

#include <analysis/dataobjects/Particle.h>
using namespace Belle2;

namespace TreeFitter {

  class FitParams;

  class ParticleBase {
  public:
    enum TFParticleType {kInteractionPoint,
                         kRecoComposite, kRecoResonance,
                         kInternalParticle, kRecoTrack,
                         kResonance, kRecoPhoton,
                         kMissingParticle
                        };

    typedef std::vector<ParticleBase*> ParticleContainer; //FT:fromLHC

    // Default constructor
    ParticleBase(Particle* particle, const ParticleBase* mother) ;

    // constructor used for InteractionPoint
    //    ParticleBase(const std::string& name);

    virtual ~ParticleBase();

    static ParticleBase* createParticle(Particle* particle,
                                        const ParticleBase* mother,
                                        bool forceFitAll = false) ;

    virtual int dim() const = 0 ;
    virtual void updateIndex(int& offset);
    virtual ErrCode initPar1(FitParams*) = 0 ; // init everything that does not need mother vtx
    virtual ErrCode initPar2(FitParams*) = 0 ; // everything else
    virtual ErrCode initCov(FitParams*) const  ;
    virtual std::string parname(int index) const ;
    virtual void print(const FitParams*) const ;

    const ParticleBase* locate(Particle* particle) const ;
    //    void locate(const LHCb::ParticleID& pid, ParticleContainer& result ) ;

    Particle* particle() const { return m_particle ; }
    const int index() const { return m_index ; }
    const ParticleBase* mother() const { return m_mother ; }
    const std::string& name() const { return m_name ; }

    virtual ErrCode projectGeoConstraint(const FitParams&, Projection&) const ;
    virtual ErrCode projectMassConstraint(const FitParams&, Projection&) const ;
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;
    virtual void forceP4Sum(FitParams&) const {} ; // force p4 conservation all along tree

    // indices to fit parameters
    virtual int type() const = 0 ;
    virtual int posIndex() const { return -1 ; }
    virtual int tauIndex() const { return -1 ; }
    virtual int momIndex() const { return -1 ; }

    // does the particle have a 3-momentum or a 4-momentum ?
    virtual bool hasEnergy() const { return false ; }

    // does the particle have is own decay vertex ? (resonances and
    // recoparticles do not)
    virtual bool hasPosition() const { return false ; }

    int eneIndex() const { return hasEnergy() ? momIndex() + 3 : -1 ; }

    // calculates the global chisquare (pretty useless)
    virtual double chiSquare(const FitParams*) const;

    // access to particle PDG parameters
    double pdgMass() const { return m_pdgMass ; }
    double pdgWidth() const { return m_pdgWidth ; }
    double pdgLifeTime() const { return m_pdgLifeTime ; }
    double pdgTau() const { return m_pdgMass > 0 ? m_pdgLifeTime / m_pdgMass : 0 ; }
    int charge() const { return m_charge ; }

    //vertex accessors (FT) --moved to Fitter.h
    //    int getVertexType() const { return m_vertextype ; }
    //    int getVertexStatus() const { return m_vertexstatus ; }
    //    void setVertexType(const VertexType type) {m_vertextype = type;}
    //    void setVertexStatus(const VertexStatus stat) {m_vertexstatus = stat;}


    // return a trajectory
    //    virtual const LHCb::Trajectory* trajectory() const { return 0 ; }

    // access to daughters
    typedef std::vector<ParticleBase*> daucontainer ; //FT:fromLHC
    typedef daucontainer::const_iterator const_iterator ; //FT:fromLHC

    daucontainer& daughters() { return m_daughters ; }//FT:fromLHC //this was in Protected
    const daucontainer& daughters() const { return m_daughters ; }//FT:fromLHC

    virtual const_iterator begin() const {  return m_daughters.begin() ; }//FT:fromLHC
    virtual const_iterator end()   const {  return m_daughters.end() ; }//FT:fromLHC

    virtual ParticleBase* addDaughter(Particle*, bool forceFitAll = false); //FT:fromLHC
    virtual void removeDaughter(const ParticleBase* pb); //FT:fromLHC

    typedef std::vector< std::pair<const ParticleBase*, int> > indexmap ;
    virtual void retrieveIndexMap(indexmap& anindexmap) const ;
    void setMother(const ParticleBase* m) { m_mother = m ; }

    typedef std::vector<Constraint> constraintlist ;
    virtual void addToConstraintList(constraintlist& alist, int depth) const = 0 ;
    // collect all particles emitted from vertex with position posindex
    void collectVertexDaughters(daucontainer& particles, int posindex) ;
    virtual int nFinalChargedCandidates() const;
    void setParticle(Particle* particle) { m_particle = particle ; }

    // collect all particles emitted from vertex with position posindex
    //    void collectVertexDaughters( daucontainer& particles, int posindex ) ;
    // set the mass constraint for this particle. return true if value changed
    //    bool setMassConstraint(bool add) {
    //      std::swap(add,m_hasMassConstraint) ;
    //      return add != m_hasMassConstraint ;
    //    }
    // set the mass of the mass constraint (use with care!)
    //    void setMassConstraint( double mass ) {
    //      m_hasMassConstraint = true ;
    //      m_pdgMass = mass ;
    //    }

    //    ChiSquare chiSquare( const FitParams& params ) const ;

    //    bool hasMassConstraint() const { return m_hasMassConstraint ; }
  protected:
    static double pdgLifeTime(Particle* particle)  ;
    static bool isAResonance(Particle* particle) ;
    static double bFieldOverC(); // Bz/c
    ErrCode initTau(FitParams* par) const ;
    //
    typedef daucontainer::iterator iterator ;
    iterator begin() { return m_daughters.begin() ; }
    iterator end()   { return m_daughters.end() ; }
  protected:
    void setIndex(int i) { m_index = i ; }
  private:
    Particle* m_particle;
    const ParticleBase* m_mother ;
    ParticleContainer m_daughters ;//FT:fromLHC
    //    const LHCb::ParticleProperty* m_prop ;
    int m_index ;
    double m_pdgMass ;      // cached mass
    double m_pdgWidth ;     // particle width (for mass constraints)
    double m_pdgLifeTime ; // cached lifetime (in cm)
    int m_charge ;      // charge
    std::string m_name ;
    //    bool m_hasMassConstraint ;

  }; // end class ParticleBase
} // end namespace TreeFitter

#endif //PARTICLEBASE_H

