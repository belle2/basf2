/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Zbynek Drasal             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <svd/modules/vxdSpacePointMaker/VXDSpacePointMakerModule.h>

#include <time.h>

// Hit classes
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <framework/dataobjects/Relation.h>
#include <generators/dataobjects/MCParticle.h>
#include <svd/dataobjects/VXDSpacePoint.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// Standard C++
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <list>
#include <cmath>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDSpacePointMaker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDSpacePointMakerModule::VXDSpacePointMakerModule() : Module(),
    m_currentLayerID(0), m_currentLadderID(0), m_currentSensorID(0), m_currentSensorUniID(0),
    m_geometry(NULL)
{
  // Set description()
  setDescription("VXDSpacePointMaker: a basf2 module producing spacepoints from PXD and SVD simhits.");

  // Add parameters
  addParam("MCPartColName", m_mcColName, "MCParticles collection name",
           string(DEFAULT_MCPARTICLES));
  addParam("InputPXDColName", m_pxdColName, "PXDSimHits collection name",
           string(DEFAULT_PXDSIMHITS));
  addParam("MCParticlesToPXDSimHitsColName", m_pxdRelName,
           "Name of MCParticles-to-PXDSimHits relation",
           string(DEFAULT_PXDSIMHITSREL));
  addParam("InputSVDColName", m_svdColName, "SVDSimHits collection name",
           string(DEFAULT_SVDSIMHITS));
  addParam("MCParticlesToSVDSimHitsColName", m_svdRelName,
           "Name of MCParticles-to-SVDSimHits relation",
           string(DEFAULT_SVDSIMHITSREL));
  addParam("OutputColName", m_outColName, "VXDSpacePoints collection name",
           string(DEFAULT_VXDSPACEPOINTS));
  addParam("MCParticlesToSpacePointsCollectionName", m_outRelName,
           "Relations between MCParticles and spacepoints",
           string(DEFAULT_VXDSPACEPOINTSREL));


  //      "Name of relation collection - MC hits to Digitizer hits. (created if non-null)", string("SVDMC2DigiHitRel"));
}

VXDSpacePointMakerModule::~VXDSpacePointMakerModule()
{
  ;
}

void VXDSpacePointMakerModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void VXDSpacePointMakerModule::beginRun()
{
  // Print run number
  B2INFO("VXDSpacePointMaker: Processing run: " << m_nRun);

  // Re-initialize geometry cache
  m_geometry = SiGeoCache::instance();
  m_geometry->refresh();
}

void VXDSpacePointMakerModule::event()
{
  // Initialize counters
  m_nPointsSaved = 0;
  m_nRelationsSaved = 0;

  //------------------------------------------------------
  // Get the collection of MCParticles from the DataStore.
  //------------------------------------------------------
  StoreArray<MCParticle> storeMCParticles(m_mcColName);
  if (!storeMCParticles) {
    B2ERROR("VXDSpacePointMaker: Input collection " << m_mcColName << " unavailable.");
  }

  //------------------------------------------------------
  // Get the collection of PXDSimHits from the Data store.
  //------------------------------------------------------
  StoreArray<PXDSimHit> storePXDHits(m_pxdColName);
  if (!storePXDHits) {
    B2ERROR("VXDSpacePointMaker: Input collection " << m_pxdColName << " unavailable.");
  }

  //------------------------------------------------------
  // Get the collection of SVDSimHits from the Data store.
  //------------------------------------------------------
  StoreArray<SVDSimHit> storeSVDHits(m_svdColName);
  if (!storeSVDHits) {
    B2ERROR("VXDSpacePointMaker: Input collection " << m_svdColName << " unavailable.");
  }

  // Number of SimHits on input
  StoreIndex nPXDSimHits = storePXDHits->GetEntries();
  StoreIndex nSVDSimHits = storeSVDHits->GetEntries();
  StoreIndex nSimHits = nPXDSimHits + nSVDSimHits;
  if (nSimHits == 0) return; // no hits to process

  // Create HitSorter structure and fill it
  HitRecordSet simHitSet; // Contains pairs SensorUniID - SimHit
  SensorSet sensorSet;    // Lists all SensorUniIDs that contain hits
  SensorUniIDManager uidCodec(0);
  for (StoreIndex iHit = 0; iHit < nPXDSimHits; ++iHit) {
    PXDSimHit* hit = storePXDHits[iHit];
    HitRecord aHitRec;
    aHitRec.m_index = iHit;
    uidCodec.setLayerID(hit->getLayerID());
    uidCodec.setLadderID(hit->getLadderID());
    uidCodec.setSensorID(hit->getSensorID());
    aHitRec.m_sensorUID = uidCodec.getSensorUniID();
    simHitSet.insert(aHitRec);
    sensorSet.insert(aHitRec.m_sensorUID);
  }
  for (StoreIndex iHit = 0; iHit < nSVDSimHits; ++iHit) {
    SVDSimHit* hit = storeSVDHits[iHit];
    HitRecord aHitRec;
    aHitRec.m_index = iHit;
    uidCodec.setLayerID(hit->getLayerID());
    uidCodec.setLadderID(hit->getLadderID());
    uidCodec.setSensorID(hit->getSensorID());
    aHitRec.m_sensorUID = uidCodec.getSensorUniID();
    simHitSet.insert(aHitRec);
    sensorSet.insert(aHitRec.m_sensorUID);
  }

  // get the sensor-side index
  SensorSideIndex& sensorIndex = simHitSet.get<SensorUIDSide>();


  //------------------------------------------------------
  // Get the MCParticle->PXDSimHit relations from the
  // DataStore and create support structure.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToPXDHits(m_pxdRelName);
  if (!storeMCToPXDHits) {
    B2ERROR("VXDSpacePointMaker: Input collection " << m_pxdRelName << " unavailable.");
  }

  // Fill with relation data
  int nMCPXDRels = storeMCToPXDHits.GetEntries();
  for (int iRel = 0; iRel < nMCPXDRels; ++iRel) {
    Relation* rel = storeMCToPXDHits[iRel];
    AtomicRelation arel;
    arel.m_from = rel->getFromIndex();
    RelList toIndices = rel->getToIndices();
    for (RelListItr idx = toIndices.begin(); idx != toIndices.end(); ++idx) {
      arel.m_to = (*idx);
      arel.m_weight = 1.0; // no way to retrieve weights.
      m_relMCPXD.insert(arel);
    }
  }

  // Get the "to-side" index to relations.
  ToSideIndex& pxdIndex = m_relMCPXD.get<ToSide>();


  //------------------------------------------------------
  // Get the MCParticle->SVDSimHit relations from the
  // DataStore and create support structure.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToSVDHits(m_svdRelName);
  if (!storeMCToSVDHits) {
    B2ERROR("VXDSpacePointMaker: Input collection " << m_svdRelName << " unavailable.");
  }

  // Fill with relation data
  int nMCSVDRels = storeMCToSVDHits.GetEntries();
  for (int iRel = 0; iRel < nMCSVDRels; ++iRel) {
    Relation* rel = storeMCToSVDHits[iRel];
    AtomicRelation arel;
    arel.m_from = rel->getFromIndex();
    RelList toIndices = rel->getToIndices();
    for (RelListItr idx = toIndices.begin(); idx != toIndices.end(); ++idx) {
      arel.m_to = (*idx);
      arel.m_weight = 1.0; // no way to retrieve weights.
      m_relMCSVD.insert(arel);
    }
  }

  // Get the "to-side" index to relations.
  ToSideIndex& svdIndex = m_relMCSVD.get<ToSide>();


  //-----------------------------------------------------
  // Create the collection of VXDSpacePoints in the
  // Data store.
  //-----------------------------------------------------
  StoreArray<VXDSpacePoint> storeVXDPts(m_outColName);
  if (!storeVXDPts) {
    B2ERROR("VXDSpacePointMaker: Cannot create output collection " << m_outColName);
  }

  //-----------------------------------------------------
  // Create the collection of MCParticle-to-VXDSpacePoints
  // in the Data store.
  //-----------------------------------------------------
  StoreArray<Relation> storeMCToVXDPts(m_outRelName);
  if (!storeMCToVXDPts) {
    B2ERROR("VXDSpacePointMaker: Cannot create output collection " << m_outRelName);
  }

  //---------------------------------------------------------------------
  // Process data in all modules hit by particles.
  //---------------------------------------------------------------------

  for (SensorSetItr iSensor = sensorSet.begin(); iSensor != sensorSet.end(); ++iSensor) {

    //---------------------------------------------------------------
    // Initialize sensor data
    //---------------------------------------------------------------

    m_currentSensorUniID = *iSensor;
    m_currentLayerID = m_geometry->getLayerID(m_currentSensorUniID);
    m_currentLadderID = m_geometry->getLadderID(m_currentSensorUniID);
    m_currentSensorID = m_geometry->getSensorID(m_currentSensorUniID);

    // Get detector type - SiGeoCache::c_pixel or SiGeoCache::c_strip.
    SiGeoCache::SiLayerType detType = m_geometry->getLayerType(m_currentLayerID);

    //------------------------------------------------------------------------
    // Construct the set of hits that lie within this sensor
    //------------------------------------------------------------------------
    std::pair<SensorSideItr, SensorSideItr> sensorHits =
      sensorIndex.equal_range(m_currentSensorUniID);

    //------------------------------------------------------------------------
    // Loop over hits in the current sensor
    //------------------------------------------------------------------------
    for (SensorSideItr hitItr = sensorHits.first; hitItr != sensorHits.second; ++ hitItr) {

      StoreIndex iHit = hitItr->m_index;

      TVector3 posLocal(0, 0, 0); // Local hit position
      float depEnergy = 0;     // Deposited energy

      if (detType == SiGeoCache::c_pixel) {
        PXDSimHit * simHit = storePXDHits[iHit];
        if (!simHit) continue; // A hole in the PXDSimHitArray
        posLocal = 0.5 * (simHit->getPosIn() + simHit->getPosOut());
        depEnergy = simHit->getEnergyDep();
      } else if (detType == SiGeoCache::c_strip) {
        SVDSimHit * simHit = storeSVDHits[iHit];
        if (!simHit) continue; // A hole in the SVDSimHitArray
        posLocal = 0.5 * (simHit->getPosIn() + simHit->getPosOut());
        depEnergy = simHit->getEnergyDep();
      }

      // Transform the hit into global reference frame
      TVector3 posGlobal;
      m_geometry->localToMaster(m_currentSensorUniID, posLocal, posGlobal);

      // save the spacepoint into output array
      int ptIndex = storeVXDPts->GetLast() + 1;
      new(storeVXDPts->AddrAt(ptIndex)) VXDSpacePoint(
        m_currentSensorUniID,
        m_currentLayerID,
        m_currentLadderID,
        m_currentSensorID,
        static_cast<float>(posGlobal.X()),
        static_cast<float>(posGlobal.Y()),
        static_cast<float>(posGlobal.Z()),
        depEnergy);
      m_nPointsSaved++;

      // Now there are relations to save.
      if (detType == SiGeoCache::c_pixel) {
        // Get the index of MCParticle corresponding to this SimHit.
        // Should be at most one. Then save the relation.
        StoreIndex iMCPart = 0;
        ToSideItr mcItr = pxdIndex.find(iHit);
        if (mcItr != pxdIndex.end())
          iMCPart = mcItr->m_from;
        if (iMCPart > 0) {
          int relIndex = storeMCToVXDPts->GetLast() + 1;
          new(storeMCToVXDPts->AddrAt(relIndex)) Relation(
            storeMCParticles, storeVXDPts, iMCPart, ptIndex, 1.0
          );
          m_nRelationsSaved++;
        }
      } else if (detType == SiGeoCache::c_strip) {
        // Get the index of MCParticle corresponding to this SimHit.
        // Should be at most one. Then save the relation.
        StoreIndex iMCPart = 0;
        ToSideItr mcItr = svdIndex.find(iHit);
        if (mcItr != svdIndex.end())
          iMCPart = mcItr->m_from;
        if (iMCPart > 0) {
          int relIndex = storeMCToVXDPts->GetLast() + 1;
          new(storeMCToVXDPts->AddrAt(relIndex)) Relation(
            storeMCParticles, storeVXDPts, iMCPart, ptIndex, 1.0
          );
          m_nRelationsSaved++;
        }
      } // detType
    } // for iHit
  } // for SensorSetItr

  m_nEvent++;
  m_relMCPXD.clear();
  m_relMCSVD.clear();
}

void VXDSpacePointMakerModule::endRun()
{
  m_nRun++;
}

void VXDSpacePointMakerModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("VXDSpacePointMaker finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");
  B2INFO("Saved " << m_nPointsSaved << " hits and " << m_nRelationsSaved << " relations.")
}

void VXDSpacePointMakerModule::printModuleParams() const
{
  B2INFO("VXDSpacePointMaker parameters:")
  B2INFO("  PXD collection name:  " << m_pxdColName)
  B2INFO("  SVD collection name:  " << m_svdColName)
  B2INFO("  Output collection name: " << m_outColName)
}


