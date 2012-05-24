#ifndef _DEFINE_TSVD_HIT_H_
#define _DEFINE_TSVD_HIT_H_


#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

namespace Belle {

  class Recsvd_cluster;

  class TSvdHit {
  public:
    // Constructor
    TSvdHit(void) : m_position(HepGeom::Point3D<double> (0., 0., 0.)), m_dssd(-1), m_rphi(NULL), m_z(NULL) {};
    TSvdHit(const HepGeom::Point3D<double>  &p, int d, Recsvd_cluster* rphi, Recsvd_cluster* z) :
      m_position(p), m_dssd(d), m_rphi(rphi), m_z(z) {};

    // Destructor
    virtual ~TSvdHit(void) {};

    // Modifiers
    inline void position(const HepGeom::Point3D<double>  &p) { m_position = p; }
    inline void dssd(const int& d) { m_dssd = d; }
    inline void rphi(Recsvd_cluster* rphi) { m_rphi = rphi; }
    inline void z(Recsvd_cluster* z) { m_z = z; }

    // Selectors
    inline const HepGeom::Point3D<double>  & position(void) const { return m_position; }
    inline int dssd(void) const { return m_dssd; }
    inline Recsvd_cluster* rphi(void) const { return m_rphi; }
    inline Recsvd_cluster* z(void) const { return m_z; }

  private:
    HepGeom::Point3D<double>  m_position;
    int m_dssd;
    Recsvd_cluster* m_rphi;
    Recsvd_cluster* m_z;
  };

  class TSvdHitPair {
  public:
    TSvdHit* first;
    TSvdHit* second;
  };

} // namespace Belle

#endif /* _DEFINE_TSVD_HIT_H_ */
