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

using namespace Belle2;


REG_MODULE(SVDTimeGrouping);


SVDTimeGroupingModule::SVDTimeGroupingModule() :
  Module()
{
  setDescription("Assigns the time-group Id to SVD clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1a. Collections.
  addParam("SVDClusters", m_svdClustersName, "SVDCluster collection name", std::string(""));

  // 2b. Module Configuration
  addParam("useDB", m_useDB, "if False, use configuration module parameters", bool(true));
  addParam("useClusterRawCoG3Time", m_useClusterRawCoG3Time,
           "Group on the basis of the raw time (CoG3)", bool(false));
  addParam("isDisabled", m_isDisabled, "if true, module is disabled", bool(false));

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

  if (!m_isDisabled) B2WARNING("Module is active.");
  if (m_tRangeHigh - m_tRangeLow < 10.) B2FATAL("tRange should not be less than 10 (hard-coded).");

  B2DEBUG(20, "SVDTimeGroupingModule \nsvdClusters: " << m_svdClusters.getName());
}



void SVDTimeGroupingModule::event()
{
  if (m_isDisabled) return;
  if (int(m_svdClusters.getEntries()) < 10) return;


  // declare and fill the histogram shaping each cluster with a normalised gaussian
  // G(cluster time, resolution)
  TH1D h_clsTime;
  createAndFillHistorgram(h_clsTime);



  // now we search for peaks and when we find one we remove it from the distribution, one by one.

  std::vector<GroupInfo> groupInfoVector; // Gauss parameters (integral, center, sigma)

  // performing the search
  searchGausPeaksInHistogram(h_clsTime, groupInfoVector);
  // resize to max
  resizeToMaxSize(groupInfoVector);
  // sorting background groups
  sortBackgroundGroups(groupInfoVector);
  // sorting signal groups
  sortSignalGroups(groupInfoVector);

  // only select few signal groups
  if (m_numberOfSignalGroups < int(groupInfoVector.size())) groupInfoVector.resize(m_numberOfSignalGroups);

  // assign the groupID to clusters
  assignGroupIdsToClusters(h_clsTime, groupInfoVector);

} // end of event





void SVDTimeGroupingModule::createAndFillHistorgram(TH1D& hist)
{

  // minimise the range of the histogram removing empty bins at the edge
  // to speed up the execution time.

  int totClusters = m_svdClusters.getEntries();

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
  B2DEBUG(21, "tRange: [" << tRangeLow << "," << tRangeHigh << "], nBin: " << nBin);

  hist = TH1D("h_clsTime", "h_clsTime", nBin, tRangeLow, tRangeHigh);
  hist.GetXaxis()->SetLimits(tRangeLow, tRangeHigh);

  for (int ij = 0; ij < totClusters; ij++) {
    double clsSize = m_svdClusters[ij]->getSize();
    bool   isUcls  = m_svdClusters[ij]->isUCluster();
    double gSigma  = (clsSize >= int(m_clsSizeVsSigma[isUcls].size()) ?
                      m_clsSizeVsSigma[isUcls].back() : m_clsSizeVsSigma[isUcls][clsSize - 1]);
    double gCenter = m_svdClusters[ij]->getClsTime();

    // adding/filling a gauss to histogram
    addGausToHistogram(hist, 1., gCenter, gSigma, m_fillSigmaN);
  }

} // end of createAndFillHistorgram


void SVDTimeGroupingModule::searchGausPeaksInHistogram(TH1D& hist, std::vector<GroupInfo>& groupInfoVector)
{

  double maxPeak     = 0.;  //   height of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]
  double maxIntegral = 0.;  // integral of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]

  bool amDone = false;
  int  roughCleaningCounter = 0; // handle to take care when fit does not conserves
  while (!amDone) {

    // take the bin corresponding to the highest peak
    int    maxBin        = hist.GetMaximumBin();
    double maxBinCenter  = hist.GetBinCenter(maxBin);
    double maxBinContent = hist.GetBinContent(maxBin);

    // Set maxPeak for the first time
    if (maxPeak == 0 &&
        maxBinCenter > m_expectedSignalTimeMin && maxBinCenter < m_expectedSignalTimeMax)
      maxPeak = maxBinContent;
    // we are done if the the height of the this peak is below threshold
    if (maxPeak != 0 && maxBinContent < maxPeak * m_fracThreshold) { amDone = true; continue;}



    // preparing the gaus function for fitting the peak
    TF1 ngaus("ngaus", myGaus,
              hist.GetXaxis()->GetXmin(), hist.GetXaxis()->GetXmax(), 3);

    // setting the parameters according to the maxBinCenter and maxBinContnet
    double maxPar0 = maxBinContent * 2.50662827463100024 * m_fitRangeHalfWidth;
    ngaus.SetParameter(0, maxBinContent);
    ngaus.SetParLimits(0,
                       maxPar0 * 0.01,
                       maxPar0 * 2.);
    ngaus.SetParameter(1, maxBinCenter);
    ngaus.SetParLimits(1,
                       maxBinCenter - m_fitRangeHalfWidth * 0.2,
                       maxBinCenter + m_fitRangeHalfWidth * 0.2);
    ngaus.SetParameter(2, m_fitRangeHalfWidth);
    ngaus.SetParLimits(2,
                       m_minSigma,
                       m_maxSigma);


    // fitting the gauss at the peak the in range [-fitRangeHalfWidth, fitRangeHalfWidth]
    int status = hist.Fit("ngaus", "NQ0", "",
                          maxBinCenter - m_fitRangeHalfWidth,
                          maxBinCenter + m_fitRangeHalfWidth);


    if (!status) {    // if fit converges

      double pars[3] = {
        ngaus.GetParameter(0),     // integral
        ngaus.GetParameter(1),     // center
        std::fabs(ngaus.GetParameter(2)) // sigma
      };

      // fit converges but paramters are at limit
      // Do a rough cleaning
      if (pars[2] <= m_minSigma + 0.01 || pars[2] >= m_maxSigma - 0.01) {
        // subtract the faulty part from the histogram
        subtractGausFromHistogram(hist, maxPar0, maxBinCenter, m_fitRangeHalfWidth, m_removeSigmaN);
        if (roughCleaningCounter++ > m_maxGroups) amDone = true;
        continue;
      }

      // Set maxIntegral for the first time
      if (maxPeak != 0 && maxIntegral == 0) maxIntegral = pars[0];
      // we are done if the the integral of the this peak is below threshold
      if (maxIntegral != 0 && pars[0] < maxIntegral * m_fracThreshold) { amDone = true; continue;}


      // now subtract the fitted gaussian from the histogram
      subtractGausFromHistogram(hist, pars[0], pars[1], pars[2], m_removeSigmaN);

      // store group information (integral, position, width)
      groupInfoVector.push_back(GroupInfo(pars[0], pars[1], pars[2]));
      B2DEBUG(21, " group " << int(groupInfoVector.size())
              << " pars[0] " << pars[0] << " pars[1] " << pars[1] << " pars[2] " << pars[2]);

      if (int(groupInfoVector.size()) >= m_maxGroups) { amDone = true; continue;}

    } else {    // fit did not converges
      // subtract the faulty part from the histogram
      subtractGausFromHistogram(hist, maxPar0, maxBinCenter, m_fitRangeHalfWidth, m_removeSigmaN);
      if (roughCleaningCounter++ > m_maxGroups) amDone = true;
      continue;
    }
  }

} // end of searchGausPeaksInHistogram



void SVDTimeGroupingModule::sortBackgroundGroups(std::vector<GroupInfo>& groupInfoVector)
{
  GroupInfo keyGroup;
  for (int ij = int(groupInfoVector.size()) - 2; ij >= 0; ij--) {
    keyGroup = groupInfoVector[ij];
    double keyGroupIntegral = std::get<0>(keyGroup);
    double keyGroupCenter = std::get<1>(keyGroup);
    bool isKeyGroupSignal = true;
    if (keyGroupIntegral != 0. &&
        (keyGroupCenter < m_expectedSignalTimeMin || keyGroupCenter > m_expectedSignalTimeMax))
      isKeyGroupSignal = false;
    if (isKeyGroupSignal) continue; // skip if signal

    int kj = ij + 1;
    while (kj < int(groupInfoVector.size())) {
      double otherGroupIntegral = std::get<0>(groupInfoVector[kj]);
      double otherGroupCenter = std::get<1>(groupInfoVector[kj]);
      bool isOtherGroupSignal = true;
      if (otherGroupIntegral != 0. &&
          (otherGroupCenter < m_expectedSignalTimeMin || otherGroupCenter > m_expectedSignalTimeMax))
        isOtherGroupSignal = false;
      if (!isOtherGroupSignal && (otherGroupIntegral > keyGroupIntegral)) break;
      groupInfoVector[kj - 1] = groupInfoVector[kj];
      kj++;
    }
    groupInfoVector[kj - 1] = keyGroup;
  }
}


void SVDTimeGroupingModule::sortSignalGroups(std::vector<GroupInfo>& groupInfoVector)
{
  if (m_signalLifetime > 0.) {
    GroupInfo keyGroup;
    for (int ij = 1; ij < int(groupInfoVector.size()); ij++) {
      keyGroup = groupInfoVector[ij];
      double keyGroupIntegral = std::get<0>(keyGroup);
      if (keyGroupIntegral <= 0) break;
      double keyGroupCenter = std::get<1>(keyGroup);
      bool isKeyGroupSignal = true;
      if (keyGroupIntegral > 0 &&
          (keyGroupCenter < m_expectedSignalTimeMin || keyGroupCenter > m_expectedSignalTimeMax))
        isKeyGroupSignal = false;
      if (!isKeyGroupSignal) break; // skip the backgrounds

      double keyWt = keyGroupIntegral * TMath::Exp(-std::fabs(keyGroupCenter - m_expectedSignalTimeCenter) / m_signalLifetime);
      int kj = ij - 1;
      while (kj >= 0) {
        double otherGroupIntegral = std::get<0>(groupInfoVector[kj]);
        double otherGroupCenter = std::get<1>(groupInfoVector[kj]);
        double grWt = otherGroupIntegral * TMath::Exp(-std::fabs(otherGroupCenter - m_expectedSignalTimeCenter) / m_signalLifetime);
        if (grWt > keyWt) break;
        groupInfoVector[kj + 1] = groupInfoVector[kj];
        kj--;
      }
      groupInfoVector[kj + 1] = keyGroup;
    }
  }
}


void SVDTimeGroupingModule::assignGroupIdsToClusters(TH1D& hist, std::vector<GroupInfo>& groupInfoVector)
{
  int totClusters = m_svdClusters.getEntries();
  double tRangeLow  = hist.GetXaxis()->GetXmin();
  double tRangeHigh = hist.GetXaxis()->GetXmax();

  // assign all clusters groupId = -1 if no groups are found
  if (int(groupInfoVector.size()) == 0)
    for (int jk = 0; jk < totClusters; jk++)
      m_svdClusters[jk]->setTimeGroupId().push_back(-1);

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
    B2DEBUG(21, " group " << ij
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

        B2DEBUG(29, "   accepted cluster " << jk
                << " clsTime " << clsTime
                << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());

      } else {

        B2DEBUG(29, "     rejected cluster " << jk
                << " clsTime " << clsTime);

        if (ij == int(groupInfoVector.size()) - 1 && // we are now at the last loop
            int(m_svdClusters[jk]->getTimeGroupId().size()) == 0) { // leftover clusters

          if (m_includeOutOfRangeClusters && clsTime < tRangeLow)
            m_svdClusters[jk]->setTimeGroupId().push_back(m_maxGroups + 1);  // underflow
          else if (m_includeOutOfRangeClusters && clsTime > tRangeHigh)
            m_svdClusters[jk]->setTimeGroupId().push_back(m_maxGroups + 2);  // overflow
          else
            m_svdClusters[jk]->setTimeGroupId().push_back(-1);               // orphan


          B2DEBUG(29, "     leftover cluster " << jk
                  << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());

        }
      }
    } // end of loop over all clusters
  }   // end of loop over groups

}
