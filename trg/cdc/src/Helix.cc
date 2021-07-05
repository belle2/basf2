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
// Filename : Helix.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track helix parameter in Belle style
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <math.h>
#include <float.h>
#include "trg/cdc/Helix.h"
#include "CLHEP/Matrix/Matrix.h"

namespace Belle2 {

//  const double
//  TRGCDCHelix::m_BFIELD = 15.0;            // KG
//  const double
//  TRGCDCHelix::m_ALPHA = 222.376063;       // = 10000. / 2.99792458 / BFIELD

  /// 2*PI
  const double
  M_PI2 = 2. * M_PI;

  /// 4*PI
  const double
  M_PI4 = 4. * M_PI;

  /// 8*PI
  const double
  M_PI8 = 8. * M_PI;

  const double
  TRGCDCHelix::ConstantAlpha = 222.376063;

  const std::string TRGCDCHelix::invalidhelix("Invalid TRGCDCHelix");
  CLHEP::HepVector TRGCDCHelix::ms_amin(5, 0), TRGCDCHelix::ms_amax(5, 0);
  bool TRGCDCHelix::ms_check_range(false);
  bool TRGCDCHelix::ms_throw_exception(false);
  bool TRGCDCHelix::ms_print_debug(false);

  bool TRGCDCHelix::set_exception(bool t)
  {
    return ms_throw_exception = t;
  }

  bool TRGCDCHelix::set_print(bool t)
  {
    return ms_print_debug = t;
  }


  void TRGCDCHelix::set_limits(const CLHEP::HepVector& a_min, const CLHEP::HepVector& a_max)
  {
    if (a_min.num_row() != 5 || a_max.num_row() != 5) return;
    ms_amin = a_min;
    ms_amax = a_max;
    ms_check_range = true;
  }


  TRGCDCHelix::TRGCDCHelix(const HepGeom::Point3D<double>&   pivot,
                           const CLHEP::HepVector& a,
                           const CLHEP::HepSymMatrix& Ea)
    : m_matrixValid(true),
      m_helixValid(false),
      m_bField(15.0),
      m_alpha(222.376063),
      m_pivot(pivot),
      m_a(a),
      m_Ea(Ea)
  {
    // m_alpha = 10000. / 2.99792458 / m_bField;
    // m_alpha = 222.376063;
    if (m_a.num_row() == 5 && m_Ea.num_row() == 5) {
      updateCache();
    }
  }

  TRGCDCHelix::TRGCDCHelix(const HepGeom::Point3D<double>&   pivot,
                           const CLHEP::HepVector& a)
    : m_matrixValid(false),
      m_helixValid(false),
      m_bField(15.0),
      m_alpha(222.376063),
      m_pivot(pivot),
      m_a(a),
      m_Ea(CLHEP::HepSymMatrix(5, 0))
  {
    // m_alpha = 222.376063;
    if (m_a.num_row() == 5) {
      updateCache();
    }
  }

  TRGCDCHelix::TRGCDCHelix(const HepGeom::Point3D<double>&   position,
                           const CLHEP::Hep3Vector& momentum,
                           double charge)
    : m_matrixValid(false),
      m_helixValid(false),
      m_bField(15.0),
      m_alpha(222.376063),
      m_pivot(position),
      m_a(CLHEP::HepVector(5, 0)),
      m_Ea(CLHEP::HepSymMatrix(5, 0))
  {
    m_a[0] = 0.;
    m_a[3] = 0.;
    double perp(momentum.perp());
    if (perp != 0.0) {
      m_a[1] = fmod(atan2(- momentum.x(), momentum.y())
                    + M_PI4, M_PI2);
      m_a[2] = charge / perp;
      m_a[4] = momentum.z() / perp;
    } else {
      m_a[2] = charge * (DBL_MAX);
    }
    // m_alpha = 222.376063;
    updateCache();
  }

  TRGCDCHelix::~TRGCDCHelix()
  {
  }

  HepGeom::Point3D<double>
  TRGCDCHelix::x(double phi) const
  {
    //
    // Calculate position (x,y,z) along helix.
    //
    // x = x0 + dr * cos(phi0) + (alpha / kappa) * (cos(phi0) - cos(phi0+phi))
    // y = y0 + dr * sin(phi0) + (alpha / kappa) * (sin(phi0) - sin(phi0+phi))
    // z = z0 + dz             - (alpha / kappa) * tan(lambda) * phi
    //

    double x = m_pivot.x() + m_ac[0] * m_cp + m_r * (m_cp - cos(m_ac[1] + phi));
    double y = m_pivot.y() + m_ac[0] * m_sp + m_r * (m_sp - sin(m_ac[1] + phi));
    double z = m_pivot.z() + m_ac[3] - m_r * m_ac[4] * phi;

    return HepGeom::Point3D<double> (x, y, z);
  }

  double*
  TRGCDCHelix::x(double phi, double p[3]) const
  {
    //
    // Calculate position (x,y,z) along helix.
    //
    // x = x0 + dr * cos(phi0) + (alpha / kappa) * (cos(phi0) - cos(phi0+phi))
    // y = y0 + dr * sin(phi0) + (alpha / kappa) * (sin(phi0) - sin(phi0+phi))
    // z = z0 + dz             - (alpha / kappa) * tan(lambda) * phi
    //

    p[0] = m_pivot.x() + m_ac[0] * m_cp + m_r * (m_cp - cos(m_ac[1] + phi));
    p[1] = m_pivot.y() + m_ac[0] * m_sp + m_r * (m_sp - sin(m_ac[1] + phi));
    p[2] = m_pivot.z() + m_ac[3] - m_r * m_ac[4] * phi;

    return p;
  }

  HepGeom::Point3D<double>
  TRGCDCHelix::x(double phi, CLHEP::HepSymMatrix& Ex) const
  {

    double x = m_pivot.x() + m_ac[0] * m_cp + m_r * (m_cp - cos(m_ac[1] + phi));
    double y = m_pivot.y() + m_ac[0] * m_sp + m_r * (m_sp - sin(m_ac[1] + phi));
    double z = m_pivot.z() + m_ac[3] - m_r * m_ac[4] * phi;

    //
    //   Calculate position error matrix.
    //   Ex(phi) = (@x/@a)(Ea)(@x/@a)^T, phi is deflection angle to specify the
    //   point to be calcualted.
    //
    // CLHEP::HepMatrix dXDA(3, 5, 0);
    // dXDA = delXDelA(phi);
    // Ex.assign(dXDA * m_Ea * dXDA.T());

    if (m_matrixValid) Ex = m_Ea.similarity(delXDelA(phi));
    else               Ex = m_Ea;

    return HepGeom::Point3D<double> (x, y, z);
  }

  CLHEP::Hep3Vector
  TRGCDCHelix::momentum(double phi) const
  {
    //
    // Calculate momentum.
    //
    // Pt = | 1/kappa | (GeV/c)
    //
    // Px = -Pt * sin(phi0 + phi)
    // Py =  Pt * cos(phi0 + phi)
    // Pz =  Pt * tan(lambda)
    //

    double pt = fabs(m_pt);
    double px = - pt * sin(m_ac[1] + phi);
    double py =   pt * cos(m_ac[1] + phi);
    double pz =   pt * m_ac[4];

    return CLHEP::Hep3Vector(px, py, pz);
  }

  CLHEP::Hep3Vector
  TRGCDCHelix::momentum(double phi, CLHEP::HepSymMatrix& Em) const
  {
    //
    // Calculate momentum.
    //
    // Pt = | 1/kappa | (GeV/c)
    //
    // Px = -Pt * sin(phi0 + phi)
    // Py =  Pt * cos(phi0 + phi)
    // Pz =  Pt * tan(lambda)
    //

    double pt = fabs(m_pt);
    double px = - pt * sin(m_ac[1] + phi);
    double py =   pt * cos(m_ac[1] + phi);
    double pz =   pt * m_ac[4];

    if (m_matrixValid) Em = m_Ea.similarity(delMDelA(phi));
    else               Em = m_Ea;

    return CLHEP::Hep3Vector(px, py, pz);
  }

  CLHEP::HepLorentzVector
  TRGCDCHelix::momentum(double phi, double mass) const
  {
    //
    // Calculate momentum.
    //
    // Pt = | 1/kappa | (GeV/c)
    //
    // Px = -Pt * sin(phi0 + phi)
    // Py =  Pt * cos(phi0 + phi)
    // Pz =  Pt * tan(lambda)
    //
    // E  = sqrt( 1/kappa/kappa * (1+tan(lambda)*tan(lambda)) + mass*mass )

    double pt = fabs(m_pt);
    double px = - pt * sin(m_ac[1] + phi);
    double py =   pt * cos(m_ac[1] + phi);
    double pz =   pt * m_ac[4];
    double E  =   sqrt(pt * pt * (1. + m_ac[4] * m_ac[4]) + mass * mass);

    return CLHEP::HepLorentzVector(px, py, pz, E);
  }


  CLHEP::HepLorentzVector
  TRGCDCHelix::momentum(double phi, double mass, CLHEP::HepSymMatrix& Em) const
  {
    //
    // Calculate momentum.
    //
    // Pt = | 1/kappa | (GeV/c)
    //
    // Px = -Pt * sin(phi0 + phi)
    // Py =  Pt * cos(phi0 + phi)
    // Pz =  Pt * tan(lambda)
    //
    // E  = sqrt( 1/kappa/kappa * (1+tan(lambda)*tan(lambda)) + mass*mass )

    double pt = fabs(m_pt);
    double px = - pt * sin(m_ac[1] + phi);
    double py =   pt * cos(m_ac[1] + phi);
    double pz =   pt * m_ac[4];
    double E  =   sqrt(pt * pt * (1. + m_ac[4] * m_ac[4]) + mass * mass);

    if (m_matrixValid) Em = m_Ea.similarity(del4MDelA(phi, mass));
    else               Em = m_Ea;

    return CLHEP::HepLorentzVector(px, py, pz, E);
  }

  CLHEP::HepLorentzVector
  TRGCDCHelix::momentum(double phi,
                        double mass,
                        HepGeom::Point3D<double>&   x,
                        CLHEP::HepSymMatrix& Emx) const
  {

    //
    // Calculate momentum.
    //
    // Pt = | 1/kappa | (GeV/c)
    //
    // Px = -Pt * sin(phi0 + phi)
    // Py =  Pt * cos(phi0 + phi)
    // Pz =  Pt * tan(lambda)
    //
    // E  = sqrt( 1/kappa/kappa * (1+tan(lambda)*tan(lambda)) + mass*mass )

    double pt = fabs(m_pt);
    double px = - pt * sin(m_ac[1] + phi);
    double py =   pt * cos(m_ac[1] + phi);
    double pz =   pt * m_ac[4];
    double E  = sqrt(pt * pt * (1. + m_ac[4] * m_ac[4]) + mass * mass);

    x.setX(m_pivot.x() + m_ac[0] * m_cp + m_r * (m_cp - cos(m_ac[1] + phi)));
    x.setY(m_pivot.y() + m_ac[0] * m_sp + m_r * (m_sp - sin(m_ac[1] + phi)));
    x.setZ(m_pivot.z() + m_ac[3] - m_r * m_ac[4] * phi);

    if (m_matrixValid) Emx = m_Ea.similarity(del4MXDelA(phi, mass));
    else               Emx = m_Ea;

    return CLHEP::HepLorentzVector(px, py, pz, E);
  }


  const HepGeom::Point3D<double>&
  TRGCDCHelix::pivot(const HepGeom::Point3D<double>&   newPivot)
  {
#if defined(BELLE_DEBUG)
    try {
#endif
      const double& dr    = m_ac[0];
      const double& phi0  = m_ac[1];
      const double& kappa = m_ac[2];
      const double& dz    = m_ac[3];
      const double& tanl  = m_ac[4];

      double rdr = dr + m_r;
      double phi = fmod(phi0 + M_PI4, M_PI2);
      double csf0 = cos(phi);
      double snf0 = (1. - csf0) * (1. + csf0);
      snf0 = sqrt((snf0 > 0.) ? snf0 : 0.);
      if (phi > M_PI) snf0 = - snf0;

      double xc = m_pivot.x() + rdr * csf0;
      double yc = m_pivot.y() + rdr * snf0;
      double csf, snf;
      if (m_r != 0.0) {
        csf = (xc - newPivot.x()) / m_r;
        snf = (yc - newPivot.y()) / m_r;
        double anrm = sqrt(csf * csf + snf * snf);
        if (anrm != 0.0) {
          csf /= anrm;
          snf /= anrm;
          phi = atan2(snf, csf);
        } else {
          csf = 1.0;
          snf = 0.0;
          phi = 0.0;
        }
      } else {
        csf = 1.0;
        snf = 0.0;
        phi = 0.0;
      }
      double phid = fmod(phi - phi0 + M_PI8, M_PI2);
      if (phid > M_PI) phid = phid - M_PI2;
      double drp = (m_pivot.x() + dr * csf0 + m_r * (csf0 - csf) - newPivot.x())
                   * csf
                   + (m_pivot.y() + dr * snf0 + m_r * (snf0 - snf) - newPivot.y()) * snf;
      double dzp = m_pivot.z() + dz - m_r * tanl * phid - newPivot.z();

      CLHEP::HepVector ap(5);
      ap[0] = drp;
      ap[1] = fmod(phi + M_PI4, M_PI2);
      ap[2] = kappa;
      ap[3] = dzp;
      ap[4] = tanl;

      //    if (m_matrixValid) m_Ea.assign(delApDelA(ap) * m_Ea * delApDelA(ap).T());
      if (m_matrixValid) m_Ea = m_Ea.similarity(delApDelA(ap));

      m_a = ap;
      m_pivot = newPivot;

      //...Are these needed?...iw...
      updateCache();
      return m_pivot;
#if defined(BELLE_DEBUG)
    } catch (...) {
      m_helixValid = false;
      if (ms_throw_exception) throw invalidhelix;
    }
#endif
    return m_pivot;
  }

  void
  TRGCDCHelix::set(const HepGeom::Point3D<double>&   pivot,
                   const CLHEP::HepVector& a,
                   const CLHEP::HepSymMatrix& Ea)
  {
    m_pivot = pivot;
    m_a = a;
    m_Ea = Ea;
    m_matrixValid = true;
    m_helixValid = false;
    updateCache();
  }

  TRGCDCHelix&
  TRGCDCHelix::operator = (const TRGCDCHelix& i)
  {
    if (this == & i) return * this;

    m_bField = i.m_bField;
    m_alpha = i.m_alpha;
    m_pivot = i.m_pivot;
    m_a = i.m_a;
    m_Ea = i.m_Ea;
    m_matrixValid = i.m_matrixValid;
    m_helixValid = i.m_helixValid;

    m_center = i.m_center;
    m_cp = i.m_cp;
    m_sp = i.m_sp;
    m_pt = i.m_pt;
    m_r  = i.m_r;
    m_ac[0] = i.m_ac[0];
    m_ac[1] = i.m_ac[1];
    m_ac[2] = i.m_ac[2];
    m_ac[3] = i.m_ac[3];
    m_ac[4] = i.m_ac[4];

    return * this;
  }

  void
  TRGCDCHelix::updateCache(void)
  {
#if defined(BELLE_DEBUG)
    checkValid();
    if (m_helixValid) {
#endif
      //
      //   Calculate TRGCDCHelix center( xc, yc ).
      //
      //   xc = x0 + (dr + (alpha / kappa)) * cos(phi0)  (cm)
      //   yc = y0 + (dr + (alpha / kappa)) * sin(phi0)  (cm)
      //
      m_ac[0] = m_a[0];
      m_ac[1] = m_a[1];
      m_ac[2] = m_a[2];
      m_ac[3] = m_a[3];
      m_ac[4] = m_a[4];

      m_cp = cos(m_ac[1]);
      m_sp = sin(m_ac[1]);
      if (m_ac[2] != 0.0) {
        if (m_ac[2] == DBL_MAX || m_ac[2] == (-DBL_MAX)) {
          m_pt = m_r = 0;
          return;
        } else {
          m_pt = 1. / m_ac[2];
          m_r = m_alpha / m_ac[2];
        }
      } else {
        m_pt = (DBL_MAX);
        m_r = (DBL_MAX);
        return;
      }

      double x = m_pivot.x() + (m_ac[0] + m_r) * m_cp;
      double y = m_pivot.y() + (m_ac[0] + m_r) * m_sp;
      m_center.setX(x);
      m_center.setY(y);
      m_center.setZ(0.);
#if defined(BELLE_DEBUG)
    } else {
      m_ac[0] = m_a[0];
      m_ac[1] = m_a[1];
      m_ac[2] = m_a[2];
      m_ac[3] = m_a[3];
      m_ac[4] = m_a[4];

      m_cp = cos(m_ac[1]);
      m_sp = sin(m_ac[1]);
      if (m_ac[2] != 0.0) {
        if (m_ac[2] == DBL_MAX || m_ac[2] == (-DBL_MAX)) {
          m_pt = m_r = 0;
          return;
        } else {
          m_pt = 1. / m_ac[2];
          m_r = m_alpha / m_ac[2];
        }
      } else {
        m_pt = (DBL_MAX);
        m_r = (DBL_MAX);
        return;
      }

      double x = m_pivot.x() + (m_ac[0] + m_r) * m_cp;
      double y = m_pivot.y() + (m_ac[0] + m_r) * m_sp;
      m_center.setX(x);
      m_center.setY(y);
      m_center.setZ(0.);
    }
#endif
  }

  CLHEP::HepMatrix
  TRGCDCHelix::delApDelA(const CLHEP::HepVector& ap) const
  {
    //
    //   Calculate Jacobian (@ap/@a)
    //   CLHEP::HepVector ap is new helix parameters and a is old helix parameters.
    //

    CLHEP::HepMatrix dApDA(5, 5, 0);

    const double& dr    = m_ac[0];
    const double& phi0  = m_ac[1];
    const double& cpa   = m_ac[2];
//cnv    const double & dz    = m_ac[3];
    const double& tnl   = m_ac[4];

    double drp   = ap[0];
    double phi0p = ap[1];
//cnv    double cpap  = ap[2];
//cnv    double dzp   = ap[3];
//cnv    double tnlp  = ap[4];

    double rdr   = m_r + dr;
    double rdrpr;
    if ((m_r + drp) != 0.0) {
      rdrpr = 1. / (m_r + drp);
    } else {
      rdrpr = (DBL_MAX);
    }
    // double csfd  = cos(phi0)*cos(phi0p) + sin(phi0)*sin(phi0p);
    // double snfd  = cos(phi0)*sin(phi0p) - sin(phi0)*cos(phi0p);
    double csfd  = cos(phi0p - phi0);
    double snfd  = sin(phi0p - phi0);
    double phid  = fmod(phi0p - phi0 + M_PI8, M_PI2);
    if (phid > M_PI) phid = phid - M_PI2;

    dApDA[0][0]  =  csfd;
    dApDA[0][1]  =  rdr * snfd;
    if (cpa != 0.0) {
      dApDA[0][2]  = (m_r / cpa) * (1.0 - csfd);
    } else {
      dApDA[0][2]  = (DBL_MAX);
    }

    dApDA[1][0]  = - rdrpr * snfd;
    dApDA[1][1]  = rdr * rdrpr * csfd;
    if (cpa != 0.0) {
      dApDA[1][2]  = (m_r / cpa) * rdrpr * snfd;
    } else {
      dApDA[1][2]  = (DBL_MAX);
    }

    dApDA[2][2]  = 1.0;

    dApDA[3][0]  = m_r * rdrpr * tnl * snfd;
    dApDA[3][1]  = m_r * tnl * (1.0 - rdr * rdrpr * csfd);
    if (cpa != 0.0) {
      dApDA[3][2]  = (m_r / cpa) * tnl * (phid - m_r * rdrpr * snfd);
    } else {
      dApDA[3][2]  = (DBL_MAX);
    }
    dApDA[3][3]  = 1.0;
    dApDA[3][4]  = - m_r * phid;

    dApDA[4][4] = 1.0;

    return dApDA;
  }

  CLHEP::HepMatrix
  TRGCDCHelix::delXDelA(double phi) const
  {
    //
    //   Calculate Jacobian (@x/@a)
    //   CLHEP::HepVector a is helix parameters and phi is internal parameter
    //   which specifys the point to be calculated for Ex(phi).
    //

    CLHEP::HepMatrix dXDA(3, 5, 0);

    const double& dr      = m_ac[0];
    const double& phi0    = m_ac[1];
    const double& cpa     = m_ac[2];
//cnv    const double & dz      = m_ac[3];
    const double& tnl     = m_ac[4];

    double cosf0phi = cos(phi0 + phi);
    double sinf0phi = sin(phi0 + phi);

    dXDA[0][0]     = m_cp;
    dXDA[0][1]     = - dr * m_sp + m_r * (- m_sp + sinf0phi);
    if (cpa != 0.0) {
      dXDA[0][2]     = - (m_r / cpa) * (m_cp - cosf0phi);
    } else {
      dXDA[0][2] = (DBL_MAX);
    }
    // dXDA[0][3]     = 0.0;
    // dXDA[0][4]     = 0.0;

    dXDA[1][0]     = m_sp;
    dXDA[1][1]     = dr * m_cp + m_r * (m_cp - cosf0phi);
    if (cpa != 0.0) {
      dXDA[1][2]     = - (m_r / cpa) * (m_sp - sinf0phi);
    } else {
      dXDA[1][2] = (DBL_MAX);
    }
    // dXDA[1][3]     = 0.0;
    // dXDA[1][4]     = 0.0;

    // dXDA[2][0]     = 0.0;
    // dXDA[2][1]     = 0.0;
    if (cpa != 0.0) {
      dXDA[2][2]     = (m_r / cpa) * tnl * phi;
    } else {
      dXDA[2][2] = (DBL_MAX);
    }
    dXDA[2][3]     = 1.0;
    dXDA[2][4]     = - m_r * phi;

    return dXDA;
  }



  CLHEP::HepMatrix
  TRGCDCHelix::delMDelA(double phi) const
  {
    //
    //   Calculate Jacobian (@m/@a)
    //   CLHEP::HepVector a is helix parameters and phi is internal parameter.
    //   CLHEP::HepVector m is momentum.
    //

    CLHEP::HepMatrix dMDA(3, 5, 0);

    const double& phi0 = m_ac[1];
    const double& cpa  = m_ac[2];
    const double& tnl  = m_ac[4];

    double cosf0phi = cos(phi0 + phi);
    double sinf0phi = sin(phi0 + phi);

    double rho;
    if (cpa != 0.)rho = 1. / cpa;
    else rho = (DBL_MAX);

    double charge = 1.;
    if (cpa < 0.)charge = -1.;

    dMDA[0][1] = -fabs(rho) * cosf0phi;
    dMDA[0][2] = charge * rho * rho * sinf0phi;

    dMDA[1][1] = -fabs(rho) * sinf0phi;
    dMDA[1][2] = -charge * rho * rho * cosf0phi;

    dMDA[2][2] = -charge * rho * rho * tnl;
    dMDA[2][4] = fabs(rho);

    return dMDA;
  }


  CLHEP::HepMatrix
  TRGCDCHelix::del4MDelA(double phi, double mass) const
  {

    //
    //   Calculate Jacobian (@4m/@a)
    //   CLHEP::HepVector a  is helix parameters and phi is internal parameter.
    //   CLHEP::HepVector 4m is 4 momentum.
    //

    CLHEP::HepMatrix d4MDA(4, 5, 0);

    double phi0 = m_ac[1];
    double cpa  = m_ac[2];
    double tnl  = m_ac[4];

    double cosf0phi = cos(phi0 + phi);
    double sinf0phi = sin(phi0 + phi);

    double rho;
    if (cpa != 0.)rho = 1. / cpa;
    else rho = (DBL_MAX);

    double charge = 1.;
    if (cpa < 0.)charge = -1.;

    double E = sqrt(rho * rho * (1. + tnl * tnl) + mass * mass);

    d4MDA[0][1] = -fabs(rho) * cosf0phi;
    d4MDA[0][2] = charge * rho * rho * sinf0phi;

    d4MDA[1][1] = -fabs(rho) * sinf0phi;
    d4MDA[1][2] = -charge * rho * rho * cosf0phi;

    d4MDA[2][2] = -charge * rho * rho * tnl;
    d4MDA[2][4] = fabs(rho);

    if (cpa != 0.0 && E != 0.0) {
      d4MDA[3][2] = (-1. - tnl * tnl) / (cpa * cpa * cpa * E);
      d4MDA[3][4] = tnl / (cpa * cpa * E);
    } else {
      d4MDA[3][2] = (DBL_MAX);
      d4MDA[3][4] = (DBL_MAX);
    }
    return d4MDA;
  }


  CLHEP::HepMatrix
  TRGCDCHelix::del4MXDelA(double phi, double mass) const
  {

    //
    //   Calculate Jacobian (@4mx/@a)
    //   CLHEP::HepVector a  is helix parameters and phi is internal parameter.
    //   CLHEP::HepVector 4xm is 4 momentum and position.
    //

    CLHEP::HepMatrix d4MXDA(7, 5, 0);

    const double& dr      = m_ac[0];
    const double& phi0    = m_ac[1];
    const double& cpa     = m_ac[2];
//cnv    const double & dz      = m_ac[3];
    const double& tnl     = m_ac[4];

    double cosf0phi = cos(phi0 + phi);
    double sinf0phi = sin(phi0 + phi);

    double rho;
    if (cpa != 0.)rho = 1. / cpa;
    else rho = (DBL_MAX);

    double charge = 1.;
    if (cpa < 0.)charge = -1.;

    double E = sqrt(rho * rho * (1. + tnl * tnl) + mass * mass);

    d4MXDA[0][1] = - fabs(rho) * cosf0phi;
    d4MXDA[0][2] = charge * rho * rho * sinf0phi;

    d4MXDA[1][1] = - fabs(rho) * sinf0phi;
    d4MXDA[1][2] = - charge * rho * rho * cosf0phi;

    d4MXDA[2][2] = - charge * rho * rho * tnl;
    d4MXDA[2][4] = fabs(rho);

    if (cpa != 0.0 && E != 0.0) {
      d4MXDA[3][2] = (- 1. - tnl * tnl) / (cpa * cpa * cpa * E);
      d4MXDA[3][4] = tnl / (cpa * cpa * E);
    } else {
      d4MXDA[3][2] = (DBL_MAX);
      d4MXDA[3][4] = (DBL_MAX);
    }

    d4MXDA[4][0] = m_cp;
    d4MXDA[4][1] = - dr * m_sp + m_r * (- m_sp + sinf0phi);
    if (cpa != 0.0) {
      d4MXDA[4][2] = - (m_r / cpa) * (m_cp - cosf0phi);
    } else {
      d4MXDA[4][2] = (DBL_MAX);
    }

    d4MXDA[5][0] = m_sp;
    d4MXDA[5][1] = dr * m_cp + m_r * (m_cp - cosf0phi);
    if (cpa != 0.0) {
      d4MXDA[5][2] = - (m_r / cpa) * (m_sp - sinf0phi);

      d4MXDA[6][2] = (m_r / cpa) * tnl * phi;
    } else {
      d4MXDA[5][2] = (DBL_MAX);

      d4MXDA[6][2] = (DBL_MAX);
    }

    d4MXDA[6][3] = 1.;
    d4MXDA[6][4] = - m_r * phi;

    return d4MXDA;
  }

  void
  TRGCDCHelix::ignoreErrorMatrix(void)
  {
    m_matrixValid = false;
    m_Ea *= 0.;
  }

  void
  TRGCDCHelix::debugPrint(void) const
  {

    const double dr   = m_a[0];
    const double phi0 = m_a[1];
    const double cpa  = m_a[2];
    const double dz   = m_a[3];
    const double tnl  = m_a[4];
    if (ms_print_debug) {
      std::cout << "TRGCDCHelix::dr = " << dr << " phi0 = " << phi0 << " cpa = " << cpa
                << " dz = " << dz << " tnl = " << tnl << std::endl;
      std::cout << "       pivot = " << m_pivot << std::endl;
    }
  }

  void
  TRGCDCHelix::checkValid(void)
  {
    if (!ms_check_range) return;
//cnv  const double adr   = abs(m_a[0]);
    const double adr   = fabs(m_a[0]);
//cnv  const double acpa   = abs(m_a[2]);
    const double acpa   = fabs(m_a[2]);
    if (!(adr >= ms_amin[0] && adr <= ms_amax[0])) {
      m_helixValid = false;
    } else if (!(acpa >= ms_amin[2] && acpa <= ms_amax[2])) {
      m_helixValid = false;
    } else {
      m_helixValid = true;
    }
    if (!m_helixValid) {
      if (m_a[0] != 0.0 || m_a[1] != 0.0 || m_a[2] != 0.0 ||
          m_a[3] != 0.0 || m_a[4] != 0.0)
        std::cout << "something wrong" << std::endl;
    }

  }

} // namespace Belle2

