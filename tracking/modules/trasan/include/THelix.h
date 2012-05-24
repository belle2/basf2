//
// $Id: THelix.h 10002 2007-02-26 06:56:17Z katayama $
//
// $Log$
// Revision 1.16  2002/01/03 11:05:06  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.15  2001/12/23 09:59:05  katayama
// removed Strings.h
//
// Revision 1.14  2001/04/25 02:55:39  yiwasaki
// cache m_ac[5] added
//
// Revision 1.13  2000/01/26 10:22:51  yiwasaki
// copy operator bug fix(reported by M.Yokoyama)
//
// Revision 1.12  1999/11/23 10:29:21  yiwasaki
// static cosnt double THelix::ConstantAlpha added
//
// Revision 1.11  1999/08/24 10:08:54  yiwasaki
// sinPhi0(), cosPhi0() functions added
//
// Revision 1.10  1999/06/15 01:28:36  yiwasaki
// bFieldZ bug fixed
//
// Revision 1.9  1999/06/06 07:34:25  yiwasaki
// i/o functions to set/get mag. field added
//
// Revision 1.8  1999/05/11 23:26:57  yiwasaki
// option to ignore error calculations added
//
// Revision 1.7  1998/11/27 08:17:24  yiwasaki
// add default value for x(), direction(), and momentum()
//
// Revision 1.6  1998/07/27 00:02:54  katayama
// endif coment fixed
//
// Revision 1.5  1998/07/08 04:35:31  jtanaka
// add some members to the constructor by Iwasaki-san and Ozaki-san
//
// Revision 1.4  1998/06/18 10:27:16  katayama
// Added several new functions from Tanaka san
//
// Revision 1.3  1998/02/20 02:06:12  yiwasaki
// New helix class
//
// Revision 1.2  1997/09/19 00:35:47  katayama
// Added Id and Log
//
//
//
//   Class THelix
//
//   Author      Date         comments
//   Y.Ohnishi   03/01/1997   original version
//   Y.Ohnishi   06/03/1997   updated
//   Y.Iwasaki   17/02/1998   BFILED removed, func. name changed, func. added
//   J.Tanaka    06/12/1998   add some utilities.
//   Y.Iwasaki   07/07/1998   cache added to speed up
//   Y.Iwasaki   25/04/2001   cache m_ac[5] added to speed up
//
#ifndef THelix_FLAG_
#define THelix_FLAG_

#include <string>


#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#ifndef CLHEP_THREEVECTOR_H
#include "CLHEP/Vector/ThreeVector.h"
#endif
#include "CLHEP/Vector/LorentzVector.h"
#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

namespace Belle {

/// THelix parameter class
  class THelix {

  public:
    /// Constructor with pivot, helix parameter a, and its error matrix.
    THelix(const HepGeom::Point3D<double>  & pivot,
           const CLHEP::HepVector& a,
           const CLHEP::HepSymMatrix& Ea);

    /// Constructor without error matrix.
    THelix(const HepGeom::Point3D<double>  & pivot,
           const CLHEP::HepVector& a);

    /// Constructor with position, momentum, and charge.
    THelix(const HepGeom::Point3D<double>  & position,
           const CLHEP::Hep3Vector& momentum,
           double charge);

    /// Destructor
    virtual ~THelix();

  public:// Selectors
    /// returns position of helix center(z = 0.);
    const HepGeom::Point3D<double>  & center(void) const;

    /// returns pivot position.
    const HepGeom::Point3D<double>  & pivot(void) const;

    /// returns radious of helix.
    double radius(void) const;

    /// returns position after rotating angle dPhi in phi direction.
    HepGeom::Point3D<double>  x(double dPhi = 0.) const;
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
    CLHEP::HepLorentzVector momentum(double dPhi, double mass, HepGeom::Point3D<double>  & x, CLHEP::HepSymMatrix& Emx) const;

  public:// Parametrization dependent functions. Prepared for tracking codes. Users should not use them.
    /// returns an element of parameters.
    double dr(void) const;
    double phi0(void) const;
    double kappa(void) const;
    double dz(void) const;
    double tanl(void) const;
    double curv(void) const;
    double sinPhi0(void) const;
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
    const HepGeom::Point3D<double>  & pivot(const HepGeom::Point3D<double>  & newPivot);

    /// sets helix pivot position, parameters, and error matrix.
    void set(const HepGeom::Point3D<double>  & pivot,
             const CLHEP::HepVector& a,
             const CLHEP::HepSymMatrix& Ea);

    /// unsets error matrix. Error calculations will be ignored after this function call until an error matrix be set again. 0 matrix will be return as a return value for error matrix when you call functions which returns an error matrix.
    void ignoreErrorMatrix(void);

    /// sets/returns z componet of the magnetic field.
    double bFieldZ(double);
    double bFieldZ(void) const;

    static void set_limits(const CLHEP::HepVector& a_min, const CLHEP::HepVector& a_max);
    static bool set_exception(bool);
    static bool set_print(bool);
  private:
    static CLHEP::HepVector ms_amin, ms_amax;
    static bool ms_check_range;
    static bool ms_print_debug;
    static bool ms_throw_exception;


  public:// Operators
    /// Copy operator
    THelix& operator = (const THelix&);

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
    void debugTHelix(void) const;
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

#if defined(BELLE_DEBUG)
#define DEBUG_HELIX debugTHelix()
#define DEBUG_PRINT debugPrint()
#else
#define DEBUG_HELIX
#define DEBUG_PRINT
#endif

//-----------------------------------------------------------------------------

#ifdef THelix_NO_INLINE
#define inline
#else
#undef inline
#define THelix_INLINE_DEFINE_HERE
#endif

#ifdef THelix_INLINE_DEFINE_HERE

  inline
  const HepGeom::Point3D<double>  &
  THelix::center(void) const
  {
#if defined(BELLE_DEBUG)
    if (!m_helixValid) {
      DEBUG_PRINT;
      if (ms_throw_exception) throw invalidhelix;
    }
#endif
    return m_center;
  }

  inline
  const HepGeom::Point3D<double>  &
  THelix::pivot(void) const
  {
    DEBUG_HELIX;
    return m_pivot;
  }

  inline
  double
  THelix::radius(void) const
  {
    DEBUG_HELIX;
    return m_r;
  }

  inline
  CLHEP::Hep3Vector
  THelix::direction(double phi) const
  {
    DEBUG_HELIX;
    return momentum(phi).unit();
  }

  inline
  double
  THelix::dr(void) const
  {
    DEBUG_HELIX;
    return m_ac[0];
  }

  inline
  double
  THelix::phi0(void) const
  {
    DEBUG_HELIX;
    return m_ac[1];
  }

  inline
  double
  THelix::kappa(void) const
  {
    DEBUG_HELIX;
    return m_ac[2];
  }

  inline
  double
  THelix::dz(void) const
  {
    DEBUG_HELIX;
    return m_ac[3];
  }

  inline
  double
  THelix::tanl(void) const
  {
    DEBUG_HELIX;
    return m_ac[4];
  }

  inline
  double
  THelix::curv(void) const
  {
    DEBUG_HELIX;
    return m_r;
  }

  inline
  const CLHEP::HepVector&
  THelix::a(void) const
  {
    DEBUG_HELIX;
    return m_a;
  }

  inline
  const CLHEP::HepSymMatrix&
  THelix::Ea(void) const
  {
    DEBUG_HELIX;
    return m_Ea;
  }

  inline
  const CLHEP::HepVector&
  THelix::a(const CLHEP::HepVector& i)
  {
    if (i.num_row() == 5) {
      m_a = i;
      m_helixValid = false;
      updateCache();
#if defined(BELLE_DEBUG)
      DEBUG_HELIX;
    } else {
      {
        std::cout << "THelix::input vector's num_row is not 5" << std::endl;
        DEBUG_PRINT;
        if (ms_throw_exception) throw invalidhelix;
      }
#endif
    }
    return m_a;
  }

  inline
  const CLHEP::HepSymMatrix&
  THelix::Ea(const CLHEP::HepSymMatrix& i)
  {
    DEBUG_HELIX;
    return m_Ea = i;
  }

  inline
  double
  THelix::bFieldZ(double a)
  {
    DEBUG_HELIX;
    m_bField = a;
    m_alpha = 10000. / 2.99792458 / m_bField;
    updateCache();
    return m_bField;
  }

  inline
  double
  THelix::bFieldZ(void) const
  {
    DEBUG_HELIX;
    return m_bField;
  }

  inline
  double
  THelix::sinPhi0(void) const
  {
    DEBUG_HELIX;
    return m_sp;
  }

  inline
  double
  THelix::cosPhi0(void) const
  {
    DEBUG_HELIX;
    return m_cp;
  }

#endif

#undef inline

} // namespace Belle

#endif /* THelix_FLAG_ */
