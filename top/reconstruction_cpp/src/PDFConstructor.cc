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
      int numRows = pixelPositions.getNumPixelRows();
      int numCols = pixelPositions.getNumPixelColumns();
      int numPixels = numRows * numCols;

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

      // loop over reflections and over pixel columns

      for (int k = kmi; k <= kma; k++) {
        for (unsigned col = 0; col < pixelPositions.getNumPixelColumns(); col++) {
          const auto& pixel = pixelPositions.get(0, col);
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
      bool ok = rangeOfX(mirror.zb, xmi, xma); //TODO: z emission >= zb
      if (not ok) return;
      int kmi = lround(xmi / bar.A);
      int kma = lround(xma / bar.A);

      // loop over reflections before mirror

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

      // determine the range of number of reflections after mirror

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

      // loop over reflections after mirror and over pixel columns

      for (int k = kmi; k <= kma; k++) {
        for (unsigned col = 0; col < pixelPositions.getNumPixelColumns(); col++) {
          const auto& pixel = pixelPositions.get(0, col);
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

      double x1 = 0;
      double y1 = deltaXD(x1, sol, xD);
      if (isnan(y1)) return false;
      if (abs(y1) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);

      double step = -dFic_dx * y1;
      for (int i = 1; i < 20; i++) { // search for zero-crossing interval
        double x2 = step * i;
        double y2 = deltaXD(x2, sol, xD);
        if (isnan(y2)) return false;
        if (abs(y2) < precision) return m_fastRaytracer->getTotalReflStatus(m_cosTotal);
        if (y2 * y1 < 0) { // zero-crossing interval identified
          for (int k = 0; k < 20; k++) { // find zero-crossing x with bisection
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
          return m_fastRaytracer->getTotalReflStatus(m_cosTotal); //TODO: poseben 'zvezen' mode za fast raytracer?
        }
        x1 = x2;
        y1 = y2;
      }

      B2ERROR("PDFConstructor::doRaytracingCorrections: zero-crossing interval not found");
      return false;
    }


    void PDFConstructor::expandSignalPDF(unsigned col, const YScanner::Derivatives& D, SignalPDF::EPeakType type)
    {
      m_ncallsExpandPDF[type]++;

      double Len = m_fastRaytracer->getPropagationLen();
      double avSpeedOfLight = Const::speedOfLight / m_groupIndex; // average speed of light in quartz
      double dt_de = (D.dLen_de + Len * m_groupIndexDerivative / m_groupIndex) / avSpeedOfLight; // cromatic
      double dt_dx =  D.dLen_dx / avSpeedOfLight; // channel x-size
      double dt_dL = (D.dLen_dL + 1 / m_yScanner->getBeta() / m_groupIndex) / avSpeedOfLight; // parallax
      double sigmaScat = D.dLen_de * m_yScanner->getSigmaScattering() / avSpeedOfLight; // multiple scattering in quartz

      const auto& pixel = m_yScanner->getPixelPositions().get(0, col);
      double L = m_yScanner->getTrackLengthInQuartz();
      double wid0 = (pow(dt_dx * pixel.Dx, 2) + pow(dt_dL * L, 2)) / 12 + pow(sigmaScat, 2);
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
      unsigned numCols = m_yScanner->getPixelPositions().getNumPixelColumns();
      for (const auto& result : m_yScanner->getResults()) {
        unsigned pixelID = col + result.row * numCols + 1;
        double RQE = 1; // TODO: channel masking, rel. efficiency
        if (RQE == 0) continue;
        auto& signalPDF = m_signalPDFs[pixelID - 1];
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


    double PDFConstructor::propagationLosses(double /*E*/, double /*propLen*/, int /*nx*/, int /*ny*/, SignalPDF::EPeakType /*type*/)
    {
      //TODO ...
      return 1;
    }

    bool PDFConstructor::rangeOfX(double z, double& xmi, double& xma)
    {
      double maxLen = (s_maxTime - m_tof) / m_groupIndex * Const::speedOfLight;
      if (maxLen < 0) return false;

      const auto& emission = m_track.getEmissionPoint();
      const auto& trk = emission.trackAngles;
      const auto& cer = cerenkovAngle();

      double dz = z - emission.position.Z();
      double cosFicLimit = (trk.cosTh * cer.cosThc - dz / maxLen) / (trk.sinTh * cer.sinThc); // at maxLen
      if (copysign(cosFicLimit, dz) < -1) return false;

      double x0 = emission.position.X();
      double cosTotal = m_yScanner->getCosTotal();
      double deltaX = maxLen * cosTotal; // from total reflection requirement (|kx| < cosTotal)
      xmi = x0 - deltaX;
      xma = x0 + deltaX;

      double theta = acos(trk.cosTh);
      if (dz < 0) theta = M_PI - theta; // rotation around x for pi (z -> -z, phi -> -phi)
      dz = abs(dz);
      double thetaCer = acos(cer.cosThc);
      if (theta - thetaCer >= M_PI / 2) return false;

      std::vector<double> x;
      if (trk.sinFi == 0) {
        x.push_back(tan(theta - thetaCer) * trk.cosFi);
        if (theta + thetaCer < M_PI / 2) x.push_back(tan(theta + thetaCer) * trk.cosFi);
        else x.push_back(INFINITY);
      } else {
        double a = -cos(theta + thetaCer) * cos(theta - thetaCer);
        double b = sin(2 * theta) * trk.cosFi;
        double c = pow(trk.sinFi * cer.sinThc, 2) - pow(trk.cosFi, 2) * sin(theta + thetaCer) * sin(theta - thetaCer);
        if (a == 0) {
          if (b == 0) return true;
          x.push_back(-c / b);
          x.push_back(copysign(INFINITY, b));
        } else {
          double D = b * b - 4 * a * c;
          if (D < 0) return true;
          D = sqrt(D);
          x.push_back((-b - D) / 2 / a);
          x.push_back((-b + D) / 2 / a);
          for (int i = 0; i < 2; i++) {
            double aa = (x[i] * cos(theta) - trk.cosFi * sin(theta)) * cer.cosThc;
            double bb = (x[i] * sin(theta) - trk.cosFi * cos(theta)) * cer.sinThc;
            double dd = trk.sinFi * cer.sinThc;
            double cosFic = aa * bb / (bb * bb + dd * dd);
            double kz = cos(theta) * cer.cosThc - sin(theta) * cer.sinThc * cosFic;
            if (kz < 0) x[i] = copysign(INFINITY, x[1 - i] - x[i]);
          }
        }
      }
      std::sort(x.begin(), x.end());

      xmi = std::max(xmi, x0 + dz * x[0]);
      xma = std::min(xma, x0 + dz * x[1]);

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


