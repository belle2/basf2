/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclCRFinder/ECLCRFinderModule.h>

// FRAMEWORK
#include <framework/core/Environment.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/Logger.h>

//ECL
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>

// MDST
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

// C++
#include <algorithm>
#include <iostream>

// NAMESPACE(S)
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLCRFinder);
REG_MODULE(ECLCRFinderPureCsI);

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
  addParam("energyCut1", m_energyCut[1], "Growth energy cut.", 10.0 * Belle2::Unit::MeV);
  addParam("energyCut2", m_energyCut[2], "Digit energy cut.", 0.5 * Belle2::Unit::MeV);
  addParam("timeCut0", m_timeCut[0], "Seed time cut (negative values for residual cut).", 99999.);
  addParam("timeCut1", m_timeCut[1], "Growth time cut (negative values for residual cut).", 99999.);
  addParam("timeCut2", m_timeCut[2], "Digit time cut (negative values for residual cut).", 99999.);
  addParam("timeCut0maxEnergy", m_timeCut_maxEnergy[0], "Time cut is only applied below this energy for seed crystals.",
           0.0 * Belle2::Unit::MeV);
  addParam("timeCut1maxEnergy", m_timeCut_maxEnergy[1], "Time cut is only applied below this energy for growth crystals.",
           0.0 * Belle2::Unit::MeV);
  addParam("timeCut2maxEnergy", m_timeCut_maxEnergy[2], "Time cut is only applied below this energy for digits.",
           0.0 * Belle2::Unit::MeV);
  addParam("mapType0", m_mapType[0], "Map type for seed crystals.", std::string("N"));
  addParam("mapType1", m_mapType[1], "Map type for growth crystals.",  std::string("N"));
  addParam("mapPar0", m_mapPar[0],
           "Map parameter for seed crystals (radius (type=R), integer (for type=N) or fraction (for type=MC)).", 1.0);
  addParam("mapPar1", m_mapPar[1],
           "Map parameter for growth crystals (radius (type=R), integer (for type=N) or fraction (for type=MC)).", 1.0);
  addParam("skipFailedTimeFitDigits", m_skipFailedTimeFitDigits, "Digits with failed fits are skipped when checking timing cuts.", 0);

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
  m_eventLevelClusteringInfo.isRequired(eventLevelClusteringInfoName());

  // Register relations.
  m_eclConnectedRegions.registerRelationTo(m_eclCalDigits);

  // Check user inputs: [2]: digit, [1]: growth, [0]: seed
  // overall energy thresholds
  if (std::isless(m_energyCut[0], m_energyCut[1])) B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[0]=" << m_energyCut[0] <<
                                                             " must be larger or equal than m_energyCut[1]=" << m_energyCut[1]);
  if (std::isless(m_energyCut[1], m_energyCut[2])) B2FATAL("ECLCRFinderModule::initialize(): m_energyCut[1]=" << m_energyCut[1] <<
                                                             " must be larger or equal than m_energyCut[2]=" << m_energyCut[2]);

  // timing threshold (can depend on energy, but we make the check here even stronger by checking that the timing is looser without checking the timing energy range)
  if (std::isgreater(m_timeCut[0], m_timeCut[1]))
    B2FATAL("ECLCRFinderModule::initialize(): m_timeCut[0] must be less or equal than m_timeCut[1].");
  if (std::isgreater(m_timeCut[1], m_timeCut[2]))
    B2FATAL("ECLCRFinderModule::initialize(): m_timeCut[1] must be less or equal than m_timeCut[2].");

  // Initialize neighbour maps.
  m_neighbourMaps.resize(2);
  m_neighbourMaps[0] = new ECL::ECLNeighbours(m_mapType[0], m_mapPar[0]);
  m_neighbourMaps[1] = new ECL::ECLNeighbours(m_mapType[1], m_mapPar[1]);

  // Resize the vectors
  m_cellIdToCheckVec.resize(8737); /**< cellid -> check digit: true if digit has been checked for neighbours. */
  m_cellIdToSeedVec.resize(8737); /**< cellid -> seed digit (type 1). */
  m_cellIdToGrowthVec.resize(8737); /**< cellid -> growth digits (type 2). */
  m_cellIdToDigitVec.resize(8737); /**< cellid -> above threshold digits. */
  m_cellIdToTempCRIdVec.resize(8737); /**< cellid -> CR. */
  m_calDigitStoreArrPosition.resize(8737);

}

void ECLCRFinderModule::beginRun()
{

}

void ECLCRFinderModule::event()
{
  B2DEBUG(200, "ECLCRFinderModule::event()");

  // Reset the vector(s).
  std::fill(m_cellIdToCheckVec.begin(), m_cellIdToCheckVec.end(), 0);
  std::fill(m_cellIdToSeedVec.begin(), m_cellIdToSeedVec.end(), 0);
  std::fill(m_cellIdToGrowthVec.begin(), m_cellIdToGrowthVec.end(), 0);
  std::fill(m_cellIdToDigitVec.begin(), m_cellIdToDigitVec.end(), 0);
  std::fill(m_cellIdToTempCRIdVec.begin(), m_cellIdToTempCRIdVec.end(), 0);

  // Fill a vector that can be used to map cellid -> store array position
  std::fill(m_calDigitStoreArrPosition.begin(), m_calDigitStoreArrPosition.end(), -1);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_calDigitStoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  // Clear the map(s).
  m_cellIdToTempCRIdMap.clear();

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
    // Start filling all crystals to a map. Growth and seed crystals are strict subsets.
    if (std::isgreaterequal(energy, m_energyCut[2])) {
      if (fitfailed > 0 and m_skipFailedTimeFitDigits > 0) continue;
      if (!fitfailed
          and energy < m_timeCut_maxEnergy[2]) { //check timing cuts only if we have a good fit and if the energy is below threshold
        if (m_timeCut[2] > 1e-9 and fabs(timeresolution) > m_timeCut[2]) continue;
        if (m_timeCut[2] < -1e-9  and fabs(timeresidual) > fabs(m_timeCut[2])) continue;
      }
      m_cellIdToDigitVec[cellid] = 1;
      B2DEBUG(250, "ECLCRFinderModule::event(), adding 'all digit' with cellid = " << cellid);

      // check growth only if they already passed the digit check
      if (std::isgreaterequal(energy, m_energyCut[1])) {
        if (!fitfailed
            and energy < m_timeCut_maxEnergy[1]) { //check timing cuts only if we have a good fit and if the energy is below threshold
          if (m_timeCut[1] > 1e-9 and fabs(timeresolution) > m_timeCut[1]) continue;
          if (m_timeCut[1] < -1e-9  and fabs(timeresidual) > fabs(m_timeCut[1])) continue;
        }
        m_cellIdToGrowthVec[cellid] = 1;
        B2DEBUG(250, "ECLCRFinderModule::event(), adding 'growth digit' with cellid = " << cellid);

        // check seed only if they already passed the growth check
        if (std::isgreaterequal(energy, m_energyCut[0])) {
          if (!fitfailed
              and energy < m_timeCut_maxEnergy[0]) { //check timing cuts only if we have a good fit and if the energy is below threshold
            if (m_timeCut[0] > 1e-9 and fabs(timeresolution) > m_timeCut[0]) continue;
            if (m_timeCut[0] < -1e-9  and fabs(timeresidual) > fabs(m_timeCut[0])) continue;
          }
        }
        m_cellIdToSeedVec[cellid] = 1;
        B2DEBUG(250, "ECLCRFinderModule::event(), adding 'seed digit' with cellid = " << cellid);
      } //end growth
    }// end digit
  }//end filling maps

  // we start with seed crystals A and attach all growth crystals B
  std::vector<std::vector<int>> connectedRegions_AB = getConnectedRegions(m_cellIdToSeedVec, m_cellIdToGrowthVec, 0);
  std::vector<int> connectedRegions_AB_flattened = flattenVector(connectedRegions_AB);
  std::vector<int> AB = oneHotVector(connectedRegions_AB_flattened, m_cellIdToSeedVec.size());

  // Check if any of the growth crystals could grow to other growth crystals
  std::vector<std::vector<int>> connectedRegions_ABB = getConnectedRegions(AB, m_cellIdToGrowthVec, 0);
  std::vector<int> connectedRegions_ABB_flattened = flattenVector(connectedRegions_ABB);
  std::vector<int> ABB = oneHotVector(connectedRegions_ABB_flattened, AB.size());

  // and finally: attach all normal digits
  std::vector<std::vector<int>> connectedRegions_ABBC = getConnectedRegions(ABB, m_cellIdToDigitVec, 0);

  //final step: merge all CRs that share at least one crystal
  std::vector<std::set<int>> connectedRegionsMerged_ABBC_sets = mergeVectorsUsingSets(connectedRegions_ABBC);

  // Create CRs and add relations to digits.
  unsigned int connectedRegionID = 0;
  for (const auto& xcr : connectedRegionsMerged_ABBC_sets) {

    // Append to store array
    const auto aCR = m_eclConnectedRegions.appendNew();

    // Set CR ID
    aCR->setCRId(connectedRegionID);
    connectedRegionID++;

    // Add all digits
    for (int x : xcr) {
      const int pos = m_calDigitStoreArrPosition[x];
      aCR->addRelationTo(m_eclCalDigits[pos], 1.0);
    }
  }

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

bool ECLCRFinderModule::areNeighbours(const int cellid1, const int cellid2, const int maptype)
{
  for (const auto& neighbour : m_neighbourMaps[maptype]->getNeighbours(cellid1)) {
    if (neighbour == cellid2) return true;
  }
  return false;
}

std::vector<int> ECLCRFinderModule::flattenVector(std::vector<std::vector<int>>& A)
{
  std::vector<int> C;
  for (const auto& B : A) {
    C.insert(C.end(), B.begin(), B.end());
  }
  std::sort(C.begin(), C.end());
  C.erase(std::unique(C.begin(), C.end()), C.end());
  return C;
}

std::vector<int> ECLCRFinderModule::oneHotVector(std::vector<int>& A, const int n)
{
  std::vector<int> C(n, 0);
  for (int x : A) {
    if (x >= 0 && x < n) {
      C[x] = 1;
    }
  }
  return C;
}

std::vector<std::set<int>> ECLCRFinderModule::mergeVectorsUsingSets(std::vector<std::vector<int>>& A)
{

  // Make empty list of sets "output"
  std::vector< std::set<int> > output;

  for (auto& vec : A) {
    std::set<int> s(vec.begin(), vec.end());

    //Check whether element intersects with any in output
    for (auto it = output.begin(); it != output.end();) {
      std::set<int> intersect;
      std::set_intersection(it->begin(), it->end(), s.begin(), s.end(),
                            std::inserter(intersect, intersect.begin()));

      if (!intersect.empty()) {
        s.insert(it->begin(), it->end());
        it = output.erase(it);
      } else ++it;
    }
    output.push_back(s);
  }

  return output;
}

std::vector<std::vector<int>> ECLCRFinderModule::getConnectedRegions(const std::vector<int>& A, const std::vector<int>& B,
                           const int maptype)
{
  std::vector<std::vector<int>> connectedRegions;

  for (unsigned int i = 0; i < A.size(); ++i) {
    if (A[i] > 0) {
      std::vector<int> region;
      region.push_back(i);

      for (unsigned int j = 0; j < B.size(); ++j) {
        if (B[j] > 0 && areNeighbours(i, j, maptype)) {
          region.push_back(j);
        }
      }

      std::sort(region.begin(), region.end());
      region.erase(unique(region.begin(), region.end()), region.end());
      connectedRegions.push_back(region);
    }
  }

  return connectedRegions;
}
