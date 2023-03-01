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

#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDTimeGrouping);

double mygaus(const double* x, const double* par)
{
  return par[0] * exp(-0.5 * std::pow((x[0] - par[1]) / par[2], 2)) / (sqrt(2.*TMath::Pi()) * par[2]);
}

SVDTimeGroupingModule::SVDTimeGroupingModule() :
  Module()
{
  setDescription("Assigns the time-group Id to SVD clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName, "SVDCluster collection name", string(""));
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
  addParam("formSigleSignalGroup", m_formSigleSignalGroup,
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

  // typedef to be used to store Gauss paramerers (integral, center, sigma)
  typedef std::tuple<double, double, double> GroupInfo;





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

  int xbin = tRangeHigh - tRangeLow;
  if (xbin < 1) xbin = 1;
  xbin *= m_rebinningFactor;
  if (xbin < 2) xbin = 2;
  B2DEBUG(1, "tRange: [" << tRangeLow << "," << tRangeHigh << "], xBin: " << xbin);





  // declare and fill the histogram shaping each cluster with a normalised gaussian
  // G(cluster time, resolution)

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
  }




  // now we search for peaks and when we find one we remove it from the distribution.

  std::vector<GroupInfo> groupInfoVector; // Gauss paramerers (integral, center, sigma)



  double maxPeak = 0.;    // height of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]
  double maxIntegral = 0.;  // integral of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]

  // bool doRoughCleaning = false; // handle to take care when fit does not conserves
  // int  roughCleaningCounter = 0;


  bool amDone = false;
  while (!amDone) {

    // take the bin corresponding to the highest peak
    int    maxBin        = h_clsTime.GetMaximumBin();
    double maxBinCenter  = h_clsTime.GetBinCenter(maxBin);
    double maxBinContent = h_clsTime.GetBinContent(maxBin);

    // Set maxPeak for the first time
    if (maxPeak == 0 &&
        maxBinCenter > m_expectedSignalTimeMin && maxBinCenter < m_expectedSignalTimeMax)
      maxPeak = maxBinContent;

    // we are done if the the height of the this peak is below threshold
    if (maxPeak != 0 && maxBinContent < maxPeak * m_fracThreshold) amDone = true;




    // preparing the gaus function for fitting the peak
    TF1 ngaus("ngaus", mygaus, tRangeLow, tRangeHigh, 3);

    // setting the parameters according to the maxBinCenter and maxBinContnet
    double maxPar0 = maxBinContent * std::sqrt(2.*TMath::Pi()) * m_fitRangeHalfWidth;
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
    int status = h_clsTime.Fit("ngaus", "NQ0", "",
                               maxBinCenter - m_fitRangeHalfWidth,
                               maxBinCenter + m_fitRangeHalfWidth);


    if (!status) {    // if fit converges

      double pars[3] = {
        ngaus.GetParameter(0),     // integral
        ngaus.GetParameter(1),     // center
        std::fabs(ngaus.GetParameter(2)) // sigma
      };

      if (pars[2] <= m_minSigma + 0.01) break;
      if (pars[2] >= m_maxSigma - 0.01) break;

      // Set maxIntegral for the first time
      if (maxPeak != 0 && maxIntegral == 0) maxIntegral = pars[0];

      // we are done if the the integral of the this peak is below threshold
      if (maxIntegral != 0 && pars[0] < maxIntegral * m_fracThreshold) amDone = true;



      // now subtract the fitted gaussian from the histogram

      // subtracting only 5(default) sigma from the peak to save computing time
      int startBin = h_clsTime.FindBin(pars[1] - m_removeSigmaN * pars[2]);
      int   endBin = h_clsTime.FindBin(pars[1] + m_removeSigmaN * pars[2]);
      if (startBin < 1) startBin = 1;
      if (endBin > xbin)  endBin = xbin;
      //
      for (int ijx = startBin; ijx <= endBin; ijx++) {
        float tbinc = h_clsTime.GetBinCenter(ijx);
        float tbincontent = h_clsTime.GetBinContent(ijx) - ngaus.Eval(tbinc);
        h_clsTime.SetBinContent(ijx, tbincontent);
      }

      // store group information (integral, position, width)
      groupInfoVector.push_back(std::make_tuple(pars[0], pars[1], pars[2]));
      B2DEBUG(1, " group " << int(groupInfoVector.size())
              << " pars[0] " << pars[0] << " pars[1] " << pars[1] << " pars[2] " << pars[2]);
      if (int(groupInfoVector.size()) >= m_maxGroups) amDone = true;
    } else break;
    // doRoughCleaning = true;

    // if (doRoughCleaning) {
    //   // to-do
    //   // subtract a gaussian of
    //   // height maxBinContent,
    //   // mean of maxBinCenter and
    //   // width of fitRangeHalfWidth
    //   doRoughCleaning = false;
    //   roughCleaningCounter++;
    // }
    // // probably the event contains too junk
    // if (roughCleaningCounter > m_maxGroups) amDone = true;

  }

  // resizing to max
  // this helps in sorting the groups
  groupInfoVector.resize(m_maxGroups, std::make_tuple(0., 0., 0.));



  // background group sorting

  // the probability of being signal is max at groupID = 0 and decreases with group number increasing.
  // the probability of being background is max at groupID = 19 and increases with group number decreasing.
  GroupInfo key;
  for (int ij = int(groupInfoVector.size()) - 2; ij >= 0; ij--) {
    key = groupInfoVector[ij];
    float keynorm = std::get<0>(key);
    float keymean = std::get<1>(key);
    bool isKeySignal = true;
    if (keynorm != 0. && (keymean < m_expectedSignalTimeMin || keymean > m_expectedSignalTimeMax)) isKeySignal = false;
    if (isKeySignal) continue;
    int kj = ij + 1;
    while (1) {
      if (kj >= int(groupInfoVector.size())) break;
      float grnorm = std::get<0>(groupInfoVector[kj]);
      float grmean = std::get<1>(groupInfoVector[kj]);
      bool isGrSignal = true;
      if (grnorm != 0. && (grmean < m_expectedSignalTimeMin || grmean > m_expectedSignalTimeMax)) isGrSignal = false;
      if (!isGrSignal && (grnorm > keynorm)) break;
      groupInfoVector[kj - 1] = groupInfoVector[kj];
      kj++;
    }
    groupInfoVector[kj - 1] = key;
  }

  // sorting signal groups based on expo-weight
  // this decreases chance of near-signal bkg groups getting picked
  if (m_signalLifetime > 0.)
    for (int ij = 1; ij < int(groupInfoVector.size()); ij++) {
      key = groupInfoVector[ij];
      float keynorm = std::get<0>(key);
      if (keynorm <= 0) break;
      float keymean = std::get<1>(key);
      bool isKeySignal = true;
      if (keynorm > 0 && (keymean < m_expectedSignalTimeMin || keymean > m_expectedSignalTimeMax)) isKeySignal = false;
      if (!isKeySignal) break;
      float keyWt = keynorm * TMath::Exp(-std::fabs(keymean - m_expectedSignalTimeCenter) / m_signalLifetime);
      int kj = ij - 1;
      while (1) {
        if (kj < 0) break;
        float grnorm = std::get<0>(groupInfoVector[kj]);
        float grmean = std::get<1>(groupInfoVector[kj]);
        float grWt = grnorm * TMath::Exp(-std::fabs(grmean - m_expectedSignalTimeCenter) / m_signalLifetime);
        if (grWt > keyWt) break;
        groupInfoVector[kj + 1] = groupInfoVector[kj];
        kj--;
      }
      groupInfoVector[kj + 1] = key;
    }

  if (m_numberOfSignalGroups < int(groupInfoVector.size())) groupInfoVector.resize(m_numberOfSignalGroups);





  // make all clusters groupless if no groups are found
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

    // for this group, accept the groups falling within 5(default) sigma of group center
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

        if (m_formSigleSignalGroup) {
          if (int(m_svdClusters[jk]->getTimeGroupId().size()) == 0)
            m_svdClusters[jk]->setTimeGroupId().push_back(0);
        } else      // assigning groupId starting from 0
          m_svdClusters[jk]->setTimeGroupId().push_back(ij);

        // writing group info to clusters.
        // this is independent of group id, that means,
        // group info is correctly associated even if formSigleSignalGroup flag is on.
        if (m_writeGroupInfo)
          m_svdClusters[jk]->setTimeGroupInfo().push_back(std::make_tuple(pars[0], pars[1], pars[2]));

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
    }
  }

}

