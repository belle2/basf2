/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/func.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <algorithm>

using namespace std;

namespace Belle2 {
  namespace TOP {

    double PDFConstructor::s_minTime = -20.0;
    double PDFConstructor::s_maxTime = 75.0;


    PDFConstructor::PDFConstructor(const TOPTrack& track, const Const::ChargedStable& hypothesis,
                                   EPDFOption PDFOption, EStoreOption storeOption):
      m_moduleID(track.getModuleID()), m_track(track), m_hypothesis(hypothesis),
      m_PDFOption(PDFOption), m_storeOption(storeOption)
    {
      if (not track.isValid()) {
        B2ERROR("TOP::PDFConstructor: TOPTrack is not valid");
        return;
      }

      m_inverseRaytracer = TOPRecoManager::getInverseRaytracer(m_moduleID);
      if (not m_inverseRaytracer) return;
      m_fastRaytracer = TOPRecoManager::getFastRaytracer(m_moduleID);
      if (not m_fastRaytracer) return;
      m_yScanner = TOPRecoManager::getYScanner(m_moduleID);
      if (not m_yScanner) return;
      m_yScanner->prepare(track.getMomentumMag(), track.getBeta(hypothesis), track.getLengthInQuartz());

      m_tof = track.getTOF(hypothesis);
      m_groupIndex = TOPGeometryPar::Instance()->getGroupIndex(m_yScanner->getMeanEnergy());
      m_groupIndexDerivative = TOPGeometryPar::Instance()->getGroupIndexDerivative(m_yScanner->getMeanEnergy());
      m_cosTotal = m_yScanner->getCosTotal();

      if (m_yScanner->isAboveThreshold()) {
        setSignalPDF();
      }
      // setDeltaRayPDF();
      // setBackgroundPDF(); ? vnesen od zunaj? kot del tracka?
    }


    void PDFConstructor::setSignalPDF()
    {
      const auto& pixelPositions = m_yScanner->getPixelPositions();
      int numPixels = pixelPositions.getNumPixels();

      // prepare the memory for storing PDF parametrization

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      for (int pixelID = 1; pixelID <= numPixels; pixelID++) {
        auto pmtType = pixelPositions.get(pixelID).pmtType;
        const auto& tts = geo->getTTS(pmtType);
        m_signalPDFs.push_back(SignalPDF(pixelID, tts));
      }

      // construct PDF analytically

      setSignalPDF_direct();
      setSignalPDF_reflected();

      // count the expected number of photons

      for (const auto& signalPDF : m_signalPDFs) {
        m_signalPhotons += signalPDF.getSum();
      }
      if (m_signalPhotons == 0) return;

      // normalize the PDF

      for (auto& signalPDF : m_signalPDFs) {
        signalPDF.normalize(m_signalPhotons);
      }
    }

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
        x0 = clip(x0, k, bar.A, xmi, xma);
        double xL = clip(-Ah, k, bar.A, xmi, xma);
        double xR = clip(Ah, k, bar.A, xmi, xma);
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
      if (minTime > s_maxTime) return;

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
      const double precision = 0.01; //TODO ?

      double x1 = 0;                    // x is dFic
      double y1 = deltaXD(x1, sol, xD); // y is the difference in xD
      if (isnan(y1)) return false;
      if (abs(y1) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
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
        if (abs(y2) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
        if (y2 * y1 < 0) { // zero-crossing interval is identified
          for (int k = 0; k < 20; k++) { // find zero-crossing using bisection
            double x = (x1 + x2) / 2;
            double y = deltaXD(x, sol, xD);
            if (isnan(y)) return false;
            if (abs(y) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
            if (y * y1 < 0) {
              x2 = x;
              y2 = y;
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

      B2ERROR("PDFConstructor::doRaytracingCorrections: zero-crossing interval not found"
              << LogVar("xD", xD) << LogVar("dFic_dx", dFic_dx) << LogVar("step", step)
              << LogVar("x1", x1) << LogVar("y1", y1));

      return false;
    }


    void PDFConstructor::expandSignalPDF(unsigned col, const YScanner::Derivatives& D, SignalPDF::EPeakType type)
    {
      m_ncallsExpandPDF[type]++;

      double Len = m_fastRaytracer->getPropagationLen();
      double avSpeedOfLight = Const::speedOfLight / m_groupIndex; // average speed of light in quartz

      // derivatives: dt/de, dt/dx, dt/dL
      double dt_de = (D.dLen_de + Len * m_groupIndexDerivative / m_groupIndex) / avSpeedOfLight; // for chromatic
      double dt_dx =  D.dLen_dx / avSpeedOfLight; // for channel x-size
      double dt_dL = (D.dLen_dL + 1 / m_yScanner->getBeta() / m_groupIndex) / avSpeedOfLight; // for parallax

      // contribution of multiple scattering in quartz
      double sigmaScat = D.dLen_de * m_yScanner->getSigmaScattering() / avSpeedOfLight;

      const auto& pixel = m_yScanner->getPixelPositions().get(col + 1);
      double L = m_yScanner->getTrackLengthInQuartz();

      // sigma squared: channel x-size, parallax, multiple scattering
      double wid0 = (pow(dt_dx * pixel.Dx, 2) + pow(dt_dL * L, 2)) / 12 + pow(sigmaScat, 2);
      // sigma squared: channel x-size, parallax, multiple scattering, chromatic
      double wid = wid0 + pow(dt_de * m_yScanner->getRMSEnergy(), 2);

      double yB = m_fastRaytracer->getYB();
      const auto& photonStates = m_fastRaytracer->getPhotonStates();
      const auto& atPrismEntrance = photonStates[photonStates.size() - 2];
      double dydz = atPrismEntrance.getKy() / atPrismEntrance.getKz();
      if (m_fastRaytracer->getNyb() % 2 != 0) dydz = -dydz;

      bool doScan = (m_PDFOption == c_Fine);
      if (m_PDFOption == c_Optimal) {
        double time = m_tof + Len / avSpeedOfLight;
        doScan = isScanRequired(col, time, wid);
      }

      m_yScanner->expand(col, yB, dydz, D, doScan);

      double numPhotons = m_yScanner->getNumPhotons() * abs(D.dFic_dx * pixel.Dx);
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

    bool PDFConstructor::isScanRequired(unsigned /*col*/, double /*time*/, double /*wid*/)
    {
      //TODO ...
      return true;
    }

    double PDFConstructor::propagationLosses(double E, double propLen, int nx, int ny, SignalPDF::EPeakType type)
    {
      double bulk = TOPGeometryPar::Instance()->getAbsorptionLength(E);
      double surf = m_yScanner->getBars().front().reflectivity;
      double p = exp(-propLen / bulk) * pow(surf, abs(nx) + abs(ny));
      if (type == SignalPDF::c_Reflected) p *= m_yScanner->getMirror().reflectivity;
      return p;
    }

    bool PDFConstructor::rangeOfX(double z, double& xmi, double& xma)
    {
      double maxLen = (s_maxTime - m_tof) / m_groupIndex * Const::speedOfLight; // maximal propagation length
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
      dz = abs(dz);
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
        if (abs(dxdz[i]) < INFINITY) {
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

      // just to make sure xmima are within the limits given by maximal propagation length
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
      if (y1 != y1 or abs(y1) == INFINITY) return -Ah;

      double x2 = (Ah - mirror.xc) / mirror.R;
      double y2 = derivativeOfReflectedX(x2, xe, ze, zd);
      if (y2 != y2 or abs(y2) == INFINITY) return -Ah;

      if (y1 * y2 > 0) return -Ah; // no minimum or maximum

      for (int i = 0; i < 50; i++) {
        double x = (x1 + x2) / 2;
        double y = derivativeOfReflectedX(x, xe, ze, zd);
        if (y != y or abs(y) == INFINITY) return -Ah;
        if (y * y1 < 0) {
          x2 = x;
          y2 = y;
        } else {
          x1 = x;
          y1 = y;
        }
      }
      double x = (x1 + x2) / 2;

      return x * mirror.R + mirror.xc;
    }


  } // namespace TOP
} // namespace Belle2


