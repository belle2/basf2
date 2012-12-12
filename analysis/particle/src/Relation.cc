#include "analysis/particle/Relation.h"

using namespace Belle1;

static Particle*     static_Particle(NULL);
static Belle2::Track*        static_Mdst_charged(NULL);
static Belle2::ECLShower*    static_ECL_shower(NULL);
static Belle2::ECLGamma*     static_ECL_gamma(NULL);
static Belle2::ECLPi0*       static_ECL_pi0(NULL);

//Default constructor
Relation::Relation()
  : m_flagChildModification(0),
    m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_mother     = NULL;
  m_mc         = NULL;
  m_charged    = NULL;
  m_ecl        = NULL;
  m_gamma      = NULL;
  m_pi0        = NULL;
  m_mcParticle = NULL;
}

Relation::Relation(Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self    = s;
  m_flagChildModification = 0;

  m_mother     = NULL;
  m_mc         = NULL;
  m_charged    = NULL;
  m_ecl        = NULL;
  m_gamma      = NULL;
  m_pi0        = NULL;
  m_mcParticle = NULL;
}

//copy constructor
Relation::Relation(const Relation& a, Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self     = s;
  m_mother   = a.m_mother;
  m_mc       = a.m_mc;
  m_children = a.children();
  //...m_finalStateParticles
  if (m_children.size() == 0) {
    m_flagChildModification = 0;
  } else {
    m_flagChildModification = 1;
  }
  m_charged    = a.m_charged;
  m_ecl        = a.m_ecl;
  m_gamma      = a.m_gamma;
  m_pi0        = a.m_pi0;
  m_mcParticle = a.m_mcParticle;

#if 0
  if (m_pi0)
    for (unsigned int i = 0; i < nChildren(); ++i)
      if (child(i).relation().m_pi0ChildCounter >= 1)
        ++(child(i).relation().m_pi0ChildCounter);
#endif
}

//Constructor with Mdst\_charged
Relation::Relation(const Belle2::Track& a, Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self    = s;
  m_charged = &a;
  m_flagChildModification = 0;

  m_mother     = NULL;
  m_mc         = NULL;
  m_ecl        = NULL;
  m_gamma      = NULL;
  m_pi0        = NULL;
  m_mcParticle = NULL;
}

//Constructor with Mdst\_gamma
Relation::Relation(const Belle2::ECLShower& a, Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self = s;
  m_ecl  = &a;
  m_flagChildModification = 0;

  m_mother     = NULL;
  m_mc         = NULL;
  m_charged    = NULL;
  m_gamma      = NULL;
  m_pi0        = NULL;
  m_mcParticle = NULL;
}

//Constructor with Mdst\_gamma
Relation::Relation(const Belle2::ECLGamma& a, Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self  = s;
  m_gamma = &a;
  m_flagChildModification = 0;

  m_mother     = NULL;
  m_mc         = NULL;
  m_charged    = NULL;
  m_ecl        = NULL;
  m_pi0        = NULL;
  m_mcParticle = NULL;
}

//Constructor with Mdst\_pi0
Relation::Relation(const Belle2::ECLPi0& a, const bool makeRelation, Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self = s;
  m_pi0  = &a;
  m_flagChildModification = 0;

  m_mother     = NULL;
  m_mc         = NULL;
  m_charged    = NULL;
  m_ecl        = NULL;
  m_gamma      = NULL;
  m_mcParticle = NULL;

#if 0
  if (makeRelation) {
    unsigned nGamma = 0;
    Mdst_gamma_Manager& gamma_mag = Mdst_gamma_Manager::get_manager();
    for (std::vector<Mdst_gamma>::iterator i = gamma_mag.begin();
         i != gamma_mag.end(); ++i) {
      if (a.gamma(0).get_ID() >= 1 &&
          a.gamma(0).get_ID() == i->get_ID()) {
        Particle* tmp = new Particle(gamma_mag[(int)(a.gamma(0).get_ID()) - 1]);
        tmp->relation().m_pi0ChildCounter = 1;
        append(*tmp);
        ++nGamma;
      }
      if (a.gamma(1).get_ID() >= 1 &&
          a.gamma(1).get_ID() == i->get_ID()) {
        Particle* tmp = new Particle(gamma_mag[(int)(a.gamma(1).get_ID()) - 1]);
        tmp->relation().m_pi0ChildCounter = 1;
        append(*tmp);
        ++nGamma;
      }
      if (nGamma == 2)break;
    }
  }
  // debug
  dout(Debugout::INFO, "Relation") << "create Mdst_pi0 : " << m_n_static_new_pi0 << std::endl;
  m_n_static_new_pi0++;
#endif
}

/**
 * Construct Relation from a Belle2::MCParticle
 */
Relation::Relation(const Belle2::MCParticle* a, Particle* s)
  : m_vee2ChildCounter(0), m_pi0ChildCounter(0)
{
  m_self        = s;
  m_mcParticle  = a;
  m_flagChildModification = 0;

  m_mother     = NULL;
  m_mc         = NULL;
  m_charged    = NULL;
  m_ecl        = NULL;
  m_gamma      = NULL;
  m_pi0        = NULL;
}


// Destructor
Relation::~Relation()
{
#if 0 // Tagir
  if (m_pi0)
    for (unsigned int i = 0; i < nChildren(); ++i)
      if (child(i).relation().m_pi0ChildCounter >= 2)
        --(child(i).relation().m_pi0ChildCounter);
      else if (child(i).relation().m_pi0ChildCounter == 1)
        delete &(child(i));
#endif
}

// Interfaces for particles.
// returns a reference to mother.
const Particle&
Relation::mother(void) const
{
  if (m_mother) return *m_mother;
  else if (static_Particle) return *static_Particle;
  // Tagir else return *(static_Particle=new Particle);
}

Particle&
Relation::mother(void)
{
  if (m_mother) return *m_mother;
  else if (static_Particle) return *static_Particle;
  // Tagir   else return *(static_Particle=new Particle);
}

// appends a child.
void
Relation::append(Particle& a)
{
  Particle* tmp = &a;
  m_children.push_back(tmp);
  m_flagChildModification = 1;
}

// removes a child.
void
Relation::remove(Particle& a)
{
  for (std::vector<Particle*>::iterator i = m_children.begin(); i != m_children.end(); ++i)
    if (&a == *i) m_children.erase(i);
  m_flagChildModification = 1;
}

// returns a reference to MC particle.
const Particle&
Relation::mc(void) const
{
  if (m_mc) return *m_mc;
  else if (static_Particle) return *static_Particle;
  // Tagir   else return *(static_Particle=new Particle);
}

Particle&
Relation::mc(void)
{
  if (m_mc) return *m_mc;
  else if (static_Particle) return *static_Particle;
  // Tagir   else return *(static_Particle=new Particle);
}

// Interfaces for MDST banks.
// returns a reference to Mdst\_charged.
const Belle2::Track&
Relation::mdstCharged(void) const
{
  if (m_charged) return *m_charged;
  else if (static_Mdst_charged) return *static_Mdst_charged;
  // Tagir   else return *(static_Mdst_charged=new Belle2::Track);
}

// returns a reference to Mdst\_gamma.
const Belle2::ECLShower&
Relation::mdstEcl(void) const
{
  if (m_ecl) return *m_ecl;
  else if (static_ECL_shower) return *static_ECL_shower;
  // Tagir   else return *(static_Mdst_gamma=new Mdst_gamma);
}

// returns a reference to Mdst\_gamma.
const Belle2::ECLGamma&
Relation::mdstGamma(void) const
{
  if (m_gamma) return *m_gamma;
  else if (static_ECL_gamma) return *static_ECL_gamma;
  // Tagir   else return *(static_Mdst_gamma=new Mdst_gamma);
}

// returns a reference to Mdst\_pi0.
const Belle2::ECLPi0&
Relation::mdstPi0(void) const
{
  if (m_pi0) return *m_pi0;
  else if (static_ECL_pi0) return *static_ECL_pi0;
  // Tagir else return *(static_Mdst_pi0=new Mdst_pi0);
}

bool
Relation::isIdenticalWith(const Relation& x, const unsigned& type) const
{
  switch (type) {
    case PC_ALL:
      if (m_charged    && x.m_charged)        return (m_charged     == (x.m_charged));
      if (m_mcParticle && x.m_mcParticle)     return (m_mcParticle  == (x.m_mcParticle));
      if (m_ecl        && x.m_ecl)            return (m_ecl         == (x.m_ecl));
      if (m_gamma      && x.m_gamma)          return (m_gamma       == (x.m_gamma));
      if (m_pi0        && x.m_pi0)            return (m_pi0         == (x.m_pi0));
      return false;
    case PC_CHARGED:
      if (m_charged   && x.m_charged)    return (m_charged    == (x.m_charged));
      return false;
    case PC_ECL:
      if (m_ecl       && x.m_ecl)        return (m_ecl        == (x.m_ecl));
      return false;
    case PC_GAMMA:
      if (m_gamma     && x.m_gamma)      return (m_gamma      == (x.m_gamma));
      return false;
    case PC_PI0:
      if (m_pi0       && x.m_pi0)        return (m_pi0        == (x.m_pi0));
      return false;
    case PC_MCPARTICLE:
      if (m_mcParticle && x.m_mcParticle) return (m_mcParticle == (x.m_mcParticle));
      return false;
    default:
      return false;
  }
}

//Operators
// copy operator
Relation&
Relation::operator = (const Relation& a)
{
  if (this == &a) return *this;

  m_self       = a.m_self;
  m_mother     = a.m_mother;
  m_mc         = a.m_mc;
  m_children   = a.children();
  //...m_finalStateParticles
  if (m_children.size() == 0) {
    m_flagChildModification = 0;
  } else {
    m_flagChildModification = 1;
  }
  m_charged    = a.m_charged;
  m_ecl        = a.m_ecl;
  m_gamma      = a.m_gamma;
  m_pi0        = a.m_pi0;
  m_mcParticle = a.m_mcParticle;
  return *this;
}
