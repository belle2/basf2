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

SVDTimeGroupComposerModule::SVDTimeGroupComposerModule() :
  Module()
{
  setDescription("Imports Clusters of the SVD detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("EventLevelTrackingInfoName", m_eventLevelTrackingInfoName,
           "EventLevelTrackingInfo collection name", string(""));

  // 2.Modification parameters:
  addParam("AverageCountPerBin", m_AverageCountPerBin, "This sets the bin width of histogram.",
           double(1.));
  addParam("XRange", m_xRange, "This sets the x-range of histogram.",
           double(200.));
  addParam("RemoveBaseline", m_removeBaseline, "Bin Content bellow this is not considered.",
           double(2.));

}



void SVDTimeGroupComposerModule::initialize()
{
  // prepare all store:
  m_svdClusters.isRequired(m_svdClustersName);

  B2DEBUG(1, "SVDTimeGroupComposerModule \nsvdClusters: " << m_svdClusters.getName());
}



void SVDTimeGroupComposerModule::event()
{
  int totClusters = m_svdClusters.getEntries();

  /** declaring histogram */
  int xbin = totClusters / m_AverageCountPerBin;
  TH1D h_clsTime("h_clsTime", "h_clsTime", xbin, -m_xRange, m_xRange);
  for (int ij = 0; ij < totClusters; ij++) {
    h_clsTime.Fill(m_svdClusters[ij]->getClsTime());
  }

  /** finalized the groups */
  int groupBegin = -1; int groupEnd = -1;
  std::vector<std::tuple<double, double, int>> groupInfo; // start, end, totCls
  for (int ij = 2; ij < h_clsTime.GetNbinsX(); ij++) {
    double sumc = h_clsTime.GetBinContent(ij);
    double suml = h_clsTime.GetBinContent(ij - 1);
    double sumr = h_clsTime.GetBinContent(ij + 1);
    // possible background
    if (sumc <= 2) sumc = 0;
    if (suml <= 2) suml = 0;
    if (sumr <= 2) sumr = 0;

    double sum = suml + sumc + sumr;
    // finding group
    if (sum > 0 && groupBegin < 0 && groupEnd < 0) { groupBegin = ij;}
    if (sum <= 0 && groupBegin > 0 && groupEnd < 0) {
      groupEnd = ij - 1;
      int clsInGroup = h_clsTime.Integral(groupBegin, groupEnd);
      double beginPos = h_clsTime.GetXaxis()->GetBinLowEdge(groupBegin);
      double endPos   = h_clsTime.GetXaxis()->GetBinLowEdge(groupEnd) +
                        h_clsTime.GetXaxis()->GetBinWidth(groupEnd);
      if (clsInGroup > 1) {
        groupInfo.push_back(std::make_tuple(beginPos, endPos, clsInGroup));
      }
      groupBegin = groupEnd = -1; // reset for new group
    }
  }

  // sorting groups in descending cluster-counts
  // this should help speed up the next process
  std::tuple<double, double, int> key;
  for (int ij = 1; ij < int(groupInfo.size()); ij++) {
    key = groupInfo[ij];
    int kj = ij - 1;
    while ((kj >= 0) &&
           ((std::get<2>(groupInfo[kj])) < (std::get<2>(key)))) {
      groupInfo[kj + 1] = groupInfo[kj];
      kj--;
    }
    groupInfo[kj + 1] = key;
  }

  std::vector<int> rejectedCls;
  rejectedCls.assign(totClusters, -1);
  int totGroups = int(groupInfo.size());
  for (int ij = 0; ij < totGroups; ij++) {
    auto beginPos    = std::get<0>(groupInfo[ij]);
    auto endPos      = std::get<1>(groupInfo[ij]);
    auto totCls      = std::get<2>(groupInfo[ij]);

    B2DEBUG(1, "SVDTimeGroupComposerModule group " << ij
            << " beginPos " << beginPos << " endPos " << endPos
            << " totCls " << totCls);
    int rejectedCount = 0;
    for (int jk = 0; jk < int(rejectedCls.size()); jk++) {
      int place = rejectedCls[jk] < 0 ? jk : rejectedCls[jk];
      double clsTime = m_svdClusters[place]->getClsTime();
      if (clsTime >= beginPos && clsTime <= endPos) {
        m_svdClusters[place]->setTimeGroupId(ij);
        B2DEBUG(1, "SVDTimeGroupComposerModule    accepted cluster " << place
                << " clsTime " << clsTime);
      } else {
        B2DEBUG(1, "SVDTimeGroupComposerModule      rejected cluster " << place
                << " clsTime " << clsTime);
        if (ij == totGroups - 1) {
          m_svdClusters[place]->setTimeGroupId(-1);
          B2DEBUG(1, "SVDTimeGroupComposerModule        orphan cluster " << place
                  << " clsTime " << clsTime);
        } else {
          rejectedCls[rejectedCount++] = place;
        }
      }
    } // for(int jk=0;jk<int(rejectedCls.size());jk++) {
    rejectedCls.resize(rejectedCount);
  } // for(int ij=0;ij<int(groupInfo.size());ij++) {

}

