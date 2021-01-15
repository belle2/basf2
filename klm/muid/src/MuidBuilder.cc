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
#include <framework/utilities/Spline.h>

/* C++ headers. */
#include <cmath>

using namespace Belle2;

MuidBuilder::MuidBuilder() : m_ReducedChiSquaredDx(0.0)
{
  for (int outcome = 1; outcome <= MuidElementNumbers::getMaximalOutcome(); ++outcome) {
    for (int lastLayer = 0; lastLayer <= MuidElementNumbers::getMaximalBarrelLayer(); ++lastLayer) {
      /* MuidElementNumbers::getMaximalBarrelLayer() and MuidElementNumbers::getMaximalEndcapForwardLayer() are 0 index based thus an addition of 2 is needed. */
      for (unsigned int layer = 0;
           layer < MuidElementNumbers::getMaximalBarrelLayer() + MuidElementNumbers::getMaximalEndcapForwardLayer() + 2;
           ++layer) {
        m_LayerPDF[outcome][lastLayer][layer] = 0.0;
      }
    }
  }
  for (int detector = 0; detector <= MuidElementNumbers::getMaximalDetector(); ++detector) {
    for (int halfNdof = 0; halfNdof <= MuidElementNumbers::getMaximalHalfNdof(); ++halfNdof) {
      m_ReducedChiSquaredThreshold[detector][halfNdof] = 0.0;
      m_ReducedChiSquaredScaleY[detector][halfNdof] = 0.0;
      m_ReducedChiSquaredScaleX[detector][halfNdof] = 0.0;
      for (int i = 0; i < MuidElementNumbers::getSizeReducedChiSquared(); ++i) {
        m_ReducedChiSquaredPDF[detector][halfNdof][i] = 0.0;
        m_ReducedChiSquaredD1[detector][halfNdof][i] = 0.0;
        m_ReducedChiSquaredD2[detector][halfNdof][i] = 0.0;
        m_ReducedChiSquaredD3[detector][halfNdof][i] = 0.0;
      }
    }
  }
}

MuidBuilder::MuidBuilder(int pdg) : m_ReducedChiSquaredDx(0.0)
{
  MuidElementNumbers::Hypothesis hypothesis = MuidElementNumbers::calculateHypothesisFromPDG(pdg);
  if (hypothesis == MuidElementNumbers::c_NotValid)
    B2FATAL("The particle associated to the PDG code " << pdg << " is not supported.");
  /* Fill PDFs by reading database. */
  fillPDFs(hypothesis);
  if (m_ReducedChiSquaredDx == 0.0)
    B2FATAL("Invalid PDFs for PDG code " <<  pdg);
}

MuidBuilder::~MuidBuilder()
{
}

void MuidBuilder::fillPDFs(MuidElementNumbers::Hypothesis hypothesis)
{
  for (int outcome = 1; outcome <= MuidElementNumbers::getMaximalOutcome(); ++outcome) {
    for (int lastLayer = 0; lastLayer <= MuidElementNumbers::getMaximalBarrelLayer(); ++lastLayer) {
      if (!(MuidElementNumbers::checkExtrapolationOutcome(outcome, lastLayer)))
        break;
      std::vector<double> layerPDF = m_LikelihoodParameters->getLongitudinalPDF(hypothesis, outcome, lastLayer);
      for (unsigned int layer = 0; layer < layerPDF.size(); ++layer) {
        m_LayerPDF[outcome][lastLayer][layer] = layerPDF[layer];
      }
    }
  }
  m_ReducedChiSquaredDx = MuidElementNumbers::getMaximalReducedChiSquared() /
                          MuidElementNumbers::getSizeReducedChiSquared(); // bin size
  for (int detector = 0; detector <= MuidElementNumbers::getMaximalDetector(); ++detector) {

    for (int halfNdof = 1; halfNdof <= MuidElementNumbers::getMaximalHalfNdof(); ++halfNdof) {
      m_ReducedChiSquaredThreshold[detector][halfNdof] = m_LikelihoodParameters->getTransverseThreshold(hypothesis, detector,
                                                         halfNdof * 2);
      m_ReducedChiSquaredScaleY[detector][halfNdof] = m_LikelihoodParameters->getTransverseScaleY(hypothesis, detector, halfNdof * 2);
      m_ReducedChiSquaredScaleX[detector][halfNdof] = m_LikelihoodParameters->getTransverseScaleX(hypothesis, detector, halfNdof * 2);
      std::vector<double> reducedChiSquaredPDF = m_LikelihoodParameters->getTransversePDF(hypothesis, detector, halfNdof * 2);
      if (reducedChiSquaredPDF.size() != MuidElementNumbers::getSizeReducedChiSquared()) {
        B2ERROR("TransversePDF vector for hypothesis " << hypothesis << "  detector " << detector
                << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MuidElementNumbers::getSizeReducedChiSquared());
        m_ReducedChiSquaredDx = 0.0; /* Invalidate the PDFs for this hypothesis. */
      } else {
        for (int i = 0; i < MuidElementNumbers::getSizeReducedChiSquared(); ++i) {
          m_ReducedChiSquaredPDF[detector][halfNdof][i] = reducedChiSquaredPDF[i];
        }
        Spline::muidSpline(MuidElementNumbers::getSizeReducedChiSquared() - 1, m_ReducedChiSquaredDx,
                           &m_ReducedChiSquaredPDF[detector][halfNdof][0],
                           &m_ReducedChiSquaredD1[detector][halfNdof][0],
                           &m_ReducedChiSquaredD2[detector][halfNdof][0],
                           &m_ReducedChiSquaredD3[detector][halfNdof][0]);
        m_ReducedChiSquaredD1[detector][halfNdof][MuidElementNumbers::getSizeReducedChiSquared() - 1] = 0.0;
        m_ReducedChiSquaredD2[detector][halfNdof][MuidElementNumbers::getSizeReducedChiSquared() - 1] = 0.0;
        m_ReducedChiSquaredD3[detector][halfNdof][MuidElementNumbers::getSizeReducedChiSquared() - 1] = 0.0;
      }
    }
  }
}

double MuidBuilder::getPDF(const KLMMuidLikelihood* muid) const
{
  return getLongitudinalPDF(muid) * getTransversePDF(muid);
}

double MuidBuilder::getLongitudinalPDF(const KLMMuidLikelihood* muid) const
{
  /* Setup the main ingredients for the calculation. */
  unsigned int outcome = muid->getOutcome();
  if ((outcome <= MuidElementNumbers::c_NotReached) || (outcome > MuidElementNumbers::getMaximalOutcome()))
    return 0.0;
  int barrelExtLayer = muid->getBarrelExtLayer();
  if (barrelExtLayer > MuidElementNumbers::getMaximalBarrelLayer())
    return 0.0;
  int endcapExtLayer = muid->getEndcapExtLayer();
  if (endcapExtLayer > MuidElementNumbers::getMaximalEndcapForwardLayer())
    return 0.0;
  unsigned int extLayerPattern = muid->getExtLayerPattern();
  unsigned int hitLayerPattern = muid->getHitLayerPattern();
  int lastLayer = (endcapExtLayer < 0) ? barrelExtLayer : endcapExtLayer;

  /* Longitudinal PDF computation for barrel. */
  double pdf = 1.0;
  unsigned int testBit = 1;
  for (int layer = 0; layer <= barrelExtLayer; ++layer) {
    if ((testBit & extLayerPattern) != 0) {
      if ((testBit & hitLayerPattern) != 0) { /* Checking the presence of a hit in the layer. */
        pdf *= m_LayerPDF[outcome][lastLayer][layer];
      } else {
        if (((layer == 0) && (outcome < MuidElementNumbers::c_CrossBarrelStopInForwardMin))
            || (layer == MuidElementNumbers::getMaximalBarrelLayer()) || (layer < barrelExtLayer)) {
          pdf *= 1 - m_LayerPDF[outcome][lastLayer][layer] * muid->getExtBKLMEfficiencyValue(layer);
        }
      }
    }
    testBit <<= 1; /* Move to next bit. */
  }
  /* Longitudinal PDF computation for endcap. */
  int maxLayer = muid->getIsForward() ? MuidElementNumbers::getMaximalEndcapForwardLayer() :
                 MuidElementNumbers::getMaximalEndcapBackwardLayer();
  testBit = 1 << (MuidElementNumbers::getMaximalBarrelLayer() + 1);
  for (int layer = 0; layer <= endcapExtLayer; ++layer) {
    if ((testBit & extLayerPattern) != 0) {
      if ((testBit & hitLayerPattern) != 0) { /* Checking the presence of a hit in the layer. */
        pdf *= m_LayerPDF[outcome][lastLayer][layer + MuidElementNumbers::getMaximalBarrelLayer() + 1];
      } else {
        if ((layer == 0) || (layer == maxLayer) || (layer < endcapExtLayer)) {
          pdf *= 1 - m_LayerPDF[outcome][lastLayer][layer + MuidElementNumbers::getMaximalBarrelLayer() + 1] *
                 muid->getExtEKLMEfficiencyValue(layer);
        }
      }
    }
    testBit <<= 1; /* move to next bit. */
  }
  return pdf;
}

double MuidBuilder::getTransversePDF(const KLMMuidLikelihood* muid) const
{
  /* Evaluate the transverse-coordinate PDF for this particleID hypothesis. */
  int ndof = muid->getDegreesOfFreedom();
  if (ndof <= 0)
    return 1.0;
  double chiSquared = muid->getChiSquared();
  if (chiSquared < 0.0)
    return 0.0;
  int halfNdof = (ndof >> 1);
  double x = chiSquared / ndof;

  /* Assume that the track crossed both barrel and endcap. */
  int detector = MuidElementNumbers::c_Both;
  if (muid->getEndcapExtLayer() < 0) {
    detector = MuidElementNumbers::c_OnlyBarrel; /* Crossed barrel only. */
  } else if (muid->getBarrelExtLayer() < 0) {
    detector = MuidElementNumbers::c_OnlyEndcap;  /* Crossed endcap only. */
  }

  /* Use spline interpolation of the logarithms of the PDF to avoid binning artifacts.
   * Use fitted tail function for reduced-chiSquared values beyond the tabulated threshold. */
  double pdf = 0.0;
  if (halfNdof > MuidElementNumbers::getMaximalHalfNdof()) { /* Extremely rare. */
    x *= m_ReducedChiSquaredScaleX[detector][MuidElementNumbers::getMaximalHalfNdof()] * halfNdof;
    pdf = m_ReducedChiSquaredScaleY[detector][MuidElementNumbers::getMaximalHalfNdof()] * std::pow(x, halfNdof - 1.0) * std::exp(-x);
  } else {
    if (x > m_ReducedChiSquaredThreshold[detector][halfNdof]) { /* Tail function for large x. */
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
    } else { /* Spline-interpolated histogram for small x. */
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
