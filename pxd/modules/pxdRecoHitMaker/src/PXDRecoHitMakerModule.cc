/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <pxd/modules/pxdRecoHitMaker/PXDRecoHitMakerModule.h>
#include <pxd/geopxd/SiGeoCache.h>

// dataobjects
#include <pxd/dataobjects/PXDHit.h>
#include <pxd/dataobjects/PXDRecoHit.h>
#include <pxd/dataobjects/RelationHolder.h>
#include <generators/dataobjects/MCParticle.h>

#include <time.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

// STL
#include <utility>
#include <list>


using namespace std;
using namespace boost;
using namespace Belle2;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRecoHitMaker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRecoHitMakerModule::PXDRecoHitMakerModule() : Module()
{
  // Set description()
  setDescription("PXDRecoHit maker");

  // Add parameters
  addParam("MCPartColName", m_mcColName, "MCParticles collection name",
           string(DEFAULT_MCPARTICLES));
  addParam("InputColName", m_inColName, "PXDHits collection name",
           string(DEFAULT_PXDHITS));
  addParam("OutputColName", m_outColName, "PXDRecoHits collection name",
           string(DEFAULT_PXDRECOHITS));
  addParam("MCParticlesToPXDHits", m_relHitName, "Relation MCPart-to-PXDHits",
           string(DEFAULT_PXDHITSREL));
  addParam("MCParticlesToPXDRecoHits", m_relRecName, "Relation MCPart-to-PXDRecoHits",
           string(DEFAULT_PXDRECOHITSREL));
}

PXDRecoHitMakerModule::~PXDRecoHitMakerModule()
{
  ;
}

void PXDRecoHitMakerModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  // printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void PXDRecoHitMakerModule::beginRun()
{
  // Print run number
  B2INFO("PXDRecoHitMaker: Processing run: " << m_nRun);

  // Re-read geometry for a new run
  SiGeoCache::instance()->refresh();
}

void PXDRecoHitMakerModule::event()
{
  //------------------------------------------------------
  // Get the collection of PXDSimHits from the Data store.
  //------------------------------------------------------
  StoreArray<MCParticle> mcPartArray(m_mcColName);
  if (!mcPartArray) {
    B2ERROR("PXDRecoHitMaker: Cannot get collection " << m_mcColName << " from the DataStore.");
  }

  //------------------------------------------------------
  // Get the collection of PXDHits from the DataStore.
  //------------------------------------------------------
  StoreArray<PXDHit> pxdInArray(m_inColName);
  if (!pxdInArray) {
    B2ERROR("PXDRecoHitMaker: Cannot get collection " << m_inColName << " from the DataStore.")
    return;
  }

  //-----------------------------------------------------
  // Get the collection of PXDRecoHits from the Data store,
  // (or have one created)
  //-----------------------------------------------------
  StoreArray<PXDRecoHit> pxdOutArray(m_outColName);
  if (!pxdOutArray) {
    B2ERROR("PXDRecoHitMaker: Cannot get collection " << m_inColName << " from the DataStore.")
    return;
  }

  //-----------------------------------------------------
  // Get the MCParticles-to-PXDHits collection from
  // the Data store, and initialize the RelationHolder.
  //-----------------------------------------------------
  StoreArray<Relation> mcHitArray(m_relHitName);
  if (!mcHitArray) {
    B2ERROR("PXDRecoHitMaker: Cannot get collection " << m_relHitName << " from the DataStore.");
  }
  // Create the relaiton holder:
  TwoSidedRelationSet relMCHit;

  // Fill with relation data
  int nMCHitRels = mcHitArray.GetEntries();
  for (int iRel = 0; iRel < nMCHitRels; ++iRel) {
    Relation* rel = mcHitArray[iRel];
    AtomicRelation arel;
    arel.m_from = rel->getFromIndex();
    RelList toIndices = rel->getToIndices();
    for (RelListItr idx = toIndices.begin(); idx != toIndices.end(); ++idx) {
      arel.m_to = (*idx);
      arel.m_weight = 1.0; // no way to retrieve weights.
      relMCHit.insert(arel);
    }
  }

  // Get the "to-side" index to relations.
  ToSideIndex& hitIndex = relMCHit.get<ToSide>();

  //-----------------------------------------------------
  // Get the MCParticles-to-PXDRecoHits collection
  // from the Data store (or have one created).
  //-----------------------------------------------------
  StoreArray<Relation> mcRecArray(m_relRecName);
  if (!mcRecArray) {
    B2ERROR("PXDRecoHitMaker: Cannot get collection " << m_relRecName << " from the DataStore.");
  }

  //---------------------------------------------------------------------
  // Convert PXDHits one by one to RecoHits.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = pxdInArray->GetEntries();

  // Loop over all hits
  for (int iHit = 0; iHit < nHits; ++iHit) {
    // Make space and turn next PXDHit into PXDRecoHit.
    new(pxdOutArray->AddrAt(iHit)) PXDRecoHit(pxdInArray[iHit]);

    // get source MCParticles and save (atomic) relation(s) to mcRecArray
    pair<ToSideItr, ToSideItr> eqRange = hitIndex.equal_range(iHit);
    for (ToSideItr mcHit = eqRange.first; mcHit != eqRange.second; ++mcHit) {
      new(mcRecArray->AddrAt(iHit))
      Relation(mcPartArray, pxdOutArray, mcHit->m_from, iHit, mcHit->m_weight);
    }
  }

  m_nEvent++;
}

void PXDRecoHitMakerModule::endRun()
{
  m_nRun++;
}

void PXDRecoHitMakerModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("PXDRecoHitMaker finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");
}

void PXDRecoHitMakerModule::printModuleParams() const
{
  B2INFO("PXDRecoHitMaker parameters:")
  B2INFO("  Input collection name:  " << m_inColName)
  B2INFO("  Output collection name: " << m_outColName)
  B2INFO("  Relation collection name: " << m_relRecName)
  ;
}
