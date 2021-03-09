/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/DeltaRayPDF.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    DeltaRayPDF::DeltaRayPDF(int moduleID):
      m_moduleID(moduleID), m_background(TOPRecoManager::getBackgroundPDF(moduleID))
    {
      if (not m_background) {
        B2ERROR("TOP::DeltaRayPDF: background PDF not found");
        return;
      }

      const auto* yScanner = TOPRecoManager::getYScanner(moduleID);
      if (not yScanner) {
        B2ERROR("TOP::DeltaRayPDF: YScanner not found");
        return;
      }

      m_pixelPositions = &(yScanner->getPixelPositions());

      m_zD = yScanner->getPrism().zD;
      m_zM = yScanner->getBars().back().zR;
      double meanE = yScanner->getMeanEnergyBeta1();
      double sigE = yScanner->getRMSEnergyBeta1();
      m_phaseIndex = TOPGeometryPar::Instance()->getPhaseIndex(meanE);
      m_groupIndex = TOPGeometryPar::Instance()->getGroupIndex(meanE);
      double dng_de = TOPGeometryPar::Instance()->getGroupIndexDerivative(meanE);
      m_dispersion = dng_de / m_groupIndex * sigE;

      const int N = 1000;
      double dkz = 1.0 / N;
      m_norms.push_back(0);
      for (int i = 0; i < N; i++) {
        double kz = (i + 0.5) * dkz;
        m_angularNorm += angularDistr(kz) * dkz;
        m_norms.push_back(m_angularNorm);
      }
      for (auto& norm : m_norms) norm /= m_angularNorm;

      const int Np = 101;
      double dx = 6.0 / (Np - 1);
      for (int i = 0; i < Np; i++) {
        double x = i * dx - 3.0;
        m_tableGaus.push_back(GausXY(x));
      }
      double sum = 0;
      for (const auto& gaus : m_tableGaus) sum += gaus.y;
      for (auto& gaus : m_tableGaus) gaus.y /= sum;
    }

    void DeltaRayPDF::prepare(const TOPTrack& track, const Const::ChargedStable& hypothesis)
    {
      const auto& emi = track.getEmissionPoint().position;
      m_xE = emi.X();
      m_yE = emi.Y();
      m_zE = emi.Z();
      m_dirFrac = directFraction(m_zE);
      m_dirT0 = (m_zE - m_zD) * m_groupIndex / Const::speedOfLight;
      m_reflT0 = (2 * m_zM - m_zD - m_zE) * m_groupIndex / Const::speedOfLight;
      m_TOF = track.getTOF(hypothesis);

      if (m_dirT0 < 0) {
        B2ERROR("TOP::DeltaRayPDF::prepare: T0 direct is negative -> set to 0");
        m_dirT0 = 0;
      }
      if (m_reflT0 < 0) {
        B2ERROR("TOP::DeltaRayPDF::prepare: T0 reflected is negative -> set to 0");
        m_reflT0 = 0;
      }

      double beta = track.getBeta(hypothesis);
      int PDGCode = abs(hypothesis.getPDGCode());
      if (PDGCode < 20) PDGCode = -PDGCode;
      if (track.getCharge() < 0) PDGCode = -PDGCode;
      double tlen = track.getLengthInQuartz();
      double tmin = TOPRecoManager::getMinTime();
      double tmax = TOPRecoManager::getMaxTime();
      double relEffi = m_background->getEfficiency();
      m_fraction = totalFraction(tmin, tmax);
      m_numPhotons = photonYield(beta, PDGCode) * tlen * m_fraction * relEffi;

      for (const auto& pixel : m_pixelPositions->getPixels()) {
        double dfi_dx = abs(m_zD - m_zE) / (pow(pixel.xc - m_xE, 2) + pow(pixel.yc - m_yE, 2) + pow(m_zD - m_zE, 2));
        m_pixelAcceptances.push_back(dfi_dx * pixel.Dx);
      }
      double sum = 0;
      const auto& pixelPDF = m_background->getPDF();
      for (size_t k = 0; k < m_pixelAcceptances.size(); k++) {
        sum += m_pixelAcceptances[k] * pixelPDF[k];
      }
      for (auto& x : m_pixelAcceptances) x /= sum;
    }

    double DeltaRayPDF::getPDFValue(int pixelID, double time) const
    {
      const auto& pixelPDF = m_background->getPDF();
      unsigned k = pixelID - 1;
      if (k < pixelPDF.size()) {
        const auto& pixel = m_pixelPositions->get(pixelID);
        double t0 = sqrt(pow(pixel.xc - m_xE, 2) + pow(pixel.yc - m_yE, 2) + pow(m_zD - m_zE, 2))
                    * m_groupIndex / Const::speedOfLight;
        return getPDFValue(time, t0 - m_dirT0, m_pixelAcceptances[k]) * pixelPDF[k];
      }
      return 0;
    }

    double DeltaRayPDF::smearedTimeDistr(double t, double t0) const
    {
      const double sigma0 = 0.15;
      double sigma = sqrt(pow(sigma0, 2) + pow(m_dispersion * t, 2));

      if (sigma < 0.001) return timeDistr(t, t0);

      double f = 0;
      for (const auto& gaus : m_tableGaus) f += timeDistr(t - gaus.x * sigma, t0) * gaus.y;
      return f;
    }

    double DeltaRayPDF::peakFraction(double tmin, double tmax, double t0) const
    {
      int nt = m_norms.size() - 1;
      int i1 = nt;
      if (tmax > t0) i1 = t0 / tmax * nt;
      int i2 = nt;
      if (tmin > t0) i2 = t0 / tmin * nt;
      return abs(m_norms[i2] - m_norms[i1]);
    }

    double DeltaRayPDF::totalFraction(double tmin, double tmax) const
    {
      double dirPeak =  peakFraction(tmin - m_TOF, tmax - m_TOF, m_dirT0);
      double reflPeak = peakFraction(tmin - m_TOF, tmax - m_TOF, m_reflT0);
      return m_dirFrac * dirPeak + (1 - m_dirFrac) * reflPeak;
    }

    double DeltaRayPDF::directFraction(double z) const
    {
      // coefficients of 5th order polynom (from fit to MC, see B2GM/TOP Software status, June 2020)
      double par[] = {0.332741, -0.00331502, 2.0801e-05, -3.43689e-09, -6.35849e-10, 3.54556e-12};

      double x = 1;
      double f = 0;
      for (auto p : par) {
        f += p * x;
        x *= z;
      }
      return f;
    }

    double DeltaRayPDF::photonYield(double beta, int PDGCode) const
    {
      // for parametrizations see B2GM/TOP Software status, June 2020
      const double averagePDE = 1.06404889; // relative to nominal PDE
      const double scaleFactor = 0.79;

      if (abs(PDGCode) == 11) { // electorns and positrons
        return 5.30 * scaleFactor / averagePDE;
      } else if (PDGCode == -2212) { // anti-protons
        double par[] = {13.5859, -28.0625, 17.2684};
        double f = par[0] + par[1] * beta + par[2] * beta * beta;
        return f * scaleFactor / averagePDE;
      } else { // other charged particles
        double par[] = { -8.15871, 10.0082, -1.25140, -120.225, 120.210};
        double f = exp(par[0] + par[1] * beta + par[2] * beta * beta) + exp(par[3] + par[4] * beta);
        return f * scaleFactor / averagePDE;
      }
    }

  } // namespace TOP
} // namespace Belle2
