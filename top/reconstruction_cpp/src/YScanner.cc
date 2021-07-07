/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/YScanner.h>
#include <top/reconstruction_cpp/func.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <map>

namespace Belle2 {
  namespace TOP {

    int YScanner::s_maxReflections = 16;

    YScanner::Derivatives::Derivatives(const InverseRaytracer::Solution& sol,
                                       const InverseRaytracer::Solution& sol_dx,
                                       const InverseRaytracer::Solution& sol_de,
                                       const InverseRaytracer::Solution& sol_dL)
    {
      if (sol_dx.step == 0) B2ERROR("TOP::YScanner::Derivatives: step (dx) is zero");
      if (sol_de.step == 0) B2ERROR("TOP::YScanner::Derivatives: step (de) is zero");
      if (sol_dL.step == 0) B2ERROR("TOP::YScanner::Derivatives: step (dL) is zero");

      dLen_dx = dLen_d(sol, sol_dx);
      dLen_de = dLen_d(sol, sol_de);
      dLen_dL = dLen_d(sol, sol_dL);

      dyD_dx = dyD_d(sol, sol_dx);
      dyD_de = dyD_d(sol, sol_de);
      dyD_dL = dyD_d(sol, sol_dL);

      dyB_dx = dyB_d(sol, sol_dx);
      dyB_de = dyB_d(sol, sol_de);
      dyB_dL = dyB_d(sol, sol_dL);

      dFic_dx = dFic_d(sol, sol_dx);
      dFic_de = dFic_d(sol, sol_de);
      dFic_dL = dFic_d(sol, sol_dL);
    }


    YScanner::YScanner(int moduleID, unsigned N): RaytracerBase(moduleID, c_Unified, c_SemiLinear),
      m_pixelPositions(PixelPositions(moduleID)),
      m_pixelMasks(PixelMasks(moduleID)),
      m_pixelEfficiencies(PixelEfficiencies(moduleID))
    {
      if (N < 2) {
        B2FATAL("TOP::YScanner: N must be > 1");
        return;
      }

      // set the table of nominal photon detection efficiencies (incl. wavelength filter)

      const auto* topgp = TOPGeometryPar::Instance();
      const auto* geo = topgp->getGeometry();
      auto qe = geo->getNominalQE(); // get a copy
      qe.applyFilterTransmission(geo->getWavelengthFilter());

      double minE = TOPGeometryPar::c_hc / qe.getMaxLambda();
      double maxE = TOPGeometryPar::c_hc / qe.getMinLambda();
      if (minE >= maxE) {
        B2FATAL("TOP::YScanner: quantum efficiency found zero for all wavelengths");
        return;
      }
      m_efficiency.set(minE, (maxE - minE) / (N - 1));

      const auto& tdc = geo->getNominalTDC();
      for (unsigned i = 0; i < N; i++) {
        double e = m_efficiency.getX(i);
        double lambda = TOPGeometryPar::c_hc / e;
        double effi = qe.getEfficiency(lambda) * tdc.getEfficiency();
        m_efficiency.entries.push_back(TableEntry(effi, e, e * e));
      }

      // set cosine of total reflection angle using photon mean energy for beta = 1

      double s = 0;
      double se = 0;
      double see = 0;
      for (const auto& entry : m_efficiency.entries) {
        double e = entry.x;
        double p = entry.y * (1 - 1 / pow(topgp->getPhaseIndex(e), 2));
        s += p;
        se += p * e;
        see += p * e * e;
      }
      if (s == 0) return;
      m_meanE0 = se / s;
      m_rmsE0 = sqrt(see / s - m_meanE0 * m_meanE0);
      m_cosTotal = sqrt(1 - 1 / pow(topgp->getPhaseIndex(m_meanE0), 2));
    }


    void YScanner::clear() const
    {
      m_momentum = 0;
      m_beta = 0;
      m_length = 0;
      m_numPhotons = 0;
      m_meanE = 0;
      m_rmsE = 0;
      m_sigmaScat = 0;
      m_aboveThreshold = false;
      m_energyDistribution.clear();
      m_quasyEnergyDistribution.clear();
      m_results.clear();
      m_scanDone = false;
    }


    void YScanner::prepare(double momentum, double beta, double length) const
    {
      clear();

      m_momentum = momentum;
      m_beta = beta;
      m_length = length;

      // set photon energy distribution, and the mean and r.m.s of photon energy

      auto area = setEnergyDistribution(beta);
      if (area == 0) return;

      // set number of Cerenkov photons per azimuthal angle

      m_numPhotons = 370 * area / (2 * M_PI);

      // set photon energy distribution convoluted with multiple scattering

      const double radLength = 12.3; // quartz radiation length [cm]
      double thetaScat = 13.6e-3 / beta / momentum * sqrt(length / 2 / radLength); // r.m.s of multiple scattering angle

      const auto* topgp = TOPGeometryPar::Instance();
      double n = topgp->getPhaseIndex(m_meanE);
      if (beta * n < 1) {
        B2ERROR("TOP::YScanner::prepare: beta * n < 1 ==> must be a bug!");
        return;
      }
      double dndE = topgp->getPhaseIndexDerivative(m_meanE);
      double dEdTheta = n * sqrt(pow(beta * n, 2) - 1) / dndE;
      m_sigmaScat = abs(thetaScat * dEdTheta); // r.m.s of multiple scattering angle converted to photon energy

      double step = m_energyDistribution.step;
      int ng = lround(3 * m_sigmaScat / step);
      std::vector<double> gaus;
      for (int i = 0; i <= ng; i++) {
        double x = step * i / m_sigmaScat;
        gaus.push_back(exp(-0.5 * x * x));
      }

      m_quasyEnergyDistribution.set(m_energyDistribution.getX(-ng), step);
      int N = m_energyDistribution.entries.size();
      double sum = 0;
      for (int k = -ng; k < N + ng; k++) {
        double s = 0;
        double se = 0;
        double see = 0;
        for (int i = -ng; i <= ng; i++) {
          double p = gaus[abs(i)] * m_energyDistribution.getY(k - i);
          double e = m_energyDistribution.getX(k - i);
          s += p;
          se += p * e;
          see += p * e * e;
        }
        if (s > 0) {
          se /= s;
          see /= s;
        }
        m_quasyEnergyDistribution.entries.push_back(TableEntry(s, se, see));
        sum += s;
      }
      for (auto& entry : m_quasyEnergyDistribution.entries) entry.y /= sum;

      m_aboveThreshold = true;
    }


    double YScanner::setEnergyDistribution(double beta) const
    {
      const auto* topgp = TOPGeometryPar::Instance();

      m_energyDistribution.set(m_efficiency);

      double s = 0;
      double se = 0;
      double see = 0;
      for (const auto& entry : m_efficiency.entries) {
        double e = entry.x;
        double p = std::max(entry.y * (1 - 1 / pow(beta * topgp->getPhaseIndex(e), 2)), 0.0);
        double ee = entry.xsq;
        m_energyDistribution.entries.push_back(TableEntry(p, e, ee));
        s += p;
        se += p * e;
        see += p * ee;
      }
      if (s == 0) return 0;

      for (auto& entry : m_energyDistribution.entries) entry.y /= s;

      m_meanE = se / s;
      m_rmsE = sqrt(see / s - m_meanE * m_meanE);

      return s * m_energyDistribution.step;
    }


    void YScanner::expand(unsigned col, double yB, double dydz, const Derivatives& D, bool doScan) const
    {
      m_results.clear();

      if (D.dyB_de == 0) return;

      double minE = m_quasyEnergyDistribution.getXmin();
      double maxE = m_quasyEnergyDistribution.getXmax();
      double pixDx  = m_pixelPositions.get(col + 1).Dx;
      double dely = (abs(D.dyB_dL) * m_length + abs(D.dyB_dx) * pixDx) / 2;
      double y1 = yB - dely;
      double y2 = yB + dely;
      if (D.dyB_de > 0) {
        y1 += D.dyB_de * (minE - m_meanE);
        y2 += D.dyB_de * (maxE - m_meanE);
      } else {
        y1 += D.dyB_de * (maxE - m_meanE);
        y2 += D.dyB_de * (minE - m_meanE);
      }
      double B = m_bars.front().B;
      int j1  = lround(y1 / B);
      int j2  = lround(y2 / B) + 1;

      if (doScan and j2 - j1 <= s_maxReflections) {
        scan(col, yB, dydz, D, j1, j2);
        m_scanDone = true;
      } else {
        merge(col, dydz, j1, j2);
        m_scanDone = false;
      }
    }


    void YScanner::scan(unsigned col, double yB, double dydz, const Derivatives& D, int j1, int j2) const
    {

      std::map<int, EnergyMask*> masks;
      for (unsigned row = 0; row < m_pixelPositions.getNumPixelRows(); row++) {

        int pixelID = m_pixelPositions.pixelID(row, col);
        if (not m_pixelMasks.isActive(pixelID)) continue;

        const auto& pixel = m_pixelPositions.get(pixelID);
        std::vector<PixelProjection> projections[2];
        PixelProjection proj;
        for (size_t k = 0; k < m_prism.unfoldedWindows.size(); k++) {
          projectPixel(pixel.yc, pixel.Dy, k, dydz, proj); // for even j
          if (proj.Dy > 0) projections[0].push_back(proj);
          projectPixel(pixel.yc, pixel.Dy, k, -dydz, proj); // for odd j
          proj.yc = -proj.yc;
          if (proj.Dy > 0) projections[1].push_back(proj);
        }
        if (projections[0].empty() and projections[1].empty()) continue;

        for (unsigned k = 0; k < 2; k++) {
          std::sort(projections[k].begin(), projections[k].end());
          for (auto& projection : projections[k]) {
            int iDy = lround(projection.Dy * 1000);
            auto& mask = masks[iDy];
            if (not mask) {
              double Dy = projection.Dy;
              double step = m_quasyEnergyDistribution.step;
              mask = new EnergyMask(D.dyB_de, D.dyB_dL, D.dyB_dx, Dy, m_length, pixel.Dx, step);
            }
            projection.mask = mask;
          }
        }

        double Ecp_old = 0;
        double wid_old = 1000;
        m_results.push_back(Result(pixelID));
        for (int j = j1; j < j2; j++) {
          double ybar = j * m_bars.front().B - yB;
          for (const auto& projection : projections[abs(j) % 2]) {
            double Ecp = (ybar + projection.yc) / D.dyB_de + m_meanE;
            double wid = projection.mask->getFullWidth();
            if (abs(Ecp - Ecp_old) > (wid + wid_old) / 2 and m_results.back().sum > 0) {
              m_results.push_back(Result(pixelID));
            }
            integrate(projection.mask, Ecp, m_results.back());
            Ecp_old = Ecp;
            wid_old = wid;
          }
        }

        if (m_results.back().sum == 0) m_results.pop_back();
      }

      for (auto& result : m_results) result.set();

      for (auto& mask : masks) {
        if (mask.second) delete mask.second;
      }

    }


    void YScanner::merge(unsigned col, double dydz, int j1, int j2) const
    {
      int Neven = func::getNumOfEven(j1, j2);
      int Nodd = j2 - j1 - Neven;

      for (unsigned row = 0; row < m_pixelPositions.getNumPixelRows(); row++) {

        int pixelID = m_pixelPositions.pixelID(row, col);
        if (not m_pixelMasks.isActive(pixelID)) continue;

        const auto& pixel = m_pixelPositions.get(pixelID);
        double Dy0 = 0;
        double Dy1 = 0;
        PixelProjection proj;
        for (size_t k = 0; k < m_prism.unfoldedWindows.size(); k++) {
          projectPixel(pixel.yc, pixel.Dy, k, dydz, proj); // for even reflections
          if (proj.Dy > 0) Dy0 += proj.Dy;
          projectPixel(pixel.yc, pixel.Dy, k, -dydz, proj); // for odd reflections
          if (proj.Dy > 0) Dy1 += proj.Dy;
        }
        if (Dy0 == 0 and Dy1 == 0) continue;

        double Dy = (Dy0 * Neven + Dy1 * Nodd) / (Neven + Nodd);
        Result result(pixelID);
        result.sum = Dy / m_bars.front().B;
        result.e0 = m_meanE;
        result.sigsq = m_rmsE * m_rmsE;
        m_results.push_back(result);
      }
    }


    void YScanner::integrate(const EnergyMask* energyMask, double Ecp, Result& result) const
    {
      const auto& mask = energyMask->getMask();

      if (mask.empty()) {
        // direct mask calculation
        for (size_t i = 0; i < m_quasyEnergyDistribution.entries.size(); i++) {
          double E = m_quasyEnergyDistribution.getX(i);
          double m = energyMask->getMask(E - Ecp);
          if (m > 0) {
            const auto& entry = m_quasyEnergyDistribution.entries[i];
            double s = entry.y * m;
            result.sum += s;
            result.e0 += entry.x * s;
            result.sigsq += entry.xsq * s;
          }
        }
      } else {
        // pre-calculated discrete mask w/ linear interpolation
        int i0 = m_quasyEnergyDistribution.getIndex(Ecp);
        double fract = -(Ecp - m_quasyEnergyDistribution.getX(i0)) / m_quasyEnergyDistribution.step;
        if (fract < 0) {
          i0++;
          fract += 1;
        }
        int N = m_quasyEnergyDistribution.entries.size() - 1;
        int M = mask.size() - 1;
        int i1 = std::max(i0 - M, 0);
        int i2 = std::min(i0 + M - 1, N);
        for (int i = i1; i <= i2; i++) {
          const auto& entry = m_quasyEnergyDistribution.entries[i];
          double m = mask[abs(i - i0)] * (1 - fract) + mask[abs(i - i0 + 1)] * fract;
          double s = entry.y * m;
          result.sum += s;
          result.e0 += entry.x * s;
          result.sigsq += entry.xsq * s;
        }
      }
    }


    double YScanner::prismEntranceY(double y, int k, double dydz) const
    {
      const auto& win = m_prism.unfoldedWindows[k];
      double dz = abs(m_prism.zD - m_prism.zFlat);
      double z1 = y * win.sz + win.z0 + win.nz * dz;
      double y1 = y * win.sy + win.y0 + win.ny * dz;
      return y1 + dydz * (m_prism.zR - z1);
    }


    void YScanner::projectPixel(double yc, double size, int k, double dydz, PixelProjection& proj) const
    {
      double halfSize = (k - m_prism.k0) % 2 == 0 ? size / 2 : -size / 2;
      double y1 = prismEntranceY(yc - halfSize, k, dydz);
      double y2 = prismEntranceY(yc + halfSize, k, dydz);

      double Bh = m_bars.front().B / 2;
      y1 = std::max(y1, -Bh);
      y2 = std::min(y2, Bh);

      proj.yc = (y1 + y2) / 2;
      proj.Dy = y2 - y1;
    }

  } //TOP
} //Belle2
