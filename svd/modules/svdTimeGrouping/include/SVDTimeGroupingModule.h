/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// framework
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

// svd
#include <svd/dbobjects/SVDRecoConfiguration.h>
#include <svd/dbobjects/SVDTimeGroupingConfiguration.h>
#include <svd/dataobjects/SVDCluster.h>

// std
#include <string>

// root
#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>


namespace Belle2 {


  /*! typedef to be used to store Gauss parameters (integral, center, sigma) */
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

    /** configure */
    void beginRun() override;

    /** EventWise jobs
     *
     * Grouping of SVD Clusters is performed here
     */
    virtual void event() override;


  protected:

    DBObjPtr<SVDRecoConfiguration> m_recoConfig; /**< SVD Reconstruction Configuration payload*/
    DBObjPtr<SVDTimeGroupingConfiguration> m_groupingConfig; /**< SVDTimeGrouping Configuration payload*/

    // Data members
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_svdEventInfoName; /**< Name of the collection to use for the SVDEventInfo */

    /**
     * the storeArray for svdClusters as member,
     * is faster than recreating link for each event
     */
    StoreArray<SVDCluster> m_svdClusters;

    bool   m_useDB              = true;  /**< if true takes the configuration from the DB objects. */
    bool   m_isDisabled         = false; /**< Disables the module if true. */
    bool   m_isDisabledIn6Samples = false; /**< Disables the module if true for 6-sample DAQ mode. */
    bool   m_isDisabledIn3Samples = false; /**< Disables the module if true for 3-sample DAQ mode. */

    /** Prepare module to work in raw-time if this parameter is set. */
    bool   m_useClusterRawTime  = false;
    /** Time algorithm to use if rawtime is computed for 6-sample DAQ mode. */
    std::string m_rawtimeRecoWith6SamplesAlgorithm  = "CoG3";
    /** Time algorithm to use if rawtime is computed for 3-sample DAQ mode. */
    std::string  m_rawtimeRecoWith3SamplesAlgorithm  = "CoG3";

    /**
     * module parameter values for 6-sample DAQ taken from SVDTimeGroupingConfiguration dbobject.
     */
    SVDTimeGroupingParameters m_usedParsIn6Samples;

    /**
     * module parameter values for 3-sample DAQ taken from SVDTimeGroupingConfiguration dbobject.
     */
    SVDTimeGroupingParameters m_usedParsIn3Samples;

    /**
     * module parameter values used.
     * if usedDB=true, then its values are taken from the SVDTimeGroupingConfiguration dbobject,
     */
    SVDTimeGroupingParameters m_usedPars;

    // helper functions

    /** Create Histogram and Fill cluster time in it
     *
     * 1. Optimize the range of the histogram and declare it
     * 2. fill the histogram shaping each cluster with a normalised gaussian G(cluster time, resolution)
     */
    void createAndFillHistorgram(TH1D& hist);

    /** Find Gaussian components in a Histogram
     *
     * 1. Highest peak is found
     * 2. It is fitted in a given range
     * 3. Fit parameters are stored
     * 4. Gaus peak is removed from histogram
     * 5. Process is repeated until a few criteria are met.
     */
    void searchGausPeaksInHistogram(TH1D& hist, std::vector<GroupInfo>& groupInfoVector);

    /*! increase the size of vector to max, this helps in sorting */
    void resizeToMaxSize(std::vector<GroupInfo>& groupInfoVector)
    {
      groupInfoVector.resize(m_usedPars.maxGroups, GroupInfo(0., 0., 0.));
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
