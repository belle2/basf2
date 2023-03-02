/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeGrouping/SVDTimeGroupingModule.h>
#include <svd/modules/svdTimeGrouping/SVDTimeGroupingHelperFunctions.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;


REG_MODULE(SVDTimeGrouping);


SVDTimeGroupingModule::SVDTimeGroupingModule() :
  Module()
{
  setDescription("Assigns the time-group Id to SVD clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName, "SVDCluster collection name", std::string(""));
  addParam("useClusterRawTime", m_useClusterRawTime, "Group on the basis of the raw time", bool(false));

  // 2. Fill time Histogram:
  addParam("tRangeLow", m_tRangeLow, "This sets the x- range of histogram [ns].",
           double(-160.));
  addParam("tRangeHigh", m_tRangeHigh, "This sets the x+ range of histogram [ns].",
           double(160.));
  addParam("rebinningFactor", m_rebinningFactor,
           "Time bin width is 1/rebinningFactor ns. Disables the module if set zero",
           int(2));
  addParam("fillSigmaN", m_fillSigmaN,
           "Number of Gaussian sigmas (= hardcoded resolutions) used to fill the time histogram for each cluster.",
           double(3.));

  // 3. Search peaks:
  addParam("minSigma", m_minSigma,
           "Lower limit of cluster time sigma for the fit for the peak-search [ns].",
           double(1.));
  addParam("maxSigma", m_maxSigma,
           "Upper limit of cluster time sigma for the fit for the peak-search [ns].",
           double(15.));
  addParam("fitRangeHalfWidth", m_fitRangeHalfWidth,
           "half width of the range in which the fit for the peak-search is performed [ns].",
           double(5.));
  addParam("removeSigmaN", m_removeSigmaN,
           "Evaluate and remove gauss upto N sigma.",
           double(5.));
  addParam("fracThreshold", m_fracThreshold,
           "Minimum fraction of candidates in a peak (wrt to the highest peak) considered for fitting in the peak-search.",
           double(0.05));
  addParam("maxGroups", m_maxGroups,
           "Maximum number of groups to be accepted.",
           int(20));

  // 4. Sort groups:
  addParam("expectedSignalTimeCenter", m_expectedSignalTimeCenter,
           "Expected time of the signal [ns].",
           double(0.));
  addParam("expectedSignalTimeMin", m_expectedSignalTimeMin,
           "Expected low range of signal hits [ns].",
           double(-50.));
  addParam("expectedSignalTimeMax", m_expectedSignalTimeMax,
           "Expected high range of signal hits [ns].",
           double(50.));
  addParam("signalLifetime", m_signalLifetime,
           "Group prominence is weighted with exponential weight with a lifetime defined by this parameter [ns].",
           double(30.));

  // 5. Signal group selection:
  addParam("numberOfSignalGroups", m_numberOfSignalGroups,
           "Number of groups expected to contain the signal clusters.",
           int(1));
  addParam("formSingleSignalGroup", m_formSingleSignalGroup,
           "Form a single super-group.",
           bool(false));
  addParam("acceptSigmaN", m_acceptSigmaN,
           "Accept clusters upto N sigma.",
           double(5.));
  addParam("writeGroupInfo", m_writeGroupInfo,
           "Write group info into SVDClusters.",
           bool(true));

  // 6. Hande out of range clusters:
  addParam("includeOutOfRangeClusters", m_includeOutOfRangeClusters,
           "Assign groups to under and overflow.",
           bool(true));

}



void SVDTimeGroupingModule::initialize()
{
  // prepare all store:
  m_svdClusters.isRequired(m_svdClustersName);

  if (m_numberOfSignalGroups != m_maxGroups) m_includeOutOfRangeClusters = false;

  if (m_rebinningFactor <= 0) B2WARNING("Module is ineffective.");
  if (m_tRangeHigh - m_tRangeLow < 10.) B2FATAL("tRange should not be less than 10 (hard-coded).");

  B2DEBUG(1, "SVDTimeGroupingModule \nsvdClusters: " << m_svdClusters.getName());
}



void SVDTimeGroupingModule::event()
{
  int totClusters = m_svdClusters.getEntries();
  if (m_rebinningFactor <= 0 || totClusters < 10) return;





  // minimise the range of the histogram removing empty bins at the edge
  // to speed up the execution time.

  double tmpRange[2] = {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
  for (int ij = 0; ij < totClusters; ij++) {
    double clsTime = m_svdClusters[ij]->getClsTime();
    if (std::isnan(tmpRange[0]) || clsTime > tmpRange[0]) tmpRange[0] = clsTime;
    if (std::isnan(tmpRange[1]) || clsTime < tmpRange[1]) tmpRange[1] = clsTime;
  }
  double tRangeHigh = m_tRangeHigh;
  double tRangeLow  = m_tRangeLow;
  if (tRangeHigh > tmpRange[0]) tRangeHigh = tmpRange[0];
  if (tRangeLow  < tmpRange[1]) tRangeLow  = tmpRange[1];

  int nBin = tRangeHigh - tRangeLow;
  if (nBin < 1) nBin = 1;
  nBin *= m_rebinningFactor;
  if (nBin < 2) nBin = 2;
  B2DEBUG(1, "tRange: [" << tRangeLow << "," << tRangeHigh << "], nBin: " << nBin);





  // declare and fill the histogram shaping each cluster with a normalised gaussian
  // G(cluster time, resolution)

  TH1D h_clsTime = TH1D("h_clsTime", "h_clsTime", nBin, tRangeLow, tRangeHigh);
  for (int ij = 0; ij < totClusters; ij++) {
    double clsSize = m_svdClusters[ij]->getSize();
    bool   isUcls  = m_svdClusters[ij]->isUCluster();
    double gSigma  = (clsSize >= int(m_clsSizeVsSigma[isUcls].size()) ?
                      m_clsSizeVsSigma[isUcls].back() : m_clsSizeVsSigma[isUcls][clsSize - 1]);
    double gCenter = m_svdClusters[ij]->getClsTime();

    // adding/filling a gauss to histogram
    addGausToHistogram(h_clsTime, 1., gCenter, gSigma, m_fillSigmaN);
  }




  // now we search for peaks and when we find one we remove it from the distribution.

  std::vector<GroupInfo> groupInfoVector; // Gauss paramerers (integral, center, sigma)

  FindGausPeaks searchPeaks(h_clsTime, groupInfoVector);
  // setting the parameters
  searchPeaks.setSignalTime(m_expectedSignalTimeCenter, m_expectedSignalTimeMin, m_expectedSignalTimeMax);
  searchPeaks.setSigmaLimits(m_minSigma, m_maxSigma);
  searchPeaks.setFitHalfWidth(m_fitRangeHalfWidth);
  searchPeaks.setSigmaN(m_removeSigmaN);
  searchPeaks.setFracThreshold(m_fracThreshold);
  searchPeaks.setMaxGroups(m_maxGroups);
  searchPeaks.setSignalLifetime(m_signalLifetime);
  // performing the search
  searchPeaks.doTheSearch();
  // resize to max
  searchPeaks.resizeToMaxSize();
  // sorting background groups
  searchPeaks.sortBackgroundGroups();
  // sorting signal groups
  searchPeaks.sortSignalGroups();

  // only select few signal groups
  if (m_numberOfSignalGroups < int(groupInfoVector.size())) groupInfoVector.resize(m_numberOfSignalGroups);



  // assign all clusters groupId = -1 if no groups are found
  if (int(groupInfoVector.size()) == 0)
    for (int jk = 0; jk < totClusters; jk++)
      m_svdClusters[jk]->setTimeGroupId().push_back(-1);



  // assign the groupID to clusters.

  // loop over all the groups
  // some groups may be dummy, ie, (0,0,0). they are skipped
  for (int ij = 0; ij < int(groupInfoVector.size()); ij++) {

    double pars[3] = {
      std::get<0>(groupInfoVector[ij]),
      std::get<1>(groupInfoVector[ij]),
      std::get<2>(groupInfoVector[ij])
    };

    if (pars[2] == 0 && ij != int(groupInfoVector.size()) - 1) continue;
    // do not continue the last loop.
    // we assign the group Id to leftover clusters at the last loop.

    // for this group, accept the clusters falling within 5(default) sigma of group center
    double lowestAcceptedTime  = pars[1] - m_acceptSigmaN * pars[2];
    double highestAcceptedTime = pars[1] + m_acceptSigmaN * pars[2];
    if (lowestAcceptedTime < tRangeLow)   lowestAcceptedTime  = tRangeLow;
    if (highestAcceptedTime > tRangeHigh) highestAcceptedTime = tRangeHigh;
    B2DEBUG(1, " group " << ij
            << " lowestAcceptedTime " << lowestAcceptedTime
            << " highestAcceptedTime " << highestAcceptedTime);

    // now loop over all the clusters to check which clusters fall in this range
    for (int jk = 0; jk < totClusters; jk++) {
      double clsTime = m_svdClusters[jk]->getClsTime();

      if (pars[2] != 0 &&   // if the last group is dummy, we straight go to leftover clusters
          clsTime >= lowestAcceptedTime && clsTime <= highestAcceptedTime) {

        if (m_formSingleSignalGroup) {
          if (int(m_svdClusters[jk]->getTimeGroupId().size()) == 0)
            m_svdClusters[jk]->setTimeGroupId().push_back(0);
        } else      // assigning groupId starting from 0
          m_svdClusters[jk]->setTimeGroupId().push_back(ij);

        // writing group info to clusters.
        // this is independent of group id, that means,
        // group info is correctly associated even if formSingleSignalGroup flag is on.
        if (m_writeGroupInfo)
          m_svdClusters[jk]->setTimeGroupInfo().push_back(GroupInfo(pars[0], pars[1], pars[2]));

        B2DEBUG(1, "   accepted cluster " << jk
                << " clsTime " << clsTime
                << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());

      } else {

        B2DEBUG(1, "     rejected cluster " << jk
                << " clsTime " << clsTime);

        if (ij == int(groupInfoVector.size()) - 1 && // we are now at the last loop
            int(m_svdClusters[jk]->getTimeGroupId().size()) == 0) { // leftover clusters

          if (m_includeOutOfRangeClusters && clsTime < tRangeLow)
            m_svdClusters[jk]->setTimeGroupId().push_back(m_maxGroups + 1);  // underflow
          else if (m_includeOutOfRangeClusters && clsTime > tRangeHigh)
            m_svdClusters[jk]->setTimeGroupId().push_back(m_maxGroups + 2);  // overflow
          else
            m_svdClusters[jk]->setTimeGroupId().push_back(-1);               // orphan


          B2DEBUG(1, "     leftover cluster " << jk
                  << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());


        }
      }
    } // end of loop over all clusters
  }   // end of loop over groups

}

