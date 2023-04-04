/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include "TObject.h"
#include "TString.h"
#include <vector>

namespace Belle2 {

  /**
   * This class store the reconstruction configuration of SVDTimeGrouping module
   */
  class SVDTimeGroupingConfiguration: public TObject {
  public:

    /**
     * structure containing the relevant informations
     * of SVDTimeGrouping module
     */
    struct SVDTimeGroupingParameters {
      /** Expected range of time histogram [ns]. */
      Float_t tRange[2];
      /** Time bin width is 1/rebinningFactor [ns]. */
      Int_t   rebinningFactor;
      /** Number of Gaussian sigmas used to fill the time histogram for each cluster. */
      Float_t fillSigmaN;
      /** Limit of cluster time sigma for the fit for the peak-search [ns]. */
      Float_t limitSigma[2];
      /** Half width of the range in which the fit for the peak-search is performed [ns]. */
      Float_t fitRangeHalfWidth;
      /** Remove upto this sigma of fitted gaus from histogram. */
      Float_t removeSigmaN;
      /** Minimum fraction of candidates in a peak (wrt to the highest peak) considered for fitting in the peak-search. */
      Float_t fracThreshold;
      /** maximum number of groups to be accepted. */
      Int_t   maxGroups;
      /** Expected time-range and mean of the signal [ns]. (min, center, max) */
      Float_t expectedSignalTime[3];
      /** Group prominence is weighted with exponential weight with a lifetime defined by this parameter [ns]. */
      Float_t signalLifetime;
      /** Number of groups expected to contain the signal clusters. */
      Int_t   numberOfSignalGroups;
      /** Assign groupID = 0 to all clusters belonging to the signal groups. */
      Bool_t  formSingleSignalGroup;
      /** Clusters are tagged within this of fitted group. */
      Float_t acceptSigmaN;
      /** Write group info in SVDCluster, otherwise kept empty. */
      Bool_t  writeGroupInfo;
      /** Assign groups to under and overflow. */
      Bool_t  includeOutOfRangeClusters;
      /**
       * Cls-time resolution based on sensor side and type,
       * sides -> 0: V, 1: U,
       * types -> 0: L3, 1: Barrel, 2: Forward.
       * vector elements are sigmas wrt cls-size.
       */
      std::vector<Float_t> m_clsSizeVsSigma[2][3];
    };

    /**
     * Default constructor
     */
    SVDTimeGroupingConfiguration(const TString& uniqueID = "")
      : m_uniqueID(uniqueID)
    {};

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /** name of the payload */
    static std::string name;

  private:

    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;

    ClassDef(SVDTimeGroupingConfiguration, 1); /**< needed by root*/

  };

}
