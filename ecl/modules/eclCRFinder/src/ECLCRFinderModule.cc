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

// THIS MODULE
#include <ecl/modules/eclCRFinder/ECLCRFinderModule.h>

// FRAMEWORK
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ECL
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>

// MDST
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

// NAMESPACE(S)
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLCRFinder)
REG_MODULE(ECLCRFinderPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLCRFinderModule::ECLCRFinderModule() : Module(), m_eclCalDigits(eclCalDigitArrayName()),
  m_eclConnectedRegions(eclConnectedRegionArrayName()), m_eventLevelClusteringInfo(eventLevelClusteringInfoName())
{
  // Set description
  setDescription("ECLCRFinderModule");

  // Parallel processing certification.
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add module parameters.
  addParam("energyCut0", m_energyCut[0], "Seed energy cut.", 10.0 * Belle2::Unit::MeV);
  addParam("energyCut1", m_energyCut[1], "Growth energy cut.", 1.5 * Belle2::Unit::MeV);
  addParam("energyCut2", m_energyCut[2], "Digit energy cut.", 0.5 * Belle2::Unit::MeV);
  addParam("energyCutBkgd0", m_energyCutBkgd[0], "Seed energy cut (for high background).", 10.0 * Belle2::Unit::MeV);
  addParam("energyCutBkgd1", m_energyCutBkgd[1], "Growth energy cut (for high background).", 1.5 * Belle2::Unit::MeV);
  addParam("energyCutBkgd2", m_energyCutBkgd[2], "Digit energy cut (for high background).", 0.5 * Belle2::Unit::MeV);
  addParam("timeCut0", m_timeCut[0], "Seed time cut (negative values for residual cut).", 99999.);
  addParam("timeCut1", m_timeCut[1], "Growth time cut (negative values for residual cut).", 99999.);
  addParam("timeCut2", m_timeCut[2], "Digit time cut (negative values for residual cut).", 99999.);
  addParam("mapType0", m_mapType[0], "Map type for seed crystals.", std::string("N"));
  addParam("mapType1", m_mapType[1], "Map type for growth crystals.",  std::string("N"));
  addParam("mapPar0", m_mapPar[0],
           "Map parameter for seed crystals (radius (type=R), integer (for type=N) or fraction (for type=MC)).", 1.0);
  addParam("mapPar1", m_mapPar[1],
           "Map parameter for growth crystals (radius (type=R), integer (for type=N) or fraction (for type=MC)).", 1.0);
  addParam("useBackgroundLevel", m_useBackgroundLevel, "Use background dependent time and energy cuts.", 0);
  addParam("skipFailedTimeFitDigits", m_skipFailedTimeFitDigits, "Digits with failed fits are skipped when checking timing cuts.", 0);
  addParam("fullBkgdCount", m_fullBkgdCount, "Full background count (via eventLevelClusteringInfo).", 182);

}

ECLCRFinderModule::~ECLCRFinderModule()
{
  ;
}

void ECLCRFinderModule::initialize()
{
  B2DEBUG(200, "ECLCRFinderModule::initialize()");

  // Register dataobjects.
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  m_eclConnectedRegions.registerInDataStore(eclConnectedRegionArrayName());
  m_eventLevelClusteringInfo.registerInDataStore(eventLevelClusteringInfoName());

  // Register relations.
  m_eclConnectedRegions.registerRelationTo(m_eclCalDigits);

  // Check user inputs.
  if (m_energyCut[0] < m_energyCut[1]) B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[0] must be larger than m_energyCut[1].");
  if (m_energyCut[1] < m_energyCut[2]) B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[1] must be larger than m_energyCut[2].");
  if (m_energyCut[0] < m_energyCut[2]) B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[0] must be larger than m_energyCut[2].");
  if (m_useBackgroundLevel > 0
      and m_energyCutBkgd[0] < m_energyCutBkgd[1])
    B2FATAL("ECLCRFinderModule::initialize(): m_energyCutBkgd[0] must be larger than m_energyCutBkgd[1].");
  if (m_useBackgroundLevel > 0
      and m_energyCutBkgd[1] < m_energyCutBkgd[2])
    B2FATAL("ECLCRFinderModule::initialize(): m_energyCutBkgd[1] must be larger than m_energyCutBkgd[2].");
  if (m_useBackgroundLevel > 0
      and m_energyCutBkgd[0] < m_energyCutBkgd[2])
    B2FATAL("ECLCRFinderModule::initialize(): m_energyCutBkgd[0] must be larger than m_energyCutBkgd[2].");
  if (m_useBackgroundLevel > 0
      and m_energyCut[0] > m_energyCutBkgd[0])
    B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[0] must be smaller than m_energyCutBkgd[0].");
  if (m_useBackgroundLevel > 0
      and m_energyCut[1] > m_energyCutBkgd[1])
    B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[1] must be smaller than m_energyCutBkgd[1].");
  if (m_useBackgroundLevel > 0
      and m_energyCut[2] > m_energyCutBkgd[2])
    B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[2] must be smaller than m_energyCutBkgd[2].");

  // Initialize neighbour maps.
  m_neighbourMaps.resize(2);
  m_neighbourMaps[0] = new ECL::ECLNeighbours(m_mapType[0], m_mapPar[0]);
  m_neighbourMaps[1] = new ECL::ECLNeighbours(m_mapType[1], m_mapPar[1]);

  // Initialize the modified energy cuts (that could depend on event-by-event backgrounds later).
  for (int i = 0; i < 3; i++) {
    m_energyCutMod[i] = m_energyCut[i];
  }

  // Resize the vectors
  m_cellIdToSeedVec.resize(8737); /**< cellid -> seed digit (type 1). */
  m_cellIdToGrowthVec.resize(8737); /**< cellid -> growth digits. */
  m_cellIdToDigitVec.resize(8737); /**< cellid -> above threshold digits (type 2). */
  m_cellIdToTempCRIdVec.resize(8737); /**< cellid -> CR. */
  m_calDigitStoreArrPosition.resize(8737);

}

void ECLCRFinderModule::beginRun()
{
  ;
}

void ECLCRFinderModule::event()
{
  B2DEBUG(200, "ECLCRFinderModule::event()");

  // Reset the vector(s).
  memset(&m_cellIdToSeedVec[0], 0, m_cellIdToSeedVec.size() * sizeof m_cellIdToSeedVec[0]);
  memset(&m_cellIdToGrowthVec[0], 0, m_cellIdToGrowthVec.size() * sizeof m_cellIdToGrowthVec[0]);
  memset(&m_cellIdToDigitVec[0], 0, m_cellIdToDigitVec.size() * sizeof m_cellIdToDigitVec[0]);
  memset(&m_cellIdToTempCRIdVec[0], 0, m_cellIdToTempCRIdVec.size() * sizeof m_cellIdToTempCRIdVec[0]);

  // Fill a vector that can be used to map cellid -> store array position
  memset(&m_calDigitStoreArrPosition[0], -1, m_calDigitStoreArrPosition.size() * sizeof m_calDigitStoreArrPosition[0]);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_calDigitStoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  // Clear the map(s).
  m_cellIdToTempCRIdMap.clear();

  // Init variables.
  m_tempCRId = 1;

  //-------------------------------------------------------
  // Get background level for this events and adjust cuts.
  if (m_useBackgroundLevel > 0) {
    const int bkgdcount = m_eventLevelClusteringInfo->getNECLCalDigitsOutOfTime();

    // This scaling can probably be more clever to be really efficienct.
    // So far just scale linearly between 0 and 280 (release-07).
    double frac = 1.0;
    if (m_fullBkgdCount > 0) frac = static_cast<double>(bkgdcount) / static_cast<double>(m_fullBkgdCount);

    B2DEBUG(175, "ECLCRFinderModule::event(), Background count for this event: " << bkgdcount << " (expected for full bkgd: " <<
            m_fullBkgdCount << ", scaling factor is " << frac << ".");

    // Scale cut values.
    for (int i = 0; i < 3; i++) {
      m_energyCutMod[i] = m_energyCut[i] + (m_energyCutBkgd[i] - m_energyCut[i]) * frac;
      B2DEBUG(200, "ECLCRFinderModule::event(), Energy cut value m_energyCutMod[" << i << "] = " << m_energyCutMod[i] <<
              ", without bkgd scale m_energyCut[" << i << "] = " << m_energyCut[i]);
    }
  }

  //-------------------------------------------------------
  // fill digits into maps
  for (const auto& eclCalDigit : m_eclCalDigits) {
    const double energy         = eclCalDigit.getEnergy();
    const double time           = eclCalDigit.getTime();
    const double timeresolution = eclCalDigit.getTimeResolution();
    const int cellid            = eclCalDigit.getCellId();
    const bool fitfailed        = eclCalDigit.isFailedFit();

    double timeresidual = 999.;
    if (!fitfailed and fabs(timeresolution) > 1e-9) {
      timeresidual = time / timeresolution;
    }

    // Negative timecut is interpreted as cut on time residual, positive cut as cut on the timeresolution!
    // Fill seed crystals to a map.
    unsigned isSeed = 0;
    if (energy >= m_energyCutMod[0]) {
      if (fitfailed > 0 and m_skipFailedTimeFitDigits > 0) continue;
      if (!fitfailed) {
        if (m_timeCut[0] > 1e-9 and fabs(timeresolution) > m_timeCut[0]) continue;
        if (m_timeCut[0] < -1e-9  and fabs(timeresidual) > fabs(m_timeCut[0])) continue;
      }

      m_cellIdToSeedVec[cellid] = 1;
      isSeed = 1;
      B2DEBUG(250, "ECLCRFinderModule::event(), adding 'seed digit' with cellid = " << cellid);
    }

    // Fill growth crystals to a map.
    unsigned isGrowth = 0;
    if (energy >= m_energyCutMod[1]) {
      if (isSeed == 0) { // if a cell is a seed, it is also a growth cell (e.g. for different timing cuts)
        if (fitfailed > 0 and m_skipFailedTimeFitDigits > 0) continue;
        if (!fitfailed) {
          if (m_timeCut[1] > 1e-9 and fabs(timeresolution) > m_timeCut[1]) continue;
          if (m_timeCut[1] < -1e-9  and fabs(timeresidual) > fabs(m_timeCut[1])) continue;
        }
      }

      m_cellIdToGrowthVec[cellid] = 1;
      isGrowth = 1;
      B2DEBUG(250, "ECLCRFinderModule::event(), adding 'growth digit' with cellid = " << cellid);
    }

    // Fill all crystals above threshold to a map (this must include growth and seed crystals!).
    if (energy >= m_energyCutMod[2]) {
      if (isGrowth == 0) { // if a cell is a growth (incl. seed), it is also a growth cell (e.g. for different timing cuts)
        if (fitfailed > 0 and m_skipFailedTimeFitDigits > 0) continue;
        if (!fitfailed) {
          if (m_timeCut[2] > 1e-9 and fabs(timeresolution) > m_timeCut[2]) continue;
          if (m_timeCut[2] < -1e-9  and fabs(timeresidual) > fabs(m_timeCut[2])) continue;
        }
      }

      m_cellIdToDigitVec[cellid] = 1;
      B2DEBUG(250, "ECLCRFinderModule::event(), adding 'digit' with cellid = " << cellid);
    }

  } // done filling digit map

  //-------------------------------------------------------
  // 'find" in a map is faster if the number of seeds is not too large, can be replaced easily once we know what seed cuts we want.
  for (unsigned int pos = 1; pos < m_cellIdToSeedVec.size(); ++pos) {
    if (m_cellIdToSeedVec[pos] > 0) {
      checkNeighbours(pos, m_tempCRId, 0);
      ++m_tempCRId; // This is just a number, will be replaced by a consecutive number later in this module
    }
  }

  //-------------------------------------------------------
  // Make CR Ids consecutive.
  std::map< int, int > tempCRIdToCRIdMap;
  std::map< int, int>::iterator tempCRIdToCRIdMapIterator;

  int CRId = 1;
  for (unsigned int i = 1; i < m_cellIdToTempCRIdVec.size(); ++i) {

    if (m_cellIdToTempCRIdVec[i] > 0) { // digit belongs to a temporary CR
      tempCRIdToCRIdMapIterator = tempCRIdToCRIdMap.find(m_cellIdToTempCRIdVec[i]);

      if (tempCRIdToCRIdMapIterator == tempCRIdToCRIdMap.end()) { // not found
        tempCRIdToCRIdMap[m_cellIdToTempCRIdVec[i]] = CRId;
        ++CRId;
      }
    }
  }

  // Create CRs and add relations to digits.
  for (const auto& entry : tempCRIdToCRIdMap) {
    int connectedRegionID = entry.second;

    // create CR

    // Append to store array.
    const auto aCR = m_eclConnectedRegions.appendNew();

    // Fill variable(s).
    aCR->setCRId(connectedRegionID);

    // Add relations to all digits in this CR.
    for (unsigned int i = 1; i < m_cellIdToTempCRIdVec.size(); ++i) {
      if (tempCRIdToCRIdMap[m_cellIdToTempCRIdVec[i]] == connectedRegionID) {

        const int pos = m_calDigitStoreArrPosition[i];
        aCR->addRelationTo(m_eclCalDigits[pos], 1.0);
      }
    }
  } // end m_cellIdToTempCRIdVec loop
}


void ECLCRFinderModule::endRun()
{
  B2DEBUG(200, "ECLCRFinderModule::endRun()");
}


void ECLCRFinderModule::terminate()
{
  B2DEBUG(200, "ECLCRFinderModule::terminate()");
  for (unsigned int i = 0; i < m_neighbourMaps.size(); i++) {
    if (m_neighbourMaps[i]) delete m_neighbourMaps[i];
  }

}


void ECLCRFinderModule::checkNeighbours(const int cellid, const int tempcrid, const int type)
{
  B2DEBUG(200, "ECLCRFinderModule::checkNeighbours() type=" << type);

  for (const auto& neighbour : m_neighbourMaps[type]->getNeighbours(cellid)) {
    B2DEBUG(200, "ECLCRFinderModule::checkNeighbours(): neighbour=" << neighbour);

    // Check if this digit is above the lowest threshold (i.e. included in m_cellIdToDigitPointerVec) to be added.
    int isAdded = 0;
    if (m_cellIdToDigitVec[neighbour] > 0) {
      updateCRs(neighbour, tempcrid);
      isAdded = 1;
    }

    // Check if we have to grow further.
    if (m_cellIdToGrowthVec[neighbour] > 0) {

      if (isAdded < 1) {
        updateCRs(neighbour, tempcrid); // it could be that this digit is not in the all digit list (eg. tight timing cuts for "all digits".
        isAdded = 1;
      }

      if (m_cellIdToTempCRIdVec[neighbour] == 0) { //found
        checkNeighbours(neighbour, tempcrid, 1);
      }
    }
  }
}


void ECLCRFinderModule::updateCRs(int cellid, int tempcr)
{
  B2DEBUG(200, "ECLCRFinderModule::updateCRs()");

  // Check if this cellid already belongs to a connected region.
  int thiscr = m_cellIdToTempCRIdVec[cellid];

  if (thiscr != 0) {

    // This cellid is already in another connected region, update all cells of this other connected region
    for (unsigned int i = 1; i < m_cellIdToTempCRIdVec.size(); ++i) {
      if (m_cellIdToTempCRIdVec[i] == thiscr) {
        m_cellIdToTempCRIdVec[i] = tempcr;
      }
    }

  } else { //not in a CR yet, add it!
    m_cellIdToTempCRIdVec[cellid] = tempcr;
  }
}
