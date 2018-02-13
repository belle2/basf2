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

#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <time.h>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(CDCDedxCorrection)

CDCDedxCorrectionModule::CDCDedxCorrectionModule() : Module()
{

  setDescription("Apply hit level corrections to the dE/dx measurements.");

  addParam("momentumCor", m_momCor, "Boolean to apply momentum correction", true);
  addParam("momentumCorFromDB", m_useDBMomCor, "Boolean to apply momentum correction from DB", true);
  addParam("scaleCor", m_scaleCor, "Boolean to apply scale correction", true);
  addParam("cosineCor", m_cosineCor, "Boolean to apply cosine correction", true);
  addParam("wireGain", m_wireGain, "Boolean to apply wire gains", false);
  addParam("runGain", m_runGain, "Boolean to apply run gain", false);
  addParam("twoDCor", m_twoDCor, "Boolean to apply 2D correction", false);
  addParam("oneDCor", m_oneDCor, "Boolean to apply 1D correction", false);

  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.25));
}

CDCDedxCorrectionModule::~CDCDedxCorrectionModule() { }

void CDCDedxCorrectionModule::initialize()
{

  // register in datastore
  m_cdcDedxTracks.isRequired();
  m_eventMetaData.isRequired();

  // make sure the calibration constants are reasonable
  // run gains
  if (m_DBRunGain->getRunGain() == 0)
    B2WARNING("Run gain is zero!");

  // wire gains
  for (unsigned int i = 0; i < 14336; ++i) {
    if (m_DBWireGains->getWireGain(i) == 0)
      B2WARNING("Wire gain " << i << " is zero!");
  }

  // cosine correction (store the bin edges for extrapolation)
  int ncosbins = m_DBCosineCor->getNBins();
  for (int i = 0; i < ncosbins; ++i) {
    double gain = m_DBCosineCor->getMean(2 / ncosbins - 1);
    if (gain == 0)
      B2ERROR("Cosine gain is zero...");
  }

  // these are arbitrary and should be extracted from the calibration
  m_alpha = 1.35630e-02;
  m_gamma = -6.78907e-04;
  m_delta = 1.18037e-02;
  m_power = 1.66268e+00;
  m_ratio = 9.94775e-01;
}

void CDCDedxCorrectionModule::event()
{

  // outputs
  StoreArray<CDCDedxTrack> dedxArray;

  // **************************************************
  //
  //  LOOP OVER EACH DEDX MEASUREMENT (TRACK LEVEL)
  //
  // **************************************************

  int run = m_eventMetaData->getRun();

  std::vector<double> momcordb = m_DBMomentumCor->getMomCor();

  //low run #
  const float momcorrlo[50] = {
    0.936667, 0.791667, 0.763456, 0.755219, 0.758876,
    0.762439, 0.769009, 0.776787, 0.783874, 0.791462,
    0.796567, 0.80445,  0.809177, 0.815605, 0.817414,
    0.822127, 0.828355, 0.83215,  0.832959, 0.833546,
    0.840324, 0.844323, 0.847539, 0.849506, 0.850848,
    0.852272, 0.854783, 0.853612, 0.861432, 0.859428,
    0.859533, 0.862021, 0.865721, 0.868412, 0.868954,
    0.872075, 0.872732, 0.872475, 0.872152, 0.876957,
    0.87419,  0.875742, 0.874523, 0.878218, 0.873543,
    0.881054, 0.874919, 0.877849, 0.886954, 0.882283
  };

  //high run #
  const float momcorrhi[50] = {
    1.14045,  0.73178,  0.709983, 0.711266, 0.716683,
    0.727419, 0.735754, 0.74534,  0.754149, 0.761252,
    0.768799, 0.77552,  0.780306, 0.786253, 0.79139,
    0.797053, 0.800905, 0.804441, 0.807102, 0.809439,
    0.815215, 0.818581, 0.821492, 0.823083, 0.824502,
    0.828764, 0.830907, 0.831392, 0.832376, 0.833232,
    0.836063, 0.839065, 0.841527, 0.84118,  0.842779,
    0.840801, 0.844476, 0.846664, 0.848733, 0.844318,
    0.84837,  0.850549, 0.852183, 0.851242, 0.856488,
    0.852705, 0.851871, 0.852278, 0.856854, 0.856848
  };

  // momentum correction
  float momcor[50];
  for (int i = 0; i < 50; ++i) {
    if (run <  3500) momcor[i] = momcorrlo[i];
    if (run >= 3500) momcor[i] = momcorrhi[i];
  }

  for (auto& dedxTrack : m_cdcDedxTracks) {
    if (dedxTrack.size() == 0) {
      B2WARNING("No good hits on this track...");
      continue;
    }

    // **************************************************
    //
    //  LOOP OVER EACH DEDX MEASUREMENT (HIT LEVEL)
    //
    // **************************************************

    // determine Roy's corrections
    double correction = 1.06578;

    double m_p = fabs(dedxTrack.getMomentum());
    int mombin = 5.0 * m_p;
    if (m_momCor) {
      if (m_useDBMomCor) correction *= m_DBMomentumCor->getMean(m_p);
      else if (mombin >= 50 || mombin < 0) correction *= momcor[49];
      else correction *= momcor[mombin];
    }

    // layer level
    int nlhits = dedxTrack.getNLayerHits();
    for (int i = 0; i < nlhits; ++i) {
      double newdedx = dedxTrack.getLayerDedx(i) / correction;
      StandardCorrection(dedxTrack.getLayer(i), dedxTrack.getWireLongestHit(i), 1.0, 1.0, dedxTrack.getCosTheta(), newdedx);
      dedxTrack.setLayerDedx(i, newdedx);
    }

    // hit level
    int nhits = dedxTrack.size();
    std::vector<double> newLayerHits;
    double newLayerHit = 0;
    for (int i = 0; i < nhits; ++i) {
      double newhitdedx = dedxTrack.getDedx(i) / correction;
      StandardCorrection(dedxTrack.getHitLayer(i), dedxTrack.getWire(i), dedxTrack.getDoca(i), dedxTrack.getEnta(i),
                         dedxTrack.getCosTheta(), newhitdedx);
      dedxTrack.setDedx(i, newhitdedx);

      newLayerHit += newhitdedx;
      if (i + 1 < nhits && dedxTrack.getHitLayer(i + 1) == dedxTrack.getHitLayer(i))
        continue;
      else {
        newLayerHits.push_back(newLayerHit);
        newLayerHit = 0;
      }
    }

    calculateMeans(&(dedxTrack.m_dedx_avg),
                   &(dedxTrack.m_dedx_avg_truncated),
                   &(dedxTrack.m_dedx_avg_truncated_err),
                   newLayerHits);
  } // end loop over tracks
}

void CDCDedxCorrectionModule::terminate()
{

  B2INFO("CDCDedxCorrectionModule exiting...");
}

void CDCDedxCorrectionModule::RunGainCorrection(double& dedx) const
{

  double gain = m_DBRunGain->getRunGain();
  if (gain != 0) dedx = dedx / gain;
  else dedx = 0;
}

void CDCDedxCorrectionModule::WireGainCorrection(int wireID, double& dedx) const
{

  // remove bad cards <---- TEMPORARY FOR COSMICS
  if ((wireID >= 2272 && wireID <= 2288) || (wireID >= 2464 && wireID <= 2480) ||
      (wireID >= 2656 && wireID <= 2672) || (wireID >= 2848 && wireID <= 2864) ||
      (wireID >= 3040 && wireID <= 3056) || (wireID >= 3232 && wireID <= 3248) ||
      (wireID >= 3296 && wireID <= 3344)) dedx = 0;
  else {
    double gain = m_DBWireGains->getWireGain(wireID);
    if (gain != 0) dedx = dedx / gain;
    else dedx = 0;
  }
}

void CDCDedxCorrectionModule::TwoDCorrection(int layer, double doca, double enta, double& dedx) const
{

  double gain = m_DB2DCor->getMean(layer, doca, enta);
  if (gain != 0) dedx = dedx / gain;
  else dedx = 0;
}


void CDCDedxCorrectionModule::OneDCleanup(int layer, double enta, double& dedx) const
{

  double gain = m_DB1DCleanup->getMean(layer, enta);
  if (gain != 0) dedx = dedx / gain;
  else dedx = 0;
}

void CDCDedxCorrectionModule::CosineCorrection(double costh, double& dedx) const
{

  double coscor = m_DBCosineCor->getMean(costh);
  if (coscor != 0) dedx = dedx / coscor;
  else dedx = 0;
}

void CDCDedxCorrectionModule::HadronCorrection(double costheta, double& dedx) const
{

  dedx = D2I(costheta, I2D(costheta, 1.00) / 1.00 * dedx) * 550;
}

void CDCDedxCorrectionModule::StandardCorrection(int layer, int wireID, double doca, double enta, double costheta,
                                                 double& dedx) const
{

  if (m_scaleCor) {
    double scale = m_DBScaleFactor->getScaleFactor();
    scale = 48.0; // temporary for cosmics...
    if (scale != 0) dedx = dedx / scale;
    else dedx = 0;
  }

  if (m_runGain)
    RunGainCorrection(dedx);

  if (m_wireGain)
    WireGainCorrection(wireID, dedx);

  if (m_twoDCor)
    TwoDCorrection(layer, doca, enta, dedx);

  if (m_oneDCor)
    OneDCleanup(layer, enta, dedx);

  if (m_cosineCor)
    CosineCorrection(costheta, dedx);

  //HadronCorrection(costheta, dedx);
}

double CDCDedxCorrectionModule::D2I(const double cosTheta, const double D) const
{

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_power) + m_delta;
  double chargeDensity = D / projection;
  double numerator     = 1 + m_alpha * chargeDensity;
  double denominator   = 1 + m_gamma * chargeDensity;

  double I = D * m_ratio * numerator / denominator;

  return I;
}

double CDCDedxCorrectionModule::I2D(const double cosTheta, const double I) const
{

  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_power) + m_delta;

  double a =  m_alpha / projection;
  double b =  1 - m_gamma / projection * (I / m_ratio);
  double c = -1.0 * I / m_ratio;

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
