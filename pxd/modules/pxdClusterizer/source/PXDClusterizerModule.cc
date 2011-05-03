/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Benjamin Schwenker,         *
 *               Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// History:
//- original version - ILCsoft VTXDigitizer by A. Raspereza
//- added procesor parameters: electronic noise, Z. Drasal Jan 2009
//- completely changed code structure, Z. Drasal Mar 2009
//- renamed to SiPxlGeom & created as standalone pckg, Z. Drasal Jul 2009
//- added geometry interface (SiPxlGeom), Z. Drasal Aug 2009
//- added parameter for cut on time to emulate the integration time, K. Prothmann Dec. 2009
//- new proper clustering implemented & noise generation updated, Z. Drasal Jan 2010
//- improved clustering procedure & corrected MCParticle weight according to deposited energy, Z. Drasal Feb 2010
//- fixed straggling function ("Landau fluctuations") in Si, Z. Drasal Mar 2010
//- implemented new Landau fluctuations (mean energy loss is calculated automatically), Z. Drasal Apr 2010
//- width of cluster parameter renamed to its proper name width of diffused cloud of e, Z. Drasal Apr 2010
//- digits map is saved (if name defined) together with hits map, TrackerPulse container used, Z. Drasal May 2010
//- corrected memory leak when not saving TrackerPulses, Z. Drasal Aug 2010
//- TrackerHit -> rawHits() contains 1 PXDSimHit, which contributed with highest weight, Z. Drasal Sep 2010
//- added effect of ganged pixels,
//- when two pixels in different rows are read-out simultaneously, Z. Drasal Sep 2010
//- implementation for basf2, Peter Kvasnicka, April 2011. Clustering separated
//  from the digitizer and implemented differently, keeping the core clustering
//  rules of the original implementation: a cluster is formed by pixels with
//  charge over seed threshold and their nearest neighbors with charge over
//  adjacent threshold). Different geometry implementation, different support
//  libraries, data and relation structures.

#include <pxd/modules/pxdClusterizer/PXDClusterizerModule.h>

// DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

// Data objects
#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/Digit.h>
#include <pxd/dataobjects/HitSorter.h>

// Framework utils
#include <framework/gearbox/Unit.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/logging/Logger.h>

// boost
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Include basic C
#include <cstdlib>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <numeric>
#include <set>
#include <cmath>
#include <time.h>

// ROOT
#include <TMath.h>
#include <TVector3.h>
#include <TRandom3.h>
#include <TGeoManager.h>

// Used namespaces
using namespace std;
using namespace boost::lambda;
using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterizerModule::PXDClusterizerModule() :
    Module(),
    m_currentLayerID(0), m_currentLadderID(0), m_currentSensorID(0),
    m_currentSensorUniID(0),
    m_geometry(NULL),
    m_cheater(new PXDCheater())
{

  // Set description()
  setDescription(
    "PXDClusterizerModule: basf2 module producing PXDHits from PXDDigits");

  // Processor parameters

  // Input and output collections
  addParam("MCPartColName", m_mcColName, "MCParticles collection name",
           string(DEFAULT_MCPARTICLES));
  addParam("DigitsCollectionName", m_digitColName, "Digits collection name",
           string(DEFAULT_PXDDIGITS));
  addParam("MCParticlesToDigitsCollectionName", m_relDigitName,
           "Relations between MCParticles and Digits", string(DEFAULT_PXDDIGITSREL));
  addParam("HitsCOllectionName", m_hitColName, "PXDHit collection name", string(
             DEFAULT_PXDHITS));
  addParam("MCParticlesToHitsColName", m_relHitName,
           "Name of MCParticles-to-PXDHits relation", string(DEFAULT_PXDHITSREL));
  addParam("DigitsToHitsColName", m_clusterColName,
           "Name of PXDDigits-to-PXDHits relation", string(DEFAULT_PXDCLUSTERS));
  addParam("ADC", m_ADC, "Simulate ADC?", bool(false));
  addParam("ADCRange", m_ADCRange,
           "Set analog-to-digital converter range 0 - ? (in e)", int(24000));
  addParam("ADCBits", m_ADCBits, "Set how many bits the ADC uses", int(8));
  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", double(200.));
  addParam("ZeroSuppressionSN", m_SNAdjacent, "S/N threshold for zero suppression", double(3.0));
  addParam("SeedSN", m_SNSeed, "S/N threshold for cluster seeds", double(5.0));
  addParam("ClusterChargeSN", m_SNTotal, "S/N threshold for cluster charge", double(8.0));
  addParam("TanLorentz", m_tanLorentzAngle, "Tangent of the Lorentz angle",
           double(0.25));
}  // PXDClusterizerModule c'tor

PXDClusterizerModule::~PXDClusterizerModule()
{
  ;
}

//
// Method called at the beginning of data processing
//
void PXDClusterizerModule::initialize()
{
  // Initialize variables
  m_nRun = 0;
  m_nEvt = 0;

  m_currentLayerID = 0;
  m_currentLadderID = 0;
  m_currentSensorID = 0;

  // Parameters are entered in basf2 unit system.
  m_ADCRange = static_cast<int>(m_ADCRange * Unit::e);
  m_ADCUnit = m_ADCRange / pow(2.0, static_cast<double>(m_ADCBits));
  m_elNoise *= Unit::e;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::ms / 1000;

  //------------------------------------------------------
  // Register the collection of PXDHits in the DataStore
  //------------------------------------------------------
  StoreArray<PXDHit> storeHits(m_hitColName);

  //------------------------------------------------------
  // Register the collection of  MCParticle->PXDHit
  // relations in the DataStore.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToHits(m_relHitName);

  //------------------------------------------------------
  // Register the collection of  PXDDigit->ClusterNumber
  // relations in the DataStore.
  //------------------------------------------------------
  StoreArray<Relation> storeClusters(m_clusterColName);

  //
  // ROOT variables
  //
#ifdef ROOT_OUTPUT
  m_rootFile = new TFile("BelleII_PXD_Hits.root", "recreate");

  m_rootFile->cd("");

  // Declare Tree
  m_rootTree = new TTree("Hits", "Hit info");

  // After 50MB it will flush the data
  m_rootTree->SetAutoFlush(-2000000);

  m_rootTree->Branch("Layer" , &m_rootLayerID , "Layer/I");
  m_rootTree->Branch("Ladder" , &m_rootLadderID , "Ladder/I");
  m_rootTree->Branch("Sensor" , &m_rootSensorID , "Sensor/I");
  m_rootTree->Branch("RPhiSim" , &m_rootSimRPhi , "SimRPhi/D");
  m_rootTree->Branch("ZSim" , &m_rootSimZ , "SimZ/D");
  m_rootTree->Branch("MCPDG" , &m_rootMCPDG , "MCPDG/I");
  m_rootTree->Branch("IsFromMCPrim", &m_rootIsFromMCPrim, "IsFromMCPrim/I");
  m_rootTree->Branch("EvtNum" , &m_rootEvtNum , "EvtNum/I");

  // Declare Efficiency histogram
  m_rootEfficiency = new TH1F("Efficiency" , "Muon hit reconstruction efficiency" , 101, -0.005, 1.005);
  m_rootMultiplicity = new TH1F("Multiplicity", "Muon hit reconstruction multiplicity" , 501, -0.005, 5.005);
  m_rootELossG4      = new TH1F("ELossG4"     , "G4 - Energy loss in silicon"                    , 200, 0.   , 200.);
  m_rootELossDigi    = new TH1F("ELossDigi"   , "Digi - Energy loss in silicon"                  , 200, 0.   , 200.);
  m_rootDigitsMap    = new TH2F("DigitsMap"   , "Digits map"                                     , 5000, 0., 5000., 1600, 0., 1600.);
  m_rootOccupancy_0 = new TH1F("Occupancy_0" , "Occupancy_0" , 101, -0.05, 10.05);
  m_rootOccupancy_1 = new TH1F("Occupancy_1" , "Occupancy_1" , 101, -0.05, 10.05);

#endif
} //PXDClusterizerModule::initialize()

//
// Method called for each run
//
void PXDClusterizerModule::beginRun()
{
  // Print run number
  B2INFO("PXDClusterizer: Processing run: " << m_nRun)

  // Re-initialize geometry cache
  m_geometry = SiGeoCache::instance();
  m_geometry->refresh();

} // PXDClusterizerModule::beginRun()

//
// Method called for each event
//
void PXDClusterizerModule::event()
{
  B2INFO("PXDClusterizer: Processing event " << m_nEvt)

  /** Number of hits stored in this event. */
  m_nHitsSaved = 0;

  /** Number of relations saved in this event. */
  m_nRelationsSaved = 0;

  // Initialize number of found muon hits versus all hits - efficiency map, resp. dep. energy in each event
#ifdef ROOT_OUTPUT

  m_rootEvtNum = m_nEvt;

  m_rootMCGenHitMap.clear();

  m_rootDepEG4   = 0.;
  m_rootDepEDigi = 0.;

#endif

  //------------------------------------------------------
  // Get the collection of MCParticles from the DataStore.
  //------------------------------------------------------
  // We only need a pointer to the array to create relations.
  StoreArray<MCParticle> storeMCParticles(m_mcColName);
  if (!storeMCParticles) {
    B2ERROR("PXDClusterizer: Input collection " << m_mcColName << " unavailable.");
  }

  //------------------------------------------------------
  // Get the collection of PXDDigiits from the DataStore.
  //------------------------------------------------------
  StoreArray<PXDDigit> storeDigits(m_digitColName);
  if (!storeDigits) {
    B2ERROR("PXDClusterizer: Input collection " << m_digitColName << " unavailable.");
  }

  // Number of digits on input
  StoreIndex nDigits = storeDigits->GetEntries();
  if (nDigits == 0) return;

  // Create HitSorter structure and fill it.
  // This structure is used to sort digits by detector.
  StoreRecordSet digitSet;
  SensorSet sensorSet;
  for (StoreIndex iDigit = 0; iDigit < nDigits; ++iDigit) {
    PXDDigit* pxdDigit = storeDigits[iDigit];
    StoreRecord aDigitRec;
    aDigitRec.m_index = iDigit;
    aDigitRec.m_sensorUniID = pxdDigit->getSensorUniID();
    digitSet.insert(aDigitRec);
    sensorSet.insert(aDigitRec.m_sensorUniID);
  }

  // get the sensor-side index
  SensorSideIndex& sensorIndex = digitSet.get<SensorUniIDSide>();

  //------------------------------------------------------
  // Get the MCParticle->PXDDigit relations from the
  // DataStore and create support structure.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToDigits(m_relDigitName);
  if (!storeMCToDigits) {
    B2ERROR("PXDClusterizer: Input collection " << m_relDigitName << " unavailable.");
  }

  // Create the index holder structure:
  TwoSidedRelationSet relMCDigits;

  // Fill with relation data
  int nMCDigitRels = storeMCToDigits.GetEntries();
  for (int iRel = 0; iRel < nMCDigitRels; ++iRel) {
    Relation* rel = storeMCToDigits[iRel];
    AtomicRelation arel;
    arel.m_from = rel->getFromIndex();
    RelList toIndices = rel->getToIndices();
    for (RelListItr idx = toIndices.begin(); idx != toIndices.end(); ++idx) {
      arel.m_to = (*idx);
      arel.m_weight = 1.0; // no way to retrieve weights.
      relMCDigits.insert(arel);
    }
  }

  // Get the "to-side" index to relations.
  ToSideIndex& digitsToMCIndex = relMCDigits.get<ToSide>();

  //------------------------------------------------------
  // Create the collection of PXDHits in the DataStore
  //------------------------------------------------------
  StoreArray<PXDHit> storeHits(m_hitColName);
  if (!storeHits) {
    B2ERROR("PXDClusterizer: Cannot initialize output collection " << m_hitColName << ".");
  }

  //------------------------------------------------------
  // Initialize the collection of  MCParticle->PXDHit
  // relations in the DataStore.
  // No need of support structure.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToHits(m_relHitName);
  if (!storeMCToHits) {
    B2ERROR("PXDClusterizer: Cannot initialize output collection " << m_relHitName << ".");
  }

  //------------------------------------------------------
  // Initialize the collection of  PXDDigit->ClusterNumber
  // relations in the DataStore.
  //------------------------------------------------------
  // Will be saved for reference, but not used due to
  // linear search times.

  StoreArray<Relation> storeClusters(m_clusterColName);
  if (!storeClusters) {
    B2ERROR("PXDClusterizer: Cannot initialize output collection " << m_clusterColName << ".");
  }

  //------------------------------------------------------
  // Loop over Digits:
  // - first over set of sensors
  // - then clusterize digits in individual sensors.
  //------------------------------------------------------

  for (SensorSetItr iSensor = sensorSet.begin(); iSensor != sensorSet.end(); ++iSensor) {

    //------------------------------------------------------------------------
    // Initialize sensor-related data.
    //------------------------------------------------------------------------

    m_currentSensorUniID = *iSensor;
    m_currentLayerID = m_geometry->getLayerID(m_currentSensorUniID);
    m_currentLadderID = m_geometry->getLadderID(m_currentSensorUniID);
    m_currentSensorID = m_geometry->getSensorID(m_currentSensorUniID);

    B2INFO("Processing sensor: Layer: " << m_currentLayerID
           << " Ladder: " << m_currentLadderID
           << " Sensor: " << m_currentSensorID)

    // Check if digitizing pixels
    if (m_geometry->getLayerType(m_currentLayerID) != SiGeoCache::c_pixel) {
      B2ERROR("PXDClusterizer::event() - sensor not of pixel type!!!")
      continue;
    }

    // Get pitch and number of pixels in both directions
    m_sensorUPitch = m_geometry->getUSensorPitch(m_currentSensorUniID);
    m_sensorUCells = m_geometry->getUSensorCells(m_currentSensorUniID);
    m_sensorVPitch = m_geometry->getVSensorPitch(m_currentSensorUniID);
    m_sensorVCells = m_geometry->getVSensorCells(m_currentSensorUniID);

    // Initialize cuts on seed charge and cluster charge.
    m_seedCut = m_elNoise * m_SNSeed;
    m_clusterChargeCut = m_elNoise * m_SNTotal;

    if (m_ADC) {
      m_seedCut = getInADCUnits(m_seedCut);
      m_clusterChargeCut = getInADCUnits(m_clusterChargeCut);
    }

    //------------------------------------------------------------------------
    // Construct the set of hits that lie within this sensor
    //------------------------------------------------------------------------
    std::pair<SensorSideItr, SensorSideItr> sensorDigits =
      sensorIndex.equal_range(m_currentSensorUniID);

    //------------------------------------------------------------------------
    // Turn digits in the current sensor into a ClsDigitSet structure.
    //------------------------------------------------------------------------
    // We can only search in linear time in DataStore arrays. We therefore use
    // special multi-index structures providing constant or log time searches.
    // Relations are simply handed over based on cluster structure in the end.
    // The hits and relations are saved sensor-wise to keep memory profile low.

    // This is an extra pass through PXDDigits, but we don't want to have all
    // digits indexed over all sensors.

    ClsDigitSet digitSet;

    for (SensorSideItr digitItr = sensorDigits.first; digitItr != sensorDigits.second; ++ digitItr) {

      StoreIndex idigit = digitItr->m_index;
      PXDDigit* pxdDigit = storeDigits[idigit];
      if (!pxdDigit) continue; // skip empty records

      ClsDigitRecord newdigit;
      newdigit.m_uCellID = pxdDigit->getUCellID();
      newdigit.m_vCellID = pxdDigit->getVCellID();
      newdigit.m_charge = pxdDigit->getCharge();
      newdigit.m_index = idigit;
      newdigit.m_cluster = 0; // 0 = no cluster
      digitSet.insert(newdigit);
    } // for digits in sensor

    // Get the cellID-side index
    CellSideIndex& digits = digitSet.get<CellSide>();

    // Get the charge-side index
    ChargeSideIndex& chargeIndex = digitSet.get<ChargeSide>();

    // Get the cluster side index
    ClusterSideIndex& clusters = digitSet.get<ClusterSide>();

    short int currentClusterNumber = 0; // all numbers are currently set to 0.
    //----------------------------------------------------------------------
    // Go through seed digits and clusterize
    //----------------------------------------------------------------------

    ChargeSideItr firstSeed = chargeIndex.lower_bound(m_elNoise * m_SNSeed);

    for (ChargeSideItr seedItr = firstSeed; seedItr != chargeIndex.end(); ++seedItr) {

      // This way we don't get seeds ordered by coordinate.
      // But we have efficient finds that work in log(n) time.

      // The current digit will either form a new cluster, or its cluster number
      // will be kept.
      short int proposedClusterNumber = currentClusterNumber + 1;

      short int uCellID = seedItr->m_uCellID;
      short int vCellID = seedItr->m_vCellID;

      //B2INFO("Scanning neighbours: (" << uCellID << ", " << vCellID << ")")

      // Search the neighbourhood. Also include the current cell (!)

      NeighbourSet neighbours;

      // include this cell (saves one index-find)
      neighbours.push_back(digitSet.project<CellSide>(seedItr));

      for (int iU = -1; iU <= 1; ++iU) {

        if ((uCellID + iU < 0) || (uCellID + iU >= m_sensorUCells))
          continue;

        for (int iV = -1; iV <= 1; ++iV) {

          if ((vCellID + iV < 0) || (vCellID + iV >= m_sensorVCells))
            continue;
          if ((iU == 0) && (iV == 0)) //central cell - already there
            continue;

          CellSideItr neighbour =
            digits.find(boost::make_tuple(uCellID + iU, vCellID + iV));

          if (neighbour != digits.end()) {
            neighbours.push_back(neighbour);
            //B2INFO("Added neighbour (" << uCellID+iU << ", " << vCellID + iV << ")")
          }

        } // iV
      }; // iU

      // Now the policy is to include all nearest-neighbor digits into the
      // cluster with lowest non-zero number found among them. If different
      // cluster numbers found, merge clusters.

      // Get the lowest cluster number from the neighbourhood.
      for (NeighbourSetItr iN = neighbours.begin(); iN != neighbours.end(); ++iN) {
        short int nClusterNo = (*iN)->m_cluster;
        if ((nClusterNo > 0) && (nClusterNo < proposedClusterNumber))
          proposedClusterNumber = nClusterNo;
      } // for neighbours I

      // Second pass: Merge cells or clusters.
      for (NeighbourSetItr iN = neighbours.begin(); iN != neighbours.end(); ++iN) {
        CellSideItr idigit = *iN;
        if (idigit->m_cluster == 0) {
          // just add cell to the cluster.
          // use modify_key to change the current cell.
          clusters.modify_key(
            digitSet.project<ClusterSide>(idigit),
            boost::lambda::_1 = proposedClusterNumber
          );
        } else if (idigit->m_cluster == proposedClusterNumber) {
          continue;
        } else {
          // modify the whole cluster
          std::pair<ClusterSideItr, ClusterSideItr> clRange =
            clusters.equal_range(idigit->m_cluster);
          for (ClusterSideItr icluster = clRange.first;
               icluster != clRange.second; ++icluster)
            clusters.modify_key(
              icluster,
              boost::lambda::_1 = proposedClusterNumber
            );
        }
      } // for neighbours II

      if (proposedClusterNumber == currentClusterNumber + 1) {
        currentClusterNumber++;
      } else if (proposedClusterNumber > currentClusterNumber + 1) {
        B2FATAL("PXDClusterizer: Mismatch in cluster numbers.")
      }

    } // for seeds

    //--------------------------------------------------------------------------
    // Save the clusters and hits.
    // -------------------------------------------------------------------------
    // No fuzzy membership. Each digit belongs to at most one cluster.
    // I will  save a reverse relation, cluster->digits. I believe this is the way
    // people will be looking at it most often.

    // We skip cluster 0, which contains unclustered non-seed digits.
    ClusterSideItr icluster = clusters.upper_bound(0);

    while (icluster != clusters.end()) {

      int currentClusterNo = icluster->m_cluster;

      // B2INFO("Processing cluster no. " << currentClusterNo)

      // find the range of current cluster number
      std::pair<ClusterSideItr, ClusterSideItr> clsRange =
        clusters.equal_range(currentClusterNo);

      // Apply cut on cluster charge.
      float clusterCharge = accumulate(clsRange.first, clsRange.second, 0,
                                       (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2))));

      if (clusterCharge < m_clusterChargeCut) {
        icluster = clsRange.second;
        continue;
      }

      // Copy digits to a ClusterDigits structure
      ClusterDigits cluster;
      cluster.insert(clsRange.first, clsRange.second);

      int iHit = storeHits->GetLast() + 1;

      // Create a space for the new hit
      PXDHit* storeHit = new(storeHits->AddrAt(iHit)) PXDHit();

      // Fill hit data.
      storeHit->setSensorUniID(m_currentSensorUniID);
      makePXDHit(cluster, storeHit);

      // Store relations of the hit to digits (in this direction)
      for (ClusterSideItr idigit = clsRange.first; idigit != clsRange.second; ++idigit) {
        int iRel = storeClusters->GetLast() + 1;
        new(storeClusters->AddrAt(iRel)) Relation(
          storeHits, storeDigits, iHit, idigit->m_index, idigit->m_charge
        );
      }

      // Save PXDHit relations to MCParticles.

      // Create the set of contributing MC particles: for each MCParticle, sum
      // its contributions over all digits that contribute to the hit.
      map<StoreIndex, float> mcWeight;

      for (ClusterSideItr idigit = clsRange.first; idigit != clsRange.second; ++idigit) {

        StoreIndex digitIndex = idigit->m_index;

        std::pair<ToSideItr, ToSideItr> mcRange = digitsToMCIndex.equal_range(digitIndex);

        for (ToSideItr iMCPart = mcRange.first; iMCPart != mcRange.second; ++iMCPart)
          mcWeight[iMCPart->m_from] += iMCPart->m_weight;

      }

      // Now save the map to the relations StoreArray.

      for (map<StoreIndex, float>::iterator iMapRel = mcWeight.begin();
           iMapRel != mcWeight.end(); ++iMapRel)

        new(storeMCToHits->AddrAt(storeMCToHits->GetLast() + 1)) Relation(
          storeMCParticles, storeHits, iMapRel->first, iHit, iMapRel->second);

      icluster = clsRange.second;
    } // loop over clusters

  } // Loop over sensors


  m_nEvt++;

  // Print event number
  if (m_nEvt % 100 == 0)B2INFO("Events processed: " << m_nEvt)
  } //PXDClusterizerModule::event()

//
// Method called after each run to check the data processed
//
void PXDClusterizerModule::endRun()
{
  m_nRun++;
} //PXDClusterizerModule::endRun()

//
// Method called after all data processing
//
void PXDClusterizerModule::terminate()
{

  //---------------------------------------------------------------------------
  // CPU time end
  //---------------------------------------------------------------------------
  m_timeCPU = clock() * Unit::ms / 1000 - m_timeCPU;

  //---------------------------------------------------------------------------
  // Print message
  //---------------------------------------------------------------------------
  B2INFO(" Time per event: "
         << std::setiosflags(std::ios::fixed | std::ios::internal)
         << std::setprecision(3)
         << m_timeCPU / m_nEvt / Unit::ms
         << " ms"
         << std::endl
         << " Processor successfully finished!"
        )

  //---------------------------------------------------------------------------
  // Clear things out of memory.
  //---------------------------------------------------------------------------

  if (m_cheater) delete m_cheater;

#ifdef ROOT_OUTPUT

  // Close file
  m_rootFile->cd("");
  m_rootFile->Write();
  m_rootFile->Close();

#endif

} //PXDClusterizerModule::terminate()

//---------------------------------------------------------------------
// Make a PXDHit from a cluster.
//---------------------------------------------------------------------
//
void PXDClusterizerModule::makePXDHit(ClusterDigits& cluster, PXDHit* storeHit)
{
  //---------------------------------------------------------------------------
  // Find cluster edges and middle zone in both coordinates.
  //---------------------------------------------------------------------------

  // U coordinate

  USideIndex& uIndex = cluster.get<USide>();

  USideItr iULow = uIndex.begin();
  short int uLow = iULow->m_uCellID;

  USideItr iUHi  = --uIndex.end();
  short int uHi  = iUHi->m_uCellID;

  short int uClusterSize = uHi - uLow + 1;

  if (uClusterSize == 1) { // The estimate is mid-position in the pixel

    storeHit->setU(m_geometry->getUCellPosition(m_currentSensorUniID, uLow));

  } else if (uClusterSize == 2) {  // Centre-of-gravity

    float chargeHi = accumulate(
                       uIndex.lower_bound(uHi),
                       uIndex.upper_bound(uHi),
                       0,
                       (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                     );

    float chargeLow = accumulate(
                        uIndex.lower_bound(uLow),
                        uIndex.upper_bound(uLow),
                        0,
                        (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                      );

    float posLow = m_geometry->getUCellPosition(m_currentSensorUniID, uLow);

    // This is safe as long as we have positive cuts on pixel charge
    float eta = chargeHi / (chargeHi + chargeLow);

    float position = posLow + eta * m_sensorUPitch;

    storeHit->setU(position);

  } else if (uClusterSize > 2) {  // Analog head-tail

    float chargeHi = accumulate(
                       uIndex.lower_bound(uHi),
                       uIndex.upper_bound(uHi),
                       0,
                       (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                     );

    float posHi = m_geometry->getUCellPosition(m_currentSensorUniID, uHi);

    float chargeCentre = accumulate(
                           uIndex.upper_bound(uLow),
                           uIndex.lower_bound(uHi),
                           0,
                           (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                         );
    chargeCentre /= (uHi - uLow - 1.0);

    float chargeLow = accumulate(
                        uIndex.lower_bound(uLow),
                        uIndex.upper_bound(uLow),
                        0,
                        (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                      );

    float posLow = m_geometry->getUCellPosition(m_currentSensorUniID, uLow);

    float position = 0.5 * (posLow + posHi)
                     + 0.5 * (chargeHi - chargeLow) / chargeCentre * m_sensorUPitch;

    storeHit->setU(position);

  } // case uClusterSize

  // V coordinate

  VSideIndex& vIndex = cluster.get<VSide>();

  VSideItr iVLow = vIndex.begin();
  short int vLow = iVLow->m_vCellID;

  VSideItr iVHi  = --vIndex.end();
  short int vHi  = iVHi->m_vCellID;

  short int vClusterSize = vHi - vLow + 1;

  if (vClusterSize == 1) {  // The estimate is mid-position in the pixel

    storeHit->setV(m_geometry->getVCellPosition(m_currentSensorUniID, vLow));

  } else if (vClusterSize == 2) {  // Centre-of-gravity

    float chargeHi = accumulate(
                       vIndex.lower_bound(vHi),
                       vIndex.upper_bound(vHi),
                       0,
                       (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                     );

    float chargeLow = accumulate(
                        vIndex.lower_bound(vLow),
                        vIndex.upper_bound(vLow),
                        0,
                        (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                      );

    float posLow = m_geometry->getVCellPosition(m_currentSensorUniID, vLow);

    // This is safe as long as we have positive cuts on pixel charge
    float eta = chargeHi / (chargeHi + chargeLow);

    float position = posLow + eta * m_sensorVPitch;

    storeHit->setV(position);

  } else if (vClusterSize > 2) {  // Analog head-tail

    float chargeHi = accumulate(
                       vIndex.lower_bound(vHi),
                       vIndex.upper_bound(vHi),
                       0,
                       (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                     );

    float posHi = m_geometry->getVCellPosition(m_currentSensorUniID, vHi);

    float chargeCentre = accumulate(
                           vIndex.upper_bound(vLow),
                           vIndex.lower_bound(vHi),
                           0,
                           (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                         );
    chargeCentre /= (vHi - vLow - 1.0);

    float chargeLow = accumulate(
                        vIndex.lower_bound(vLow),
                        vIndex.upper_bound(vLow),
                        0,
                        (boost::lambda::_1 + ret<float>(bind(&ClsDigitRecord::m_charge, boost::lambda::_2)))
                      );

    float posLow = m_geometry->getVCellPosition(m_currentSensorUniID, vLow);

    float position = 0.5 * (posLow + posHi)
                     + 0.5 * (chargeHi - chargeLow) / chargeCentre * m_sensorVPitch;

    storeHit->setV(position);

  } // case vClusterSize

  //---------------------------------------------------------------------------
  // Correct for Lorentz shift
  //---------------------------------------------------------------------------
  // TODO:
  // - Derive tanLorentzAngle from magnetic field.
  // - I believe there's a more precise implementation of the shift.

  double uLorentzCorrection =
    0.5 * m_geometry->getSensorThickness(m_currentSensorUniID)
    * m_tanLorentzAngle;

  storeHit->setU(storeHit->getU() - uLorentzCorrection);

  // double vLorentzCorrection = 0.0;


  //---------------------------------------------------------------------------
  // Estimate coordinate errors.
  //---------------------------------------------------------------------------
  // We estimate theta angle of the hit and use a PXDCheater pre-set errors.
  // This is very primitive and should be improved by a cleverer method.

  // Estimate theta angle
  TVector3 oLocal(0.0, storeHit->getU(), storeHit->getV());
  TVector3 oGlobal(0.0, 0.0, 0.0);

  m_geometry->localToMaster(m_currentSensorUniID, oLocal, oGlobal);

  double theta = oGlobal.Theta() * Unit::rad;

  double uError =
    m_cheater->getResolutionRPhi(m_currentLayerID, m_currentLadderID, m_currentSensorID, theta);
  storeHit->setUError(uError);

  double vError =
    m_cheater->getResolutionZ(m_currentLayerID, m_currentLadderID, m_currentSensorID, theta);
  storeHit->setVError(vError);

  // There is now no means of estimating error covariance.
  storeHit->setUVCov(0.0);

  // Deposited energy
  float clusterCharge = accumulate(cluster.begin(), cluster.end(), 0,
                                   boost::lambda::_1 + bind<float>(&ClsDigitRecord::m_charge, boost::lambda::_2)
                                  );
  storeHit->setEnergyDep(clusterCharge);

} // PXDClusterizerModule::makeHit


//---------------------------------------------------------------------
// Print module parameters.
//---------------------------------------------------------------------
void PXDClusterizerModule::printModuleParams() const
{
  B2INFO("\nPXDClusterizerModule parameters: ")
  if (m_ADC) {
    B2INFO("  ADC range:                       "
           << m_ADCRange);
    B2INFO("  ADC bits:                        "
           << m_ADCBits);
  }
  B2INFO("  Electronics noise:               "
         << m_elNoise)
  B2INFO("  Tangent of Lorentz angle:        "
         << m_tanLorentzAngle)
  B2INFO("  Zero-suppression S/N cut:         "
         << m_SNAdjacent)
  B2INFO("  Seed S/N cut:                     "
         << m_SNSeed)
  B2INFO("  Cluster charge S/N cut:           "
         << m_SNTotal)

}

