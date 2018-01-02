/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackExtrapolateG4e/MuidPar.h>
#include <tracking/dataobjects/Muid.h>

#include <fstream>
#include <exception>
#include <cmath>

#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  MuidPar::MuidPar() : m_ReducedChiSquaredDx(0.0)
  {
  }

  MuidPar::MuidPar(int expNo, const char hypothesisName[]) : m_ReducedChiSquaredDx(0.0)
  {
    fillPDFs(expNo, hypothesisName);
    if (m_ReducedChiSquaredDx == 0.0) { B2FATAL("Failed to read " << hypothesisName << " PDFs for experiment " << expNo); }
  }

  MuidPar::~MuidPar()
  {
  }

  void MuidPar::fillPDFs(int expNo, const char hypothesisName[])
  {

    int myExpNo = expNo + 1;
    char line[128];
    GearDir content("/Detector/Tracking/MuidParameters");
    bool exists = false;
    while (!exists && (myExpNo > 0)) {
      sprintf(line, "/Experiment[@exp=\"%d\"]/%s/", --myExpNo, hypothesisName);
      exists = content.exists(line);
    }
    if (exists) {
      content.append(line);
    } else {
      B2FATAL("MuidPar::fillPDFs(): Required XML content /Detector/Tracking/MuidParameters not found for expt #" << expNo <<
              " or earlier");
    }

    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      sprintf(line, "LayerProfile/Outcome[@outcome=\"%d\"]/", outcome);
      GearDir outcomeContent(content);
      outcomeContent.append(line);
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        if ((outcome == 1) && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == 2) && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == 3) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == 4) && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == 5) && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == 6) && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= 7) && (outcome <= 21) && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= 22) && (outcome <= 36) && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= 37) && (outcome <= 51) && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= 52) && (outcome <= 66) && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
        sprintf(line, "LastLayer[@layer=\"%d\"]", lastLayer);
        std::vector<double> layerPDF = outcomeContent.getArray(line);
        for (unsigned int layer = 0; layer < layerPDF.size(); ++layer) {
          m_LayerPDF[outcome][lastLayer][layer] = layerPDF[layer];
        }
      }
    }

    m_ReducedChiSquaredDx = MUID_ReducedChiSquaredLimit / MUID_ReducedChiSquaredNbins;   // bin size
    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {
      sprintf(line, "TransversePDF/BarrelAndEndcap");
      if (detector == 1) { sprintf(line, "TransversePDF/BarrelOnly"); }
      if (detector == 2) { sprintf(line, "TransversePDF/EndcapOnly"); }
      GearDir detectorContent(content);
      detectorContent.append(line);

      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Tail/Threshold", 2 * halfNdof);
        m_ReducedChiSquaredThreshold[detector][halfNdof] = detectorContent.getDouble(line);
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Tail/ScaleY", 2 * halfNdof);
        m_ReducedChiSquaredScaleY[detector][halfNdof] = detectorContent.getDouble(line);
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Tail/ScaleX", 2 * halfNdof);
        m_ReducedChiSquaredScaleX[detector][halfNdof] = detectorContent.getDouble(line);
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Histogram", 2 * halfNdof);
        std::vector<double> reducedChiSquaredPDF = detectorContent.getArray(line);
        if (reducedChiSquaredPDF.size() != MUID_ReducedChiSquaredNbins) {
          B2ERROR("TransversePDF vector for hypothesis " << hypothesisName << "  detector " << detector
                  << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MUID_ReducedChiSquaredNbins);
          m_ReducedChiSquaredDx = 0.0; // invalidate the PDFs for this hypothesis
        } else {
          for (int i = 0; i < MUID_ReducedChiSquaredNbins; ++i) {
            m_ReducedChiSquaredPDF[detector][halfNdof][i] = reducedChiSquaredPDF[i];
          }
          spline(MUID_ReducedChiSquaredNbins - 1, m_ReducedChiSquaredDx,
                 &m_ReducedChiSquaredPDF[detector][halfNdof][0],
                 &m_ReducedChiSquaredD1[detector][halfNdof][0],
                 &m_ReducedChiSquaredD2[detector][halfNdof][0],
                 &m_ReducedChiSquaredD3[detector][halfNdof][0]);
          m_ReducedChiSquaredD1[detector][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
          m_ReducedChiSquaredD2[detector][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
          m_ReducedChiSquaredD3[detector][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
        }
      }
    }

    if (myExpNo == expNo) {
      B2DEBUG(1, "MuidPar::fillPDFs(): Loaded " << hypothesisName << " PDFs for expt #" << myExpNo);
    } else {
      B2DEBUG(1, "MuidPar::fillPDFs(): Loaded " << hypothesisName << " PDFs for expt #" << myExpNo << " (requested #" << expNo << ")");
    }
  }

  void MuidPar::spline(int n, double dx, double Y[], double B[], double C[], double D[])
  {

    // Generate the spline interpolation coefficients B, C, D to smooth out a
    // binned histogram. Restrictions:  equal-size bins. more than 3 bins.

    D[0] = dx;
    C[1] = (Y[1] - Y[0]) / dx;
    for (int i = 1; i < n - 1; i++) {
      D[i]   = dx;
      B[i]   = dx * 4.0;
      C[i + 1] = (Y[i + 1] - Y[i]) / dx;
      C[i]   = C[i + 1] - C[i];
    }
    B[0]   = -dx;
    B[n - 1] = -dx;
    C[0]   = (C[2]   - C[1]) / 6.0;
    C[n - 1] = -(C[n - 2] - C[n - 3]) / 6.0;
    for (int i = 1; i < n; i++) {
      double temp = dx / B[i - 1];
      B[i] -= temp * dx;
      C[i] -= temp * C[i - 1];
    }
    C[n - 1] /= B[n - 1];
    for (int i = n - 2; i >= 0; i--) {
      C[i] = (C[i] - D[i] * C[i + 1]) / B[i];
    }
    B[n - 1] = (Y[n - 1] - Y[n - 2]) / dx + (C[n - 2] + C[n - 1] * 2.0) * dx;
    for (int i = 0; i < n - 1; i++) {
      B[i] = (Y[i + 1] - Y[i]) / dx - (C[i + 1] + C[i] * 2.0) * dx;
      D[i] = (C[i + 1] - C[i]) / dx;
      C[i] = C[i] * 3.0;
    }
    C[n - 1] = C[n - 1] * 3.0;
    D[n - 1] = D[n - 2];
  }

  double MuidPar::getPDF(const Muid* muid, bool isForward) const
  {
    return getPDFLayer(muid, isForward) * getPDFRchisq(muid);
  }

  double MuidPar::getPDFLayer(const Muid* muid, bool isForward) const
  {

    // outcome:  0=Not in KLM, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit

    int outcome = muid->getOutcome();
    if ((outcome <= 0) || (outcome > 4)) return 0.0;

    int barrelExtLayer = muid->getBarrelExtLayer();
    int endcapExtLayer = muid->getEndcapExtLayer();
    if (barrelExtLayer > MUID_MaxBarrelLayer) return 0.0;
    if (endcapExtLayer > MUID_MaxForwardEndcapLayer) return 0.0;
    unsigned int extLayerPattern = muid->getExtLayerPattern();
    unsigned int hitLayerPattern = muid->getHitLayerPattern();

    // Use finer granularity for non-zero outcome:
    //  1: stop in barrel
    //  2: stop in forward endcap (without crossing barrel)
    //  3: exit from barrel (without crossing endcap)
    //  4: exit from forward endcap (without crossing barrel)
    //  5: stop in forward endcap (after crossing barrel)
    //  6: exit from forward endcap (after crossing barrel)
    //  7-21: stop in forward endcap (after crossing barrel)
    //  22-36: stop in backward endcap (after crossing barrel)
    //  37-51: exit from forward endcap (after crossing barrel)
    //  52-66: exit from backward endcap (after crossing barrel)

    int lastLayer = barrelExtLayer;
    if (outcome == 2) { // forward endcap stop (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? 2 : 5; // forward or backward endcap stop (no barrel hits)
      } else {
        outcome = (isForward ? 7 : 22) + barrelExtLayer; // forward/backward endcap stop (B+E)
      }
    } else if (outcome == 4) { // forward endcap exit (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? 4 : 6;  // forward or backward endcap exit (no barrel hits)
      } else {
        outcome = (isForward ? 37 : 52) + barrelExtLayer; // forward/backward endcap exit (B+E)
      }
    }

    // Get probability density that each crossed layer should have a hit.

    double pdf = 1.0;
    unsigned int testBit = 1;
    for (int layer = 0; layer <= barrelExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitLayerPattern) != 0) {
          pdf *= m_LayerPDF[outcome][lastLayer][layer];
        } else {
          if (((layer == 0) && (outcome < 7)) || (layer == MUID_MaxBarrelLayer) || (layer < barrelExtLayer))
            pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer]);
        }
      }
      testBit <<= 1; // move to next bit
    }
    int maxLayer = isForward ? MUID_MaxForwardEndcapLayer : MUID_MaxBackwardEndcapLayer;
    testBit = 1 << (MUID_MaxBarrelLayer + 1);
    for (int layer = 0; layer <= endcapExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitLayerPattern) != 0) {
          pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
        } else {
          if ((layer == 0) || (layer == maxLayer) || (layer < endcapExtLayer))
            pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
        }
      }
      testBit <<= 1; // move to next bit
    }

    return pdf;

  }

  double MuidPar::getPDFRchisq(const Muid* muid) const
  {

    // Evaluate the transverse-coordinate PDF for this particleID hypothesis.

    int ndof = muid->getDegreesOfFreedom();
    if (ndof <= 0) return 1.0;
    double chiSquared = muid->getChiSquared();
    if (chiSquared < 0.0) return 0.0;
    int halfNdof = (ndof >> 1);
    double x = chiSquared / ndof;

    // Assume that the track crossed both barrel and endcap
    int detector = 0;
    if (muid->getEndcapExtLayer() < 0) {
      detector = 1; // crossed barrel only
    } else if (muid->getBarrelExtLayer() < 0) {
      detector = 2;  // crossed endcap only
    }

    // Use spline interpolation of the logarithms of the PDF to avoid binning artifacts.
    // Use fitted tail function for reduced-chiSquared values beyond the tabulated threshold.

    double pdf = 0.0;
    if (halfNdof > MUID_MaxHalfNdof) { // extremely rare
      x *= m_ReducedChiSquaredScaleX[detector][MUID_MaxHalfNdof] * halfNdof;
      if (halfNdof == 1) {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * std::exp(-x);
      } else if (halfNdof == 2) {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * x * std::exp(-x);
      } else if (halfNdof == 3) {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * x * x * std::exp(-x);
      } else {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * std::pow(x, halfNdof - 1.0) * std::exp(-x);
      }
    } else {
      if (x > m_ReducedChiSquaredThreshold[detector][halfNdof]) { // tail function for large x
        x *= m_ReducedChiSquaredScaleX[detector][halfNdof] * halfNdof;
        if (halfNdof == 1) {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * std::exp(-x);
        } else if (halfNdof == 2) {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * x * std::exp(-x);
        } else if (halfNdof == 3) {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * x * x * std::exp(-x);
        } else {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * std::pow(x, halfNdof - 1.0) * std::exp(-x);
        }
      } else { // spline-interpolated histogram for small x
        x -= 0.5 * m_ReducedChiSquaredDx;
        int i  = (int)(x / m_ReducedChiSquaredDx);
        double logPdf = m_ReducedChiSquaredPDF[detector][halfNdof][i];
        double dx = x - i * m_ReducedChiSquaredDx;
        logPdf += dx * (m_ReducedChiSquaredD1[detector][halfNdof][i] +
                        dx * (m_ReducedChiSquaredD2[detector][halfNdof][i] +
                              dx * m_ReducedChiSquaredD3[detector][halfNdof][i]));
        pdf = std::exp(logPdf);
      }
    }
    return pdf;
  }

} // end of namespace Belle2
