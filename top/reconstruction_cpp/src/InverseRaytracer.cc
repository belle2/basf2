/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/InverseRaytracer.h>
#include <top/reconstruction_cpp/func.h>
#include <framework/logging/Logger.h>
#include <cmath>

namespace Belle2 {
  namespace TOP {

    double InverseRaytracer::s_maxLen = 10000;

    InverseRaytracer::CerenkovAngle::CerenkovAngle(double cosTheta):
      cosThc(cosTheta), sinThc(sqrt(1 - cosTheta * cosTheta))
    {}


    InverseRaytracer::Solution::Solution(double cfi, double sfi):
      cosFic(cfi), sinFic(sfi)
    {}


    void InverseRaytracer::Solution::setDirection(const CerenkovAngle& cer, const TOPTrack::TrackAngles& trk)
    {
      double a = trk.cosTh * cer.sinThc * cosFic + trk.sinTh * cer.cosThc;
      double b = cer.sinThc * sinFic;
      kx = a * trk.cosFi - b * trk.sinFi;
      ky = a * trk.sinFi + b * trk.cosFi;
      kz = trk.cosTh * cer.cosThc - trk.sinTh * cer.sinThc * cosFic;
    }

    void InverseRaytracer::Solution::setTotalReflStatus(double A, double B, double cosTotal)
    {
      totRefl = (abs(kx) < cosTotal or abs(xD) < A / 2) and (abs(ky) < cosTotal or abs(yB) < B / 2);
    }

    void InverseRaytracer::Solution::setTotalReflStatus(double A, double B, double cosTotal, double Kx, double Ky)
    {
      bool beforeMirror = (abs(kx) < cosTotal or Nxm == 0) and (abs(ky) < cosTotal or Nym == 0);
      bool afterMirror = (abs(Kx) < cosTotal or abs(xD) < A / 2) and (abs(Ky) < cosTotal or abs(yB) < B / 2);
      totRefl = beforeMirror and afterMirror;
    }

    bool InverseRaytracer::Solution::getStatus() const
    {
      return (len > 0 and len < s_maxLen and totRefl);
    }

    void InverseRaytracer::clear() const
    {
      m_solutions[0].clear();
      m_solutions[1].clear();
      m_ok[0] = false;
      m_ok[1] = false;
    }

    int InverseRaytracer::solveDirect(double xD, double zD,
                                      const TOPTrack::AssumedEmission& assumedEmission,
                                      const CerenkovAngle& cer, double step) const
    {
      const auto& emiPoint = assumedEmission.position;
      const auto& trk = assumedEmission.trackAngles;
      bool first = m_solutions[0].empty();
      if (first) {
        m_emiPoint = emiPoint;
        m_cer = cer;
        m_trk = trk;
      }

      double dx = xD - emiPoint.X();
      double dz = zD - emiPoint.Z();

      double dxdz = dx / dz;
      if (not solve(dxdz, cer, trk)) return c_NoEquationSolution;

      for (int i = 0; i < 2; i++) {
        auto& sol = m_solutions[i].back();
        sol.xD = xD;
        sol.zD = zD;
        sol.step = step;
        sol.len = dz / sol.kz;
        double dydz = sol.ky / sol.kz;
        sol.yD = emiPoint.Y() + dydz * dz;
        sol.yB = emiPoint.Y() + dydz * (m_prism.zR - emiPoint.Z());
        if (first) {
          m_ok[i] = true;
          sol.setTotalReflStatus(m_bars[0].A, m_bars[0].B, m_cosTotal);
        }
        if (not sol.getStatus()) m_ok[i] = false;
      }

      if (not(m_ok[0] or m_ok[1])) return c_NoPhysicsSolution;

      return m_solutions[0].size() - 1;
    }


    int InverseRaytracer::solveReflected(double xD, double zD, int Nxm, double xmMin, double xmMax,
                                         const TOPTrack::AssumedEmission& assumedEmission,
                                         const CerenkovAngle& cer, double step) const
    {
      const auto& emiPoint = assumedEmission.position;
      const auto& trk = assumedEmission.trackAngles;
      bool first = m_solutions[0].empty();
      if (first) {
        m_emiPoint = emiPoint;
        m_cer = cer;
        m_trk = trk;
      }

      double A = m_bars[0].A;
      double B = m_bars[0].B;

      double xM = 0;
      double zM = 0;
      double dxdz = 0;
      if (Nxm % 2 == 0) {
        double xE = func::unfold(emiPoint.X(), -Nxm, A);
        double zE = emiPoint.Z();
        bool ok = findReflectionPoint(xE, zE, xD, zD, xmMin, xmMax, xM, zM, dxdz);
        if (not ok) return c_NoReflectionPoint;
      } else {
        double xE = func::unfold(emiPoint.X(), Nxm, A);
        double zE = emiPoint.Z();
        bool ok = findReflectionPoint(xE, zE, xD, zD, xmMin, xmMax, xM, zM, dxdz);
        if (not ok) return c_NoReflectionPoint;
        dxdz = -dxdz;
      }

      bool ok = solve(dxdz, cer, trk);
      if (not ok) return c_NoEquationSolution;

      double normX = (xM - m_mirror.xc) / m_mirror.R;
      double normZ = (zM - m_mirror.zc) / m_mirror.R;

      for (int i = 0; i < 2; i++) {
        auto& sol = m_solutions[i].back();
        sol.xD = xD;
        sol.zD = zD;
        sol.step = step;
        sol.Nxm = Nxm;

        double len = (zM - emiPoint.Z()) / sol.kz;
        if (len < 0) {
          m_ok[i] = false;
          continue;
        }
        sol.len = len;
        int Nym = lround((emiPoint.Y() + len * sol.ky) / B);

        double kx = func::unfold(sol.kx, Nxm);
        double ky = func::unfold(sol.ky, Nym);
        double kz = sol.kz;
        double s = 2 * (kx * normX + kz * normZ);
        kx -= s * normX;
        kz -= s * normZ;

        len = (zD - zM) / kz;
        if (len < 0) {
          m_ok[i] = false;
          continue;
        }
        sol.len += len;

        sol.yD = m_mirror.yc + len * ky;
        sol.yB = m_mirror.yc + (m_prism.zR - zM) / kz * ky;
        sol.Nym = Nym;

        if (first) {
          m_ok[i] = true;
          sol.setTotalReflStatus(A, B, m_cosTotal, kx, ky);
        }
        if (not sol.getStatus()) m_ok[i] = false;
      }

      if (not(m_ok[0] or m_ok[1])) return c_NoPhysicsSolution;

      return m_solutions[0].size() - 1;
    }


    int InverseRaytracer::solveForReflectionPoint(double xM, int Nxm,
                                                  const TOPTrack::AssumedEmission& assumedEmission,
                                                  const CerenkovAngle& cer) const
    {
      const auto& emiPoint = assumedEmission.position;
      const auto& trk = assumedEmission.trackAngles;
      bool first = m_solutions[0].empty();
      if (first) {
        m_emiPoint = emiPoint;
        m_cer = cer;
        m_trk = trk;
      }

      double A = m_bars[0].A;
      double B = m_bars[0].B;

      double zM = m_mirror.zc + sqrt(pow(m_mirror.R, 2) - pow(xM - m_mirror.xc, 2));
      double dx = func::unfold(xM, Nxm, A) - emiPoint.X();
      double dz = zM - emiPoint.Z();
      double dxdz = dx / dz;

      bool ok = solve(dxdz, cer, trk);
      if (not ok) return c_NoEquationSolution;

      double normX = (xM - m_mirror.xc) / m_mirror.R;
      double normZ = (zM - m_mirror.zc) / m_mirror.R;
      double zD = m_prism.zD;

      for (int i = 0; i < 2; i++) {
        auto& sol = m_solutions[i].back();
        sol.zD = zD;
        sol.Nxm = Nxm;

        double len = (zM - emiPoint.Z()) / sol.kz;
        if (len < 0) {
          m_ok[i] = false;
          continue;
        }
        sol.len = len;
        int Nym = lround((emiPoint.Y() + len * sol.ky) / B);

        double kx = func::unfold(sol.kx, Nxm);
        double ky = func::unfold(sol.ky, Nym);
        double kz = sol.kz;
        double s = 2 * (kx * normX + kz * normZ);
        kx -= s * normX;
        kz -= s * normZ;

        len = (zD - zM) / kz;
        if (len < 0) {
          m_ok[i] = false;
          continue;
        }
        sol.len += len;

        sol.xD = xM + len * kx;
        sol.yD = m_mirror.yc + len * ky;
        sol.yB = m_mirror.yc + (m_prism.zR - zM) / kz * ky;
        sol.Nym = Nym;
        m_ok[i] = sol.getStatus();
      }

      if (not(m_ok[0] or m_ok[1])) return c_NoPhysicsSolution;

      return m_solutions[0].size() - 1;
    }

    bool InverseRaytracer::isNymDifferent() const
    {
      for (int i = 0; i < 2; i++) {
        const auto& solutions = m_solutions[i];
        if (m_ok[i] and solutions.back().Nym != solutions.front().Nym) return true;
      }
      return false;
    }

    bool InverseRaytracer::solve(double dxdz, const CerenkovAngle& cer, const TOPTrack::TrackAngles& trk) const
    {
      double a = (dxdz * trk.cosTh - trk.cosFi * trk.sinTh) * cer.cosThc;
      double b = (dxdz * trk.sinTh + trk.cosFi * trk.cosTh) * cer.sinThc;
      double d = trk.sinFi * cer.sinThc;

      double B = b * b + d * d;
      if (B == 0) return false;

      if (d == 0) {
        double cfic = a / b;
        if (abs(cfic) > 1) return false;
        double sfic = sqrt(1 - cfic * cfic);
        m_solutions[0].push_back(Solution(cfic, sfic));
        m_solutions[1].push_back(Solution(cfic, -sfic));
      } else {
        double D = B - a * a;
        if (D < 0) return false;
        D = d * sqrt(D);
        double ab = a * b;
        double cfic = (ab + D) / B;
        m_solutions[0].push_back(Solution(cfic, (b * cfic - a) / d));
        cfic = (ab - D) / B;
        m_solutions[1].push_back(Solution(cfic, (b * cfic - a) / d));
      }
      m_solutions[0].back().setDirection(cer, trk);
      m_solutions[1].back().setDirection(cer, trk);

      return true;
    }


    double InverseRaytracer::getDeltaXE(double x, double xe, double ze, double xd, double zd) const
    {
      double z = sqrt(1 - x * x);
      double kx = (x - xd);
      double kz = (z - zd);
      double s = 2 * (kx * x + kz * z);
      kx -= s * x;
      kz -= s * z;

      return x + (ze - z) * kx / kz - xe;
    }


    bool InverseRaytracer::findReflectionPoint(double xE, double zE, double xD, double zD,
                                               double xmMin, double xmMax,
                                               double& xM, double& zM, double& dxdz) const
    {
      double xe = (xE - m_mirror.xc) / m_mirror.R;
      double ze = (zE - m_mirror.zc) / m_mirror.R;
      double xd = (xD - m_mirror.xc) / m_mirror.R;
      double zd = (zD - m_mirror.zc) / m_mirror.R;

      double x1 = (xmMin - m_mirror.xc) / m_mirror.R;
      double y1 = getDeltaXE(x1, xe, ze, xd, zd);

      double x2 = (xmMax - m_mirror.xc) / m_mirror.R;
      double y2 = getDeltaXE(x2, xe, ze, xd, zd);

      if (y1 * y2 > 0) return false; // no (single) solution

      for (int i = 0; i < 20; i++) {
        double x = (x1 + x2) / 2;
        double y = getDeltaXE(x, xe, ze, xd, zd);
        if (y * y1 < 0) {
          x2 = x;
        } else {
          x1 = x;
          y1 = y;
        }
      }
      double x = (x1 + x2) / 2;
      double z = sqrt(1 - x * x);
      xM = x * m_mirror.R + m_mirror.xc;
      zM = z * m_mirror.R + m_mirror.zc;

      double kx = (x - xd);
      double kz = (z - zd);
      double s = 2 * (kx * x + kz * z);
      kx -= s * x;
      kz -= s * z;
      dxdz = kx / kz;

      return true;
    }


    PhotonState InverseRaytracer::getReconstructedPhoton(const Solution& sol, double DFic) const
    {
      if (DFic == 0) {
        return PhotonState(m_emiPoint, sol.kx, sol.ky, sol.kz);
      } else {
        double cosDFic = cos(DFic);
        double sinDFic = sin(DFic);
        double cosFic = sol.cosFic * cosDFic - sol.sinFic * sinDFic;
        double sinFic = sol.sinFic * cosDFic + sol.cosFic * sinDFic;
        double a = m_trk.cosTh * m_cer.sinThc * cosFic + m_trk.sinTh * m_cer.cosThc;
        double b = m_cer.sinThc * sinFic;
        double kx = a * m_trk.cosFi - b * m_trk.sinFi;
        double ky = a * m_trk.sinFi + b * m_trk.cosFi;
        double kz = m_trk.cosTh * m_cer.cosThc - m_trk.sinTh * m_cer.sinThc * cosFic;
        return PhotonState(m_emiPoint, kx, ky, kz);
      }
    }


  } //TOP
} //Belle2

