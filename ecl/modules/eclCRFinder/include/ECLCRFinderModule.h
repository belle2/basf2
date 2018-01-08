/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Belle II Connected Region Finder (CRF). Starting with 'seed' cells     *
 * above an energy energyCut0. Add neighbouring crystals above energyCut2.*
 * If neighbouring crystal is above energyCut1, repeat this. Timing cuts  *
 * can be set for each digit type and the energy cuts can be made         *
 * background dependent using the event-by-event background measurements. *
 * Digits with failed time fits automatically pass timing cuts by default.*
 * The CRF must run once before the splitters and splitters must only use *
 * digits contained in a CR. Digits from different CRs must not be mixed. *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCRFINDERMODULE_H_
#define ECLCRFINDERMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// ECL
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLEventInformation.h>

namespace Belle2 {

  /** Class to find connected regions */
  class ECLCRFinderModule : public Module {

  public:
    /** Constructor. */
    ECLCRFinderModule();

    /** Destructor. */
    virtual ~ECLCRFinderModule();

    /** Initialize. */
    virtual void initialize();

    /** Begin. */
    virtual void beginRun();

    /** Event. */
    virtual void event();

    /** End run. */
    virtual void endRun();

    /** Terminate (close ROOT files here if you have opened any). */
    virtual void terminate();

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

    /** Store object pointer: ECLEventInformation. */
    StoreObjPtr<ECLEventInformation> m_eclEventInformation;

    /** Name to be used for default or PureCsI option: ECLCalDigits.*/
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Name to be used for default option: ECLConnectedRegions.*/
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

    /** Name to be used for default option: ECLEventInformation.*/
    virtual const char* eclEventInformationName() const
    { return "ECLEventInformation" ; }

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
    double m_energyCutMod[3]; /**< modified energy cut taking into account bkgd per event for seed, neighbours, ...*/
    int m_tempCRId; /**< Temporary CR ID*/

    /** Digit vectors. */
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

    /** Name to be used for PureCsI option: ECLEventInformationPureCsI.*/
    virtual const char* eclEventInformationName() const override
    { return "ECLEventInformationPureCsI" ; }

  }; // end of ECLCovarianceMatrixPureCsIModule

} // end of Belle2 namespace

#endif // ECLCRFINDERMODULE_H_
