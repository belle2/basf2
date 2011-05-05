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
#include <svd/modules/svdDigitizer/SVDDigiModule.h>

#include <time.h>

// Data objects
#include <generators/dataobjects/MCParticle.h>
#include <framework/dataobjects/Relation.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDHit.h>
#include <pxd/dataobjects/RelationHolder.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigi)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigiModule::SVDDigiModule() : Module(),
    m_cheater(new SVDcheater()),
    m_uniID(new SensorUniIDManager(0)),
    m_random(new TRandom3(0))
{
  // Set description()
  setDescription("SVDDigitizer");

  // Add parameters
  addParam("MCPartColName", m_mcColName, "MCParticles collection name",
           string(DEFAULT_MCPARTICLES));
  addParam("InputColName", m_inColName, "Input collection name",
           string(DEFAULT_SVDSIMHITS));
  addParam("OutputColName", m_outColName, "Output collection name",
           string(DEFAULT_SVDHITS));
  addParam("MCParticlesToSVDSimHits", m_relSimName, "Relation MCPart-to-SVDSimHits",
           string(DEFAULT_SVDSIMHITSREL));
  addParam("MCParticlesToSVDHits", m_relHitName, "Relation MCPart-to-SVDHits",
           string(DEFAULT_SVDHITSREL));
}

SVDDigiModule::~SVDDigiModule()
{
  if (m_random) delete m_random;
  if (m_uniID) delete m_uniID;
  if (m_cheater) delete m_cheater;
}

void SVDDigiModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  printModuleParams();

  // Initialize new StoreArrays
  StoreArray<SVDHit> svdOutArray(m_outColName);
  StoreArray<Relation> mcHitArray(m_relHitName);

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void SVDDigiModule::beginRun()
{
  // Print run number
  B2INFO("SVDDigi: Processing run: " << m_nRun);
}

void SVDDigiModule::event()
{
  //------------------------------------------------------
  // Get the collection of MCParticles from the Data store.
  //------------------------------------------------------
  StoreArray<MCParticle> mcPartArray(m_mcColName);
  if (!mcPartArray) {
    B2ERROR("SVDDigi: Cannot get collection " << m_mcColName << " from the DataStore.");
  }

  //------------------------------------------------------
  // Get the collection of SVDSimHits from the Data store.
  //------------------------------------------------------
  StoreArray<SVDSimHit> svdSimArray(m_inColName);
  if (!svdSimArray) {
    B2ERROR("SVDDigi: Input collection " << m_inColName << " unavailable.");
  }

  //-----------------------------------------------------
  // Get the MCParticles-to-SVDSimHits collection from
  // the Data store, and initialize the RelationHolder.
  //-----------------------------------------------------
  StoreArray<Relation> mcSimArray(m_relSimName);
  if (!mcSimArray) {
    B2ERROR("SVDDigi: Cannot get collection " << m_relSimName << " from the DataStore.");
  }
  // Create the relation holder:
  TwoSidedRelationSet relMCSim;

  // Fill with relation data
  int nMCSimRels = mcSimArray.GetEntries();
  for (int iRel = 0; iRel < nMCSimRels; ++iRel) {
    Relation* rel = mcSimArray[iRel];
    AtomicRelation arel;
    arel.m_from = rel->getFromIndex();
    RelList toIndices = rel->getToIndices();
    for (RelListItr idx = toIndices.begin(); idx != toIndices.end(); ++idx) {
      arel.m_to = (*idx);
      arel.m_weight = 1.0; // no way to retrieve weights.
      relMCSim.insert(arel);
    }
  }

  // Get the "to-side" index to relations.
  ToSideIndex& simIndex = relMCSim.get<ToSide>();


  //-----------------------------------------------------
  // Get the collection of SVDHits from the Data store,
  // (or have one created)
  //-----------------------------------------------------
  StoreArray<SVDHit> svdHitArray(m_outColName);
  if (!svdHitArray) {
    B2ERROR("SVDDigi: Output collection " << m_outColName << " unavailable.");
  }

  //-----------------------------------------------------
  // Get the MCParticles-to-SVDHits collection
  // from the Data store (or have one created).
  //-----------------------------------------------------
  StoreArray<Relation> mcHitArray(m_relHitName);
  if (!mcHitArray) {
    B2ERROR("SVDDigi: Cannot get collection " << m_relHitName << " from the DataStore.");
  }

  //---------------------------------------------------------------------
  // Convert SimHits one by one to digitizer hits.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = svdSimArray->GetEntries();

  // Loop over all hits
  for (int iHit = 0; iHit < nHits; ++iHit) {
    // Get a simhit
    SVDSimHit* aSimHit = svdSimArray[iHit];

    // The processing is simplistic here, one in / one out.

    // Create a digi hit
    new(svdHitArray->AddrAt(iHit)) SVDHit();
    SVDHit* newHit = svdHitArray[iHit];

    // Geometry
    m_uniID->setUniID(0);
    short int layerID = aSimHit->getLayerID();
    short int ladderID = aSimHit->getLadderID();
    short int sensorID = aSimHit->getSensorID();

    m_uniID->setLayerID(layerID);
    m_uniID->setLadderID(ladderID);
    m_uniID->setSensorID(sensorID);
    newHit->setSensorUniID(m_uniID->getSensorUniID());

    // Position + smear + errors
    TVector3 posIn = aSimHit->getPosIn();
    TVector3 posOut = aSimHit->getPosOut();
    float u = 0.5 * (posIn[1] + posOut[1]);   // mid-positions
    float v = 0.5 * (posIn[2] + posOut[2]);

    // Get resolutions from the cheater
    float du = m_cheater->getResolutionRPhi(layerID, ladderID, sensorID, aSimHit->getTheta());
    float dv = m_cheater->getResolutionZ(layerID, ladderID, sensorID, aSimHit->getTheta());
    float duv = 0; // covariance zero in detector coordinates

    // Smear
    u = u + m_random->Gaus(0, du);
    v = v + m_random->Gaus(0, dv);

    newHit->setU(u); newHit->setUError(du);
    newHit->setV(v); newHit->setVError(dv);
    newHit->setUVCov(duv);

    // Physics
    double eDep = aSimHit->getEnergyDep();
    double deDep = 0;
    newHit->setEnergyDep(eDep); newHit->setEnergyDepError(deDep);

    // get source MCParticles and save (atomic) relation(s) to mcHitArray
    pair<ToSideItr, ToSideItr> eqRange = simIndex.equal_range(iHit);
    for (ToSideItr mcSimHit = eqRange.first; mcSimHit != eqRange.second; ++mcSimHit) {
      new(mcHitArray->AddrAt(mcHitArray->GetLast() + 1))
      Relation(mcPartArray, svdHitArray, mcSimHit->m_from, iHit, mcSimHit->m_weight);
    }
  } // for iHit

  m_nEvent++;
}

void SVDDigiModule::endRun()
{
  m_nRun++;
}

void SVDDigiModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("SVDDigi finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");
}

void SVDDigiModule::printModuleParams() const
{
  B2INFO("SVDDigi parameters:")
  B2INFO("  MCParticles coll name:  " << m_mcColName)
  B2INFO("  Input collection name:  " << m_inColName)
  B2INFO("  MCPart->SimHits name:   " << m_relSimName)
  B2INFO("  Output collection name: " << m_outColName)
  B2INFO("  MCPart->SVDHits name:   " << m_relHitName)
}


