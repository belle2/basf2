/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <vxd/dataobjects/VxdID.h>
#include <utility>


namespace Belle2 {

  /** The VXDTFTBAnalyzerModule
   *
   * reads clusters and TrackCandidates and stores the information in a root file for histogramming
   *
   */
  class VXDTFTBAnalyzerModule : public Module {

  public:
    typedef unsigned short uShort; /**< shortcut for unsigned short */
    typedef std::pair<VxdID::baseType, uShort> SensorAndCluster; /**< .first is sensorID, .second is number of clusters */
    typedef std::pair<VxdID::baseType, std::vector<uShort> > SensorAndClusterCollection; /**< .first is sensorID, .second is number of clusterCombinations */

    /**
     * Constructor of the module.
     */
    VXDTFTBAnalyzerModule();

    /** Destructor of the module. */
    virtual ~VXDTFTBAnalyzerModule();

    /** Initializes the Module.
     */
    virtual void initialize();

    /**
     * Prints a header for each new run.
     */
    virtual void beginRun();

    /** Prints the full information about the event, run and experiment number. */
    virtual void event();

    /**
     * Prints a footer for each run which ended.
     */
    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  protected:
    /** resetting counters*/
    void resetCounters() {
      m_countReferenceTrackLengthTotal = 0;
      m_countReferenceTrackLengthSvd = 0;
      m_countReferenceTrackLengthPxd = 0;
      m_countReferenceTrackLengthTel = 0;
      m_countEvaluateTrackLengthTotal = 0;
      m_countEvaluateTrackLengthSvd = 0;
      m_countEvaluateTrackLengthPxd = 0;
      m_countEvaluateTrackLengthTel = 0;
      m_eventCounter = 0;
      m_pxdClusterCounter = 0;
      m_svdClusterCounter = 0;
    }

    int m_eventCounter; /**< knows current event number */
    int m_stepSize; /**< Informes the user that  event: (eventCounter-modulo(stepSize)) is currently executed */
    int m_pxdClusterCounter; /**< counts total number of pxd clusters occured */
    int m_svdClusterCounter; /**< counts total number of pxd clusters occured */
    std::string m_PARAMnameContainerTCevaluate; /**< set here the first name of the containers to be compared (this one is the test-container) */
    std::string m_PARAMnameContainerTCreference; /**< set here the second name of the containers to be compared (this one is used as reference for evaluating the first one) */
    int m_countBothTrue; /**< counts events where both (reference and evaluate) sources produced TCs */
    int m_countEvalTrueRefFalse; /**< counts events where the evaluate source produced TCs but the reference didn't */
    int m_countEvalFalseRefTrue; /**< counts events where the reference source produced TCs but the evaluate didn't */
    int m_countBothFalse; /**< counts events where none of the (reference and evaluate) sources produced TCs */
    int m_countBothTrueFull; /**< counts events where both (reference and evaluate) sources produced TCs with 8 clusters */
    int m_countEvalTrueRefFalseFull; /**< counts events where the evaluate source produced TCs with 8 clusters but the reference didn't */
    int m_countEvalFalseRefTrueFull; /**< counts events where the reference source produced TCs with 8 svd-clusters but the evaluate didn't */
    int m_countBothFalseFull; /**< counts events where none of the (reference and evaluate) sources produced TCs with 8 clusters */
    std::vector<int> m_referenceTrackLengthTotal; /**< collects the total number of cluster-indices of each reference TC */
    std::vector<int> m_referenceTrackLengthSvd; /**< collects the number of cluster-indices of the SVD of each reference TC */
    std::vector<int> m_referenceTrackLengthPxd; /**< collects the number of cluster-indices of the PXD of each reference TC */
    std::vector<int> m_referenceTrackLengthTel; /**< collects the number of cluster-indices of the TEL of each reference TC */
    std::vector<int> m_evaluateTrackLengthTotal; /**< collects the total number of cluster-indices of each evaluate TC */
    std::vector<int> m_evaluateTrackLengthSvd; /**< collects the total number of cluster-indices of the SVD of each evaluate TC */
    std::vector<int> m_evaluateTrackLengthPxd; /**< collects the total number of cluster-indices of the PXD of each evaluate TC */
    std::vector<int> m_evaluateTrackLengthTel; /**< collects the total number of cluster-indices of the TEL of each evaluate TC */
    int m_countReferenceTrackLengthTotal; /**< counts the total number of cluster-indices of each reference TC */
    int m_countReferenceTrackLengthSvd; /**< counts the number of cluster-indices of the SVD of each reference TC */
    int m_countReferenceTrackLengthPxd; /**< counts the number of cluster-indices of the PXD of each reference TC */
    int m_countReferenceTrackLengthTel; /**< counts the number of cluster-indices of the TEL of each reference TC */
    int m_countEvaluateTrackLengthTotal; /**< counts the total number of cluster-indices of each evaluate TC */
    int m_countEvaluateTrackLengthSvd; /**< counts the total number of cluster-indices of the SVD of each evaluate TC */
    int m_countEvaluateTrackLengthPxd; /**< counts the total number of cluster-indices of the PXD of each evaluate TC */
    int m_countEvaluateTrackLengthTel; /**< counts the total number of cluster-indices of the TEL of each evaluate TC */


    std::vector<SensorAndClusterCollection> m_overallCombinationsPerSensor; /**< collecting for each SVDsensor and event the number of clusterCombinations */
    std::vector<uShort> m_overallCombinationsCount; /**< collecting for each event the number of clusterCombinations */
  private:
  };
}
