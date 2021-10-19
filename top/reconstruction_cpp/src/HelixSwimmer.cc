/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/HelixSwimmer.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <limits>

using namespace std;

namespace Belle2 {
  namespace TOP {

    void HelixSwimmer::set(const TVector3& position, const TVector3& momentum, double charge, double Bz)
    {
      double p = momentum.Mag();
      double pT = momentum.Perp();
      double b = -Bz * charge * Const::speedOfLight;
      if (abs(b / Unit::T) > pT) { // helix for R < 100 m
        m_R = pT / abs(b);
        m_omega = b / p;
        m_kz = momentum.Z() / p;
        m_phi0 = momentum.Phi() - copysign(M_PI / 2, m_omega);
        m_xc = position.X() - m_R * cos(m_phi0);
        m_yc = position.Y() - m_R * sin(m_phi0);
        m_z0 = position.Z();
        m_T0 = 2 * M_PI / abs(m_omega);
      } else { // straight line
        m_omega = 0; // distinguisher between straight line and helix
        x0 = position.X();
        y0 = position.Y();
        z0 = position.Z();
        kx = momentum.X() / p;
        ky = momentum.Y() / p;
        kz = momentum.Z() / p;
      }
    }

    void HelixSwimmer::setTransformation(const TRotation& rotation, const TVector3& translation)
    {
      m_rotationInv = rotation.Inverse();
      m_translation = translation;
    }

    void HelixSwimmer::moveReferencePosition(double length)
    {
      if (m_omega != 0) {
        m_phi0 += m_omega * length;
        m_z0 += m_kz * length;
      } else {
        x0 += kx * length;
        y0 += ky * length;
        z0 += kz * length;
      }
    }

    TVector3 HelixSwimmer::getPosition(double length) const
    {
      if (m_omega != 0) {
        double phi = m_phi0 + m_omega * length;
        TVector3 vec(m_xc + m_R * cos(phi), m_yc + m_R * sin(phi), m_z0 + m_kz * length);
        return m_rotationInv * (vec - m_translation);
      } else {
        TVector3 vec(x0 + kx * length, y0 + ky * length, z0 + kz * length);
        return m_rotationInv * (vec - m_translation);
      }
    }

    TVector3 HelixSwimmer::getDirection(double length) const
    {
      if (m_omega != 0) {
        double phi = m_phi0 + m_omega * length;
        double k_T = m_omega * m_R;
        TVector3 vec(-k_T * sin(phi), k_T * cos(phi), m_kz);
        return m_rotationInv * vec;
      } else {
        TVector3 vec(kx, ky, kz);
        return m_rotationInv * vec;
      }
    }

    double HelixSwimmer::getDistanceToPlane(const TVector3& point, const TVector3& normal) const
    {
      if (m_omega != 0) {
        auto r = point - TVector3(m_xc, m_yc, m_z0);
        double phi = normal.Phi();
        double s = r * normal;
        if (abs(s) > m_R) return std::numeric_limits<double>::quiet_NaN(); // no solution

        double t = shortestDistance(s / m_R, phi);
        double v = m_kz * normal.Z();
        if (v == 0) return t;

        double t_prev = t;
        double dt_prev = 0;
        for (int i = 0; i < 100; i++) {
          double cosAlpha = (s - v * t) / m_R;
          if (abs(cosAlpha) > 1) {
            return std::numeric_limits<double>::quiet_NaN(); // no solution
          }
          t = shortestDistance(cosAlpha, phi);
          double dt = t - t_prev;
          if (dt == 0 or (dt + dt_prev) == 0) return t;
          t_prev = t;
          dt_prev = dt;
        }
        if (abs(dt_prev) < 1e-6) {
          B2DEBUG(20, "TOP::HelixSwimmer::getDistanceToPlane: not converged"
                  << LogVar("v", v) << LogVar("dt", dt_prev));
          return t;
        } else {
          B2DEBUG(20, "TOP::HelixSwimmer::getDistanceToPlane: not converged"
                  << LogVar("v", v) << LogVar("dt", dt_prev));
          return std::numeric_limits<double>::quiet_NaN();
        }
      } else {
        auto r = point - TVector3(x0, y0, z0);
        auto v = TVector3(kx, ky, kz);
        return (r * normal) / (v * normal);
      }
    }

  } //TOP
} //Belle2

