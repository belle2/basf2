/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  class ECLConnectedRegion;
  class ECLCalDigit;
  class EventLevelClusteringInfo;

  namespace ECL {
    class ECLNeighbours;
  }

  /** Class to find connected regions */
  class ECLCRFinderModule : public Module {

  public:
    /** Constructor. */
    ECLCRFinderModule();

    /** Destructor. */
    virtual ~ECLCRFinderModule();

    /** Initialize. */
    virtual void initialize() override;

    /** Begin. */
    virtual void beginRun() override;

    /** Event. */
    virtual void event() override;

    /** End run. */
    virtual void endRun() override;

    /** Terminate (close ROOT files here if you have opened any). */
    virtual void terminate() override;

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

    /** Store object pointer: EventLevelClusteringInfo. */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo;

    /** Name to be used for default or PureCsI option: ECLCalDigits.*/
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Name to be used for default option: ECLConnectedRegions.*/
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

    /** Name to be used for default option: EventLevelClusteringInfo.*/
    virtual const char* eventLevelClusteringInfoName() const
    { return "EventLevelClusteringInfo" ; }

  private:

    // Module parameters
    double m_energyCut[3]; /**< Energy cut for seed, neighbours, ...*/
    double m_energyCutBkgd[3]; /**< Energy cut (for high background) for seed, neighbours, ...*/
    double m_timeCut[3]; /**< Time cut for seed, neighbours, ...*/
    std::string m_mapType[2]; /**< Neighbour map types.*/
    double m_mapPar[2]; /**< Parameters for neighbour maps.*/
    int m_useBackgroundLevel; /**< Background dependend energy and timing cuts.*/
    int m_skipFailedTimeFitDigits; /**< Handling of digits with failed time fits.*/
    int m_fullBkgdCount; /**< Number of expected background digits at full background. TODO move to DB*/

    /** Other variables. */
    double m_energyCutMod[3] {}; /**< modified energy cut taking into account bkgd per event for seed, neighbours, ...*/
    int m_tempCRId = -1; /**< Temporary CR ID*/

    /** Digit vectors. */
    std::vector <int>  m_cellIdToCheckVec; /**< cellid -> check digit. */
    std::vector <int>  m_cellIdToSeedVec; /**< cellid -> seed digit. */
    std::vector <int>  m_cellIdToGrowthVec; /**< cellid -> growth digits. */
    std::vector <int>  m_cellIdToDigitVec; /**< cellid -> above threshold digits. */

    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector< int > m_calDigitStoreArrPosition;

    // USE POSITION IN STORE ARRAY!!!

    /** Connected Region map. */
    std::vector < int > m_cellIdToTempCRIdVec; /**< cellid -> temporary CR.*/
    std::map < int, int > m_cellIdToTempCRIdMap; /**< cellid -> temporary CR.*/

    /** Neighbour maps. */
    std::vector<ECL::ECLNeighbours*> m_neighbourMaps;

    /** Neighbour finder. */
    void checkNeighbours(const int cellid, const int tempcrid, const int type);

    /** Update CRs. */
    void updateCRs(int cellid, int tempcr);

  };

  /** Class to find connected regions, pureCsI version*/
  class ECLCRFinderPureCsIModule : public ECLCRFinderModule {
  public:
    /** Name to be used for PureCsI option: ECLCalDigitsPureCsI.*/
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** Name to be used for PureCsI option: ECLConnectedRegionsPureCsI.*/
    virtual const char* eclConnectedRegionArrayName() const override
    { return "ECLConnectedRegionsPureCsI" ; }

    /** Name to be used for PureCsI option: EventLevelClusteringInfoPureCsI.*/
    virtual const char* eventLevelClusteringInfoName() const override
    { return "EventLevelClusteringInfoPureCsI" ; }

  }; // end of ECLCovarianceMatrixPureCsIModule

} // end of Belle2 namespace
