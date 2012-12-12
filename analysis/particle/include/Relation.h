#ifndef PARTICLE_CLASS_RELATION_H
#define PARTICLE_CLASS_RELATION_H

#include "analysis/particle/constant.h"

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <tracking/dataobjects/Track.h>
#include <generators/dataobjects/MCParticle.h>

#include <iostream>
#include <string>
#include <vector>

namespace Belle1 {
  class Particle;
}

/// Relation class supplies you interfaces to other objects, such as mother particle, MC particle, and MDST banks.
namespace Belle1 {
  class Relation {

  public:
    /// Default constructor
    Relation();
    /// Copy constructor
    Relation(const Relation&, Particle * = NULL);
    /// Tagir
    Relation(Particle * = NULL);
    /// Constructor with Mdst\_charged
    Relation(const Belle2::Track&, Particle * = NULL);
    /// Constructor with Mdst\_gamma
    Relation(const Belle2::ECLShower&, Particle * = NULL);
    /// Constructor with Mdst\_gamma
    Relation(const Belle2::ECLGamma&, Particle * = NULL);
    /// Constructor with Mdst\_pi0
    Relation(const Belle2::ECLPi0&, const bool makeRelation = true, Particle * = NULL);

    /**
     * Construct Relation from a MCParticle
     */
    Relation(const Belle2::MCParticle*, Particle * = NULL);

    /// Destructor
    virtual ~Relation();

  public:// General interfaces
    /// returns class name.
    virtual std::string className(void) { return std::string("Relation"); }

    /// dumps debug information. (not implement.)
    virtual void dump(const std::string& keyword = std::string(""),
                      const std::string& prefix  = std::string("")) const {};

  public:// Interfaces for particles.
    /// returns a const reference to mother.
    virtual const Particle& mother(void) const;

    /// returns a reference to mother.
    virtual Particle& mother(void);

    /// sets a reference to mother and returns it.
    virtual const Particle& mother(Particle& a) { return *(m_mother = &a); }

    /// returns a number of children.
    virtual unsigned int nChildren(void) const { return m_children.size(); }

    /// returns a const reference to i'th child.
    virtual const Particle& child(unsigned int i) const { return *(m_children[i]); }

    /// returns a reference to i'th child.
    virtual Particle& child(unsigned int i) { return *(m_children[i]); }

    /// returns a list of children.
    virtual const std::vector<Particle*> & children(void) const { return m_children; }

    /// returns a const reference to MC particle.
    virtual const Particle& mc(void) const;

    /// returns a reference to MC particle.
    virtual Particle& mc(void);

    /// sets a reference to MC particle and returns it.
    virtual const Particle& mc(Particle& a) { return *(m_mc = &a); }

    /// append a child.
    virtual void append(Particle&);

    /// remove a child.
    virtual void remove(Particle&);

  public:// Interfaces for MDST banks.
    /// returns a reference to Mdst\_charged.
    virtual const Belle2::Track& mdstCharged(void) const;

    /// sets a reference to Mdst\_charged and returns it.
    virtual const Belle2::Track& mdstCharged(const Belle2::Track& a) { return *(m_charged = &a);}

    /// returns a reference to Mdst\_gamma.
    virtual const Belle2::ECLShower& mdstEcl(void) const;

    /// sets a reference to Mdst\_gamma and returns it.
    virtual const Belle2::ECLShower& mdstEcl(const Belle2::ECLShower& a) { return *(m_ecl = &a);}

    /// returns a reference to Mdst\_gamma.
    virtual const Belle2::ECLGamma& mdstGamma(void) const;

    /// sets a reference to Mdst\_gamma and returns it.
    virtual const Belle2::ECLGamma& mdstGamma(const Belle2::ECLGamma& a) { return *(m_gamma = &a);}

    /// returns a reference to Mdst\_pi0.
    virtual const Belle2::ECLPi0& mdstPi0(void) const;

    /// sets a reference to Mdst\_pi0 and returns it.
    virtual const Belle2::ECLPi0& mdstPi0(const Belle2::ECLPi0& a) { return *(m_pi0 = &a); }

    /**
     * Returns a pointer to linked generated MCParticle. NULL if the particle doesn't
     * have a MCParticle linked.
     * @return A pointer to the linked MCParticle. NULL if the particle doesn't
     * have a MCParticle linked.
     */
    virtual const Belle2::MCParticle* getMCParticle(void) const { return m_mcParticle; }

    /**
     * Sets a pointer to linked generated MCParticle and returns it.
     * @param  mcLink A pointer to the linked MCParticle.
     * @return A pointer to the linked MCParticle.
     */
    virtual const Belle2::MCParticle* setMCParticle(const Belle2::MCParticle* mcLink) { return m_mcParticle = mcLink; }

    /**
     * Resets the link to MCParticle. The pointer to MCParticle is set to NULL.
     */
    virtual void resetMCParticle(void) { m_mcParticle = NULL; }

    /// identifies to particles.
    virtual bool isIdenticalWith(const Relation&, const unsigned& type = PC_ALL) const;

  public://operators
    /// copy operator.
    Relation& operator = (const Relation&);

  private:// Protected members
    Particle* m_self;
    Particle* m_mother;
    std::vector<Particle*> m_children;
    Particle* m_mc;
    const Belle2::Track*      m_charged;
    const Belle2::ECLShower*  m_ecl;
    const Belle2::ECLGamma*   m_gamma;
    const Belle2::ECLPi0*     m_pi0;
    const Belle2::MCParticle* m_mcParticle; /**< Pointer to the linked MCParticle */

    mutable unsigned int m_flagChildModification;

    int m_vee2ChildCounter;
    int m_pi0ChildCounter;
  };
}
#endif /* PARTICLE_CLASS_RELATION_H */
