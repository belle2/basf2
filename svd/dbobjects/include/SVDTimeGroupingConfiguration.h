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
    {

      setTimeGroupingParameters("CoG3", 6).tRange[0] = -160;
      setTimeGroupingParameters("CoG3", 6).tRange[1] = 160;
      setTimeGroupingParameters("CoG3", 6).rebinningFactor = 2;
      setTimeGroupingParameters("CoG3", 6).fillSigmaN = 3;
      setTimeGroupingParameters("CoG3", 6).limitSigma[0] = 1;
      setTimeGroupingParameters("CoG3", 6).limitSigma[1] = 15;
      setTimeGroupingParameters("CoG3", 6).fitRangeHalfWidth = 5;
      setTimeGroupingParameters("CoG3", 6).removeSigmaN = 5;
      setTimeGroupingParameters("CoG3", 6).fracThreshold = 0.05;
      setTimeGroupingParameters("CoG3", 6).maxGroups = 20;
      setTimeGroupingParameters("CoG3", 6).expectedSignalTime[0] = -50;
      setTimeGroupingParameters("CoG3", 6).expectedSignalTime[1] = 0;
      setTimeGroupingParameters("CoG3", 6).expectedSignalTime[2] = 50;
      setTimeGroupingParameters("CoG3", 6).signalLifetime = 30;
      setTimeGroupingParameters("CoG3", 6).numberOfSignalGroups = 1;
      setTimeGroupingParameters("CoG3", 6).formSingleSignalGroup = false;
      setTimeGroupingParameters("CoG3", 6).acceptSigmaN = 5;
      setTimeGroupingParameters("CoG3", 6).writeGroupInfo = true;
      setTimeGroupingParameters("CoG3", 6).includeOutOfRangeClusters = true;
      setTimeGroupingParameters("CoG3", 6).clsSigma[0][0] = {3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805};
      setTimeGroupingParameters("CoG3", 6).clsSigma[0][1] = {6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485};
      setTimeGroupingParameters("CoG3", 6).clsSigma[1][0] = {3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805};
      setTimeGroupingParameters("CoG3", 6).clsSigma[1][1] = {6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485};
      setTimeGroupingParameters("CoG3", 6).clsSigma[2][0] = {3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805};
      setTimeGroupingParameters("CoG3", 6).clsSigma[2][1] = {6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485};


      // initialize all other algorithms same as CoG3-6sample.
      // values must be checked in the globaltag before use

      setTimeGroupingParameters("ELS3", 6) = getTimeGroupingParameters("CoG3", 6);
      setTimeGroupingParameters("CoG6", 6) = getTimeGroupingParameters("CoG3", 6);

      setTimeGroupingParameters("CoG3", 3) = getTimeGroupingParameters("CoG3", 6);
      setTimeGroupingParameters("ELS3", 3) = getTimeGroupingParameters("CoG3", 6);
      setTimeGroupingParameters("CoG6", 3) = getTimeGroupingParameters("CoG3", 6);
    };

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /**
     * Returns the reference to the parameters
     */
    SVDTimeGroupingParameters getTimeGroupingParameters(const TString& alg,
                                                        const Int_t& mode) const
    {
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
      B2FATAL("This state is forbidden");
    };



    /**
     * Sets all the parameters
     */
    SVDTimeGroupingParameters& setTimeGroupingParameters(const TString& alg, const Int_t& mode)
    {
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

    ClassDef(SVDTimeGroupingConfiguration, 1); /**< needed by root*/

  };

}
