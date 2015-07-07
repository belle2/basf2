/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/cdc/modules/CDCStraightLineFitter/StraightLine.h>

namespace Belle2 {
  namespace CDCTB {

    bool StraightLine::crossPlane(const TVector3& point, const TVector3& normal)
    {
      double path = -((m_point - point) * normal) / (m_dir * normal);
      if (!std::isfinite(path)) return false;
      m_path = path;
      return true;
    }


    bool StraightLine::closestApproach(StraightLine& line)
    {

      TVector3 diff = m_point - line.getReferencePoint();
      double a = line.getDirection() * m_dir;
      double b = diff * m_dir;
      double c = diff * line.getDirection();
      double d = a * a - 1;

      double lam = (b - a * c) / d;
      if (!std::isfinite(lam)) return false;

      double mu = (a * b - c) / d;
      if (!std::isfinite(mu)) return false;

      m_path = lam;
      line.setPath(mu);
      return true;

    }

    double StraightLine::getLengthInCell(double R1, double R2,
                                         double phi1, double phi2) const
    {

      TVector2 r0 = m_point.XYvector();
      TVector2 dir = m_dir.XYvector();
      TVector2 r1, r2, r3, r4;
      r1.SetMagPhi(R1, phi1);
      r2.SetMagPhi(R1, phi2);
      r3.SetMagPhi(R2, phi1);
      r4.SetMagPhi(R2, phi2);

      std::vector<TVector2> crossPoints;
      if (appendCrossPoint(crossPoints, r1, r2, r0, dir) == 2) {
        return (crossPoints[1] - crossPoints[0]).Mod() / sin(m_dir.Theta());
      }
      if (appendCrossPoint(crossPoints, r3, r4, r0, dir) == 2) {
        return (crossPoints[1] - crossPoints[0]).Mod() / sin(m_dir.Theta());
      }
      if (appendCrossPoint(crossPoints, r1, r3, r0, dir) == 2) {
        return (crossPoints[1] - crossPoints[0]).Mod() / sin(m_dir.Theta());
      }
      if (appendCrossPoint(crossPoints, r2, r4, r0, dir) == 2) {
        return (crossPoints[1] - crossPoints[0]).Mod() / sin(m_dir.Theta());
      }

      return 0;
    }


    int StraightLine::appendCrossPoint(std::vector<TVector2>& crossPoints,
                                       const TVector2& r1, const TVector2& r2,
                                       const TVector2& r0, const TVector2& dir) const
    {
      double dx10 = r1.X() - r0.X();
      double dy10 = r1.Y() - r0.Y();
      double dx12 = r1.X() - r2.X();
      double dy12 = r1.Y() - r2.Y();
      double kx = dir.X();
      double ky = dir.Y();
      double lambda = (dx10 * ky - dy10 * kx) / (dx12 * ky - dy12 * kx);
      if (lambda > 0 and lambda < 1) {
        crossPoints.push_back(r1 + lambda * (r2 - r1));
      }
      return crossPoints.size();
    }



  } // CDCTB
} // Belle2

