//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : cosmicsHelix.h
// Section  : Cosmic generator
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
// Temporary adopted to use in the cosmic generator without dependency on TRG
// Sergey Yashchenko (sergey.yaschenko@desy.de)
//-----------------------------------------------------------------------------
// Description : A class to represent a track helix parameter in Belle style
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef COSMICSHELIX_
#define COSMICSHELIX_

#include <string>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Geometry/Point3D.h"

namespace Belle2 {

/// cosmicsHelix parameter class
  class cosmicsHelix {

  public:
    /// Constructor with pivot, helix parameter a, and its error matrix.
    cosmicsHelix(const HepGeom::Point3D<double>&   pivot,
                 const CLHEP::HepVector& a,
                 const CLHEP::HepSymMatrix& Ea);

    /// Constructor without error matrix.
    cosmicsHelix(const HepGeom::Point3D<double>&   pivot,
                 const CLHEP::HepVector& a);

    /// Constructor with position, momentum, and charge.
    cosmicsHelix(const HepGeom::Point3D<double>&   position,
                 const CLHEP::Hep3Vector& momentum,
                 double charge);

    /// Destructor
    virtual ~cosmicsHelix();

  public:// Selectors

    /// returns position of helix center(z = 0.);
    const HepGeom::Point3D<double>&   center(void) const;

    /// returns pivot position.
    const HepGeom::Point3D<double>&   pivot(void) const;

    /// returns radious of helix.
    double radius(void) const;

    /// returns position after rotating angle dPhi in phi direction.
    HepGeom::Point3D<double>  x(double dPhi = 0.) const;

    /// returns position after rotating angle dPhi in phi direction.
    double* x(double dPhi, double p[3]) const;

    /// returns position and convariance matrix(Ex) after rotation.
    HepGeom::Point3D<double>  x(double dPhi, CLHEP::HepSymMatrix& Ex) const;

    /// returns direction vector after rotating angle dPhi in phi direction.
    CLHEP::Hep3Vector direction(double dPhi = 0.) const;

    /// returns momentum vector after rotating angle dPhi in phi direction.
    CLHEP::Hep3Vector momentum(double dPhi = 0.) const;

    /// returns momentum vector after rotating angle dPhi in phi direction.
    CLHEP::Hep3Vector momentum(double dPhi, CLHEP::HepSymMatrix& Em) const;

    /// returns 4momentum vector after rotating angle dPhi in phi direction.
    CLHEP::HepLorentzVector momentum(double dPhi, double mass) const;

    /// returns 4momentum vector after rotating angle dPhi in phi direction.
    CLHEP::HepLorentzVector momentum(double dPhi, double mass, CLHEP::HepSymMatrix& Em) const;

    /// returns 4momentum vector after rotating angle dPhi in phi direction.
    CLHEP::HepLorentzVector momentum(double dPhi, double mass, HepGeom::Point3D<double>&   x, CLHEP::HepSymMatrix& Emx) const;

  public:// Parametrization dependent functions. Prepared for tracking codes. Users should not use them.

    /// returns dr.
    double dr(void) const;

    /// returns phi0.
    double phi0(void) const;

    /// returns kappa.
    double kappa(void) const;

    /// returns dz.
    double dz(void) const;

    /// returns tanl.
    double tanl(void) const;

    /// returns curvurture.
    double curv(void) const;

    /// returns sin(phi0).
    double sinPhi0(void) const;

    /// return cos(phi0).
    double cosPhi0(void) const;

    /// returns helix parameters.
    const CLHEP::HepVector& a(void) const;

    /// returns error matrix.
    const CLHEP::HepSymMatrix& Ea(void) const;

  public:// Modifiers
    /// sets helix parameters.
    const CLHEP::HepVector& a(const CLHEP::HepVector& newA);

    /// sets helix paramters and error matrix.
    const CLHEP::HepSymMatrix& Ea(const CLHEP::HepSymMatrix& newdA);

    /// sets pivot position.
    const HepGeom::Point3D<double>&   pivot(const HepGeom::Point3D<double>&   newPivot);

    /// sets helix pivot position, parameters, and error matrix.
    void set(const HepGeom::Point3D<double>&   pivot,
             const CLHEP::HepVector& a,
             const CLHEP::HepSymMatrix& Ea);

    /// unsets error matrix. Error calculations will be ignored after this function call until an error matrix be set again. 0 matrix will be return as a return value for error matrix when you call functions which returns an error matrix.
    void ignoreErrorMatrix(void);

    /// returns z componet of the magnetic field.
    double bFieldZ(void) const;

    /// sets and returns z componet of the magnetic field.
    double bFieldZ(double);

    static void set_limits(const CLHEP::HepVector& a_min,
                           const CLHEP::HepVector& a_max);
    static bool set_exception(bool);
    static bool set_print(bool);

  private:
    static CLHEP::HepVector ms_amin, ms_amax;
    static bool ms_check_range;
    static bool ms_print_debug;
    static bool ms_throw_exception;


  public:// Operators
    /// Copy operator
    cosmicsHelix& operator = (const cosmicsHelix&);

  public:// Mathmatical functions
    CLHEP::HepMatrix delApDelA(const CLHEP::HepVector& ap) const;
    CLHEP::HepMatrix delXDelA(double phi) const;
    CLHEP::HepMatrix delMDelA(double phi) const;
    CLHEP::HepMatrix del4MDelA(double phi, double mass) const;
    CLHEP::HepMatrix del4MXDelA(double phi, double mass) const;

  private:
    void updateCache(void);
    void checkValid(void);
    void debugPrint(void) const;
    void debugcosmicsHelix(void) const;

  public:
    /// Constant alpha for uniform field.
    static const double ConstantAlpha;

  private:
    bool m_matrixValid;
    bool m_helixValid;
    double m_bField;
    double m_alpha;
    HepGeom::Point3D<double>  m_pivot;
    CLHEP::HepVector m_a;
    CLHEP::HepSymMatrix m_Ea;

  private: // caches
    HepGeom::Point3D<double>  m_center;
    double m_cp;
    double m_sp;
    double m_pt;
    double m_r;
    double m_ac[5];

    static const std::string invalidhelix;

  };

//-----------------------------------------------------------------------------

  inline
  const HepGeom::Point3D<double>&
  cosmicsHelix::center(void) const
  {
    return m_center;
  }

  inline
  const HepGeom::Point3D<double>&
  cosmicsHelix::pivot(void) const
  {
    return m_pivot;
  }

  inline
  double
  cosmicsHelix::radius(void) const
  {
    return m_r;
  }

  inline
  CLHEP::Hep3Vector
  cosmicsHelix::direction(double phi) const
  {
    return momentum(phi).unit();
  }

  inline
  double
  cosmicsHelix::dr(void) const
  {
    return m_ac[0];
  }

  inline
  double
  cosmicsHelix::phi0(void) const
  {
    return m_ac[1];
  }

  inline
  double
  cosmicsHelix::kappa(void) const
  {
    return m_ac[2];
  }

  inline
  double
  cosmicsHelix::dz(void) const
  {
    return m_ac[3];
  }

  inline
  double
  cosmicsHelix::tanl(void) const
  {
    return m_ac[4];
  }

  inline
  double
  cosmicsHelix::curv(void) const
  {
    return m_r;
  }

  inline
  const CLHEP::HepVector&
  cosmicsHelix::a(void) const
  {
    return m_a;
  }

  inline
  const CLHEP::HepSymMatrix&
  cosmicsHelix::Ea(void) const
  {
    return m_Ea;
  }

  inline
  const CLHEP::HepVector&
  cosmicsHelix::a(const CLHEP::HepVector& i)
  {
    if (i.num_row() == 5) {
      m_a = i;
      m_helixValid = false;
      updateCache();
    }
    return m_a;
  }

  inline
  const CLHEP::HepSymMatrix&
  cosmicsHelix::Ea(const CLHEP::HepSymMatrix& i)
  {
    return m_Ea = i;
  }

  inline
  double
  cosmicsHelix::bFieldZ(double a)
  {
    m_bField = a;
    m_alpha = 10000. / 2.99792458 / m_bField;
    updateCache();
    return m_bField;
  }

  inline
  double
  cosmicsHelix::bFieldZ(void) const
  {
    return m_bField;
  }

  inline
  double
  cosmicsHelix::sinPhi0(void) const
  {
    return m_sp;
  }

  inline
  double
  cosmicsHelix::cosPhi0(void) const
  {
    return m_cp;
  }

} // namespace Belle2

#endif
