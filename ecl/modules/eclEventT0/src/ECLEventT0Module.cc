/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty hearty@physics.ubc.ca                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclEventT0/ECLEventT0Module.h>

//Root
#include <TMath.h>

//Frameork
#include <framework/dataobjects/EventT0.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>

using namespace Belle2;
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
  addParam("ethresh", m_ethresh, "Minimum energy (GeV) for an ECLCalDigit to be used", 0.06);
  addParam("maxDigitT", m_maxDigitT, "Maximum absolute time (ns) for an ECLCalDigit to be used", 150.);
  addParam("sigmaScale", m_sigmaScale, "Scale factor between dt99 and ECLCalDigit time resolution", 0.15);
  addParam("maxT0", m_maxT0, "Maximum absolute value (ns) for T0", 135.);
  addParam("T0bin", m_T0bin, "Step size between T0 hypotheses (ns)", 1.);
  addParam("primaryT0", m_primaryT0, "Select which T0 estimate is primary", 0);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void ECLEventT0Module::initialize()
{
  /** Register the data object */
  m_eventT0.registerInDataStore();
  m_eclCalDigitArray.isRequired();
}

void ECLEventT0Module::event()
{

  //-----------------------------------------------------------------
  /** Make up vector of times and uncertainties of selected digits, and record min and max */
  std::vector<float> tLike;
  std::vector<float> sigmaLike;
  float tmin = 9999.;
  int itmin = -1;
  float tmax = -9999.;
  int itmax = -1;
  for (auto& eclCalDigit : m_eclCalDigitArray) {
    float digitT = eclCalDigit.getTime();
    float dt99 = eclCalDigit.getTimeResolution();
    if (eclCalDigit.getEnergy() > m_ethresh && abs(digitT) < m_maxDigitT && dt99 < 1000.) {
      tLike.push_back(digitT);
      sigmaLike.push_back(dt99 * m_sigmaScale);
      if (digitT < tmin) {
        tmin = digitT;
        itmin = tLike.size() - 1;
      }
      if (digitT > tmax) {
        tmax = digitT;
        itmax = tLike.size() - 1;
      }
    }
  }
  int nLike = tLike.size();

  //-----------------------------------------------------------------
  /** Check that we have enough digits to run the process */
  double T0 = -99999.;
  double T0Unc = m_maxT0;
  int nT0Values = 0;
  std::vector<float> localT0;
  std::vector<float> localT0Unc;

  if (nLike == 1) {
    nT0Values = 1;
    T0 = tLike[0];
    T0Unc = sigmaLike[0];
    localT0.push_back(T0);
    localT0Unc.push_back(T0Unc);

  } else if (nLike >= 2) {

    //-----------------------------------------------------------------
    /** Minimum and maximum T0 to test. Range should be at least the specified value. */
    float minrange = 50.; /* performance is not sensitive to the particular value */
    tmin = tLike[itmin] - 3 * sigmaLike[itmin]; /* 3 sigma to ensure minima is away from boundary. Specific value is not important. */
    if (tmin < -m_maxT0) {tmin = -m_maxT0;}
    tmax = tLike[itmax] + 3 * sigmaLike[itmax];
    if (tmax > m_maxT0) {tmax = m_maxT0;}

    if (tmin > m_maxT0 - minrange) {
      tmin = m_maxT0 - minrange;
      tmax = m_maxT0;
    } else if (tmax < minrange - m_maxT0) {
      tmin = -m_maxT0;
      tmax = minrange - m_maxT0;
    } else if (tmax - tmin < minrange) {
      float drange = 0.5 * (minrange + tmin - tmax);
      tmin = tmin - drange;
      tmax = tmax + drange;
    }

    //-----------------------------------------------------------------
    /** Ready to make up the list of T0 hypotheses */
    int nhypo = (0.5 + tmax - tmin) / m_T0bin;
    float trange = nhypo * m_T0bin;
    std::vector<float> t0hypo;
    for (int it0 = 0; it0 < nhypo; it0++) {
      t0hypo.push_back(tmin + (it0 + 0.5)*m_T0bin);
    }

    //-----------------------------------------------------------------
    /** Signal fraction for each T0 is the fraction of digits within 3 sigma */
    std::vector<float> sigFracVsT0(nhypo, 0.);
    std::vector<float> chiVsT0(nhypo, 0.);
    float minChi2 = 99999.;
    int it0minChi2 = -1;
    for (int it0 = 0; it0 < nhypo; it0++) {
      float n3sig = 0.;
      for (int iLike = 0; iLike < nLike; iLike++) {
        if (abs(tLike[iLike] - t0hypo[it0]) < 3.*sigmaLike[iLike]) {n3sig++;}
      }
      float signalFrac = n3sig / nLike;
      sigFracVsT0[it0] = signalFrac;

      /** Chi square for each T0 hypothesis */
      float chi2 = 0.;
      for (int iLike = 0; iLike < nLike; iLike++) {
        float arg = (tLike[iLike] - t0hypo[it0]) / sigmaLike[iLike];
        float sigProb = sigFracVsT0[it0] * TMath::Exp(-0.5 * arg * arg) / sigmaLike[iLike] / sqrt(2.*TMath::Pi()) +
                        (1. - sigFracVsT0[it0]) / trange;
        chi2 += -2.*TMath::Log(sigProb);
      }
      chiVsT0[it0] = chi2;

      /** Record the absolute minimum chi square */
      if (chi2 < minChi2) {
        minChi2 = chi2;
        it0minChi2 = it0;
      }
    }

    //-----------------------------------------------------------------
    /* Look for local minima in the chi square vs T0 hypothesis distribution. */
    /* Local minima have negative slope on the lower side, positive slope on the upper. */
    /* Calculate slope using the bin and its neighbour, if they have the same sigFrac; */
    /* otherwise, use the closest adjacent pair that do have the same sigFrac. */

    /** Slope on the lower side of each bin */
    std::vector<float> LHslope(nhypo, 0.);
    float slope = 0.;
    for (int it0 = 1; it0 < nhypo; it0++) {
      if (sigFracVsT0[it0 - 1] == sigFracVsT0[it0]) {
        slope = chiVsT0[it0] - chiVsT0[it0 - 1];
      }
      LHslope[it0] = slope;
    }

    /** Slope on the upper side of each bin */
    std::vector<float> HLslope(nhypo, 0.);
    slope = 0.;
    for (int it0 = nhypo - 2; it0 >= 0; it0--) {
      if (sigFracVsT0[it0] == sigFracVsT0[it0 + 1]) {
        slope = chiVsT0[it0 + 1] - chiVsT0[it0];
      }
      HLslope[it0] = slope;
    }

    //-----------------------------------------------------------------
    /** Find all local minima, and keep track of the one with the best chi square */
    /*  and the one closest to 0 */
    float minLocalChi2 = chiVsT0[0];
    int it0min = -1;
    float T0ClosestTo0 = 99999.;
    int itClosest = -1;
    std::vector<int> itlocal;
    for (int it0 = 1; it0 < nhypo - 1; it0++) {
      if ((LHslope[it0] < 0. && HLslope[it0] > 0.) || (LHslope[it0] == 0. && HLslope[it0] > 0. && LHslope[it0 - 1] < 0.
                                                       && HLslope[it0 - 1] == 0.)) {
        localT0.push_back(t0hypo[it0]);
        itlocal.push_back(it0);
        if (chiVsT0[it0] < minLocalChi2) {
          minLocalChi2 = chiVsT0[it0];
          it0min = it0;
        }
        if (abs(t0hypo[it0]) < abs(T0ClosestTo0)) {
          T0ClosestTo0 = t0hypo[it0];
          itClosest = it0;
        }
      }
    }
    nT0Values = localT0.size();

    //-----------------------------------------------------------------
    /** Look for chi square to increase by 4 to calculate approx uncertainty */
    for (int imin = 0; imin < nT0Values; imin++) {
      float localUnc = m_maxT0;
      if (itlocal[imin] > 0 && itlocal[imin] < nhypo) {
        float chiTarget = chiVsT0[itlocal[imin]] + 4.;
        int ih = itlocal[imin];
        do {ih++;} while (chiVsT0[ih] < chiTarget && ih < nhypo - 1);
        int il = itlocal[imin];
        do {il--;} while (chiVsT0[il] < chiTarget && il > 0);
        localUnc = (t0hypo[ih] - t0hypo[il]) / 4.;
      }
      localT0Unc.push_back(localUnc);
    }

    //-----------------------------------------------------------------
    /** Select one value as the best T0 */
    int itsel = it0min;
    if (m_primaryT0 == 1) {itsel = it0minChi2;}
    if (m_primaryT0 == 2) {itsel = itClosest;}

    if (itsel >= 0) {
      T0 = t0hypo[itsel];

      /** look for chi sq to increase by 4 to calculate uncertainty */
      if (itsel > 0 && itsel < nhypo) {
        float chiTarget = chiVsT0[itsel] + 4.;
        int ih = itsel;
        do {ih++;} while (chiVsT0[ih] < chiTarget && ih < nhypo - 1);
        float ta = t0hypo[ih - 1];
        float tb = t0hypo[ih];
        float ca = chiVsT0[ih - 1];
        float cb = chiVsT0[ih];
        float th = tb;
        if (cb != ca) {th = ta + (tb - ta) * (chiTarget - ca) / (cb - ca);}
        int il = itsel;
        do {il--;} while (chiVsT0[il] < chiTarget && il > 0);
        ta = t0hypo[il];
        tb = t0hypo[il + 1];
        ca = chiVsT0[il];
        cb = chiVsT0[il + 1];
        float tl = ta;
        if (cb != ca) {tl = ta + (tb - ta) * (chiTarget - ca) / (cb - ca);}
        T0Unc = (th - tl) / 4.;
      }
    }
  }

  //-----------------------------------------------------------------
  /** Upload to EventT0 class */
  if (!m_eventT0) {m_eventT0.create();}

  /** Store all local minima */
  for (int it = 0; it < nT0Values; it++) {
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(localT0[it], localT0Unc[it], Const::ECL));
  }

  /** Store the selected T0 as the primary one, but only if a proper T0 value has been found */
  if (T0 > -99998.0) {
    m_eventT0->setEventT0(T0, T0Unc, Const::ECL);
  }
}
