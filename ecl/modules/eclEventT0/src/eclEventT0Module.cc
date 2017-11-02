/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: czhearty                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclEventT0/eclEventT0Module.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/dataobjects/EventT0.h>

using namespace Belle2;
using namespace ECL;
using namespace std;

int iEvent = 0;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclEventT0)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

eclEventT0Module::eclEventT0Module() : Module()
{
  // Set module properties
  setDescription("EventT0 calculated using ECLCalDigits");

  // Parameter definitions

}

void eclEventT0Module::initialize()
{

  //..ECL geometry
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();
  for (int crysID = 0; crysID < 8736; crysID++) {
    TVector3 CellPosition = eclp->GetCrystalPos(crysID);
    m_xcrys.push_back(CellPosition.X());
    m_ycrys.push_back(CellPosition.Y());
    m_zcrys.push_back(CellPosition.Z());
  }
}

void eclEventT0Module::event()
{
  iEvent++;
  //-----------------------------------------------------------------
  /** Record indices of ECLCalDigits above threshold */
  std::vector<int> iabove;
  int nCalDigit = m_eclCalDigitArray.getEntries();
  for (int idig = 0; idig < nCalDigit; idig++) {
    if (m_eclCalDigitArray[idig]->getEnergy() > m_ethresh) {iabove.push_back(idig);}
  }
  int nAbove = iabove.size();

  //-----------------------------------------------------------------
  /** Build list of digits that are isolated from each other, working from maximum
   energy downwards */
  std::vector<float> digitE;
  std::vector<float> digitT;
  std::vector<float> weightedT;
  std::vector<float> weight;
  std::vector<int> dveto;
  dveto.resize(nAbove);
  float eMax;
  do {

    /** Locate the maximum energy digit that has not been marked as being close to a
     previously selected digit */
    eMax = 0.;
    int idigitMax = -1;
    for (int ia = 0; ia < nAbove; ia++) {
      int idig = iabove[ia];
      float denergy = m_eclCalDigitArray[idig]->getEnergy();
      if (dveto[ia] != 1 && denergy > eMax) {
        eMax = denergy;
        idigitMax = idig;
      }
    }

    /** Record the maximum energy digit, if there is one */
    if (eMax > 0.) {
      float denergy = m_eclCalDigitArray[idigitMax]->getEnergy();
      float dtime = m_eclCalDigitArray[idigitMax]->getTime();
      digitE.push_back(denergy);
      digitT.push_back(dtime);
      weight.push_back(denergy * denergy);
      weightedT.push_back(denergy * denergy * dtime);

      /** Mark all of the digits close to this one (including itself) */
      int id0 = m_eclCalDigitArray[idigitMax]->getCellId() - 1;
      float x0 = m_xcrys[id0];
      float y0 = m_ycrys[id0];
      float z0 = m_zcrys[id0];
      for (int ia = 0; ia < nAbove; ia++) {
        if (dveto[ia] != 1) {
          int idig = iabove[ia];
          int id1 = m_eclCalDigitArray[idig]->getCellId() - 1;
          float x1 = m_xcrys[id1];
          float y1 = m_ycrys[id1];
          float z1 = m_zcrys[id1];
          float dr2 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0);
          if (dr2 < m_isolationDr2) {dveto[ia] = 1;}
        }
      }
    }
  } while (eMax > 0.);
  int nIsolated = digitT.size();

  //-----------------------------------------------------------------
  /** Locate and label digits that are outliers in time */
  std::vector<bool> isNotAnOutlier(nIsolated, true);
  int noutlier = nIsolated / 3;

  /** See which digit is the farthest from the average of the others.
   Repeat this "noutlier" times. */
  int firstOutlier = -1;
  for (int iout = 0; iout < noutlier; iout++) {

    /** Average time of all digits excluding one, and those previously excluded */
    int outDigit = -1;
    float delta2max = 0.;
    for (int iexcluded = 0; iexcluded < nIsolated; iexcluded++) {
      if (isNotAnOutlier[iexcluded]) {
        float sumwt = 0.;
        float sumweight = 0.;
        for (int iother = 0; iother < nIsolated; iother++) {
          if (iother != iexcluded && isNotAnOutlier[iother]) {
            sumwt += weightedT[iother];
            sumweight += weight[iother];
          }
        }
        float tother = sumwt / sumweight;
        float tsigmaOther2 = 1. / sumweight;
        float tsigmaExcl2 = 1. / weight[iexcluded];
        float delta2 = (digitT[iexcluded] - tother);
        delta2 = delta2 * delta2 / (tsigmaOther2 + tsigmaExcl2);
        if (delta2 > delta2max) {
          delta2max = delta2;
          outDigit = iexcluded;
        }
      }
    }
    isNotAnOutlier.at(outDigit) = false;
    if (iout == 0) {firstOutlier = outDigit;}
  }

  //-----------------------------------------------------------------
  /** Calculate T0. Weight is 1/sigma_t**2, where sigma_t is proportional to 1/E */
  float weightSum = 0.;
  float tWeightSum = 0.;
  for (int isel = 0; isel < nIsolated; isel++) {
    if (isNotAnOutlier[isel]) {
      weightSum += weight[isel];
      tWeightSum += weightedT[isel];
    }
  }
  double T0 = 0.;
  double WeightedUnc = m_stdDevLarge;
  if (weightSum > 0.) {
    T0 = tWeightSum / weightSum;
    WeightedUnc = m_stdDevNom * m_ethresh / sqrt(weightSum);
  }

  //-----------------------------------------------------------------
  /** Find the standard deviation of all times excluding the
   first outlier as a quality check on the T0 measurement */
  double stdDevAllBut1 = m_stdDevLarge;
  if (nIsolated >= 2) {
    double nSum = nIsolated;
    if (firstOutlier != -1) {nSum = nIsolated - 1.;}
    double mean = 0.;
    for (int isel = 0; isel < nIsolated; isel++) {
      if (isel != firstOutlier) {mean += digitT[isel];}
    }
    mean = mean / nSum;
    stdDevAllBut1 = 0.;
    for (int isel = 0; isel < nIsolated; isel++) {
      if (isel != firstOutlier) {
        stdDevAllBut1 += (digitT[isel] - mean) * (digitT[isel] - mean);
      }
    }
    stdDevAllBut1 = sqrt(stdDevAllBut1 / (nSum - 1.));
  }

  //-----------------------------------------------------------------
  /** Pick the larger of the two possible uncertainties, and upload */
  double T0Unc = WeightedUnc;
  if (stdDevAllBut1 > T0Unc) {T0Unc = stdDevAllBut1;}
  EventT0 eclT0;
  eclT0.addEventT0(T0, T0Unc, m_eclID);

  if (iEvent > 100) {return;}
  printf("%4d %6.1f %6.1f %6.1f\n", iEvent, T0, stdDevAllBut1, WeightedUnc);

}


