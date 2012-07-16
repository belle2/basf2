#include "analysis/particle/Particle.h"

using namespace Belle2;

static ParticleUserInfo* static_ParticleUserInfo(NULL);

// constructors
Particle::Particle()
  : m_usable(UNUSABLE), m_userInfo(NULL)
{
  m_momentum = new Momentum();
  m_relation = new Relation(this);
  m_pType    = new Ptype();
}

Particle::Particle(const Particle& a)
  : m_userInfo(NULL)
{
  m_momentum = new Momentum(a.momentum());
  m_relation = new Relation(a.relation(), this);
  m_pType    = new Ptype(a.pType());
  m_name   = a.m_name;
  m_usable = a.m_usable;

  if (a.m_userInfo)
    m_userInfo = a.userInfo().clone();
}

Particle::Particle(const HepLorentzVector& a, const Ptype& ptype)
  : m_usable(USABLE), m_userInfo(NULL)
{
  m_momentum = new Momentum(a);
  m_relation = new Relation(this);
  m_pType    = new Ptype(ptype);
}

Particle::Particle(const Momentum& a, const Ptype& ptype)
  : m_usable(USABLE),  m_userInfo(NULL)
{
  m_momentum = new Momentum(a);
  m_relation = new Relation(this);
  m_pType    = new Ptype(ptype);
}

Particle::Particle(const Track& a, const Ptype& ptype,
                   const Hep3Vector& pivot)
  : m_usable(USABLE),  m_userInfo(NULL)
{
  m_momentum = new Momentum(a, ptype, pivot);
  m_relation = new Relation(a, this);
  m_pType    = new Ptype(ptype);
}

Particle::Particle(const MdstGamma& a)
  : m_usable(USABLE),  m_userInfo(NULL)
{
  m_momentum = new Momentum(a);
  m_relation = new Relation(a, this);
  m_pType    = new Ptype(22);
}

Particle::Particle(const MdstPi0& a, const bool makeRelation)
  : m_usable(USABLE),  m_userInfo(NULL)
{
  m_momentum = new Momentum(a);
  m_relation = new Relation(a, makeRelation, this);
  m_pType    = new Ptype(111);
}

Particle::Particle(RecCRECL& a)
  : m_usable(USABLE),  m_userInfo(NULL)
{
  m_momentum = new Momentum(a);
  m_relation = new Relation(a, this);
  m_pType    = new Ptype(22);
}

// destructor
Particle::~Particle()
{
  delete m_momentum;
  delete m_relation;
  delete m_pType;
  if (m_userInfo) delete m_userInfo;
}

// append daughter
Particle& Particle::append_daughter(Particle& d)
{
  relation().append(d);
  if (d.m_userInfo) {
    if (! m_userInfo) {
      userInfo(d.userInfo());
    } else {
      userInfo().append(d.userInfo());
    }
  }
  return *this;
}

// Operators
/// Copy operator
Particle&
Particle::operator=(const Particle& a)
{
  if (this == &a) return *this;

  delete m_momentum;
  m_momentum = new Momentum(a.momentum());

  delete m_relation;
  m_relation = new Relation(a.relation(), this);

  delete m_pType;
  m_pType    = new Ptype(a.pType());

  m_name   = a.m_name;
  m_usable = a.m_usable;

  if (m_userInfo) delete m_userInfo;
  if (a.m_userInfo) m_userInfo = a.userInfo().clone();
  else m_userInfo = NULL;
  return *this;
}

bool
Particle::usable(const bool& n)
{
  switch (n) {
    case USABLE:
      m_usable = USABLE;
      break;
    case UNUSABLE:
      m_usable = UNUSABLE;
      break;
  }
  return m_usable;
}

bool
Particle::unusable(bool n)
{
  switch (n) {
    case USABLE:
      m_usable = USABLE;
      break;
    case UNUSABLE:
      m_usable = UNUSABLE;
      break;
  }
  return m_usable;
}

#if 0
Particle
Particle::deepCopy(void)
{
  Particle copied(*this);

  //...m_children
  std::vector<Particle*> children;
  unsigned size  = copied.relation().nChildren();
  //std::vector<unsigned> fitted;
  for (unsigned i = 0; i < size; ++i) {
    children.push_back(new Particle(copied.relation().child(i).deepCopy()));
  }
  if (mdstVee2()) {
    for (unsigned int i = 0; i < m_relation->nChildren(); ++i) {
      --(m_relation->child(i).relation().m_vee2ChildCounter);
    }
  }
  if (mdstPi0()) {
    for (unsigned int i = 0; i < m_relation->nChildren(); ++i) {
      --(m_relation->child(i).relation().m_pi0ChildCounter);
    }
  }

  copied.relation().removeAll();
  for (unsigned i = 0; i < size; ++i) {
    copied.relation().append(*children[i]);
  }

  return copied;
}

void
Particle::deepDelete(void)
{
  if (this->relation().nChildren() == 0)return;
  for (unsigned i = 0; i < this->relation().nChildren(); ++i) {
    this->relation().child(i).deepDelete();
    delete &this->relation().child(i);
  }
  this->relation().removeAll();
}
#endif

const ParticleUserInfo&
Particle::userInfo(void) const
{
  if (m_userInfo)return *m_userInfo;
  else {
//      if(static_ParticleUserInfo)return *static_ParticleUserInfo;
//      else return *(static_ParticleUserInfo = new ParticleUserInfo);
    return *static_ParticleUserInfo;
  }
}

ParticleUserInfo&
Particle::userInfo(void)
{
  if (m_userInfo)return *m_userInfo;
  else {
//      if(static_ParticleUserInfo)return *static_ParticleUserInfo;
//      else return *(static_ParticleUserInfo = new ParticleUserInfo);
    return *static_ParticleUserInfo;
  }
}

const ParticleUserInfo&
Particle::userInfo(const ParticleUserInfo& info)
{
  if (m_userInfo) {
    *m_userInfo = info;
  } else {
    m_userInfo = info.clone();
  }
  return *m_userInfo;
}

void
Particle::dump(const std::string& keyword, const std::string& prefix) const
{
  bool full = false;
  if (keyword.find("full") != std::string::npos) full = true;

  std::cout << "Particle " << prefix;
  if (m_name == std::string(""))
    if (pType().lund()) std::cout << pType().lund() << std::endl;
    else std::cout << "Particle " << m_name;
  if (full || keyword.find("mass") != std::string::npos)     std::cout << "Particle" << " m = " << mass() << std::endl;
  if (full || keyword.find("momentum") != std::string::npos) std::cout << "Particle" << " p = " << p() << std::endl;
  if (full || keyword.find("position") != std::string::npos) std::cout << "Particle" << " x = " << x() << std::endl;
  if (full || keyword.find("dpx") != std::string::npos) std::cout << "Particle" << " dpx = " << std::endl; std::cout << momentum().dpx() << std::endl;
  std::cout << "Particle" << std::endl;

  if (full || keyword.find("recursive") != std::string::npos) {
    unsigned n = nChildren();
    if (n) {
      for (unsigned i = 0; i < n; i++)
        child(i).dump(keyword, prefix + "    ");
    }
  }
}
