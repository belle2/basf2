#ifndef PARTICLE_CLASS_RELATION_H
#define PARTICLE_CLASS_RELATION_H

#include "analysis/particle/constant.h"

#include <iostream>
#include <string>
#include <vector>

namespace Belle2 {
  class Particle;
  class Track;
  class MdstGamma;
  class MdstPi0;
  class RecCRECL;
  class MCParticle;
}

/// Relation class supplies you interfaces to other objects, such as mother particle, MC particle, and MDST banks.
namespace Belle2 {
  class Relation {

  public:
    /// Default constructor
    Relation();
    /// Copy constructor
    Relation(const Relation&, Particle * = NULL);
    /// Tagir
    Relation(Particle * = NULL);
    /// Constructor with Mdst\_charged
    Relation(const Track&, Particle * = NULL);
    /// Constructor with Mdst\_gamma
    Relation(const MdstGamma&, Particle * = NULL);
    /// Constructor with Mdst\_pi0
    Relation(const MdstPi0&, const bool makeRelation = true, Particle * = NULL);
    /// Constructor with Mdst\_ecl
    Relation(const RecCRECL&, Particle * = NULL);

    /**
     * Construct Relation from a MCParticle
     */
    Relation(const MCParticle*, Particle * = NULL);

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
    virtual const Track& mdstCharged(void) const;

    /// sets a reference to Mdst\_charged and returns it.
    virtual const Track& mdstCharged(const Track& a) { return *(m_charged = &a);}

    /// returns a reference to Mdst\_gamma.
    virtual const MdstGamma& mdstGamma(void) const;

    /// sets a reference to Mdst\_gamma and returns it.
    virtual const MdstGamma& mdstGamma(const MdstGamma& a) { return *(m_gamma = &a);}

    /// returns a reference to Mdst\_pi0.
    virtual const MdstPi0& mdstPi0(void) const;

    /// sets a reference to Mdst\_pi0 and returns it.
    virtual const MdstPi0& mdstPi0(const MdstPi0& a) { return *(m_pi0 = &a); }

    /// returns a reference to Mdst\_ecl.
    virtual const RecCRECL& mdstEcl(void) const;

    /// sets a reference to Mdst\_ecl and returns it.
    virtual const RecCRECL& mdstEcl(const RecCRECL& a) { return *(m_ecl = &a); }

    /**
     * Returns a pointer to linked generated MCParticle. NULL if the particle doesn't
     * have a MCParticle linked.
     * @return A pointer to the linked MCParticle. NULL if the particle doesn't
     * have a MCParticle linked.
     */
    virtual const MCParticle* getMCParticle(void) const { return m_mcParticle; }

    /**
     * Sets a pointer to linked generated MCParticle and returns it.
     * @param  mcLink A pointer to the linked MCParticle.
     * @return A pointer to the linked MCParticle.
     */
    virtual const MCParticle* setMCParticle(const MCParticle* mcLink) { return m_mcParticle = mcLink; }

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
    const Track*      m_charged;
    const MdstGamma*  m_gamma;
    const MdstPi0*    m_pi0;
    const RecCRECL*   m_ecl;
    const MCParticle* m_mcParticle; /**< Pointer to the linked MCParticle */

    mutable unsigned int m_flagChildModification;

    int m_vee2ChildCounter;
    int m_pi0ChildCounter;
  };
}
#endif /* PARTICLE_CLASS_RELATION_H */
