/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeGroupComposer/SVDTimeGroupComposerModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDTimeGroupComposer);

double mygaus(double* x, double* par)
{
  return par[0] * exp(-0.5 * std::pow((x[0] - par[1]) / par[2], 2)) / (sqrt(2.*TMath::Pi()) * par[2]);
}

SVDTimeGroupComposerModule::SVDTimeGroupComposerModule() :
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
  addParam("averageCountPerBin", m_AverageCountPerBin,
           "This sets the bin width of histogram. Setting it zero or less disables the module.",
           double(1.));
  addParam("threshold", m_threshold, "Bin Content bellow this is not considered.",
           double(1.));

  addParam("applyCentralLimit", m_applyCentralLimit,
           "Sum adjacent bins many times to form the groups.",
           bool(true));
  addParam("expSignalLoc", m_expSignalLoc,
           "Expected location of signal.",
           double(0.));
  addParam("signalRangeLow", m_signalRangeLow,
           "Expected time range of signal hits.",
           double(-40.));
  addParam("signalRangeHigh", m_signalRangeHigh,
           "Expected time range of signal hits.",
           double(40.));
  addParam("factor", m_factor,
           "Fine divisions of histogram.",
           int(10));
  addParam("iteration", m_iteration,
           "Number of summations of the histogram.",
           int(10));

  addParam("gausFill", m_gausFill,
           "Fill cluster times with gauss uncertainity.",
           bool(true));
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
           double(0.01));
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
           int(10));
  addParam("writeGroupInfo", m_writeGroupInfo,
           "Write group info into SVDClusters.",
           bool(false));

  addParam("includeOutOfRangeClusters", m_includeOutOfRangeClusters,
           "Assign groups to under and overflow.",
           bool(true));
  addParam("useOnlyOneGroup", m_useOnlyOneGroup,
           "Only one group is kept.",
           bool(false));
  addParam("timeBasedSort", m_timeBasedSort,
           "Clusters belonging to the group nearest to zero is first.",
           bool(false));

  addParam("signalGroupSelection", m_signalGroupSelection,
           "Choose one group near expected signal location.",
           bool(false));
  addParam("flatSignalCut", m_flatSignalCut,
           "Select all clusters within signal range around first group.",
           bool(false));
}



void SVDTimeGroupComposerModule::initialize()
{
  // prepare all store:
  m_svdClusters.isRequired(m_svdClustersName);

  if (m_signalGroupSelection || m_flatSignalCut || m_useOnlyOneGroup)
    m_includeOutOfRangeClusters = false;

  if (m_useOnlyOneGroup) B2WARNING("Only the first group is kept.");
  if (m_signalGroupSelection) B2WARNING("Only the Probable signal group is chosen.");
  if (m_AverageCountPerBin <= 0 || m_factor <= 0) B2WARNING("Module is ineffective.");
  if (m_tRangeHigh - m_tRangeLow < 10.) B2FATAL("tRange should not be less than 10 (hard-coded).");

  B2DEBUG(1, "SVDTimeGroupComposerModule \nsvdClusters: " << m_svdClusters.getName());
  if (m_AverageCountPerBin > 0 && m_factor > 0) {
    B2INFO("SVDTimeGroupComposer : tRange                    = [" << m_tRangeLow << "," << m_tRangeHigh << "]");
    B2INFO("SVDTimeGroupComposer : signalRange               = [" << m_signalRangeLow << "," << m_signalRangeHigh << "]");
    B2INFO("SVDTimeGroupComposer : applyCentralLimit         = " << m_applyCentralLimit);
    if (!m_applyCentralLimit) {
      B2INFO("SVDTimeGroupComposer : averageCountPerBin        = " << m_AverageCountPerBin);
      B2INFO("SVDTimeGroupComposer : threshold                 = " << m_threshold);
    } else {
      B2INFO("SVDTimeGroupComposer : factor                    = " << m_factor);
      B2INFO("SVDTimeGroupComposer : iteration                 = " << m_iteration);
    }
    B2INFO("SVDTimeGroupComposer : includeOutOfRangeClusters = " << m_includeOutOfRangeClusters);
  } // if(m_AverageCountPerBin > 0 && m_factor > 0) {
}



void SVDTimeGroupComposerModule::event()
{
  int totClusters = m_svdClusters.getEntries();
  if (m_AverageCountPerBin <= 0 || m_factor <= 0 || totClusters <= 0) return;

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
  if (!m_gausFill)
    xbin *= clsInSignalRange / (m_signalRangeHigh - m_signalRangeLow);
  if (xbin < 1) xbin = 1;
  if (m_applyCentralLimit) xbin *= m_factor;
  else xbin /= m_AverageCountPerBin;
  if (xbin < 2) xbin = 2;
  B2DEBUG(1, "tRange: [" << tRangeLow << "," << tRangeHigh << "], xBin: " << xbin);


  /** declaring histogram */
  TH1D h_clsTime[2];
  h_clsTime[0] = TH1D("h_clsTime_0", "h_clsTime_0", xbin, tRangeLow, tRangeHigh);
  h_clsTime[1] = TH1D("h_clsTime_1", "h_clsTime_1", xbin, tRangeLow, tRangeHigh);
  for (int ij = 0; ij < totClusters; ij++) {
    if (!m_gausFill)
      h_clsTime[0].Fill(m_svdClusters[ij]->getClsTime());
    else {      // if (!m_gausFill)
      float clsSize = m_svdClusters[ij]->getSize();
      bool  isUcls  = m_svdClusters[ij]->isUCluster();
      float gSigma  = (clsSize >= int(m_clsSizeVsSigma[isUcls].size()) ?
                       m_clsSizeVsSigma[isUcls].back() : m_clsSizeVsSigma[isUcls][clsSize - 1]);
      float gCenter = m_svdClusters[ij]->getClsTime();
      int  startBin = h_clsTime[0].FindBin(gCenter - m_fillSigmaN * gSigma);
      int    endBin = h_clsTime[0].FindBin(gCenter + m_fillSigmaN * gSigma);
      if (startBin < 1) startBin = 1;
      if (endBin > xbin) endBin = xbin;
      for (int ijx = startBin; ijx <= endBin; ijx++) {
        float tbinc = h_clsTime[0].GetBinCenter(ijx);
        h_clsTime[0].Fill(tbinc, TMath::Gaus(tbinc, gCenter, gSigma, true));
      }
    } // if (!m_gausFill)
  }   // for (int ij = 0; ij < totClusters; ij++) {

  bool currentHisto = false;
  if (m_applyCentralLimit) {
    int counter = 0;
    while (1) {
      if (counter >= m_iteration) break;
      for (int ij = 1; ij <= xbin; ij++) {
        double sumc = h_clsTime[currentHisto].GetBinContent(ij);
        double suml = (ij == 1    ? 0. : h_clsTime[currentHisto].GetBinContent(ij - 1));
        double sumr = (ij == xbin ? 0. : h_clsTime[currentHisto].GetBinContent(ij + 1));
        double sum = suml + sumc + sumr;
        h_clsTime[!currentHisto].SetBinContent(ij, sum);
      } // for(int ij=1;ij<=xbin;ij++) {
      currentHisto = !currentHisto;
      counter++;
    } // while(1) {

  } else {
    for (int ij = 1; ij <= xbin; ij++) {
      double sumc = h_clsTime[currentHisto].GetBinContent(ij);
      double suml = (ij == 1    ? 0. : h_clsTime[currentHisto].GetBinContent(ij - 1));
      double sumr = (ij == xbin ? 0. : h_clsTime[currentHisto].GetBinContent(ij + 1));
      // possible background
      if (sumc <= m_threshold) sumc = 0;
      if (suml <= m_threshold) suml = 0;
      if (sumr <= m_threshold) sumr = 0;
      double sum = suml + sumc + sumr;
      h_clsTime[!currentHisto].SetBinContent(ij, sum);
    } // for (int ij = 1; ij <= xbin; ij++) {
    currentHisto = !currentHisto;
  } // if(m_applyCentralLimit) {

  if (!m_gausFill) {

    /** finalized the groups */
    int groupBegin = -1; int groupEnd = -1;
    std::vector<std::tuple<double, double, double>> groupInfo; // start, end, totCls
    for (int ij = 1; ij <= xbin; ij++) {
      double sum = h_clsTime[currentHisto].GetBinContent(ij);
      // finding group
      if (sum > 0 && groupBegin < 0 && groupEnd < 0) { groupBegin = ij;}
      if ((sum <= 0 || ij == xbin) && groupBegin > 0 && groupEnd < 0) {
        groupEnd = ij - 1;
        double clsInGroup = h_clsTime[currentHisto].Integral(groupBegin, groupEnd);
        double beginPos = h_clsTime[currentHisto].GetXaxis()->GetBinLowEdge(groupBegin);
        double endPos   = h_clsTime[currentHisto].GetXaxis()->GetBinLowEdge(groupEnd) +
                          h_clsTime[currentHisto].GetXaxis()->GetBinWidth(groupEnd);
        if (clsInGroup > 1) {
          groupInfo.push_back(std::make_tuple(beginPos, endPos, clsInGroup));
          B2DEBUG(1, " group " << ij
                  << " beginPos " << beginPos << " endPos " << endPos
                  << " totCls " << clsInGroup);
        }
        groupBegin = groupEnd = -1; // reset for new group
      }
    }

    // sorting groups in descending cluster-counts or distance from zero
    // this should help speed up the next process
    std::tuple<double, double, int> key;
    for (int ij = 1; ij < int(groupInfo.size()); ij++) {
      key = groupInfo[ij];
      int kj = ij - 1;
      if (!m_timeBasedSort) // sort in cluster size
        while ((kj >= 0) &&
               ((std::get<2>(groupInfo[kj])) < (std::get<2>(key)))) {
          groupInfo[kj + 1] = groupInfo[kj];
          kj--;
        } else    // sort in distance from zero
        while ((kj >= 0) &&
               std::min(std::fabs(std::get<0>(groupInfo[kj])), std::fabs(std::get<1>(groupInfo[kj]))) > std::min(std::fabs(std::get<0>(key)),
                   std::fabs(std::get<1>(key)))) {
          groupInfo[kj + 1] = groupInfo[kj];
          kj--;
        }
      groupInfo[kj + 1] = key;
    }

    if (m_useOnlyOneGroup && int(groupInfo.size()) > 1) // keep only one group
      groupInfo.resize(1);

    std::vector<int> rejectedCls;
    rejectedCls.assign(totClusters, -1);
    int totGroups = int(groupInfo.size());
    for (int ij = 0; ij < totGroups; ij++) {
      auto beginPos    = std::get<0>(groupInfo[ij]);
      auto endPos      = std::get<1>(groupInfo[ij]);
      auto totCls      = std::get<2>(groupInfo[ij]);

      B2DEBUG(1, " group " << ij
              << " beginPos " << beginPos << " endPos " << endPos
              << " totCls " << totCls);
      int rejectedCount = 0;
      for (int jk = 0; jk < int(rejectedCls.size()); jk++) {
        int place = rejectedCls[jk] < 0 ? jk : rejectedCls[jk];
        double clsTime = m_svdClusters[place]->getClsTime();
        if (clsTime >= beginPos && clsTime <= endPos) {
          m_svdClusters[place]->getTimeGroupId().push_back(ij);
          B2DEBUG(1, "   accepted cluster " << place
                  << " clsTime " << clsTime);
        } else {
          B2DEBUG(1, "     rejected cluster " << place
                  << " clsTime " << clsTime);
          if (ij == totGroups - 1) {                              // leftover clusters
            if (m_includeOutOfRangeClusters &&
                clsTime < tRangeLow)
              m_svdClusters[place]->getTimeGroupId().push_back(ij + 1);       // underflow
            else if (m_includeOutOfRangeClusters &&
                     clsTime > tRangeHigh)
              m_svdClusters[place]->getTimeGroupId().push_back(ij + 2);       // overflow
            else
              m_svdClusters[place]->getTimeGroupId().push_back(-1);           // orphan
            B2DEBUG(1, "     leftover cluster " << place
                    << " GroupId " << m_svdClusters[place]->getTimeGroupId().back());
          } else {
            rejectedCls[rejectedCount++] = place;
          }
        }
      } // for(int jk=0;jk<int(rejectedCls.size());jk++) {
      rejectedCls.resize(rejectedCount);
    } // for(int ij=0;ij<int(groupInfo.size());ij++) {

  } else {      // if (!m_gausFill) {

    std::vector<std::tuple<double, double, double>> groupInfo; // pars
    double maxPeak = 0.;
    double maxNorm = 0.;
    while (1) {

      int maxBin       = h_clsTime[currentHisto].GetMaximumBin();
      double maxBinPos = h_clsTime[currentHisto].GetBinCenter(maxBin);
      double maxBinCnt = h_clsTime[currentHisto].GetBinContent(maxBin);
      if (maxPeak == 0 && maxBinPos > m_signalRangeLow && maxBinPos < m_signalRangeHigh)
        maxPeak = maxBinCnt;
      if (maxPeak != 0 && maxBinCnt < maxPeak * m_fracThreshold) break;

      TF1 ngaus("ngaus", mygaus, tRangeLow, tRangeHigh, 3);
      double maxPar0 = maxBinCnt * std::sqrt(2.*TMath::Pi()) * m_timeSpread;
      ngaus.SetParameter(0, maxBinCnt); ngaus.SetParLimits(0, maxPar0 * 0.01, maxPar0 * 2.);
      ngaus.SetParameter(1, maxBinPos); ngaus.SetParLimits(1, maxBinPos - m_timeSpread * 0.2, maxBinPos + m_timeSpread * 0.2);
      ngaus.SetParameter(2, m_timeSpread); ngaus.SetParLimits(2, m_minSigma, m_maxSigma);
      int status = h_clsTime[currentHisto].Fit("ngaus", "NQ0", "", maxBinPos - m_timeSpread, maxBinPos + m_timeSpread);
      if (!status) {
        double pars[3] = {ngaus.GetParameter(0), ngaus.GetParameter(1), std::fabs(ngaus.GetParameter(2))};
        if (pars[2] <= m_minSigma + 0.01) break;
        if (pars[2] >= m_maxSigma - 0.01) break;
        if (maxPeak != 0 && maxNorm == 0) maxNorm = pars[0];
        if (maxNorm != 0 && pars[0] < maxNorm * m_fracThreshold) break;
        // std::cout<<pars[0]<<"\t"<<pars[1]<<"\t"<<pars[2]<<std::endl;

        int startBin = h_clsTime[currentHisto].FindBin(pars[1] - m_calSigmaN * pars[2]);
        int   endBin = h_clsTime[currentHisto].FindBin(pars[1] + m_calSigmaN * pars[2]);
        if (startBin < 1) startBin = 1;
        if (endBin > xbin)  endBin = xbin;
        for (int ijx = startBin; ijx <= endBin; ijx++) {
          float tbinc = h_clsTime[currentHisto].GetBinCenter(ijx);
          float tbincontent = h_clsTime[currentHisto].GetBinContent(ijx) - ngaus.Eval(tbinc);
          h_clsTime[currentHisto].SetBinContent(ijx, tbincontent);
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
    // sorting signal groups based on gauss-weightage
    for (int ij = 1; ij < int(groupInfo.size()); ij++) {
      key = groupInfo[ij];
      float keynorm = std::get<0>(key);
      if (keynorm <= 0) break;
      float keymean = std::get<1>(key);
      float keysig  = std::get<2>(key);
      bool isKeySignal = true;
      if (keynorm > 0 && (keymean < m_signalRangeLow || keymean > m_signalRangeHigh)) isKeySignal = false;
      if (!isKeySignal) break;
      int kj = ij - 1;
      while (1) {
        if (kj < 0) break;
        float grnorm = std::get<0>(groupInfo[kj]);
        float grmean = std::get<1>(groupInfo[kj]);
        if (grnorm * TMath::Exp(-std::fabs(grmean) / 10.) > keynorm * TMath::Exp(-std::fabs(keymean) / 10.)) break;
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

    if (int(groupInfo.size()) == 0) // make all clusters orphan
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
  } // if (!m_gausFill) {
}

