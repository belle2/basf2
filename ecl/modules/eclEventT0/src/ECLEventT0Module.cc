/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: czhearty                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclEventT0/ECLEventT0Module.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreObjPtr.h>


using namespace Belle2;
using namespace ECL;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLEventT0)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLEventT0Module::ECLEventT0Module() : Module()
{
  // Set module properties
  setDescription("EventT0 calculated using ECLCalDigits");
  addParam("isolationDr2", m_isolationDr2, "Miniumum distance squared between digits", 900.);
  addParam("ethresh", m_ethresh, "Minimum energy for a CalDigit to be used", 0.1);
  addParam("stdDevNom", m_stdDevNom, "Nominal time resolution ns for a CalDigit with E=ethresh", 20.);
  addParam("stdDevLarge", m_stdDevLarge, "Reported resolution for events with 0 or 1 selected CalDigits", 1000.);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void ECLEventT0Module::initialize()
{

  /** Register the data object */
  StoreObjPtr<EventT0> eventT0("EventT0");
  eventT0.registerInDataStore("EventT0");
  m_eclCalDigitArray.isRequired();

  /** ECL geometry */
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();
  m_xcrys.resize(8736);
  m_ycrys.resize(8736);
  m_zcrys.resize(8736);
  for (int crysID = 0; crysID < 8736; crysID++) {
    TVector3 CellPosition = eclp->GetCrystalPos(crysID);
    m_xcrys[crysID] = CellPosition.X();
    m_ycrys[crysID] = CellPosition.Y();
    m_zcrys[crysID] = CellPosition.Z();
  }
}

void ECLEventT0Module::event()
{
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
      const int idig = iabove[ia];
      const float denergy = m_eclCalDigitArray[idig]->getEnergy();
      if (dveto[ia] != 1 && denergy > eMax) {
        eMax = denergy;
        idigitMax = idig;
      }
    }

    /** Record the maximum energy digit, if there is one */
    if (eMax > 0.) {
      const float denergy = m_eclCalDigitArray[idigitMax]->getEnergy();
      const float dtime = m_eclCalDigitArray[idigitMax]->getTime();
      digitE.push_back(denergy);
      digitT.push_back(dtime);
      weight.push_back(denergy * denergy);
      weightedT.push_back(denergy * denergy * dtime);

      /** Mark all of the digits close to this one (including itself) */
      const int id0 = m_eclCalDigitArray[idigitMax]->getCellId() - 1;
      const float x0 = m_xcrys[id0];
      const float y0 = m_ycrys[id0];
      const float z0 = m_zcrys[id0];
      for (int ia = 0; ia < nAbove; ia++) {
        if (dveto[ia] != 1) {
          const int idig = iabove[ia];
          const int id1 = m_eclCalDigitArray[idig]->getCellId() - 1;
          const float x1 = m_xcrys[id1];
          const float y1 = m_ycrys[id1];
          const float z1 = m_zcrys[id1];
          const float dr2 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0);
          if (dr2 < m_isolationDr2) {dveto[ia] = 1;}
        }
      }
    }
  } while (eMax > 0.);
  const int nIsolated = digitT.size();

  //-----------------------------------------------------------------
  /** Locate and label digits that are outliers in time */
  std::vector<bool> isNotAnOutlier(nIsolated, true);
  const int noutlier = nIsolated / 3;

  /** See which digit is the farthest from the average of the others.
   Repeat this "noutlier" times. */
  int firstOutlier = -1;
  for (int iout = 0; iout < noutlier; iout++) {

    /** Average time of all digits excluding one, and those previously excluded */
    int outDigit = -1;
    float delta2max = -1.;
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
  StoreObjPtr<EventT0> eventT0("EventT0");
  if (!eventT0) {eventT0.create();}
  eventT0->setEventT0(T0, T0Unc, Const::ECL);
}


