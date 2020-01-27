/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alberto Martini                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/muid/MuidBuilder.h>

/* KLM headers. */
#include <klm/dataobjects/KLMMuidLikelihood.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <cmath>
#include <fstream>

using namespace std;
namespace Belle2 {

  MuidBuilder::MuidBuilder() : m_ReducedChiSquaredDx(0.0)
  {
    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        for (unsigned int layer = 0; layer < MUID_MaxBarrelLayer + MUID_MaxForwardEndcapLayer + 2;
             ++layer) { // MUID_MaxBarrelLayer and MUID_MaxForwardEndcapLayer are 0 index based thus an addition of 2 is needed
          m_LayerPDF[outcome][lastLayer][layer] = 0.0;
        }
      }
    }
    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {
      for (int halfNdof = 0; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        m_ReducedChiSquaredThreshold[detector][halfNdof] = 0.0;
        m_ReducedChiSquaredScaleY[detector][halfNdof] = 0.0;
        m_ReducedChiSquaredScaleX[detector][halfNdof] = 0.0;
        for (int i = 0; i < MUID_ReducedChiSquaredNbins; ++i) {
          m_ReducedChiSquaredPDF[detector][halfNdof][i] = 0.0;
          m_ReducedChiSquaredD1[detector][halfNdof][i] = 0.0;
          m_ReducedChiSquaredD2[detector][halfNdof][i] = 0.0;
          m_ReducedChiSquaredD3[detector][halfNdof][i] = 0.0;
        }
      }
    }
  }

  MuidBuilder::MuidBuilder(int expNo, const char hypothesisName[]) : m_ReducedChiSquaredDx(0.0)
  {
    //fill PDFs by reading database
    fillPDFs(hypothesisName);
    if (m_ReducedChiSquaredDx == 0.0) { B2FATAL("Failed to read " << hypothesisName << " PDFs for experiment " << expNo); }
  }

  MuidBuilder::~MuidBuilder()
  {
  }


  void MuidBuilder::fillPDFs(const char hypothesisName[])
  {
    vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
    int hypothesis = -1;
    for (unsigned int ii = 0; ii < hypotheses.size(); ii++) { if (hypothesisName == hypotheses[ii]) {hypothesis = ii; break;}}
    if (hypothesis == -1) B2FATAL("MuidBuilder::fillPDFs(): hypothesisName " << hypothesisName << "is not expected. ");

    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        if ((outcome == MuidBuilder::EMuidOutcome::c_StopInBarrel)
            && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == MuidBuilder::EMuidOutcome::c_StopInForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == MuidBuilder::EMuidOutcome::c_ExitBarrel) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == MuidBuilder::EMuidOutcome::c_ExitForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == MuidBuilder::EMuidOutcome::c_StopInBackwardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == MuidBuilder::EMuidOutcome::c_ExitBackWardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= MuidBuilder::EMuidOutcome::c_CrossBarrelStopInForwardMin)
            && (outcome <= MuidBuilder::EMuidOutcome::c_CrossBarrelStopInForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= MuidBuilder::EMuidOutcome::c_CrossBarrelStopInBackwardMin)
            && (outcome <= MuidBuilder::EMuidOutcome::c_CrossBarrelStopInBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= MuidBuilder::EMuidOutcome::c_CrossBarrelExitForwardMin)
            && (outcome <= MuidBuilder::EMuidOutcome::c_CrossBarrelExitForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= MuidBuilder::EMuidOutcome::c_CrossBarrelExitBackwardMin)
            && (outcome <= MuidBuilder::EMuidOutcome::c_CrossBarrelExitBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
        std::vector<double> layerPDF = m_muidParameters->getProfile(hypothesis, outcome, lastLayer);
        for (unsigned int layer = 0; layer < layerPDF.size(); ++layer) {
          m_LayerPDF[outcome][lastLayer][layer] = layerPDF[layer];
        }
      }
    }

    m_ReducedChiSquaredDx = MUID_ReducedChiSquaredLimit / MUID_ReducedChiSquaredNbins;   // bin size
    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {

      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        m_ReducedChiSquaredThreshold[detector][halfNdof] = m_muidParameters->getThreshold(hypothesis, detector, halfNdof * 2);
        m_ReducedChiSquaredScaleY[detector][halfNdof] = m_muidParameters->getScaleY(hypothesis, detector, halfNdof * 2);
        m_ReducedChiSquaredScaleX[detector][halfNdof] = m_muidParameters->getScaleX(hypothesis, detector, halfNdof * 2);
        std::vector<double> reducedChiSquaredPDF = m_muidParameters->getPDF(hypothesis, detector, halfNdof * 2);
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

  }

  void MuidBuilder::spline(int n, double dx, double Y[], double B[], double C[], double D[])
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


  double MuidBuilder::getPDF(const KLMMuidLikelihood* muid, bool isForward) const
  {
    return getPDFLayer(muid, isForward) * getPDFRchisq(muid);
  }


  double MuidBuilder::getPDFLayer(const KLMMuidLikelihood* muid, bool isForward) const
  {

    int outcome = muid->getOutcome();
    if ((outcome <= 0) || (outcome > c_ExitForwardEndcap)) return 0.0;

    int barrelExtLayer = muid->getBarrelExtLayer();
    int endcapExtLayer = muid->getEndcapExtLayer();
    if (barrelExtLayer > MUID_MaxBarrelLayer) return 0.0;
    if (endcapExtLayer > MUID_MaxForwardEndcapLayer) return 0.0;
    unsigned int extLayerPattern = muid->getExtLayerPattern();
    unsigned int hitLayerPattern = muid->getHitLayerPattern();

    int lastLayer = barrelExtLayer;
    if (outcome == c_StopInForwardEndcap) { // forward endcap stop (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? c_StopInForwardEndcap : c_StopInBackwardEndcap; // forward or backward endcap stop (no barrel hits)
      } else {
        outcome = (isForward ? c_CrossBarrelStopInForwardMin : c_CrossBarrelStopInBackwardMin) +
                  barrelExtLayer; // forward/backward endcap stop (B+E)
      }
    } else if (outcome == c_ExitForwardEndcap) { // forward endcap exit (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? c_ExitForwardEndcap : c_ExitBackWardEndcap;  // forward or backward endcap exit (no barrel hits)
      } else {
        outcome = (isForward ? c_CrossBarrelExitForwardMin : c_CrossBarrelExitBackwardMin) +
                  barrelExtLayer; // forward/backward endcap exit (B+E)
      }
    }


    //Pdf treatment used to avoid layer inefficiency problems
    double pdf = 1.0;
    unsigned int testBit = 1;

    for (int layer = 0; layer <= barrelExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitLayerPattern) != 0) {//checking the presence of a hit in the layer
          pdf *= m_LayerPDF[outcome][lastLayer][layer];
        } else {
          if (((layer == 0) && (outcome < c_CrossBarrelStopInForwardMin)) || (layer == MUID_MaxBarrelLayer) || (layer < barrelExtLayer)) {
            pdf *= 1 - m_LayerPDF[outcome][lastLayer][layer] * muid->getExtBKLMEfficiencyValue(layer);
          }
        }
      }
      testBit <<= 1; // move to next bit
    }

    int maxLayer = isForward ? MUID_MaxForwardEndcapLayer : MUID_MaxBackwardEndcapLayer;
    testBit = 1 << (MUID_MaxBarrelLayer + 1);
    for (int layer = 0; layer <= endcapExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitLayerPattern) != 0) {//checking the presence of a hit in the layer
          pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
        } else {
          if ((layer == 0) || (layer == maxLayer) || (layer < endcapExtLayer)) {
            pdf *= 1 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1] * muid->getExtEKLMEfficiencyValue(layer);
          }
        }
      }
      testBit <<= 1; // move to next bit
    }

    return pdf;

  }


  double MuidBuilder::getPDFRchisq(const KLMMuidLikelihood* muid) const
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
      pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * std::pow(x, halfNdof - 1.0) * std::exp(-x);
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