/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include "TObject.h"
#include "TString.h"

#include <vector>

namespace Belle2 {

  /**
   * structure containing the relevant informations
   * of SVDTimeGrouping module
   */
  struct SVDTimeGroupingParameters: public TObject {
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
     * types -> 0: L3, 1: Barrel, 2: Forward.
     * sides -> 0: V, 1: U,
     * vector elements are sigmas wrt cls-size.
     */
    std::vector<Float_t> clsSigma[3][2];

    ClassDef(SVDTimeGroupingParameters, 1); /**< needed by root*/
  };

  /**
   * This class store the reconstruction configuration of SVDTimeGrouping module
   */
  class SVDTimeGroupingConfiguration: public TObject {
  public:

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

    /**
     * Returns the reference to the parameters
     */
    SVDTimeGroupingParameters getTimeGroupingParameters(const TString& alg,
                                                        const Int_t& mode,
                                                        const Bool_t& isRawTime = false) const
    {
      if (!isRawTime) {
        if (mode == 6) {
          if (alg == "CoG3")
            return m_parsForCoG3In6Samples;
          else if (alg == "ELS3")
            return m_parsForELS3In6Samples;
          else if (alg == "CoG6")
            return m_parsForCoG6In6Samples;
        } else if (mode == 3) {
          if (alg == "CoG3")
            return m_parsForCoG3In3Samples;
          else if (alg == "ELS3")
            return m_parsForELS3In3Samples;
          else if (alg == "CoG6")
            return m_parsForCoG6In3Samples;
        }
      } else {
        if (mode == 6) {
          if (alg == "CoG3")
            return m_rawtimeParsForCoG3In6Samples;
          else if (alg == "ELS3")
            return m_rawtimeParsForELS3In6Samples;
          else if (alg == "CoG6")
            return m_rawtimeParsForCoG6In6Samples;
        } else if (mode == 3) {
          if (alg == "CoG3")
            return m_rawtimeParsForCoG3In3Samples;
          else if (alg == "ELS3")
            return m_rawtimeParsForELS3In3Samples;
          else if (alg == "CoG6")
            return m_rawtimeParsForCoG6In3Samples;
        }
      }
      B2FATAL("This state is forbidden");
    };



    /**
     * Sets all the parameters
     */
    SVDTimeGroupingParameters& setTimeGroupingParameters(const TString& alg, const Int_t& mode, const Bool_t& isRawTime = false)
    {
      if (!isRawTime) {
        if (mode == 6) {
          if (alg == "CoG3")
            return m_parsForCoG3In6Samples;
          else if (alg == "ELS3")
            return m_parsForELS3In6Samples;
          else if (alg == "CoG6")
            return m_parsForCoG6In6Samples;
        } else if (mode == 3) {
          if (alg == "CoG3")
            return m_parsForCoG3In3Samples;
          else if (alg == "ELS3")
            return m_parsForELS3In3Samples;
          else if (alg == "CoG6")
            return m_parsForCoG6In3Samples;
        }
      } else {
        if (mode == 6) {
          if (alg == "CoG3")
            return m_rawtimeParsForCoG3In6Samples;
          else if (alg == "ELS3")
            return m_rawtimeParsForELS3In6Samples;
          else if (alg == "CoG6")
            return m_rawtimeParsForCoG6In6Samples;
        } else if (mode == 3) {
          if (alg == "CoG3")
            return m_rawtimeParsForCoG3In3Samples;
          else if (alg == "ELS3")
            return m_rawtimeParsForELS3In3Samples;
          else if (alg == "CoG6")
            return m_rawtimeParsForCoG6In3Samples;
        }
      }
      B2FATAL("This state is forbidden");
    };

    /** name of the payload */
    static std::string name;

  private:

    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;

    /** parameters for CoG6 time-algorithm in 6-sample DAQ mode */
    SVDTimeGroupingParameters m_parsForCoG6In6Samples;
    /** parameters for CoG3 time-algorithm in 6-sample DAQ mode */
    SVDTimeGroupingParameters m_parsForCoG3In6Samples;
    /** parameters for ELS3 time-algorithm in 6-sample DAQ mode */
    SVDTimeGroupingParameters m_parsForELS3In6Samples;

    /** parameters for CoG6 time-algorithm in 3-sample DAQ mode */
    SVDTimeGroupingParameters m_parsForCoG6In3Samples;
    /** parameters for CoG3 time-algorithm in 3-sample DAQ mode */
    SVDTimeGroupingParameters m_parsForCoG3In3Samples;
    /** parameters for ELS3 time-algorithm in 3-sample DAQ mode */
    SVDTimeGroupingParameters m_parsForELS3In3Samples;


    /** rawtime (calibration) parameters for CoG6 time-algorithm in 6-sample DAQ mode */
    SVDTimeGroupingParameters m_rawtimeParsForCoG6In6Samples;
    /** rawtime (calibration) parameters for CoG3 time-algorithm in 6-sample DAQ mode */
    SVDTimeGroupingParameters m_rawtimeParsForCoG3In6Samples;
    /** rawtime (calibration) parameters for ELS3 time-algorithm in 6-sample DAQ mode */
    SVDTimeGroupingParameters m_rawtimeParsForELS3In6Samples;

    /** rawtime (calibration) parameters for CoG6 time-algorithm in 3-sample DAQ mode */
    SVDTimeGroupingParameters m_rawtimeParsForCoG6In3Samples;
    /** rawtime (calibration) parameters for CoG3 time-algorithm in 3-sample DAQ mode */
    SVDTimeGroupingParameters m_rawtimeParsForCoG3In3Samples;
    /** rawtime (calibration) parameters for ELS3 time-algorithm in 3-sample DAQ mode */
    SVDTimeGroupingParameters m_rawtimeParsForELS3In3Samples;

    ClassDef(SVDTimeGroupingConfiguration, 1); /**< needed by root*/

  };

}
