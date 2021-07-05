/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Helix.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track helix parameter in Belle style
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHelix_FLAG_
#define TRGCDCHelix_FLAG_

#include <string>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Geometry/Point3D.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHelix TRGCDCHelix
#endif

namespace Belle2 {

/// TRGCDCHelix parameter class
  class TRGCDCHelix {

  public:
    /// Constructor with pivot, helix parameter a, and its error matrix.
    TRGCDCHelix(const HepGeom::Point3D<double>&   pivot,
                const CLHEP::HepVector& a,
                const CLHEP::HepSymMatrix& Ea);

    /// Constructor without error matrix.
    TRGCDCHelix(const HepGeom::Point3D<double>&   pivot,
                const CLHEP::HepVector& a);

    /// Constructor with position, momentum, and charge.
    TRGCDCHelix(const HepGeom::Point3D<double>&   position,
                const CLHEP::Hep3Vector& momentum,
                double charge);

    /// Default copy constructor
    TRGCDCHelix(const TRGCDCHelix&) = default;

    /// Destructor
    virtual ~TRGCDCHelix();

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

    /// set limits for helix parameters
    static void set_limits(const CLHEP::HepVector& a_min,
                           const CLHEP::HepVector& a_max);

    /// set to throw exception or not
    static bool set_exception(bool);

    /// set to print debug info or not
    static bool set_print(bool);

  private:
    /// limits for helix parameters
    static CLHEP::HepVector ms_amin;
    /// limits for helix parameters
    static CLHEP::HepVector ms_amax;
    /// range in checked or not
    static bool ms_check_range;
    /// print debug info or not
    static bool ms_print_debug;
    /// throw exception or not
    static bool ms_throw_exception;


  public:// Operators
    /// Copy operator
    TRGCDCHelix& operator = (const TRGCDCHelix&);

  public:// Mathmatical functions
    /// Mathmatical functions
    CLHEP::HepMatrix delApDelA(const CLHEP::HepVector& ap) const;
    /// Mathmatical functions
    CLHEP::HepMatrix delXDelA(double phi) const;
    /// Mathmatical functions
    CLHEP::HepMatrix delMDelA(double phi) const;
    /// Mathmatical functions
    CLHEP::HepMatrix del4MDelA(double phi, double mass) const;
    /// Mathmatical functions
    CLHEP::HepMatrix del4MXDelA(double phi, double mass) const;

  private:
    /// update Caches
    void updateCache(void);
    /// check validity
    void checkValid(void);
    /// print debug info
    void debugPrint(void) const;
    /// function not defined
    void debugTRGCDCHelix(void) const;

  public:
    /// Constant alpha for uniform field.
    static const double ConstantAlpha;

  private:
    /// matrix validity
    bool m_matrixValid;
    /// helix validity
    bool m_helixValid;
    /// magnetic field
    double m_bField;
    /// alpha parameter
    double m_alpha;
    /// pivot
    HepGeom::Point3D<double>  m_pivot;
    /// a HepVector parameter
    CLHEP::HepVector m_a;
    /// Ea HepSymMatrix parameter
    CLHEP::HepSymMatrix m_Ea;

  private: // caches
    /// caches
    HepGeom::Point3D<double>  m_center;
    /// caches
    double m_cp;
    /// caches
    double m_sp;
    /// caches
    double m_pt;
    /// caches
    double m_r;
    /// caches
    double m_ac[5];

    /// string of invalid helix
    static const std::string invalidhelix;

  };

//-----------------------------------------------------------------------------

  inline
  const HepGeom::Point3D<double>&
  TRGCDCHelix::center(void) const
  {
    return m_center;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCHelix::pivot(void) const
  {
    return m_pivot;
  }

  inline
  double
  TRGCDCHelix::radius(void) const
  {
    return m_r;
  }

  inline
  CLHEP::Hep3Vector
  TRGCDCHelix::direction(double phi) const
  {
    return momentum(phi).unit();
  }

  inline
  double
  TRGCDCHelix::dr(void) const
  {
    return m_ac[0];
  }

  inline
  double
  TRGCDCHelix::phi0(void) const
  {
    return m_ac[1];
  }

  inline
  double
  TRGCDCHelix::kappa(void) const
  {
    return m_ac[2];
  }

  inline
  double
  TRGCDCHelix::dz(void) const
  {
    return m_ac[3];
  }

  inline
  double
  TRGCDCHelix::tanl(void) const
  {
    return m_ac[4];
  }

  inline
  double
  TRGCDCHelix::curv(void) const
  {
    return m_r;
  }

  inline
  const CLHEP::HepVector&
  TRGCDCHelix::a(void) const
  {
    return m_a;
  }

  inline
  const CLHEP::HepSymMatrix&
  TRGCDCHelix::Ea(void) const
  {
    return m_Ea;
  }

  inline
  const CLHEP::HepVector&
  TRGCDCHelix::a(const CLHEP::HepVector& i)
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
  TRGCDCHelix::Ea(const CLHEP::HepSymMatrix& i)
  {
    return m_Ea = i;
  }

  inline
  double
  TRGCDCHelix::bFieldZ(double a)
  {
    m_bField = a;
    m_alpha = 10000. / 2.99792458 / m_bField;
    updateCache();
    return m_bField;
  }

  inline
  double
  TRGCDCHelix::bFieldZ(void) const
  {
    return m_bField;
  }

  inline
  double
  TRGCDCHelix::sinPhi0(void) const
  {
    return m_sp;
  }

  inline
  double
  TRGCDCHelix::cosPhi0(void) const
  {
    return m_cp;
  }

} // namespace Belle2

#endif /* TRGCDCHelix_FLAG_ */
