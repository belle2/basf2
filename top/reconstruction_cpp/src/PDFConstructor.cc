/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/func.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

namespace Belle2 {
  namespace TOP {

    PDFConstructor::PDFConstructor(const TOPTrack& track, const Const::ChargedStable& hypothesis,
                                   EPDFOption PDFOption, EStoreOption storeOption, double overrideMass):
      m_moduleID(track.getModuleID()), m_track(track), m_hypothesis(hypothesis),
      m_inverseRaytracer(TOPRecoManager::getInverseRaytracer(m_moduleID)),
      m_fastRaytracer(TOPRecoManager::getFastRaytracer(m_moduleID)),
      m_yScanner(TOPRecoManager::getYScanner(m_moduleID)),
      m_backgroundPDF(TOPRecoManager::getBackgroundPDF(m_moduleID)),
      m_deltaRayPDF(m_moduleID),
      m_PDFOption(PDFOption), m_storeOption(storeOption)
    {
      if (not track.isValid()) {
        B2ERROR("TOP::PDFConstructor: TOPTrack is not valid, cannot continue");
        return;
      }

      m_valid = m_inverseRaytracer != 0 and m_fastRaytracer != 0 and m_yScanner != 0 and m_backgroundPDF != 0;
      if (not m_valid) {
        B2ERROR("TOP::PDFConstructor: missing reconstruction objects, cannot continue");
        return;
      }

      m_beta = track.getBeta(hypothesis, overrideMass);
      m_yScanner->prepare(track.getMomentumMag(), m_beta, track.getLengthInQuartz());
      m_tof = track.getTOF(hypothesis, 0, overrideMass);

      m_groupIndex = TOPGeometryPar::Instance()->getGroupIndex(m_yScanner->getMeanEnergy());
      m_groupIndexDerivative = TOPGeometryPar::Instance()->getGroupIndexDerivative(m_yScanner->getMeanEnergy());
      m_cosTotal = m_yScanner->getCosTotal();
      m_minTime = TOPRecoManager::getMinTime();
      m_maxTime = TOPRecoManager::getMaxTime();
      m_selectedHits = track.getSelectedHits();
      m_bkgRate = track.getBkgRate();

      // prepare the memory for storing signal PDF

      const auto& pixelPositions = m_yScanner->getPixelPositions();
      int numPixels = pixelPositions.getNumPixels();
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      for (int pixelID = 1; pixelID <= numPixels; pixelID++) {
        auto pmtType = pixelPositions.get(pixelID).pmtType;
        const auto& tts = geo->getTTS(pmtType);
        m_signalPDFs.push_back(SignalPDF(pixelID, tts));
      }

      // construct PDF

      if (m_yScanner->isAboveThreshold()) {
        setSignalPDF();
      }

      m_deltaRayPDF.prepare(track, hypothesis);
      m_deltaPhotons = m_deltaRayPDF.getNumPhotons();

      m_bkgPhotons = std::max(m_bkgRate * (m_maxTime - m_minTime), 0.1);

      // release the memory not needed anymore

      m_rayTracers.clear();
    }


    void PDFConstructor::setSignalPDF()
    {
      // construct PDF analytically

      const auto& prism = m_inverseRaytracer->getPrism();

      if (m_track.getEmissionPoint().position.Z() > prism.zR) {
        setSignalPDF_direct();
        setSignalPDF_reflected();
      } else {
        setSignalPDF_prism();
      }

      // count expected number of signal photons

      for (const auto& signalPDF : m_signalPDFs) {
        m_signalPhotons += signalPDF.getSum();
      }
      if (m_signalPhotons == 0) return;

      // normalize PDF

      for (auto& signalPDF : m_signalPDFs) {
        signalPDF.normalize(m_signalPhotons);
      }
    }

    // signal PDF construction for track crossing bar segments -------------------------------------------

    void PDFConstructor::setSignalPDF_direct()
    {
      const auto& pixelPositions = m_yScanner->getPixelPositions();
      const auto& bar = m_inverseRaytracer->getBars().front();
      const auto& prism = m_inverseRaytracer->getPrism();

      // determine the range of number of reflections in x

      double xmi = 0, xma = 0;
      bool ok = rangeOfX(prism.zD, xmi, xma);
      if (not ok) return;
      int kmi = lround(xmi / bar.A);
      int kma = lround(xma / bar.A);

      // loop over reflections in x and over pixel columns

      for (int k = kmi; k <= kma; k++) {
        for (unsigned col = 0; col < pixelPositions.getNumPixelColumns(); col++) {
          const auto& pixel = pixelPositions.get(col + 1);
          if (pixel.Dx == 0) continue;
          double xD = func::unfold(pixel.xc, k, bar.A);
          if (xD < xmi or xD > xma) continue;
          InverseRaytracerDirect direct;
          setSignalPDF(direct, col, xD, prism.zD);
        }
      }
    }

    void PDFConstructor::setSignalPDF_reflected()
    {
      const auto& bar = m_inverseRaytracer->getBars().back();
      const auto& prism = m_inverseRaytracer->getPrism();
      const auto& mirror = m_inverseRaytracer->getMirror();
      const auto& emiPoint = m_track.getEmissionPoint().position;

      // determine the range of number of reflections in x before mirror

      double xmi = 0, xma = 0;
      bool ok = rangeOfX(mirror.zb, xmi, xma);
      if (not ok) return;
      int kmi = lround(xmi / bar.A);
      int kma = lround(xma / bar.A);

      // loop over reflections in x before mirror

      double xE = emiPoint.X();
      double zE = emiPoint.Z();
      double Ah = bar.A / 2;
      for (int k = kmi; k <= kma; k++) {
        double x0 = findReflectionExtreme(xE, zE, prism.zD, k, bar.A, mirror);
        x0 = func::clip(x0, k, bar.A, xmi, xma);
        double xL = func::clip(-Ah, k, bar.A, xmi, xma);
        double xR = func::clip(Ah, k, bar.A, xmi, xma);
        if (x0 > xL) setSignalPDF_reflected(k, xL, x0);
        if (x0 < xR) setSignalPDF_reflected(k, x0, xR);
      }
    }


    void PDFConstructor::setSignalPDF_reflected(int Nxm, double xmMin, double xmMax)
    {
      const auto& pixelPositions = m_yScanner->getPixelPositions();
      const auto& bar = m_inverseRaytracer->getBars().back();
      const auto& prism = m_inverseRaytracer->getPrism();

      // determine the range of number of reflections in x after mirror

      std::vector<double> xDs;
      double minLen = 1e10;
      if (not detectionPositionX(xmMin, Nxm, xDs, minLen)) return;
      if (not detectionPositionX(xmMax, Nxm, xDs, minLen)) return;
      if (xDs.size() < 2) return;

      double minTime = m_tof + minLen * m_groupIndex / Const::speedOfLight;
      if (minTime > m_maxTime) return;

      std::sort(xDs.begin(), xDs.end());
      double xmi = xDs.front();
      double xma = xDs.back();

      int kmi = lround(xmi / bar.A);
      int kma = lround(xma / bar.A);

      // loop over reflections in x after mirror and over pixel columns

      for (int k = kmi; k <= kma; k++) {
        for (unsigned col = 0; col < pixelPositions.getNumPixelColumns(); col++) {
          const auto& pixel = pixelPositions.get(col + 1);
          if (pixel.Dx == 0) continue;
          double xD = func::unfold(pixel.xc, k, bar.A);
          if (xD < xmi or xD > xma) continue;
          InverseRaytracerReflected reflected;
          setSignalPDF(reflected, col, xD, prism.zD, Nxm, xmMin, xmMax);
        }
      }

    }


    bool PDFConstructor::detectionPositionX(double xM, int Nxm, std::vector<double>& xDs, double& minLen)
    {
      m_inverseRaytracer->clear();
      int i0 = m_inverseRaytracer->solveForReflectionPoint(xM, Nxm, m_track.getEmissionPoint(), cerenkovAngle());
      if (i0 < 0) return false;

      bool ok = false;
      for (unsigned i = 0; i < 2; i++) {
        if (not m_inverseRaytracer->getStatus(i)) continue;
        const auto& solutions = m_inverseRaytracer->getSolutions(i);
        const auto& sol = solutions[i0];
        xDs.push_back(sol.xD);
        minLen = std::min(minLen, sol.len);
        ok = true;
      }

      return ok;
    }


    bool PDFConstructor::doRaytracingCorrections(const InverseRaytracer::Solution& sol, double dFic_dx, double xD)
    {
      const double precision = 0.01; // [cm]

      double x1 = 0;                    // x is dFic
      double y1 = deltaXD(x1, sol, xD); // y is the difference in xD
      if (isnan(y1)) return false;
      if (std::abs(y1) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
      int n1 = m_fastRaytracer->getNxm();

      double step = -dFic_dx * y1;
      for (int i = 1; i < 20; i++) { // search for zero-crossing interval
        double x2 = step * i;
        double y2 = deltaXD(x2, sol, xD);
        if (isnan(y2)) return false;
        int n2 = m_fastRaytracer->getNxm();
        if (n2 != n1) { // x2 is passing the discontinuity caused by different reflection number
          double x3 = x2;
          x2 = x1;
          y2 = y1;
          for (int k = 0; k < 20; k++) { // move x2 to discontinuity using bisection
            double x = (x2 + x3) / 2;
            double y = deltaXD(x, sol, xD);
            if (isnan(y)) return false;
            int n = m_fastRaytracer->getNxm();
            if (n == n1) {
              x2 = x;
              y2 = y;
            } else {
              x3 = x;
            }
          }
          if (y2 * y1 > 0) return false; // solution does not exist
        }
        if (std::abs(y2) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
        if (y2 * y1 < 0) { // zero-crossing interval is identified
          for (int k = 0; k < 20; k++) { // find zero-crossing using bisection
            double x = (x1 + x2) / 2;
            double y = deltaXD(x, sol, xD);
            if (isnan(y)) return false;
            if (std::abs(y) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
            if (y * y1 < 0) {
              x2 = x;
            } else {
              x1 = x;
              y1 = y;
            }
          }
          return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
        }
        x1 = x2;
        y1 = y2;
      }

      return false;
    }


    bool PDFConstructor::setDerivatives(YScanner::Derivatives& D, double dL, double de, double dFic)
    {
      while (m_rayTracers.size() < 3) m_rayTracers.push_back(*m_fastRaytracer); // push back a copy of the object

      bool ok = true;
      auto& rayTracer_dL = m_rayTracers[0];
      for (int i = 0; i < 10; i++) {
        ok = raytrace(rayTracer_dL, dL);
        if (ok) break;
        dL = - dL / 2;
      }
      if (not ok) return false;

      auto& rayTracer_de = m_rayTracers[1];
      for (int i = 0; i < 10; i++) {
        ok = raytrace(rayTracer_de, 0, de);
        if (ok) break;
        de = - de / 2;
      }
      if (not ok) return false;

      auto& rayTracer_dFic = m_rayTracers[2];
      for (int i = 0; i < 10; i++) {
        ok = raytrace(rayTracer_dFic, 0, 0, dFic);
        if (ok) break;
        dFic = - dFic / 2;
      }
      if (not ok) return false;

      // partial derivatives on L, e and Fic

      double dLen_dL = (rayTracer_dL.getPropagationLen() - m_fastRaytracer->getPropagationLen()) / dL;
      double dLen_de = (rayTracer_de.getPropagationLen() - m_fastRaytracer->getPropagationLen()) / de;
      double dLen_dFic = (rayTracer_dFic.getPropagationLen() - m_fastRaytracer->getPropagationLen()) / dFic;

      double dyB_dL = (rayTracer_dL.getYB() - m_fastRaytracer->getYB()) / dL;
      double dyB_de = (rayTracer_de.getYB() - m_fastRaytracer->getYB()) / de;
      double dyB_dFic = (rayTracer_dFic.getYB() - m_fastRaytracer->getYB()) / dFic;

      double dx_dL = (rayTracer_dL.getXD() - m_fastRaytracer->getXD()) / dL;
      double dx_de = (rayTracer_de.getXD() - m_fastRaytracer->getXD()) / de;
      double dx_dFic = (rayTracer_dFic.getXD() - m_fastRaytracer->getXD()) / dFic;

      if (dx_dFic == 0) return false;

      // derivatives on L, e, and x. Derivatives on L and e have to be given at constant x.

      D.dLen_dx = dLen_dFic / dx_dFic;
      D.dLen_de = dLen_de - dLen_dFic * dx_de / dx_dFic;
      D.dLen_dL = dLen_dL - dLen_dFic * dx_dL / dx_dFic;

      D.dyB_dx = dyB_dFic / dx_dFic;
      D.dyB_de = dyB_de - dyB_dFic * dx_de / dx_dFic;
      D.dyB_dL = dyB_dL - dyB_dFic * dx_dL / dx_dFic;

      D.dFic_dx = 1 / dx_dFic;
      D.dFic_de = - dx_de / dx_dFic;
      D.dFic_dL = - dx_dL / dx_dFic;

      return true;
    }


    bool PDFConstructor::raytrace(const FastRaytracer& rayTracer, double dL, double de, double dFic)
    {
      const auto& emi = m_track.getEmissionPoint(dL);
      const auto& trk = emi.trackAngles;
      const auto& cer = cerenkovAngle(de);

      double fic = m_Fic + dFic;
      double cosFic = cos(fic);
      double sinFic = sin(fic);
      double a = trk.cosTh * cer.sinThc * cosFic + trk.sinTh * cer.cosThc;
      double b = cer.sinThc * sinFic;
      double kx = a * trk.cosFi - b * trk.sinFi;
      double ky = a * trk.sinFi + b * trk.cosFi;
      double kz = trk.cosTh * cer.cosThc - trk.sinTh * cer.sinThc * cosFic;
      PhotonState photon(emi.position, kx, ky, kz);
      rayTracer.propagate(photon, true);
      if (not rayTracer.getPropagationStatus()) return false;

      if (rayTracer.getNxm() != m_fastRaytracer->getNxm()) return false;
      if (rayTracer.getNym() != m_fastRaytracer->getNym()) return false;
      if (rayTracer.getNxe() != m_fastRaytracer->getNxe()) return false;
      if (rayTracer.getNye() != m_fastRaytracer->getNye()) return false;
      if (rayTracer.getNxb() != m_fastRaytracer->getNxb()) return false;
      if (rayTracer.getNyb() != m_fastRaytracer->getNyb()) return false;

      if (rayTracer.getExtraStates().empty() or m_fastRaytracer->getExtraStates().empty()) return true;

      if (rayTracer.getExtraStates().back().getNx() != m_fastRaytracer->getExtraStates().back().getNx()) return false;
      if (rayTracer.getExtraStates().back().getNy() != m_fastRaytracer->getExtraStates().back().getNy()) return false;

      return true;
    }


    void PDFConstructor::expandSignalPDF(unsigned col, const YScanner::Derivatives& D, SignalPDF::EPeakType type)
    {
      m_ncallsExpandPDF[type]++;

      double Len = m_fastRaytracer->getPropagationLen();
      double speedOfLightQuartz = Const::speedOfLight / m_groupIndex; // average speed of light in quartz

      // difference of propagation times of true and fliped prism
      double dTime = m_fastRaytracer->getPropagationLenDelta() / speedOfLightQuartz;

      // derivatives: dt/de, dt/dx, dt/dL
      double dt_de = (D.dLen_de + Len * m_groupIndexDerivative / m_groupIndex) / speedOfLightQuartz;
      double dt_dx = D.dLen_dx / speedOfLightQuartz;
      double dt_dL = D.dLen_dL / speedOfLightQuartz + 1 / m_beta / Const::speedOfLight;

      // contribution of multiple scattering in quartz
      double sigmaScat = D.dLen_de * m_yScanner->getSigmaScattering() / speedOfLightQuartz;

      // contribution of quartz surface roughness at single reflection and effective number of reflections
      double sigmaAlpha = D.dLen_de * m_yScanner->getSigmaAlpha() / speedOfLightQuartz;
      int Ny_eff = 2 * std::abs(m_fastRaytracer->getNym()) + std::abs(m_fastRaytracer->getNyb());

      const auto& pixel = m_yScanner->getPixelPositions().get(col + 1);
      double L = m_track.getLengthInQuartz();

      // sigma squared: pixel size, parallax, propagation time difference, multiple scattering, surface roughness
      double wid0 = (pow(dt_dx * pixel.Dx, 2) + pow(dt_dL * L, 2) + pow(dTime, 2)) / 12 + pow(sigmaScat, 2) +
                    pow(sigmaAlpha, 2) * Ny_eff;

      // sigma squared: adding chromatic contribution
      double wid = wid0 + pow(dt_de * m_yScanner->getRMSEnergy(), 2);

      double yB = m_fastRaytracer->getYB();
      const auto& photonStates = m_fastRaytracer->getPhotonStates();
      const auto& atPrismEntrance = photonStates[photonStates.size() - 2];
      double dydz = atPrismEntrance.getKy() / atPrismEntrance.getKz();
      if (m_fastRaytracer->getNyb() % 2 != 0) dydz = -dydz;

      bool doScan = (m_PDFOption == c_Fine);
      if (m_PDFOption == c_Optimal) {
        double time = m_tof + Len / speedOfLightQuartz;
        doScan = m_track.isScanRequired(col, time, wid);
      }

      m_yScanner->expand(col, yB, dydz, D, Ny_eff, doScan);

      double numPhotons = m_yScanner->getNumPhotons() * std::abs(D.dFic_dx * pixel.Dx);
      int nx = m_fastRaytracer->getNx();
      int ny = m_fastRaytracer->getNy();
      for (const auto& result : m_yScanner->getResults()) {
        double RQE = m_yScanner->getPixelEfficiencies().get(result.pixelID);
        if (RQE == 0) continue;
        auto& signalPDF = m_signalPDFs[result.pixelID - 1];
        double dE = result.e0 - m_yScanner->getMeanEnergy();
        double propLen = Len + D.dLen_de * dE;
        double speedOfLight = Const::speedOfLight / TOPGeometryPar::Instance()->getGroupIndex(result.e0);

        SignalPDF::PDFPeak peak;
        peak.t0 = m_tof + propLen / speedOfLight;
        peak.wid = wid0 + dt_de * dt_de * result.sigsq;
        peak.nph = numPhotons * result.sum * RQE * propagationLosses(result.e0, propLen, nx, ny, type);
        peak.fic = func::within2PI(m_Fic + D.dFic_de * dE);
        signalPDF.append(peak);

        if (m_storeOption == c_Reduced) continue;

        SignalPDF::PDFExtra extra;
        extra.thc = acos(getCosCerenkovAngle(result.e0));
        extra.e = result.e0;
        extra.sige = result.sigsq;
        extra.Nxm = m_fastRaytracer->getNxm();
        extra.Nxb = m_fastRaytracer->getNxb();
        extra.Nxe = m_fastRaytracer->getNxe();
        extra.Nym = m_fastRaytracer->getNym();
        extra.Nyb = m_fastRaytracer->getNyb();
        extra.Nye = m_fastRaytracer->getNye();
        extra.xD = m_fastRaytracer->getXD();
        extra.yD = m_fastRaytracer->getYD();
        extra.zD = m_fastRaytracer->getZD();
        extra.yB = m_fastRaytracer->getYB();
        const auto& firstState =  photonStates.front();
        extra.kxE = firstState.getKx();
        extra.kyE = firstState.getKy();
        extra.kzE = firstState.getKz();
        const auto& lastState =  photonStates.back();
        extra.kxD = lastState.getKx();
        extra.kyD = lastState.getKy();
        extra.kzD = lastState.getKz();
        extra.type = type;
        signalPDF.append(extra);
      }

    }

    double PDFConstructor::propagationLosses(double E, double propLen, int nx, int ny,
                                             SignalPDF::EPeakType type) const
    {
      double bulk = TOPGeometryPar::Instance()->getAbsorptionLength(E);
      double surf = m_yScanner->getBars().front().reflectivity;
      double p = exp(-propLen / bulk) * pow(surf, std::abs(nx) + std::abs(ny));
      if (type == SignalPDF::c_Reflected) p *= std::min(m_yScanner->getMirror().reflectivity, 1.0);
      return p;
    }

    bool PDFConstructor::rangeOfX(double z, double& xmi, double& xma)
    {
      double maxLen = (m_maxTime - m_tof) / m_groupIndex * Const::speedOfLight; // maximal propagation length
      if (maxLen < 0) return false;

      const auto& emission = m_track.getEmissionPoint();
      const auto& trk = emission.trackAngles;
      const auto& cer = cerenkovAngle();

      // range in x from propagation lenght limit

      double dz = z - emission.position.Z();
      double cosFicLimit = (trk.cosTh * cer.cosThc - dz / maxLen) / (trk.sinTh * cer.sinThc); // at maxLen
      double cosLimit = (dz > 0) ? cosFicLimit : -cosFicLimit;
      if (cosLimit < -1) return false; // photons cannot reach the plane at z within propagation lenght limit

      std::vector<double> xmima;
      double x0 = emission.position.X();
      if (cosLimit > 1) {
        xmima.push_back(x0 - maxLen);
        xmima.push_back(x0 + maxLen);
      } else {
        double a = trk.cosTh * cer.sinThc * cosFicLimit + trk.sinTh * cer.cosThc;
        double b = cer.sinThc * sqrt(1 - cosFicLimit * cosFicLimit);
        xmima.push_back(x0 + maxLen * (a * trk.cosFi - b * trk.sinFi));
        xmima.push_back(x0 + maxLen * (a * trk.cosFi + b * trk.sinFi));
        std::sort(xmima.begin(), xmima.end());
      }
      xmi = xmima[0];
      xma = xmima[1];

      // range in x from minimal/maximal possible extensions in x, if they exist (d(kx/kz)/dFic = 0)

      double theta = acos(trk.cosTh);
      if (dz < 0) theta = M_PI - theta; // rotation around x by 180 deg. (z -> -z, phi -> -phi)
      dz = std::abs(dz);
      double thetaCer = acos(cer.cosThc);
      if (theta - thetaCer >= M_PI / 2) return false; // photons cannot reach the plane at z

      std::vector<double> dxdz;
      double a = -cos(theta + thetaCer) * cos(theta - thetaCer);
      double b = sin(2 * theta) * trk.cosFi;
      double c = pow(trk.sinFi * cer.sinThc, 2) - pow(trk.cosFi, 2) * sin(theta + thetaCer) * sin(theta - thetaCer);
      double D = b * b - 4 * a * c;
      if (D < 0) return true; // minimum and maximum do not exist, range is given by propagation length limit
      if (a != 0) {
        D = sqrt(D);
        dxdz.push_back((-b - D) / 2 / a);
        dxdz.push_back((-b + D) / 2 / a);
      } else {
        if (b == 0) return true; // minimum and maximum do not exist, range is given by propagation length limit
        dxdz.push_back(-c / b);
        dxdz.push_back(copysign(INFINITY, b));
      }
      std::vector<double> cosFic(2, cosLimit);
      for (int i = 0; i < 2; i++) {
        if (std::abs(dxdz[i]) < INFINITY) {
          double aa = (dxdz[i] * cos(theta) - trk.cosFi * sin(theta)) * cer.cosThc;
          double bb = (dxdz[i] * sin(theta) + trk.cosFi * cos(theta)) * cer.sinThc;
          double dd = trk.sinFi * cer.sinThc;
          cosFic[i] = aa * bb / (bb * bb + dd * dd);
          double kz = cos(theta) * cer.cosThc - sin(theta) * cer.sinThc * cosFic[i];
          if (kz < 0) dxdz[i] = copysign(INFINITY, dxdz[1 - i] - dxdz[i]);
        }
      }
      if (dxdz[0] > dxdz[1]) {
        std::reverse(dxdz.begin(), dxdz.end());
        std::reverse(cosFic.begin(), cosFic.end());
      }
      for (int i = 0; i < 2; i++) {
        if (cosFic[i] < cosLimit) xmima[i] = x0 + dxdz[i] * dz;
      }

      // just to make sure xmi/xma are within the limits given by maximal propagation length
      xmi = std::max(xmima[0], x0 - maxLen);
      xma = std::min(xmima[1], x0 + maxLen);

      return xma > xmi;
    }


    double PDFConstructor::derivativeOfReflectedX(double x, double xe, double ze, double zd) const
    {
      double z = sqrt(1 - x * x);
      double kx = (x - xe);
      double kz = (z - ze);
      double s = 2 * (kx * x + kz * z);
      double qx = kx - s * x;
      double qz = kz - s * z;

      double der_z = -x / z;
      double der_s = 2 * (kx + der_z * kz);
      double der_qx = (1 - s) - der_s * x;
      double der_qz = (1 - s) * der_z - der_s * z;

      return 1 - der_z * qx / qz + (zd - z) * (der_qx * qz - der_qz * qx) / (qz * qz);
    }


    double PDFConstructor::findReflectionExtreme(double xE, double zE, double zD, int Nxm, double A,
                                                 const RaytracerBase::Mirror& mirror) const
    {

      if (Nxm % 2 == 0) {
        xE = func::unfold(xE, -Nxm, A);
      } else {
        xE = func::unfold(xE, Nxm, A);
      }

      double xe = (xE - mirror.xc) / mirror.R;
      double ze = (zE - mirror.zc) / mirror.R;
      double zd = (zD - mirror.zc) / mirror.R;

      double Ah = A / 2;

      double x1 = (-Ah - mirror.xc) / mirror.R;
      double y1 = derivativeOfReflectedX(x1, xe, ze, zd);
      if (y1 != y1 or std::abs(y1) == INFINITY) return -Ah;

      double x2 = (Ah - mirror.xc) / mirror.R;
      double y2 = derivativeOfReflectedX(x2, xe, ze, zd);
      if (y2 != y2 or std::abs(y2) == INFINITY) return -Ah;

      if (y1 * y2 > 0) return -Ah; // no minimum or maximum

      for (int i = 0; i < 50; i++) {
        double x = (x1 + x2) / 2;
        double y = derivativeOfReflectedX(x, xe, ze, zd);
        if (y != y or std::abs(y) == INFINITY) return -Ah;
        if (y * y1 < 0) {
          x2 = x;
        } else {
          x1 = x;
          y1 = y;
        }
      }
      double x = (x1 + x2) / 2;

      return x * mirror.R + mirror.xc;
    }

    // signal PDF construction for track crossing prism --------------------------------------------------

    void PDFConstructor::setSignalPDF_prism()
    {
      const auto& pixelPositions = m_yScanner->getPixelPositions();
      const auto& prism = m_inverseRaytracer->getPrism();
      double speedOfLightQuartz = Const::speedOfLight / m_groupIndex; // average speed of light in quartz

      double xE = m_track.getEmissionPoint().position.X();
      int nxmi = (xE > 0) ? 0 : -1;
      int nxma = (xE > 0) ? 1 : 0;

      for (const auto& pixel : pixelPositions.getPixels()) {
        if (not m_yScanner->getPixelMasks().isActive(pixel.ID)) continue;
        double RQE = m_yScanner->getPixelEfficiencies().get(pixel.ID);
        if (RQE == 0) continue;
        auto& signalPDF = m_signalPDFs[pixel.ID - 1];
        for (int Nxe = nxmi; Nxe <= nxma; Nxe++) {
          for (size_t k = 0; k < prism.unfoldedWindows.size(); k++) {
            const auto sol = prismSolution(pixel, k, Nxe);
            if (sol.len == 0 or std::abs(sol.L) > m_track.getLengthInQuartz() / 2) continue;

            bool ok = prismRaytrace(sol);
            if (not ok) continue;
            int Nye = k - prism.k0;
            if (Nye != m_fastRaytracer->getNy() or Nxe != m_fastRaytracer->getNx()) continue;
            if (not m_fastRaytracer->getTotalReflStatus(m_cosTotal)) continue;
            const auto firstState = m_fastRaytracer->getPhotonStates().front();  // a copy of
            const auto lastState = m_fastRaytracer->getPhotonStates().back();  // a copy of

            double slope = lastState.getKy() / lastState.getKz();
            double dz = prism.zD - prism.zFlat;
            double y2 = std::min(pixel.yc + pixel.Dy / 2, prism.yUp + slope * dz);
            double y1 = std::max(pixel.yc - pixel.Dy / 2, prism.yDown + slope * dz);
            double Dy = y2 - y1;
            if (Dy < 0) continue;

            double dL = 0.1; // cm
            for (int i = 0; i < 4; i++) {
              ok = prismRaytrace(sol, dL);
              ok = ok and Nye == m_fastRaytracer->getNy() and Nxe == m_fastRaytracer->getNx();
              if (ok) break;
              dL = - dL / 2;
            }
            if (not ok) continue;
            const auto lastState_dL = m_fastRaytracer->getPhotonStates().back();  // a copy of

            double dFic = 0.01; // rad
            for (int i = 0; i < 4; i++) {
              ok = prismRaytrace(sol, 0, dFic);
              ok = ok and Nye == m_fastRaytracer->getNy() and Nxe == m_fastRaytracer->getNx();
              if (ok) break;
              dFic = - dFic / 2;
            }
            if (not ok) continue;
            const auto lastState_dFic = m_fastRaytracer->getPhotonStates().back();  // a copy of

            double de = 0.1; // eV
            for (int i = 0; i < 4; i++) {
              ok = prismRaytrace(sol, 0, 0, de);
              ok = ok and Nye == m_fastRaytracer->getNy() and Nxe == m_fastRaytracer->getNx();
              if (ok) break;
              de = - de / 2;
            }
            if (not ok) continue;
            const auto lastState_de = m_fastRaytracer->getPhotonStates().back();  // a copy of

            double dx_dL = (lastState_dL.getX() - lastState.getX()) / dL;
            double dy_dL = (lastState_dL.getY() - lastState.getY()) / dL;
            double dx_dFic = (lastState_dFic.getX() - lastState.getX()) / dFic;
            double dy_dFic = (lastState_dFic.getY() - lastState.getY()) / dFic;
            double Jacobi = dx_dL * dy_dFic - dy_dL * dx_dFic;
            double numPhotons = m_yScanner->getNumPhotonsPerLen() * pixel.Dx * Dy / std::abs(Jacobi) * RQE;

            double dLen_de = (lastState_de.getPropagationLen() - lastState.getPropagationLen()) / de;
            double dLen_dL = (lastState_dL.getPropagationLen() - lastState.getPropagationLen()) / dL;
            double dLen_dFic = (lastState_dFic.getPropagationLen() - lastState.getPropagationLen()) / dFic;

            double dt_de = (dLen_de + sol.len * m_groupIndexDerivative / m_groupIndex) / speedOfLightQuartz;
            double dt_dL = dLen_dL / speedOfLightQuartz;
            double dt_dFic = dLen_dFic / speedOfLightQuartz;

            double chromatic = pow(dt_de * m_yScanner->getRMSEnergy(), 2);

            double DL = (dy_dFic * pixel.Dx - dx_dFic * pixel.Dy) / Jacobi;
            double DFic = (dx_dL * pixel.Dy - dy_dL * pixel.Dx) / Jacobi;
            double paralax = (pow(dt_dL * DL, 2) + pow(dt_dFic * DFic, 2)) / 12;

            double scattering = pow(dLen_de * m_yScanner->getSigmaScattering() / speedOfLightQuartz, 2);

            SignalPDF::PDFPeak peak;
            peak.t0 = m_tof + sol.L / m_beta / Const::speedOfLight + sol.len / speedOfLightQuartz;
            peak.wid = chromatic + paralax + scattering;
            peak.nph = 1 - exp(-numPhotons); // because photons that pile-up are counted as one
            peak.fic = atan2(sol.sinFic, sol.cosFic);
            signalPDF.append(peak);

            if (m_storeOption == c_Reduced) continue;

            SignalPDF::PDFExtra extra;
            extra.thc = acos(getCosCerenkovAngle(m_yScanner->getMeanEnergy()));
            extra.e = m_yScanner->getMeanEnergy();
            extra.sige = m_yScanner->getRMSEnergy();
            extra.Nxe = Nxe;
            extra.Nye = Nye;
            extra.xD = lastState.getXD();
            extra.yD = lastState.getYD();
            extra.zD = lastState.getZD();
            extra.kxE = firstState.getKx();
            extra.kyE = firstState.getKy();
            extra.kzE = firstState.getKz();
            extra.kxD = lastState.getKx();
            extra.kyD = lastState.getKy();
            extra.kzD = lastState.getKz();
            extra.type = SignalPDF::c_Direct;
            signalPDF.append(extra);

          } // reflections in y (unfolded prism windows)
        } // reflections in x
      } // pixels

    }


    bool PDFConstructor::prismRaytrace(const PrismSolution& sol, double dL, double dFic, double de)
    {
      const auto& emi = m_track.getEmissionPoint(sol.L + dL);
      const auto& trk = emi.trackAngles;
      const auto& cer = cerenkovAngle(de);

      double cosDFic = 1;
      double sinDFic = 0;
      if (dFic != 0) {
        cosDFic = cos(dFic);
        sinDFic = sin(dFic);
      }
      double cosFic = sol.cosFic * cosDFic - sol.sinFic * sinDFic;
      double sinFic = sol.sinFic * cosDFic + sol.cosFic * sinDFic;
      double a = trk.cosTh * cer.sinThc * cosFic + trk.sinTh * cer.cosThc;
      double b = cer.sinThc * sinFic;
      double kx = a * trk.cosFi - b * trk.sinFi;
      double ky = a * trk.sinFi + b * trk.cosFi;
      double kz = trk.cosTh * cer.cosThc - trk.sinTh * cer.sinThc * cosFic;
      PhotonState photon(emi.position, kx, ky, kz);
      m_fastRaytracer->propagate(photon);

      return m_fastRaytracer->getPropagationStatus();
    }


    PDFConstructor::PrismSolution PDFConstructor::prismSolution(const PixelPositions::PixelData& pixel,
                                                                unsigned k, int nx)
    {
      const auto& prism = m_inverseRaytracer->getPrism();
      const auto& win = prism.unfoldedWindows[k];
      double dz = std::abs(prism.zD - prism.zFlat);
      ROOT::Math::XYZPoint rD(func::unfold(pixel.xc, nx, prism.A),
                              pixel.yc * win.sy + win.y0 + win.ny * dz,
                              pixel.yc * win.sz + win.z0 + win.nz * dz);

      double L = 0;
      for (int iter = 0; iter < 100; iter++) {
        auto sol = prismSolution(rD, L);
        if (std::abs(sol.L) > m_track.getLengthInQuartz() / 2) return sol;
        if (std::abs(sol.L - L) < 0.01) return sol;
        L = sol.L;
      }
      B2DEBUG(20, "TOP::PDFConstructor::prismSolution: iterations not converging");
      return PrismSolution();
    }


    PDFConstructor::PrismSolution PDFConstructor::prismSolution(const ROOT::Math::XYZPoint& rD, double L)
    {
      const auto& emi = m_track.getEmissionPoint(L);

      // transformation of detection position to system of particle

      auto r = rD - emi.position;
      const auto& trk = emi.trackAngles;
      double xx = r.X() * trk.cosFi + r.Y() * trk.sinFi;
      double y = -r.X() * trk.sinFi + r.Y() * trk.cosFi;
      double x = xx * trk.cosTh - r.Z() * trk.sinTh;
      double z = xx * trk.sinTh + r.Z() * trk.cosTh;

      // solution

      double rho = sqrt(x * x + y * y);
      const auto& cer = cerenkovAngle();

      PrismSolution sol;
      sol.len = rho / cer.sinThc;
      sol.L = L + z - sol.len * cer.cosThc;
      sol.cosFic = x / rho;
      sol.sinFic = y / rho;

      return sol;
    }

    // log likelihood calculation ------------------------------------------------------------------------

    PDFConstructor::LogL PDFConstructor::getLogL() const
    {
      if (not m_valid) {
        B2ERROR("TOP::PDFConstructor::getLogL(): object status is invalid - cannot provide log likelihood");
        return LogL(0);
      }

      LogL LL(getExpectedPhotons());
      for (const auto& hit : m_selectedHits) {
        if (hit.time < m_minTime or hit.time > m_maxTime) continue;
        double f = pdfValue(hit.pixelID, hit.time, hit.timeErr);
        if (f <= 0) {
          auto ret = m_zeroPixels.insert(hit.pixelID);
          if (ret.second) {
            B2ERROR("TOP::PDFConstructor::getLogL(): PDF value is zero or negative"
                    << LogVar("slotID", m_moduleID)
                    << LogVar("pixelID", hit.pixelID) << LogVar("time", hit.time) << LogVar("PDFValue", f));
          }
          continue;
        }
        LL.logL += log(f);
        LL.numPhotons++;
        LL.effectiveSignalYield += m_f0 / f;
      }
      return LL;
    }


    PDFConstructor::LogL PDFConstructor::getLogL(double t0, double minTime, double maxTime, double sigt) const
    {
      if (not m_valid) {
        B2ERROR("TOP::PDFConstructor::getLogL(): object status is invalid - cannot provide log likelihood");
        return LogL(0);
      }

      LogL LL(getExpectedPhotons(minTime - t0, maxTime - t0));
      for (const auto& hit : m_selectedHits) {
        if (hit.time < minTime or hit.time > maxTime) continue;
        double f = pdfValue(hit.pixelID, hit.time - t0, hit.timeErr, sigt);
        if (f <= 0) {
          auto ret = m_zeroPixels.insert(hit.pixelID);
          if (ret.second) {
            B2ERROR("TOP::PDFConstructor::getLogL(): PDF value is zero or negative"
                    << LogVar("slotID", m_moduleID)
                    << LogVar("pixelID", hit.pixelID) << LogVar("time", hit.time) << LogVar("PDFValue", f));
          }
          continue;
        }
        LL.logL += log(f);
        LL.numPhotons++;
        LL.effectiveSignalYield += m_f0 / f;
      }
      return LL;
    }


    PDFConstructor::LogL PDFConstructor::getBackgroundLogL(double minTime, double maxTime) const
    {
      if (not m_valid) {
        B2ERROR("TOP::PDFConstructor::getBackgroundLogL(): object status is invalid - cannot provide log likelihood");
        return LogL(0);
      }

      double bkgPhotons = m_bkgPhotons * (maxTime - minTime) / (m_maxTime - m_minTime);

      LogL LL(bkgPhotons);
      for (const auto& hit : m_selectedHits) {
        if (hit.time < minTime or hit.time > maxTime) continue;
        double f = bkgPhotons * m_backgroundPDF->getPDFValue(hit.pixelID);
        if (f <= 0) {
          auto ret = m_zeroPixels.insert(hit.pixelID);
          if (ret.second) {
            B2ERROR("TOP::PDFConstructor::getBackgroundLogL(): PDF value is zero or negative"
                    << LogVar("slotID", m_moduleID)
                    << LogVar("pixelID", hit.pixelID) << LogVar("time", hit.time) << LogVar("PDFValue", f));
          }
          continue;
        }
        LL.logL += log(f);
        LL.numPhotons++;
      }
      return LL;
    }


    const std::vector<PDFConstructor::LogL>&
    PDFConstructor::getPixelLogLs(double t0, double minTime, double maxTime, double sigt) const
    {
      if (not m_valid) {
        B2ERROR("TOP::PDFConstructor::getPixelLogLs(): object status is invalid - cannot provide log likelihoods");
        return m_pixelLLs;
      }

      initializePixelLogLs(minTime - t0, maxTime - t0);

      for (const auto& hit : m_selectedHits) {
        if (hit.time < minTime or hit.time > maxTime) continue;
        double f = pdfValue(hit.pixelID, hit.time - t0, hit.timeErr, sigt);
        if (f <= 0) {
          auto ret = m_zeroPixels.insert(hit.pixelID);
          if (ret.second) {
            B2ERROR("TOP::PDFConstructor::getPixelLogLs(): PDF value is zero or negative"
                    << LogVar("slotID", m_moduleID)
                    << LogVar("pixelID", hit.pixelID) << LogVar("time", hit.time) << LogVar("PDFValue", f));
          }
          continue;
        }
        unsigned k = hit.pixelID - 1;
        auto& LL = m_pixelLLs[k];
        LL.logL += log(f);
        LL.numPhotons++;
        LL.effectiveSignalYield += m_f0 / f;
      }

      return m_pixelLLs;
    }

    void PDFConstructor::initializePixelLogLs(double minTime, double maxTime) const
    {
      m_pixelLLs.clear();

      double pb = (maxTime - minTime) / (m_maxTime - m_minTime);
      double bfot = pb * m_bkgPhotons + getExpectedDeltaPhotons(minTime, maxTime);
      for (const auto* other : m_pdfOtherTracks) bfot += other->getExpectedDeltaPhotons(minTime, maxTime);

      const auto& pixelPDF = m_backgroundPDF->getPDF();
      for (const auto& signalPDF : m_signalPDFs) {
        unsigned k = signalPDF.getPixelID() - 1;
        double phot = signalPDF.getIntegral(minTime, maxTime) * m_signalPhotons + bfot * pixelPDF[k];
        for (const auto* other : m_pdfOtherTracks) {
          const auto& otherPDFs = other->getSignalPDF();
          phot += otherPDFs[k].getIntegral(minTime, maxTime) * other->getExpectedSignalPhotons();
        }
        m_pixelLLs.push_back(LogL(phot));
      }
    }

    const std::vector<PDFConstructor::Pull>& PDFConstructor::getPulls() const
    {
      if (m_pulls.empty() and m_valid) {
        for (const auto& hit : m_selectedHits) {
          if (hit.time < m_minTime or hit.time > m_maxTime) continue;
          appendPulls(hit);
        }
      }

      return m_pulls;
    }

    void PDFConstructor::appendPulls(const TOPTrack::SelectedHit& hit) const
    {
      unsigned k = hit.pixelID - 1;
      if (k >= m_signalPDFs.size()) return;
      const auto& signalPDF = m_signalPDFs[k];

      double sfot = m_signalPhotons + m_deltaPhotons + m_bkgPhotons;
      double signalFract = m_signalPhotons / sfot;
      double wid0 = hit.timeErr * hit.timeErr;
      double minT0 = m_maxTime;
      double sum = 0;
      auto i0 = m_pulls.size();
      for (const auto& peak : signalPDF.getPDFPeaks()) {
        minT0 = std::min(minT0, peak.t0);
        for (const auto& gaus : signalPDF.getTTS()->getTTS()) {
          double sig2 = peak.wid + gaus.sigma * gaus.sigma + wid0; // sigma squared!
          double x = pow(hit.time - peak.t0 - gaus.position, 2) / sig2;
          if (x > 100) continue;
          double wt = signalFract * peak.nph * gaus.fraction / sqrt(2 * M_PI * sig2) * exp(-x / 2);
          sum += wt;
          m_pulls.push_back(Pull(hit.pixelID, hit.time, peak.t0, gaus.position, sqrt(sig2), peak.fic - M_PI, wt));
        }
      }

      double bg = (m_deltaPhotons * m_deltaRayPDF.getPDFValue(hit.pixelID, hit.time) +
                   m_bkgPhotons * m_backgroundPDF->getPDFValue(hit.pixelID)) / sfot;
      sum += bg;
      m_pulls.push_back(Pull(hit.pixelID, hit.time, minT0, 0, 0, 0, bg));

      if (sum == 0) return;
      for (size_t i = i0; i < m_pulls.size(); i++) m_pulls[i].wt /= sum;
    }


  } // namespace TOP
} // namespace Belle2


