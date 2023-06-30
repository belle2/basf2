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
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

// svd
#include <svd/dbobjects/SVDRecoConfiguration.h>
#include <svd/dbobjects/SVDSpacePointSNRFractionSelector.h>
#include <svd/dbobjects/SVDTimeGroupingConfiguration.h>
#include <svd/calibration/SVDHitTimeSelection.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

// tracking
#include <tracking/spacePointCreation/SpacePoint.h>

// std
#include <string>

// root
#include <TFile.h>


namespace Belle2 {
  /**
   * Imports Clusters of the SVD detector and converts them to spacePoints.
   *
   */
  class SVDSpacePointCreatorModule : public Module {

  public:



    /** Constructor */
    SVDSpacePointCreatorModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize() override;


    /** configure */
    void beginRun() override;


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;


    /** final output with mini-feedback */
    virtual void terminate() override;


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeCounters();


  protected:

    DBObjPtr<SVDRecoConfiguration> m_recoConfig; /**< SVD Reconstruction Configuration payload*/
    DBObjPtr<SVDTimeGroupingConfiguration> m_groupingConfig; /**< SVDTimeGrouping Configuration payload*/

    // Data members
    std::string m_svdClustersName = "SVDClusters"; /**< SVDCluster collection name */

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    std::string m_spacePointsName = ""; /**< SpacePoints collection name */

    StoreArray<SpacePoint>
    m_spacePoints; /**< the storeArray for spacePoints as member, is faster than recreating link for each event */

    std::string m_eventLevelTrackingInfoName = ""; /**< Name of the EventLevelTrackingInfo */

    std::string m_svdEventInfoName; /**< Name of the collection to use for the SVDEventInfo */

    float m_minClusterTime = -999; /**< clusters with time below this value are not considered to make spacePoints*/

    // modification parameters
    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

    bool m_onlySingleClusterSpacePoints =
      false; /**< standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */

    std::string m_inputPDF = ""; /**< File path of root file containing pdf histograms */

    TFile* m_calibrationFile = nullptr; /**< Pointer to root TFile containing PDF histograms */

    bool m_useLegacyNaming = true; /**< Choice between PDF naming conventions */

    bool m_useQualityEstimator = false; /**< Standard is true. Can be turned off in case accessing pdf root file is causing errors */
    //counters for testing
    unsigned int m_TESTERSVDClusterCtr = 0; /**< counts total number of SVDCluster occured */

    unsigned int m_TESTERSpacePointCtr = 0; /**< counts total number of SpacePoints occured */

    unsigned int m_numMaxSpacePoints = 7e4; /**< do not crete SPs if their number exceeds m_numMaxSpacePoints, tuned with BG19*/

    SVDHitTimeSelection m_HitTimeCut; /**< selection based on clustr time db object*/

    bool m_useSVDGroupInfoIn6Sample = false; /**< Use SVD group info to reject combinations in 6-sample DAQ mode */
    bool m_useSVDGroupInfoIn3Sample = false; /**< Use SVD group info to reject combinations in 3-sample DAQ mode */

    SVDNoiseCalibrations m_NoiseCal; /**< SVDNoise calibrations db object*/

    DBObjPtr<SVDSpacePointSNRFractionSelector>
    m_svdSpacePointSNRFractionSelector; /**< Payload of selection functions and thresholds applied on SVDSpacePoint */

    bool m_useDBForSNRFraction =
      true;  /**< if true takes the configuration from the DB objects for SVDSpacePointSNRFractionSelector. */
    bool m_useSVDSpacePointSNRFractionFor6Samples =
      false; /**< Use SVDSpacePointSNRFractionSelector to reject combinations in 6-sample DAQ mode */
    bool m_useSVDSpacePointSNRFractionFor3Samples =
      false; /**< Use SVDSpacePointSNRFractionSelector to reject combinations in 3-sample DAQ mode */

    /**
     * module parameter values for 6-sample DAQ taken from SVDTimeGroupingConfiguration dbobject.
     */
    SVDTimeGroupingParameters m_usedParsIn6Samples;

    /**
     * module parameter values for 3-sample DAQ taken from SVDTimeGroupingConfiguration dbobject.
     */
    SVDTimeGroupingParameters m_usedParsIn3Samples;

    bool m_forceGroupingFromDB = true; /**< use the configuration from SVDRecConfiguration DB. */
    bool m_useParamFromDB = true; /**< use the configuration from SVDTimeGroupingConfiguration DB. */
  };
} // end namespace Belle2
