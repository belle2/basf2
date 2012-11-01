#ifndef PARTICLE_CLASS_PARTICLE_H
#define PARTICLE_CLASS_PARTICLE_H

#include <string>

#include "analysis/particle/constant.h"
#include "analysis/particle/ParticleUserInfo.h"
#include "analysis/particle/Momentum.h"
#include "analysis/particle/Ptype.h"
#include "analysis/particle/Relation.h"

#include <generators/dataobjects/MCParticle.h>

using namespace CLHEP;

/**
 * A class to store the reconstructed Particle information.
 * Particle class supplies you interfaces to various particle
 * information, such as momentum, pid, etc. through its private
 * member objects, Momentum, PID, Ptype, and Relation.
 */

namespace Belle2 {

  class Particle {

  public:
    /**
     *  Default constructor
     */
    Particle();

    /// Copy constructor
    Particle(const Particle&);
    /// Constructor with momentum
    Particle(const HepLorentzVector&, const Ptype&);
    /// Constructors with Momentum
    Particle(const Momentum&, const Ptype&);
    /// Constructor with Mdst\_charged(by pType)
    Particle(const Track&, const Ptype&,
             const Hep3Vector & = Hep3Vector(0., 0., 0.));
    /// Constructor with Mdst\_gamma
    Particle(const ECLShower&);
    /// Constructor with Mdst\_pi0
    Particle(const ECLPi0&, const bool makeRelation = true);

    /**
     * Construct Particle from a MCParticle
     */
    Particle(const MCParticle*);

    /// Destructor
    virtual ~Particle();

  public:// General interfaces
    /// returns object name.
    virtual const std::string& name(void) const { return m_name; }

    /// sets object name and returns it.
    virtual const std::string& name(const std::string& a) { return m_name = a; }

    /// returns class name.
    virtual std::string className(void) { return std::string("Particle"); }

    /// dumps debug information. Keywords are 'mass', 'momentum', 'position', 'recursive', and 'full'. 'recursive' is to dump children also. 'full' is equivalant to specifying all keywords.
    virtual void dump(const std::string& keyword = std::string("mass momentum"),
                      const std::string& prefix = std::string("")) const;

    /// append daughter
    virtual Particle& append_daughter(Particle&);

  public:// Momentum interfaces
    /// returns a const reference to Momentum Class.
    virtual const Momentum& momentum(void) const { return *m_momentum; }

    /// returns a reference to Momentum Class.
    virtual Momentum& momentum(void) { return *m_momentum; }

    /// sets a reference to Momentum Class and returns it.
    virtual const Momentum& momentum(const Momentum& a) { return *m_momentum = a; }

    /// returns momentum.
    virtual const HepLorentzVector& p(void) const { return m_momentum->p(); }

    /// returns 3momentum. (not reference)
    virtual const Hep3Vector p3(void) const { return m_momentum->p().vect(); }

    /// returns position.
    virtual const Hep3Vector& x(void) const { return m_momentum->x(); }

    /// returns magnitude of momentum.
    virtual double ptot(void) const { return (m_momentum->p()).vect().mag(); }

    /// returns x component of momentum.
    virtual double px(void) const { return (m_momentum->p()).x(); }

    /// returns y component of momentum.
    virtual double py(void) const { return (m_momentum->p()).y(); }

    /// returns z component of momentum.
    virtual double pz(void) const { return (m_momentum->p()).z(); }

    /// returns energy.
    virtual double e(void) const { return (m_momentum->p()).t(); }

    /// returns invariant mass.
    virtual double mass(void) const { return m_momentum->mass(); }

  public:// Ptype interfaces
    /// returns a const reference to Ptype.
    virtual const Ptype& pType(void) const { return *m_pType; }

    /// returns a reference to Ptype.
    virtual Ptype& pType(void) { return *m_pType; }

    /// sets a reference to Ptype and returns it.
    virtual const Ptype& pType(const Ptype& a) { return *m_pType = a; }

    /// returns charge. (in units of e)
    virtual double charge(void) const { return m_pType->charge(); }

    /// returns LUND7 particle code.
    virtual int lund(void) const { return m_pType->lund(); }

  public:// Relation interfaces
    /// returns a const reference to Relation.
    virtual const Relation& relation(void) const { return *m_relation; }

    /// returns a reference to Relation.
    virtual Relation& relation(void) { return *m_relation; }

    /// sets a reference to Relation and returns it.
    virtual const Relation& relation(const Relation& a) { return *m_relation = a; }

    /// returns a const reference to mother.
    virtual const Particle& mother(void) const { return m_relation->mother(); }

    /// returns a number of children.
    virtual unsigned nChildren(void) const { return m_relation->nChildren(); }

    /// returns a const reference to i'th child.
    virtual const Particle& child(unsigned i) const { return m_relation->child(i); }

    /// returns a reference to i'th child.
    virtual Particle& child(unsigned i) { return m_relation->child(i); }

    /// returns a const reference to MC particle.
    virtual const Particle& mc(void) const { return m_relation->mc(); }

    /// returns a reference to MC particle.
    virtual Particle& mc(void)  { return m_relation->mc(); }

    /// returns a reference to Mdst\_charged.
    virtual const Track& mdstCharged(void) const { return m_relation->mdstCharged(); }

    /// returns a reference to Mdst\_gamma.
    virtual const ECLShower& mdstGamma(void) const { return m_relation->mdstGamma(); }

    /// returns a reference to Mdst\_pi0.
    virtual const ECLPi0& mdstPi0(void) const { return m_relation->mdstPi0(); }

    /**
     * Returns a pointer to linked generated MCParticle. NULL if the particle doesn't
     * have a MCParticle linked.
     * @return A pointer to the linked MCParticle. NULL if the particle doesn't
     * have a MCParticle linked.
     */
    virtual const MCParticle* getMCParticle(void) const { return m_relation->getMCParticle(); }

  public:// User Definition Object
    /// returns a pointer of "user definition object".
    virtual const ParticleUserInfo& userInfo(void) const;

    /// returns a pointer of "user definition object".
    virtual ParticleUserInfo& userInfo(void);

    /// sets and returns a pointer of "user definition object".
    virtual const ParticleUserInfo& userInfo(const ParticleUserInfo&);

  public:// Operators
    /// copy operator
    Particle& operator = (const Particle&);

    /// bool operator : returns "usable" info.
    operator bool() const { return m_usable; }

    /// ! operator : returns "!usable" info.
    bool operator !() const { return !m_usable; }

    /// returns "usable" info. (please use usable() not this func.)
    virtual bool isUsable(void) { return usable(); }

    /// returns "usable" info.
    virtual bool usable(void) { return m_usable; }

    /// sets "usable" info and returns it.
    virtual bool usable(const bool&);

    // sets "not usable" info and returns it.
    virtual bool unusable(bool = UNUSABLE);

    /// copies(all private member is created by "new") and returns it.
    //  virtual Particle deepCopy(void);

    /// deletes children's objects made by deepCopy().
    //  virtual void deepDelete(void);

  private:
    bool m_usable;

    std::string    m_name;
    Momentum* m_momentum;
    Relation* m_relation;
    Ptype*    m_pType;
    ParticleUserInfo* m_userInfo;
  };
}
#endif /* PARTICLE_CLASS_PARTICLE_H */
