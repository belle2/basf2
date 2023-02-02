/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeGrouping/SVDTimeGroupingModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDTimeGrouping);

double mygaus(double* x, double* par)
{
  return par[0] * exp(-0.5 * std::pow((x[0] - par[1]) / par[2], 2)) / (sqrt(2.*TMath::Pi()) * par[2]);
}

SVDTimeGroupingModule::SVDTimeGroupingModule() :
  Module()
{
  setDescription("Imports Clusters of the SVD detector and Assign time-group Id.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("EventLevelTrackingInfoName", m_eventLevelTrackingInfoName,
           "EventLevelTrackingInfo collection name", string(""));

  // 2.Modification parameters:
  addParam("tRangeLow", m_tRangeLow, "This sets the x-range of histogram in ns.",
           double(-160.));
  addParam("tRangeHigh", m_tRangeHigh, "This sets the x-range of histogram in ns.",
           double(160.));
  addParam("expSignalLoc", m_expSignalLoc,
           "Expected location of signal.",
           double(0.));
  addParam("signalRangeLow", m_signalRangeLow,
           "Expected time range of signal hits.",
           double(-50.));
  addParam("signalRangeHigh", m_signalRangeHigh,
           "Expected time range of signal hits.",
           double(50.));
  addParam("factor", m_factor,
           "Fine divisions of histogram.",
           int(2));

  addParam("fillSigmaN", m_fillSigmaN,
           "Fill cluster times upto N sigma.",
           double(3.));
  addParam("calSigmaN", m_calSigmaN,
           "Evaluate gauss upto N sigma.",
           double(5.));
  addParam("accSigmaN", m_accSigmaN,
           "Accept hits upto N sigma.",
           double(3.));
  addParam("fracThreshold", m_fracThreshold,
           "Do not fit bellow this threshold.",
           double(0.05));
  addParam("minSigma", m_minSigma,
           "Lower limit of cluster time sigma for fit.",
           double(1.));
  addParam("maxSigma", m_maxSigma,
           "Upper limit of cluster time sigma for fit.",
           double(15.));
  addParam("timeSpread", m_timeSpread,
           "Time range for the fit.",
           double(5.));
  addParam("maxGroups", m_maxGroups,
           "Maximum groups to be accepted.",
           int(20));
  addParam("writeGroupInfo", m_writeGroupInfo,
           "Write group info into SVDClusters.",
           bool(true));

  addParam("includeOutOfRangeClusters", m_includeOutOfRangeClusters,
           "Assign groups to under and overflow.",
           bool(true));
  addParam("useOnlyOneGroup", m_useOnlyOneGroup,
           "Only one group is kept.",
           bool(false));
  addParam("exponentialSort", m_exponentialSort,
           "Group prominence is weighted with exponential weight.",
           double(30.));

  addParam("signalGroupSelection", m_signalGroupSelection,
           "Choose one group near expected signal location.",
           bool(false));
  addParam("flatSignalCut", m_flatSignalCut,
           "Select all clusters within signal range around first group.",
           bool(false));
}



void SVDTimeGroupingModule::initialize()
{
  // prepare all store:
  m_svdClusters.isRequired(m_svdClustersName);

  if (m_signalGroupSelection || m_flatSignalCut || m_useOnlyOneGroup)
    m_includeOutOfRangeClusters = false;

  if (m_useOnlyOneGroup) B2WARNING("Only the first group is kept.");
  if (m_signalGroupSelection) B2WARNING("Only the Probable signal group is chosen.");
  if (m_factor <= 0) B2WARNING("Module is ineffective.");
  if (m_tRangeHigh - m_tRangeLow < 10.) B2FATAL("tRange should not be less than 10 (hard-coded).");

  B2DEBUG(1, "SVDTimeGroupingModule \nsvdClusters: " << m_svdClusters.getName());
}



void SVDTimeGroupingModule::event()
{
  int totClusters = m_svdClusters.getEntries();
  if (m_factor <= 0 || totClusters <= 0) return;

  // number of clusters in signalRange
  int clsInSignalRange = 0;
  double tmax = -1000; double tmin = 1000;
  for (int ij = 0; ij < totClusters; ij++) {
    double clsTime = m_svdClusters[ij]->getClsTime();
    if (clsTime > m_signalRangeLow &&
        clsTime < m_signalRangeHigh)
      clsInSignalRange++;
    if (clsTime > tmax) tmax = clsTime;
    if (clsTime < tmin) tmin = clsTime;
  }
  double tRangeHigh = m_tRangeHigh;
  double tRangeLow  = m_tRangeLow;
  if (tRangeHigh > tmax) tRangeHigh = tmax;
  if (tRangeLow  < tmin) tRangeLow  = tmin;

  int xbin = tRangeHigh - tRangeLow;
  if (xbin < 1) xbin = 1;
  xbin *= m_factor;
  if (xbin < 2) xbin = 2;
  B2DEBUG(1, "tRange: [" << tRangeLow << "," << tRangeHigh << "], xBin: " << xbin);


  /** declaring histogram */
  TH1D h_clsTime = TH1D("h_clsTime", "h_clsTime", xbin, tRangeLow, tRangeHigh);
  for (int ij = 0; ij < totClusters; ij++) {
    float clsSize = m_svdClusters[ij]->getSize();
    bool  isUcls  = m_svdClusters[ij]->isUCluster();
    float gSigma  = (clsSize >= int(m_clsSizeVsSigma[isUcls].size()) ?
                     m_clsSizeVsSigma[isUcls].back() : m_clsSizeVsSigma[isUcls][clsSize - 1]);
    float gCenter = m_svdClusters[ij]->getClsTime();
    int  startBin = h_clsTime.FindBin(gCenter - m_fillSigmaN * gSigma);
    int    endBin = h_clsTime.FindBin(gCenter + m_fillSigmaN * gSigma);
    if (startBin < 1) startBin = 1;
    if (endBin > xbin) endBin = xbin;
    for (int ijx = startBin; ijx <= endBin; ijx++) {
      float tbinc = h_clsTime.GetBinCenter(ijx);
      h_clsTime.Fill(tbinc, TMath::Gaus(tbinc, gCenter, gSigma, true));
    }
  } // for (int ij = 0; ij < totClusters; ij++) {

  std::vector<std::tuple<double, double, double>> groupInfo; // pars
  double maxPeak = 0.;
  double maxNorm = 0.;
  while (1) {

    int maxBin       = h_clsTime.GetMaximumBin();
    double maxBinPos = h_clsTime.GetBinCenter(maxBin);
    double maxBinCnt = h_clsTime.GetBinContent(maxBin);
    if (maxPeak == 0 && maxBinPos > m_signalRangeLow && maxBinPos < m_signalRangeHigh)
      maxPeak = maxBinCnt;
    if (maxPeak != 0 && maxBinCnt < maxPeak * m_fracThreshold) break;

    TF1 ngaus("ngaus", mygaus, tRangeLow, tRangeHigh, 3);
    double maxPar0 = maxBinCnt * std::sqrt(2.*TMath::Pi()) * m_timeSpread;
    ngaus.SetParameter(0, maxBinCnt); ngaus.SetParLimits(0, maxPar0 * 0.01, maxPar0 * 2.);
    ngaus.SetParameter(1, maxBinPos); ngaus.SetParLimits(1, maxBinPos - m_timeSpread * 0.2, maxBinPos + m_timeSpread * 0.2);
    ngaus.SetParameter(2, m_timeSpread); ngaus.SetParLimits(2, m_minSigma, m_maxSigma);
    int status = h_clsTime.Fit("ngaus", "NQ0", "", maxBinPos - m_timeSpread, maxBinPos + m_timeSpread);
    if (!status) {
      double pars[3] = {ngaus.GetParameter(0), ngaus.GetParameter(1), std::fabs(ngaus.GetParameter(2))};
      if (pars[2] <= m_minSigma + 0.01) break;
      if (pars[2] >= m_maxSigma - 0.01) break;
      if (maxPeak != 0 && maxNorm == 0) maxNorm = pars[0];
      if (maxNorm != 0 && pars[0] < maxNorm * m_fracThreshold) break;

      int startBin = h_clsTime.FindBin(pars[1] - m_calSigmaN * pars[2]);
      int   endBin = h_clsTime.FindBin(pars[1] + m_calSigmaN * pars[2]);
      if (startBin < 1) startBin = 1;
      if (endBin > xbin)  endBin = xbin;
      for (int ijx = startBin; ijx <= endBin; ijx++) {
        float tbinc = h_clsTime.GetBinCenter(ijx);
        float tbincontent = h_clsTime.GetBinContent(ijx) - ngaus.Eval(tbinc);
        h_clsTime.SetBinContent(ijx, tbincontent);
      }

      // print
      groupInfo.push_back(std::make_tuple(pars[0], pars[1], pars[2]));
      B2DEBUG(1, " group " << int(groupInfo.size())
              << " pars[0] " << pars[0] << " pars[1] " << pars[1] << " pars[2] " << pars[2]);
      if (int(groupInfo.size()) >= m_maxGroups) break;
    } else break;   // if(!status) {

  } // while(1) {

  // resizing to max
  groupInfo.resize(m_maxGroups, std::make_tuple(0., 0., 0.));

  // sorting groups
  // possible signal first, then others
  std::tuple<double, double, double> key;
  for (int ij = int(groupInfo.size()) - 2; ij >= 0; ij--) {
    key = groupInfo[ij];
    float keynorm = std::get<0>(key);
    float keymean = std::get<1>(key);
    bool isKeySignal = true;
    if (keynorm != 0. && (keymean < m_signalRangeLow || keymean > m_signalRangeHigh)) isKeySignal = false;
    if (isKeySignal) continue;
    int kj = ij + 1;
    while (1) {
      if (kj >= int(groupInfo.size())) break;
      float grnorm = std::get<0>(groupInfo[kj]);
      float grmean = std::get<1>(groupInfo[kj]);
      bool isGrSignal = true;
      if (grnorm != 0. && (grmean < m_signalRangeLow || grmean > m_signalRangeHigh)) isGrSignal = false;
      if (!isGrSignal && (grnorm > keynorm)) break;
      groupInfo[kj - 1] = groupInfo[kj];
      kj++;
    }
    groupInfo[kj - 1] = key;
  }

  // sorting signal groups based on expo-weight
  // this decreases chance of near-signal bkg groups getting picked
  if (m_exponentialSort > 0.)
    for (int ij = 1; ij < int(groupInfo.size()); ij++) {
      key = groupInfo[ij];
      float keynorm = std::get<0>(key);
      if (keynorm <= 0) break;
      float keymean = std::get<1>(key);
      bool isKeySignal = true;
      if (keynorm > 0 && (keymean < m_signalRangeLow || keymean > m_signalRangeHigh)) isKeySignal = false;
      if (!isKeySignal) break;
      float keyWt = keynorm * TMath::Exp(-std::fabs(keymean) / m_exponentialSort);
      int kj = ij - 1;
      while (1) {
        if (kj < 0) break;
        float grnorm = std::get<0>(groupInfo[kj]);
        float grmean = std::get<1>(groupInfo[kj]);
        float grWt = grnorm * TMath::Exp(-std::fabs(grmean) / m_exponentialSort);
        if (grWt > keyWt) break;
        groupInfo[kj + 1] = groupInfo[kj];
        kj--;
      }
      groupInfo[kj + 1] = key;
    }

  if ((m_useOnlyOneGroup || m_signalGroupSelection || m_flatSignalCut)
      && int(groupInfo.size()) > 1) // keep only one group
    groupInfo.resize(1);

  if (m_flatSignalCut)
    groupInfo[0] = std::make_tuple(std::get<0>(groupInfo[0]),
                                   std::get<1>(groupInfo[0]),
                                   (m_signalRangeHigh - m_signalRangeLow) * 0.5 / m_accSigmaN);

  // make all clusters groupless if no groups are found
  if (int(groupInfo.size()) == 0)
    for (int jk = 0; jk < totClusters; jk++)
      m_svdClusters[jk]->getTimeGroupId().push_back(-1);

  for (int ij = 0; ij < int(groupInfo.size()); ij++) {
    double pars[3] = {std::get<0>(groupInfo[ij]), std::get<1>(groupInfo[ij]), std::get<2>(groupInfo[ij])};
    if (pars[2] == 0) continue;
    double beginPos = pars[1] - m_accSigmaN * pars[2];
    double   endPos = pars[1] + m_accSigmaN * pars[2];
    if (beginPos < tRangeLow) beginPos = tRangeLow;
    if (endPos > tRangeHigh)  endPos   = tRangeHigh;
    B2DEBUG(1, " group " << ij
            << " beginPos " << beginPos << " endPos " << endPos);
    for (int jk = 0; jk < totClusters; jk++) {
      double clsTime = m_svdClusters[jk]->getClsTime();
      if (clsTime >= beginPos && clsTime <= endPos) {
        m_svdClusters[jk]->getTimeGroupId().push_back(ij);
        if (m_writeGroupInfo) m_svdClusters[jk]->getTimeGroupInfo().push_back(std::make_tuple(pars[0], pars[1], pars[2]));
        B2DEBUG(1, "   accepted cluster " << jk
                << " clsTime " << clsTime);
      } else {
        B2DEBUG(1, "     rejected cluster " << jk
                << " clsTime " << clsTime);
        if (ij == int(groupInfo.size()) - 1 && !int(m_svdClusters[jk]->getTimeGroupId().size())) { // leftover clusters
          if (m_includeOutOfRangeClusters &&
              clsTime < tRangeLow)
            m_svdClusters[jk]->getTimeGroupId().push_back(ij + 1);       // underflow
          else if (m_includeOutOfRangeClusters &&
                   clsTime > tRangeHigh)
            m_svdClusters[jk]->getTimeGroupId().push_back(ij + 2);       // overflow
          else
            m_svdClusters[jk]->getTimeGroupId().push_back(-1);           // orphan
          B2DEBUG(1, "     leftover cluster " << jk
                  << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());
        }
      }
    } // for (int jk = 0; jk < totClusters; jk++) {
  } // for (int ij = 0; ij < int(groupInfo.size()); ij++) {

}

