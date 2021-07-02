/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/Constraint.h>
#include <analysis/VertexFitting/TreeFitter/Projection.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>
#include <analysis/VertexFitting/TreeFitter/ConstraintConfiguration.h>
#include <Eigen/Core>

#include <analysis/dataobjects/Particle.h>

namespace TreeFitter {

  class FitParams;
  class ConstraintConfiguration;

  /** base class for all particles */
  class ParticleBase {

  public:

    /** particle types  */
    enum TFParticleType {kInteractionPoint,
                         kOrigin,
                         kRecoComposite,
                         kRecoResonance,
                         kInternalParticle,
                         kRecoTrack,
                         kResonance,
                         kRecoPhoton,
                         kRecoKlong,
                         kMissingParticle
                        };

    /** default constructor  */
    ParticleBase(Belle2::Particle* particle, const ParticleBase* mother, const ConstraintConfiguration* config) ;

    /** constructor used for final states */
    ParticleBase(Belle2::Particle* particle, const ParticleBase* mother) ;


    /** constructor only used by inter action point (ip constraint)  */
    ParticleBase(const std::string& name);

    /** destructor, actually does something  */
    virtual ~ParticleBase();

    /**  alias */
    typedef std::vector<Constraint> constraintlist;

    /**  alias */
    typedef std::vector< std::pair<const ParticleBase*, int> > indexmap;

    /** create the according treeFitter particle obj for a basf2 particle type  */
    static ParticleBase* createParticle(Belle2::Particle* particle,
                                        const ParticleBase* mother,
                                        const ConstraintConfiguration& config,
                                        bool forceFitAll = false
                                       );

    /** create a custom origin particle or a beamspot*/
    static ParticleBase* createOrigin(Belle2::Particle* daughter,
                                      const ConstraintConfiguration& config,
                                      bool forceFitAll
                                     );

    /** init particle that does not need a mother vertex  */
    virtual ErrCode initMotherlessParticle(FitParams&) = 0;

    /** init particle that does need a mother vertex  */
    virtual ErrCode initParticleWithMother(FitParams&) = 0;

    /** init covariance matrix */
    virtual ErrCode initCovariance(FitParams&) const;

    /**  get dimension of constraint */
    virtual int dim() const = 0 ;

    /** this sets the index for momentum, position, etc. in the statevector  */
    virtual void updateIndex(int& offset);

    /**  get name of parameter i */
    virtual std::string parname(int index) const ;

    /**  get particle base from basf2 particle */
    const ParticleBase* locate(Belle2::Particle* particle) const ;

    /**  get basf2 particle  */
    Belle2::Particle* particle() const { return m_particle ; }

    /** get index  */
    int index() const { return m_index ; }

    /** getMother() / hasMother() */
    const ParticleBase* mother() const;

    /**  get name of the particle */
    const std::string& name() const { return m_name ; }

    /**  project geometrical constraint */
    virtual ErrCode projectGeoConstraint(const FitParams&, Projection&) const ;

    /** project mass constraint using the particles parameters */
    virtual ErrCode projectMassConstraintParticle(const FitParams&, Projection&) const ;

    /** project mass constraint using the parameters of the daughters */
    virtual ErrCode projectMassConstraintDaughters(const FitParams&, Projection&) const ;

    /** project mass constraint abstract */
    virtual ErrCode projectMassConstraint(const FitParams&, Projection&) const ;

    /** project constraint.   */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

    /**  force p4 sum conservation all allong the tree */
    virtual void forceP4Sum(FitParams&) const {} ;

    /**  get particle type */
    virtual int type() const = 0 ;

    /**  get vertex index (in statevector!) */
    virtual int posIndex() const { return -1 ; }

    /** get tau index */
    virtual int tauIndex() const { return -1 ; }

    /**  get momentum index */
    virtual int momIndex() const { return -1 ; }

    // does the particle have a 3-momentum or a 4-momentum ?
    /** get momentum dimension */
    virtual bool hasEnergy() const { return false ; }

    /** get false  */
    virtual bool hasPosition() const { return false ; }

    /** get energy index  */
    int eneIndex() const { return hasEnergy() ? momIndex() + 3 : -1 ; }

    /**  get chi2 */
    virtual double chiSquare(const FitParams&) const;

    /** get pdg mass  */
    double pdgMass() const { return m_pdgMass ; }

    /** get pdg width */
    double pdgWidth() const { return m_pdgWidth ; }

    /**  get pdg lifetime */
    double pdgLifeTime() const { return m_pdgLifeTime ; }

    /** get Tau */
    double pdgTime() const { return m_pdgMass > 0 ? m_pdgLifeTime : 0; }

    /**  get charge */
    int charge() const { return m_charge ; }

    /** add daughter  */
    virtual ParticleBase* addDaughter(Belle2::Particle*, const ConstraintConfiguration& config, bool forceFitAll = false);

    /** remove daughter */
    virtual void removeDaughter(const ParticleBase* pb);

    /** get index map  */
    virtual void retrieveIndexMap(indexmap& anindexmap) const ;

    /** set mother  */
    void setMother(const ParticleBase* m) { m_mother = m ; }

    /** add to constraint list  */
    virtual void addToConstraintList(constraintlist& alist, int depth) const = 0 ;

    /** get vertex daughters */
    void collectVertexDaughters(std::vector<ParticleBase*>& particles, int posindex) ;

    /** number of charged candidates */
    virtual int nFinalChargedCandidates() const;

    /** set the relation to basf2 particle type */
    void setParticle(Belle2::Particle* particle) { m_particle = particle ; }

  protected:

    /** just an alias */
    typedef std::vector<ParticleBase*> ParticleContainer;

    /** get pdg lifetime */
    static double pdgLifeTime(Belle2::Particle* particle)  ;

    /** controls if a particle is treated as a resonance(lifetime=0) or a particle that has a finite lifetime.
     * A finite life time means it will register a geo constraint for this particle
     * */
    static bool isAResonance(Belle2::Particle* particle) ;

    /** Bz/c  */
    static double bFieldOverC();

    /** initialises tau as a length  */
    ErrCode initTau(FitParams& par) const ;

    /** set Index (in statevector) */
    void setIndex(int i) { m_index = i ; }

    /** pointer to framework type  */
    Belle2::Particle* m_particle;

    /** motherparticle */
    const ParticleBase* m_mother;

    /** daughter container  */
    std::vector<ParticleBase*> m_daughters;

    /** decay length less than 1 micron  */
    bool m_isStronglyDecayingResonance;

    /** has all the constraint config */
    const ConstraintConfiguration* m_config;

  private:
    /** index */
    int m_index;

    /** pdg mass  */
    const double m_pdgMass;

    /**  particle width */
    double m_pdgWidth;

    /** lifetime in cm  */
    const double m_pdgLifeTime;

    /** charge  */
    int m_charge;

    /** name  */
    std::string m_name;

  }; // end class ParticleBase
} // end namespace TreeFitter
