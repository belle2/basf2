/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Y.Ohnishi, Y.Ohnishi, Y.Iwasaki, J.Tanaka                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HELIX_H
#define HELIX_H

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
#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif
using HepGeom::Point3D;

typedef CLHEP::HepVector Vector;
typedef CLHEP::HepSymMatrix SymMatrix;
typedef CLHEP::Hep3Vector Vector3;
typedef CLHEP::HepLorentzVector VectorL;
typedef CLHEP::HepLorentzVector Vector4;
typedef CLHEP::HepMatrix Matrix;

using CLHEP::HepVector;
using CLHEP::HepSymMatrix;
using CLHEP::Hep3Vector;
using CLHEP::HepLorentzVector;
using CLHEP::HepMatrix;

namespace Belle2 {
  /** @todo Helix : Is this type of class perhaps needed in various packages, would this kind of mathematical structure perhaps make more sense in a framework/utilities folder? */
  namespace CDC {
/// Helix parameter class
    class Helix {

    public:
      /// Constructor with pivot, helix parameter a, and its error matrix.
      Helix(const HepPoint3D& pivot,
            const HepVector& a,
            const HepSymMatrix& Ea);

      /// Constructor without error matrix.
      Helix(const HepPoint3D& pivot,
            const HepVector& a);

      /// Constructor with position, momentum, and charge.
      Helix(const HepPoint3D& position,
            const Hep3Vector& momentum,
            double charge);

      /// Destructor
      virtual ~Helix();

    public:// Selectors
      /// returns position of helix center(z = 0.);
      const HepPoint3D& center(void) const;

      /// returns pivot position.
      const HepPoint3D& pivot(void) const;

      /// returns radious of helix.
      double radius(void) const;

      /**
       * returns position after rotating angle dPhi in phi direction.
       *
       * x = x0 + dr * cos(phi0) + (alpha / kappa) * (cos(phi0) - cos(phi0+phi))
       * y = y0 + dr * sin(phi0) + (alpha / kappa) * (sin(phi0) - sin(phi0+phi))
       * z = z0 + dz             - (alpha / kappa) * tan(lambda) * phi
       * @return HepPoint3D
       */

      HepPoint3D x(double dPhi = 0.) const;

      /**
       * returns position after rotating angle dPhi in phi direction.
       *
       * x = x0 + dr * cos(phi0) + (alpha / kappa) * (cos(phi0) - cos(phi0+phi))
       * y = y0 + dr * sin(phi0) + (alpha / kappa) * (sin(phi0) - sin(phi0+phi))
       * z = z0 + dz             - (alpha / kappa) * tan(lambda) * phi
       * @return double[3]
       */

      double* x(double dPhi, double p[3]) const;

      /// returns position and convariance matrix(Ex) after rotation.
      HepPoint3D x(double dPhi, HepSymMatrix& Ex) const;

      /// returns direction vector after rotating angle dPhi in phi direction.
      Hep3Vector direction(double dPhi = 0.) const;

      /// returns momentum vector after rotating angle dPhi in phi direction.
      Hep3Vector momentum(double dPhi = 0.) const;

      /// returns momentum vector after rotating angle dPhi in phi direction.
      Hep3Vector momentum(double dPhi, HepSymMatrix& Em) const;

      /// returns 4momentum vector after rotating angle dPhi in phi direction.
      HepLorentzVector momentum(double dPhi, double mass) const;

      /// returns 4momentum vector after rotating angle dPhi in phi direction.
      HepLorentzVector momentum(double dPhi, double mass, HepSymMatrix& Em) const;

      /// returns 4momentum vector after rotating angle dPhi in phi direction.
      HepLorentzVector momentum(double dPhi, double mass, HepPoint3D& x, HepSymMatrix& Emx) const;

    public:

      // Parametrization dependent functions.
      // Prepared for tracking codes.
      // Users should not use them.

      /**
       * Return helix parameter dr.
       */

      double dr(void) const;

      /**
       * Return helix parameter phi0.
       */
      double phi0(void) const;

      /**
       * Return helix parameter kappa.
       */
      double kappa(void) const;

      /**
       * Return helix parameter dz.
       */
      double dz(void) const;

      /**
       * Return helix parameter tangent lambda.
       */
      double tanl(void) const;

      /**
       * Return curvature of helix.
       */
      double curv(void) const;

      /**
       * Return sin phi0.
       */

      double sinPhi0(void) const;

      /**
       * Return cos phi0.
       */
      double cosPhi0(void) const;

      /**
       * Returns helix parameters.
       */
      const HepVector& a(void) const;

      /**
       * Returns error matrix.
       */
      const HepSymMatrix& Ea(void) const;

    public:// Modifiers
      /// Sets helix parameters.
      const HepVector& a(const HepVector& newA);

      /// Sets helix paramters and error matrix.
      const HepSymMatrix& Ea(const HepSymMatrix& newdA);

      /// Sets pivot position.
      const HepPoint3D& pivot(const HepPoint3D& newPivot);

      /// Sets helix pivot position, parameters, and error matrix.
      void set(const HepPoint3D& pivot,
               const HepVector& a,
               const HepSymMatrix& Ea);

      /**
       * Unsets error matrix.
       * Error calculations will be ignored after this function call
       * until an error matrix be set again. 0 matrix will be return
       * as a return value for error matrix when you call functions
       * which returns an error matrix.
       */

      void ignoreErrorMatrix(void);

      /**
       * Sets/returns z componet of the magnetic field.
       *
       * @param[in] bz z-component of the magnetic field.
       * @attention Helix param. alpha is also stored.
       */

      double bFieldZ(double bz);

      /**
       * Returns z componet of the magnetic field.
       *
       */
      double bFieldZ(void) const;

      static void set_limits(const HepVector& a_min, const HepVector& a_max);
      static bool set_exception(bool);

      /**
       * Set print option for debugging.
       */
      static bool set_print(bool);

    private:
      static HepVector ms_amin;
      static HepVector ms_amax; //!
      static bool ms_check_range;     //! Check the helix parameter's range.
      static bool ms_print_debug;     /*! Debug option flag. */
      static bool ms_throw_exception; /*! Throw exception flag. */

    public: // Operators

      Helix& operator = (const Helix&);       /// Copy operator.

    public: // Mathmatical functions

      HepMatrix delApDelA(const HepVector& ap) const;
      HepMatrix delXDelA(double phi) const; ///
      HepMatrix delMDelA(double phi) const; ///
      HepMatrix del4MDelA(double phi, double mass) const; ///
      HepMatrix del4MXDelA(double phi, double mass) const; ///

    private:
      /**
       *
       */
      void updateCache(void);

      /**
       * Check whether helix parameters is valid or not.
       * Sets m_helixValid.
       *
       */
      void checkValid(void);

      /**
       * Print the helix parameters to stdout.
       */
      void debugPrint(void) const;

      /**
       *
       */
      void debugHelix(void) const;
    public:
      /// Constant alpha for uniform field.
      static const double ConstantAlpha;

    private:

      bool m_matrixValid; ///
      bool m_helixValid;  /// True: helix valid, False: helix not valid.
      double m_bField;    /*! Magnetic field, assuming uniform Bz in the unit of  kG. */
      double m_alpha;     /*! 10000.0/(speed of light)/B. */
      HepPoint3D m_pivot; /*! Pivot */
      HepVector m_a;      /*! Helix parameter. */
      HepSymMatrix m_Ea;  /*! Error of the helix parameter. */

    private: // caches

      HepPoint3D m_center;
      double m_cp; /// Chache of the cos phi0
      double m_sp; /// Chache of the sin phi0
      double m_pt;
      double m_r;
      double m_ac[5]; /// Cache of the helix parameter.

      static const std::string invalidhelix; /// String "Invalid Helix".

    };

#if defined(BELLE_DEBUG)
#define DEBUG_HELIX debugHelix()
#define DEBUG_PRINT debugPrint()
#else
#define DEBUG_HELIX
#define DEBUG_PRINT
#endif

//-----------------------------------------------------------------------------

#ifdef Helix_NO_INLINE
#define inline
#else
#undef inline
#define Helix_INLINE_DEFINE_HERE
#endif

#ifdef Helix_INLINE_DEFINE_HERE

    inline
    const HepPoint3D&
    Helix::center(void) const
    {
#if defined(BELLE_DEBUG)
      if (!m_helixValid) {
        DEBUG_PRINT;
        if (msthrow_exception) throw invalidhelix;
      }
#endif
      return m_center;
    }

    inline
    const HepPoint3D&
    Helix::pivot(void) const
    {
      DEBUG_HELIX;
      return m_pivot;
    }

    inline
    double
    Helix::radius(void) const
    {
      DEBUG_HELIX;
      return m_r;
    }

    inline
    Hep3Vector
    Helix::direction(double phi) const
    {
      DEBUG_HELIX;
      return momentum(phi).unit();
    }

    inline
    double
    Helix::dr(void) const
    {
      DEBUG_HELIX;
      return m_ac[0];
    }

    inline
    double
    Helix::phi0(void) const
    {
      DEBUG_HELIX;
      return m_ac[1];
    }

    inline
    double
    Helix::kappa(void) const
    {
      DEBUG_HELIX;
      return m_ac[2];
    }

    inline
    double
    Helix::dz(void) const
    {
      DEBUG_HELIX;
      return m_ac[3];
    }

    inline
    double
    Helix::tanl(void) const
    {
      DEBUG_HELIX;
      return m_ac[4];
    }

    inline
    double
    Helix::curv(void) const
    {
      DEBUG_HELIX;
      return m_r;
    }

    inline
    const HepVector&
    Helix::a(void) const
    {
      DEBUG_HELIX;
      return m_a;
    }

    inline
    const HepSymMatrix&
    Helix::Ea(void) const
    {
      DEBUG_HELIX;
      return m_Ea;
    }

    inline
    const HepVector&
    Helix::a(const HepVector& i)
    {
      if (i.num_row() == 5) {
        m_a = i;
        m_helixValid = false;
        updateCache();
#if defined(BELLE_DEBUG)
        DEBUG_HELIX;
      } else {
        {
          std::cout << "Helix::input vector's num_row is not 5" << std::endl;
          DEBUG_PRINT;
          if (ms_throw_exception) throw invalidhelix;
        }
#endif
      }
      return m_a;
    }

    inline
    const HepSymMatrix&
    Helix::Ea(const HepSymMatrix& i)
    {
      DEBUG_HELIX;
      return m_Ea = i;
    }

    inline
    double
    Helix::bFieldZ(double a)
    {
      DEBUG_HELIX;
      m_bField = a;
      m_alpha = 10000. / 2.99792458 / m_bField;
      updateCache();
      return m_bField;
    }

    inline
    double
    Helix::bFieldZ(void) const
    {
      DEBUG_HELIX;
      return m_bField;
    }

    inline
    double
    Helix::sinPhi0(void) const
    {
      DEBUG_HELIX;
      return m_sp;
    }

    inline
    double
    Helix::cosPhi0(void) const
    {
      DEBUG_HELIX;
      return m_cp;
    }

#endif

#undef inline
  }
} // end of namespace Belle2

#endif /* HELIX_H */
