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

// C++
#include <set>

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
    double m_timeCut[3]; /**< Time cut for seed, neighbours, ...*/
    double m_timeCut_maxEnergy[3]; /**< Time cut is only applied below this energy, ...*/
    std::string m_mapType[2]; /**< Neighbour map types.*/
    double m_mapPar[2]; /**< Parameters for neighbour maps.*/
    int m_skipFailedTimeFitDigits; /**< Handling of digits with failed time fits.*/

    /** Other variables. */
    bool m_isOnlineProcessing{false}; /**< flag for identifying the online processing. */

    int m_tempCRId = -1; /**< Temporary CR ID*/

    /** Digit vectors. */
    std::vector <int>  m_cellIdToCheckVec; /**< cellid -> check digit. */
    std::vector <int>  m_cellIdToSeedVec; /**< cellid -> seed digit. */
    std::vector <int>  m_cellIdToGrowthVec; /**< cellid -> growth digits. */
    std::vector <int>  m_cellIdToDigitVec; /**< cellid -> above threshold digits. */

    /** vector (ECLElementNumbers::c_NCrystals + 1 entries) with cell id to store array positions */
    std::vector< int > m_calDigitStoreArrPosition;

    /** Connected Region map. */
    std::vector < int > m_cellIdToTempCRIdVec; /**< cellid -> temporary CR.*/
    std::map < int, int > m_cellIdToTempCRIdMap; /**< cellid -> temporary CR.*/

    /** Neighbour maps. */
    std::vector<ECL::ECLNeighbours*> m_neighbourMaps;

    /** Check if two crystals are neighbours. */
    // void checkNeighbours(const int cellid, const int tempcrid, const int type);
    bool areNeighbours(const int cellid1, const int cellid2, const int maptype);

    /** Convert vector of cell ids to 0/1 vectors from 1-8737. */
    std::vector<int> oneHotVector(std::vector<int> A, const int n);

    /** Convert vector of vectors to one long vector. */
    std::vector<int> flattenVector(std::vector<std::vector<int>> A);

    /** Find all lists of cell-ids that share at least one cell. */
    std::vector<std::set<int>> mergeVectorsUsingSets(std::vector<std::vector<int>> A);

    /** Get all connected regions. */
    std::vector<std::vector<int>> getConnectedRegions(const std::vector<int>& A, const std::vector<int>& B, const int maptype);

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
