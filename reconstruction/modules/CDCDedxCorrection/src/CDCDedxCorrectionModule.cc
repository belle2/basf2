/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxCorrection/CDCDedxCorrectionModule.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <cmath>
#include <algorithm>
#include "TMath.h"

using namespace Belle2;
using namespace Dedx;

REG_MODULE(CDCDedxCorrection)

CDCDedxCorrectionModule::CDCDedxCorrectionModule() : Module()
{

  setDescription("Apply hit level corrections to the dE/dx measurements.");
  addParam("relativeCorrections", m_relative, "If true, apply corrections relative to those used in production", true);
  addParam("momentumCor", m_momCor, "Boolean to apply momentum correction", false);
  addParam("momentumCorFromDB", m_useDBMomCor, "Boolean to apply momentum correction from DB", false);
  addParam("scaleCor", m_scaleCor, "Boolean to apply scale correction", false);
  addParam("cosineCor", m_cosineCor, "Boolean to apply cosine correction", false);
  addParam("wireGain", m_wireGain, "Boolean to apply wire gains", false);
  addParam("runGain", m_runGain, "Boolean to apply run gain", false);
  addParam("twoDCell", m_twoDCell, "Boolean to apply 2D correction", false);
  addParam("oneDCell", m_oneDCell, "Boolean to apply 1D correction", false);
  addParam("cosineEdge", m_cosineEdge, "Boolean to apply cosine edge correction", true);
  addParam("nonlADC", m_nonlADC, "Boolean to apply non-linear adc correction", true);
  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.25));
}

CDCDedxCorrectionModule::~CDCDedxCorrectionModule() { }

void CDCDedxCorrectionModule::initialize()
{
  // register in datastore
  m_cdcDedxTracks.isRequired();

  // make sure the calibration constants are reasonable
  // run gains
  if (m_DBRunGain->getRunGain() == 0) {
    B2WARNING("Run gain is zero for this run");
  }

  // wire gains
  for (unsigned int i = 0; i < 14336; ++i) {
    if (m_DBWireGains->getWireGain(i) == 0)
      B2WARNING("Wire gain is zero for this wire: " << i);
  }

  // cosine correction (store the bin edges for extrapolation)
  unsigned int ncosbins = m_DBCosineCor->getSize();
  for (unsigned int i = 0; i < ncosbins; ++i) {
    double gain = m_DBCosineCor->getMean(i);
    if (gain == 0)
      B2ERROR("Cosine gain is zero for this bin " << i);
  }

  // get the hadron correction parameters
  if (!m_DBHadronCor || m_DBHadronCor->getSize() == 0) {
    B2WARNING("No hadron correction parameters!");
    for (int i = 0; i < 4; ++i)
      m_hadronpars.push_back(0.0);
    m_hadronpars.push_back(1.0);
  } else m_hadronpars = m_DBHadronCor->getHadronPars();
}

void CDCDedxCorrectionModule::event()
{

  // outputs
  StoreArray<CDCDedxTrack> dedxArray;

  // **************************************************
  //  LOOP OVER EACH DEDX MEASUREMENT (TRACK LEVEL)
  // **************************************************
  for (auto& dedxTrack : m_cdcDedxTracks) {
    if (dedxTrack.size() == 0) {
      B2WARNING("No good hits on this track...");
      continue;
    }

    // **************************************************
    //  LOOP OVER EACH DEDX MEASUREMENT (HIT LEVEL)
    // **************************************************
    // hit level
    int nhits = dedxTrack.size();
    double costh = dedxTrack.getCosTheta();
    std::vector<double> newLayerHits;
    double newLayerDe = 0, newLayerDx = 0;

    if (costh < TMath::Cos(150.0 * TMath::DegToRad()))continue; //-0.866
    if (costh > TMath::Cos(17.0 * TMath::DegToRad())) continue; //0.95

    for (int i = 0; i < nhits; ++i) {

      //pay attention to deadwire or gain uses
      //getADCount is already corrected w/ non linear ADC payload
      //getADCbasecount is now uncorrect ADC
      int jadcbase = dedxTrack.getADCBaseCount(i);
      double jLayer = dedxTrack.getHitLayer(i);
      double jWire = dedxTrack.getWire(i);
      double jNDocaRS = dedxTrack.getDocaRS(i) / dedxTrack.getCellHalfWidth(i);
      double jEntaRS = dedxTrack.getEntaRS(i);
      double jPath = dedxTrack.getPath(i);

      double correction = dedxTrack.m_scale * dedxTrack.m_runGain * dedxTrack.m_cosCor * dedxTrack.m_cosEdgeCor *
                          dedxTrack.getTwoDCorrection(i) * dedxTrack.getOneDCorrection(i) * dedxTrack.getNonLADCCorrection(i);
      if (dedxTrack.getWireGain(i) > 0)correction *= dedxTrack.getWireGain(i); //also keep dead wire

      //Modify hit level dedx
      double newhitdedx = (m_relative) ? 1 / correction : 1.0;
      StandardCorrection(jadcbase, jLayer, jWire, jNDocaRS, jEntaRS, jPath, costh, newhitdedx);
      dedxTrack.setDedx(i, newhitdedx);

      // do track level dedx and modifiy after loop over hits
      // rel const -> upto 6 from calibrated GT and 2 are direct from dedx track (no rel cal for them now)
      // abs const -> upto 6 from calibrated GT and 2 are direct from default GT
      if (m_relative) {
        //prewire gains need old tracks (old bad wire) and post need track new wg (new dead wire)
        //get same base adc + rel correction factor
        correction *= GetCorrection(jadcbase, jLayer, jWire, jNDocaRS, jEntaRS, costh);
        if (!m_DBWireGains && dedxTrack.getWireGain(i) == 0)correction = 0;
      } else {
        //get modifed adc + abs correction factor
        correction = GetCorrection(jadcbase, jLayer, jWire, jNDocaRS, jEntaRS, costh);
      }

      // combine hits accross layers
      if (correction != 0) {
        newLayerDe += jadcbase / correction;
        newLayerDx += jPath;
      }

      if (i + 1 < nhits && dedxTrack.getHitLayer(i + 1) == jLayer) {
        continue;
      } else {
        if (newLayerDx != 0)newLayerHits.push_back(newLayerDe / newLayerDx * std::sqrt(1 - costh * costh));
        newLayerDe = 0;
        newLayerDx = 0;
      }
    }

    // recalculate the truncated means
    calculateMeans(&(dedxTrack.m_dedxAvg),
                   &(dedxTrack.m_dedxAvgTruncatedNoSat),
                   &(dedxTrack.m_dedxAvgTruncatedErr),
                   newLayerHits);

    dedxTrack.m_dedxAvgTruncated = dedxTrack.m_dedxAvgTruncatedNoSat;
    HadronCorrection(costh, dedxTrack.m_dedxAvgTruncated);
  } // end loop over tracks
}

void CDCDedxCorrectionModule::terminate()
{

  B2INFO("CDCDedxCorrectionModule exiting...");
}

void CDCDedxCorrectionModule::RunGainCorrection(double& dedx) const
{

  double gain = m_DBRunGain->getRunGain();
  if (gain != 0) {
    dedx = dedx / gain;
  } else dedx = 0;
}

void CDCDedxCorrectionModule::WireGainCorrection(int wireID, double& dedx) const
{
  double gain = m_DBWireGains->getWireGain(wireID);
  if (gain != 0) dedx = dedx / gain;
  else {
    //rel-abs method needs all wire for cal but w/ this method post calis (e.g.final RG)
    //will also see all hitdedx but that is not an issue for track level calibration
    if (m_relative)dedx = 0;
  }
}

void CDCDedxCorrectionModule::TwoDCorrection(int layer, double doca, double enta, double& dedx) const
{

  double gain = (m_DB2DCell) ? m_DB2DCell->getMean(layer, doca, enta) : 1.0;
  if (gain != 0) dedx = dedx / gain;
  else dedx = 0;
}


void CDCDedxCorrectionModule::OneDCorrection(int layer, double enta, double& dedx) const
{
  double gain = (m_DB1DCell) ? m_DB1DCell->getMean(layer, enta) : 1.0;
  if (gain != 0) dedx = dedx / gain;
  else dedx = 0;
}

void CDCDedxCorrectionModule::CosineCorrection(double costh, double& dedx) const
{
  double coscor = m_DBCosineCor->getMean(costh);
  if (coscor != 0) dedx = dedx / coscor;
  else dedx = 0;
}

void CDCDedxCorrectionModule::CosineEdgeCorrection(double costh, double& dedx) const
{

  double cosedgecor = m_DBCosEdgeCor->getMean(costh);
  if (cosedgecor != 0) dedx = dedx / cosedgecor;
  else dedx = 0;
}


void CDCDedxCorrectionModule::HadronCorrection(double costheta, double& dedx) const
{
  dedx = D2I(costheta, I2D(costheta, 1.00) / 1.00 * dedx);
}

void CDCDedxCorrectionModule::StandardCorrection(int adc, int layer, int wireID, double doca, double enta, double length,
                                                 double costheta, double& dedx) const
{

  if (!m_relative && m_nonlADC)
    adc = m_DBNonlADC->getCorrectedADC(adc, layer);

  dedx *= adc * std::sqrt(1 - costheta * costheta) / length;

  if (m_scaleCor) {
    double scale = m_DBScaleFactor->getScaleFactor();
    if (scale != 0) dedx = dedx / scale;
    else dedx = 0;
  }

  if (m_runGain)
    RunGainCorrection(dedx);

  if (m_wireGain)
    WireGainCorrection(wireID, dedx);

  if (m_cosineCor)
    CosineCorrection(costheta, dedx);

  //only if constants are abs are for specific costh
  if (!m_relative && m_cosineEdge && (costheta <= -0.850 || costheta >= 0.950))
    CosineEdgeCorrection(costheta, dedx);

  if (m_twoDCell)
    TwoDCorrection(layer, doca, enta, dedx);

  if (m_oneDCell)
    OneDCorrection(layer, enta, dedx);

}


double CDCDedxCorrectionModule::GetCorrection(int& adc, int layer, int wireID, double doca, double enta, double costheta) const
{
  double correction = 1.0;
  if (m_scaleCor) correction *= m_DBScaleFactor->getScaleFactor();
  if (m_runGain) correction *= m_DBRunGain->getRunGain();
  if (m_wireGain) correction *= m_DBWireGains->getWireGain(wireID);
  if (m_twoDCell) correction *= m_DB2DCell->getMean(layer, doca, enta);
  if (m_oneDCell) correction *= m_DB1DCell->getMean(layer, enta);
  if (m_cosineCor) correction *= m_DBCosineCor->getMean(costheta);

  //these last two are only for abs constant
  if (!m_relative) {
    if (m_nonlADC)adc = m_DBNonlADC->getCorrectedADC(adc, layer);
    if (m_cosineEdge && (costheta <= -0.850 || costheta >= 0.950)) {
      correction *= m_DBCosEdgeCor->getMean(costheta);
    }
  }
  return correction;
}


double CDCDedxCorrectionModule::D2I(const double cosTheta, const double D) const
{
  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];
  if (projection == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + m_hadronpars[0] * chargeDensity;
  double denominator   = 1 + m_hadronpars[1] * chargeDensity;

  if (denominator == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return D;
  }

  double I = D * m_hadronpars[4] * numerator / denominator;
  return I;
}

double CDCDedxCorrectionModule::I2D(const double cosTheta, const double I) const
{
  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];

  if (projection == 0 || m_hadronpars[4] == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return I;
  }

  double a =  m_hadronpars[0] / projection;
  double b =  1 - m_hadronpars[1] / projection * (I / m_hadronpars[4]);
  double c = -1.0 * I / m_hadronpars[4];

  if (b == 0 && a == 0) {
    B2WARNING("both a and b coefficiants for hadron correction are 0");
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a) : -c / b;
  if (D < 0) {
    B2WARNING("D is less 0! will try another solution");
    D = (a != 0) ? (-b - sqrt(b * b + 4.0 * a * c)) / (2.0 * a) : -c / b;
    if (D < 0) {
      B2WARNING("D is still less 0! just return uncorrectecd value");
      return I;
    }
  }

  return D;
}


void CDCDedxCorrectionModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr,
                                             const std::vector<double>& dedx) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());
  sortedDedx.erase(std::remove(sortedDedx.begin(), sortedDedx.end(), 0), sortedDedx.end());
  sortedDedx.shrink_to_fit();

  double truncatedMeanTmp = 0.0;
  double meanTmp = 0.0;
  double sumOfSquares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();

  // add a factor of 0.51 here to make sure we are rounding appropriately...
  const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.51);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.51);
  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
    if (i >= lowEdgeTrunc and i < highEdgeTrunc) {
      truncatedMeanTmp += sortedDedx[i];
      sumOfSquares += sortedDedx[i] * sortedDedx[i];
      numValuesTrunc++;
    }
  }

  if (numDedx != 0) {
    meanTmp /= numDedx;
  }
  if (numValuesTrunc != 0) {
    truncatedMeanTmp /= numValuesTrunc;
  } else {
    truncatedMeanTmp = meanTmp;
  }

  *mean = meanTmp;
  *truncatedMean = truncatedMeanTmp;

  if (numValuesTrunc > 1) {
    *truncatedMeanErr = sqrt(sumOfSquares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(
                          numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}
