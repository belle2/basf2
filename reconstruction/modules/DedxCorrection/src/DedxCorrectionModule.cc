/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/DedxCorrection/DedxCorrectionModule.h>
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

REG_MODULE(DedxCorrection)

DedxCorrectionModule::DedxCorrectionModule() : Module()
{

  setDescription("Apply hit level corrections to the dE/dx measurements.");

  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.25));
}

DedxCorrectionModule::~DedxCorrectionModule() { }

void DedxCorrectionModule::initialize()
{

  // register in datastore
  m_cdcDedxTracks.isRequired();


  // make sure the calibration constants are reasonable
  // run gains
  if (m_DBRunGain->getRunGain() == 0)
    B2ERROR("Run gain is zero!");

  // wire gains
  for (unsigned int i = 0; i < 14336; ++i) {
    if (m_DBWireGains->getWireGain(i) == 0)
      B2ERROR("Wire gain " << i << " is zero!");
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

void DedxCorrectionModule::event()
{

  // outputs
  StoreArray<CDCDedxTrack> dedxArray;

  // **************************************************
  //
  //  LOOP OVER EACH DEDX MEASUREMENT (TRACK LEVEL)
  //
  // **************************************************

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

    // hit level
    int nhits = dedxTrack.size();
    for (int i = 0; i < nhits; ++i) {

      double newdedx = dedxTrack.getDedx(i);
      StandardCorrection(dedxTrack.getWire(i), dedxTrack.getCosTheta(), newdedx);
      dedxTrack.setDedx(i, newdedx);
    } // end loop over hits

    // layer level
    int nlhits = dedxTrack.getNLayerHits();
    for (int i = 0; i < nlhits; ++i) {

      double newdedx = dedxTrack.getLayerDedx(i);
      StandardCorrection(dedxTrack.getWireLongestHit(i), dedxTrack.getCosTheta(), newdedx);
      dedxTrack.setLayerDedx(i, newdedx);
    } // end loop over hits

    calculateMeans(&(dedxTrack.m_dedx_avg),
                   &(dedxTrack.m_dedx_avg_truncated),
                   &(dedxTrack.m_dedx_avg_truncated_err),
                   dedxTrack.m_lDedx);
  } // end loop over tracks
}

void DedxCorrectionModule::terminate()
{

  B2INFO("DedxCorrectionModule exiting...");
}

void DedxCorrectionModule::RunGainCorrection(double& dedx) const
{

  double gain = m_DBRunGain->getRunGain();
  if (gain != 0)
    dedx = dedx / gain;
}

void DedxCorrectionModule::WireGainCorrection(int wireID, double& dedx) const
{

  double gain = m_DBWireGains->getWireGain(wireID);
  if (gain != 0)
    dedx = dedx / gain;
}

void DedxCorrectionModule::CosineCorrection(double costh, double& dedx) const
{

  double coscor = m_DBCosineCor->getMean(costh);
  if (coscor != 0)
    dedx = dedx / coscor;
}

void DedxCorrectionModule::HadronCorrection(double costheta, double& dedx) const
{

  dedx = D2I(costheta, I2D(costheta, 1.00) / 1.00 * dedx) * 550;
}

void DedxCorrectionModule::StandardCorrection(int wireID, double costheta, double& dedx) const
{

  RunGainCorrection(dedx);

  WireGainCorrection(wireID, dedx);

  CosineCorrection(costheta, dedx);

  //HadronCorrection(costheta, dedx);
}

double DedxCorrectionModule::D2I(const double cosTheta, const double D) const
{

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_power) + m_delta;
  double chargeDensity = D / projection;
  double numerator     = 1 + m_alpha * chargeDensity;
  double denominator   = 1 + m_gamma * chargeDensity;

  double I = D * m_ratio * numerator / denominator;

  return I;
}

double DedxCorrectionModule::I2D(const double cosTheta, const double I) const
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


void DedxCorrectionModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr,
                                          const std::vector<double>& dedx) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());

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
