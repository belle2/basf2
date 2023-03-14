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
#include <cmath>

using namespace std;
using namespace ROOT::Math;

namespace Belle2 {
  namespace TOP {

    void HelixSwimmer::set(const XYZPoint& position, const XYZVector& momentum, double charge, double Bz)
    {
      double p = momentum.R();
      double pT = momentum.Rho();
      double b = -Bz * charge * Const::speedOfLight;
      if (std::abs(b / Unit::T) > pT) { // helix for R < 100 m
        m_R = pT / std::abs(b);
        m_omega = b / p;
        m_kz = momentum.Z() / p;
        m_phi0 = momentum.Phi() - copysign(M_PI / 2, m_omega);
        m_xc = position.X() - m_R * cos(m_phi0);
        m_yc = position.Y() - m_R * sin(m_phi0);
        m_z0 = position.Z();
        m_T0 = 2 * M_PI / std::abs(m_omega);
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

    XYZPoint HelixSwimmer::getPosition(double length) const
    {
      if (m_omega != 0) {
        double phi = m_phi0 + m_omega * length;
        XYZPoint vec(m_xc + m_R * cos(phi), m_yc + m_R * sin(phi), m_z0 + m_kz * length);
        return m_transformInv * vec;
      } else {
        XYZPoint vec(x0 + kx * length, y0 + ky * length, z0 + kz * length);
        return m_transformInv * vec;
      }
    }

    XYZVector HelixSwimmer::getDirection(double length) const
    {
      if (m_omega != 0) {
        double phi = m_phi0 + m_omega * length;
        double k_T = m_omega * m_R;
        XYZVector vec(-k_T * sin(phi), k_T * cos(phi), m_kz);
        return m_transformInv * vec;
      } else {
        XYZVector vec(kx, ky, kz);
        return m_transformInv * vec;
      }
    }

    double HelixSwimmer::getDistanceToPlane(const XYZPoint& point, const XYZVector& normal) const
    {
      if (m_omega != 0) {
        auto r = point - XYZPoint(m_xc, m_yc, m_z0);
        double phi = normal.Phi();
        double s = r.Dot(normal);
        if (std::abs(s) > m_R) return std::numeric_limits<double>::quiet_NaN(); // no solution

        double t = shortestDistance(s / m_R, phi);
        double v = m_kz * normal.Z();
        if (v == 0) return t;

        double t_prev = t;
        double dt_prev = 0;
        for (int i = 0; i < 100; i++) {
          double cosAlpha = (s - v * t) / m_R;
          if (std::abs(cosAlpha) > 1) {
            return std::numeric_limits<double>::quiet_NaN(); // no solution
          }
          t = shortestDistance(cosAlpha, phi);
          double dt = t - t_prev;
          if (dt == 0 or (dt + dt_prev) == 0) return t;
          t_prev = t;
          dt_prev = dt;
        }
        if (std::abs(dt_prev) < 1e-6) {
          B2DEBUG(20, "TOP::HelixSwimmer::getDistanceToPlane: not converged"
                  << LogVar("v", v) << LogVar("dt", dt_prev));
          return t;
        } else {
          B2DEBUG(20, "TOP::HelixSwimmer::getDistanceToPlane: not converged"
                  << LogVar("v", v) << LogVar("dt", dt_prev));
          return std::numeric_limits<double>::quiet_NaN();
        }
      } else {
        auto r = point - XYZPoint(x0, y0, z0);
        auto v = XYZVector(kx, ky, kz);
        return r.Dot(normal) / v.Dot(normal);
      }
    }

  } //TOP
} //Belle2

