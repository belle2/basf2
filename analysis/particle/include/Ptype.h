#ifndef PARTICLE_CLASS_PTYPE_H
#define PARTICLE_CLASS_PTYPE_H

#include <stdlib.h>

namespace Belle2 {

  class Ptype {
  public:
    /// Default constructor
    Ptype(void) {}
    /// Constructor with LUND7 particle code
    Ptype(const int);

    /// Destructor
    virtual ~Ptype(void) {}

    /// returns LUND7 particle code.
    inline const int lund(void) const;

    /// returns mass(nominal mass).
    inline const double mass(void) const;

    /// returns charge. (in units of e)
    inline const int charge(void) const;

  private:
    int    m_lund;
    double m_mass;
    int    m_charge;
  };

  inline const int    Ptype::lund(void)   const { return m_lund; }
  inline const double Ptype::mass(void)   const { return m_mass; }
  inline const int    Ptype::charge(void) const { return m_charge; }
}
#endif /* PARTICLE_CLASS_PTYPE_H */
