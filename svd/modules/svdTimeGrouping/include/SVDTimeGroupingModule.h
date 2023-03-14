/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>

#include <string>

#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>

namespace Belle2 {


  /*! typedef to be used to store Gauss paramerers (integral, center, sigma) */
  typedef std::tuple<double, double, double> GroupInfo;



  /**
   * Imports Clusters of the SVD detector and converts them to spacePoints.
   *
   */
  class SVDTimeGroupingModule : public Module {

  public:



    /** Constructor */
    SVDTimeGroupingModule();


    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize() override;


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;


  protected:


    // Data members
    std::string m_svdClustersName = "SVDClusters"; /**< SVDCluster collection name */

    /**
     * the storeArray for svdClusters as member,
     * is faster than recreating link for each event
     *
     */
    StoreArray<SVDCluster> m_svdClusters;

    bool   m_useClusterRawTime  = false; /**< Prepare module to work in raw-time if this
      parameter is set. */

    double m_tRangeLow          = -160.; /**< Expected low range of time histogram [ns]. */
    double m_tRangeHigh         =  160.; /**< Expected high range of time histogram [ns]. */
    int    m_rebinningFactor    =   2; /**< Time bin width is 1/m_rebinningFactor ns. Also disables
            the module if zero. */
    double m_fillSigmaN         = 3.; /**< Number of Gaussian sigmas (= hardcoded resolutions) used
           to fill the time histogram for each cluster. */
    double m_minSigma           = 1.; /**< Lower limit of cluster time sigma for the fit for the
           peak-search [ns]. */
    double m_maxSigma           = 15.; /**< Upper limit of cluster time sigma for the fit for the
            peak-search [ns]. */
    double m_fitRangeHalfWidth  = 5.; /**< Half width of the range in which the fit for the
           peak-search is performed [ns]. */
    double m_removeSigmaN       = 5.; /**< Remove upto this sigma of fitted gaus from histogram. */
    double m_fracThreshold      = 0.05; /**< Minimum fraction of candidates in a peak (wrt to the
             highest peak) considered for fitting in the peak-search. */
    int    m_maxGroups          = 20; /**< maximum number of groups to be accepted. */
    double m_expectedSignalTimeCenter =   0; /**< Expected time of the signal [ns]. */
    double m_expectedSignalTimeMin    = -50; /**< Expected low range of signal hits [ns]. */
    double m_expectedSignalTimeMax    =  50; /**< Expected high range of signal hits [ns]. */
    double m_signalLifetime     = 30.; /**< Group prominence is weighted with exponential weight
            with a lifetime defined by this parameter [ns]. */
    int    m_numberOfSignalGroups = 1; /**< Number of groups expected to contain the signal
            clusters. */
    bool   m_formSingleSignalGroup = false; /**< Assign groupID = 0 to all clusters belonging to
                the signal groups. */
    double m_acceptSigmaN       = 5.;    /**< Clusters are tagged within this of fitted group. */
    bool   m_writeGroupInfo     = true;  /**< Write group info in SVDCluster, otherwise kept empty. */
    bool   m_includeOutOfRangeClusters = true; /**< Assign groups to under and overflow. */

    /*! cls-time resolution based on clsSize -> 0: V, 1: U */
    std::vector<double> m_clsSizeVsSigma[2] = {
      {3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805},
      {6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485}
    };



    // helper functions

    /** Create Histogram and Fill cluster time in it
     *
     * 1. Optimize the range of the histogram and declare it
     * 2. fill the histogram shaping each cluster with a normalised gaussian G(cluster time, resolution)
     */
    void createAndFillHistorgram(TH1D& hist);

    /** Find Gaussian compoments in a Histogram
     *
     * 1. Highest peak is found
     * 2. It is fitted in a given range
     * 3. Fit paramter is stored
     * 4. Gaus peak is removed from histogram
     * 5. Process is repeated until a few criterias are met.
     */
    void searchGausPeaksInHistogram(TH1D& hist, std::vector<GroupInfo>& groupInfoVector);

    /*! increase the size of vector to max, this helps in sorting */
    void resizeToMaxSize(std::vector<GroupInfo>& groupInfoVector)
    {
      groupInfoVector.resize(m_maxGroups, GroupInfo(0., 0., 0.));
    }

    /** Sort Background Groups
     *
     * The probability of being signal is max at groupID = 0 and decreases with group number increasing.
     * The probability of being background is max at groupID = 19 and increases with group number decreasing.
     */
    void sortBackgroundGroups(std::vector<GroupInfo>& groupInfoVector);

    /** Sort Signals
     *
     * Sorting signal groups based on exponential weight.
     * This decreases chance of near-signal bkg groups getting picked.
     * The probability of being signal is max at groupID = 0 and decreases with group number increasing.
     */
    void sortSignalGroups(std::vector<GroupInfo>& groupInfoVector);

    /** Assign groupId to the clusters
     *
     * 1. Looped over all the groups
     * 2. Acceptance time-range is computed using the center and m_acceptSigmaN
     * 3. Looped over all the clusters
     * 4. Clusters in the range is assigned the respective groupId
     */
    void assignGroupIdsToClusters(TH1D& hist, std::vector<GroupInfo>& groupInfoVector);

  };



  /*! Gaus function to be used in the fit. */
  inline double myGaus(const double* x, const double* par)
  {
    return par[0] * TMath::Gaus(x[0], par[1], par[2], true);
  }


  /** Add (or Subtract) a Gaussian to (or from) a histogram
   *
   * The gauss is calculated upto the sigmaN passed to the function.
   */
  inline void addGausToHistogram(TH1D& hist,
                                 const double& integral, const double& center, const double& sigma,
                                 const double& sigmaN, const bool& isAddition = true)
  {
    int startBin = hist.FindBin(center - sigmaN * sigma);
    int   endBin = hist.FindBin(center + sigmaN * sigma);
    if (startBin < 1) startBin = 1;
    if (endBin > (hist.GetNbinsX())) endBin = hist.GetNbinsX();

    for (int ijx = startBin; ijx <= endBin; ijx++) {
      double tbinc       = hist.GetBinCenter(ijx);
      double tbincontent = hist.GetBinContent(ijx);

      if (isAddition) tbincontent += integral * TMath::Gaus(tbinc, center, sigma, true);
      else tbincontent -= integral * TMath::Gaus(tbinc, center, sigma, true);

      hist.SetBinContent(ijx, tbincontent);
    }
  }


  /**  Subtract a Gaussian from a histogram
   *
   * The gauss is calculated upto the sigmaN passed to the function.
   */
  inline void subtractGausFromHistogram(TH1D& hist,
                                        const double& integral, const double& center, const double& sigma,
                                        const double& sigmaN)
  {
    addGausToHistogram(hist, integral, center, sigma, sigmaN, false);
  }




} // end namespace Belle2
