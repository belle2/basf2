/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 **************************************************************************/

#include <examples/modules/makeSimulationTreeModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/Relation.h>
#include <framework/gearbox/Unit.h>

#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(makeSimulationTree)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

makeSimulationTreeModule::makeSimulationTreeModule() : Module()
{
  //Set module properties
  setDescription("creates a root tree with information from simulation.");

  //Parameter definition
  addParam("outputFileName", m_outputFileName, "Output rootfile name", string("simulationTree.root"));
  addParam("InputMCParticleCollection", m_mcParticleInputColName, "The name of the input MCParticle collection.", string(DEFAULT_MCPARTICLES));
}


void makeSimulationTreeModule::initialize()
{
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "SimulationTreeFile", 1);
  m_file->cd();
  m_tree = new TTree("SIMTREE", "SIMTREE");

  //declare a tree branch for all data members of MCParticle.
  m_tree->Branch("exp", &m_expNum, "exp/I");
  m_tree->Branch("evt", &m_evtNum, "evt/I");
  m_tree->Branch("run", &m_runNum, "run/I");
  m_tree->Branch("index", &m_MCP_index, "index/I");
  m_tree->Branch("status", &m_MCP_status, "status/I");
  m_tree->Branch("mass", &m_MCP_mass, "mass/F");
  m_tree->Branch("energy", &m_MCP_energy, "energy/F");

  m_tree->Branch("px", &m_MCP_px, "px/F");
  m_tree->Branch("py", &m_MCP_py, "py/F");
  m_tree->Branch("pz", &m_MCP_pz, "pz/F");

  m_tree->Branch("mother", &m_MCP_mother, "mother/I");
  m_tree->Branch("firstdaughter", &m_MCP_firstdaughter, "firstdaughter/I");
  m_tree->Branch("lastdaughter", &m_MCP_lastdaughter, "lastdaughter/I");
  m_tree->Branch("PIDcode", &m_MCP_pdg, "PIDcode/I");

  m_tree->Branch("validVertex", &m_MCP_validVertex, "validVertex/O");
  m_tree->Branch("productionVertex_x", &m_MCP_productionVertex_x, "productionVertex_x/F");
  m_tree->Branch("productionVertex_y", &m_MCP_productionVertex_y, "productionVertex_y/F");
  m_tree->Branch("productionVertex_z", &m_MCP_productionVertex_z, "productionVertex_z/F");
  m_tree->Branch("productionTime", &m_MCP_productionTime, "productionTime/F");

  m_tree->Branch("decayVertex_x", &m_MCP_decayVertex_x, "decayVertex_x/F");
  m_tree->Branch("decayVertex_y", &m_MCP_decayVertex_y, "decayVertex_y/F");
  m_tree->Branch("decayVertex_z", &m_MCP_decayVertex_z, "decayVertex_z/F");
  m_tree->Branch("decayTime", &m_MCP_decayTime, "decayTime/F");


  //tree branches for the PXD hits:
  m_tree->Branch("PXD_hcount", &m_PXD_hcount, "PXD_hcount/I");
  m_tree->Branch("PXD_layerID", &m_PXD_layerID, "PXD_layerID[PXD_hcount]/I");
  m_tree->Branch("PXD_ladderID", &m_PXD_ladderID, "PXD_ladderID[PXD_hcount]/I");
  m_tree->Branch("PXD_sensorID", &m_PXD_sensorID, "PXD_sensorID[PXD_hcount]/I");

  m_tree->Branch("PXD_posInX", &m_PXD_posInX, "PXD_posInX[PXD_hcount]/F");
  m_tree->Branch("PXD_posInY", &m_PXD_posInY, "PXD_posInY[PXD_hcount]/F");
  m_tree->Branch("PXD_posInZ", &m_PXD_posInZ, "PXD_posInZ[PXD_hcount]/F");

  m_tree->Branch("PXD_posOutX", &m_PXD_posOutX, "PXD_posOutX[PXD_hcount]/F");
  m_tree->Branch("PXD_posOutY", &m_PXD_posOutY, "PXD_posOutY[PXD_hcount]/F");
  m_tree->Branch("PXD_posOutZ", &m_PXD_posOutZ, "PXD_posOutZ[PXD_hcount]/F");

  m_tree->Branch("PXD_theta", &m_PXD_theta, "PXD_theta[PXD_hcount]/F");
  m_tree->Branch("PXD_momInX", &m_PXD_momInX, "PXD_momInX[PXD_hcount]/F");
  m_tree->Branch("PXD_momInY", &m_PXD_momInY, "PXD_momInY[PXD_hcount]/F");
  m_tree->Branch("PXD_momInZ", &m_PXD_momInZ, "PXD_momInZ[PXD_hcount]/F");

  m_tree->Branch("PXD_trackID", &m_PXD_trackID, "PXD_trackID[PXD_hcount]/I");
  m_tree->Branch("PXD_PDGcode", &m_PXD_PDGcode, "PXD_PDGcode[PXD_hcount]/I");

  m_tree->Branch("PXD_energyDep", &m_PXD_energyDep, "PXD_energyDep[PXD_hcount]/F");
  m_tree->Branch("PXD_stepLength", &m_PXD_stepLength, "PXD_stepLength[PXD_hcount]/F");
  m_tree->Branch("PXD_globalTime", &m_PXD_globalTime, "PXD_globalTime[PXD_hcount]/F");


  //tree branches for the SVD hits:
  m_tree->Branch("SVD_hcount", &m_SVD_hcount, "SVD_hcount/I");
  m_tree->Branch("SVD_layerID", &m_SVD_layerID, "SVD_layerID[SVD_hcount]/I");
  m_tree->Branch("SVD_ladderID", &m_SVD_ladderID, "SVD_ladderID[SVD_hcount]/I");
  m_tree->Branch("SVD_sensorID", &m_SVD_sensorID, "SVD_sensorID[SVD_hcount]/I");

  m_tree->Branch("SVD_posInX", &m_SVD_posInX, "SVD_posInX[SVD_hcount]/F");
  m_tree->Branch("SVD_posInY", &m_SVD_posInY, "SVD_posInY[SVD_hcount]/F");
  m_tree->Branch("SVD_posInZ", &m_SVD_posInZ, "SVD_posInZ[SVD_hcount]/F");

  m_tree->Branch("SVD_posOutX", &m_SVD_posOutX, "SVD_posOutX[SVD_hcount]/F");
  m_tree->Branch("SVD_posOutY", &m_SVD_posOutY, "SVD_posOutY[SVD_hcount]/F");
  m_tree->Branch("SVD_posOutZ", &m_SVD_posOutZ, "SVD_posOutZ[SVD_hcount]/F");

  m_tree->Branch("SVD_theta", &m_SVD_theta, "SVD_theta[SVD_hcount]/F");
  m_tree->Branch("SVD_momInX", &m_SVD_momInX, "SVD_momInX[SVD_hcount]/F");
  m_tree->Branch("SVD_momInY", &m_SVD_momInY, "SVD_momInY[SVD_hcount]/F");
  m_tree->Branch("SVD_momInZ", &m_SVD_momInZ, "SVD_momInZ[SVD_hcount]/F");

  m_tree->Branch("SVD_trackID", &m_SVD_trackID, "SVD_trackID[SVD_hcount]/I");
  m_tree->Branch("SVD_PDGcode", &m_SVD_PDGcode, "SVD_PDGcode[SVD_hcount]/I");

  m_tree->Branch("SVD_energyDep", &m_SVD_energyDep, "SVD_energyDep[SVD_hcount]/F");
  m_tree->Branch("SVD_stepLength", &m_SVD_stepLength, "SVD_stepLength[SVD_hcount]/F");
  m_tree->Branch("SVD_globalTime", &m_SVD_globalTime, "SVD_globalTime[SVD_hcount]/F");

}


void makeSimulationTreeModule::event()
{
  //get run, experiment and event number:
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_expNum = eventMetaDataPtr->getExperiment();
  m_runNum = eventMetaDataPtr->getRun();
  m_evtNum = eventMetaDataPtr->getEvent();

  //get collection of MCParticles from the data store:
  StoreArray<MCParticle> MCParticles(DEFAULT_MCPARTICLES);

  //get collection of relations and PXDSimHits and SVDSimHits
  StoreArray <Relation> PXDrelationArray("MCParticleToPXDSimHits");  // Relations zwischen MCParticles und PXDSimHits
//   StoreArray <PXDSimHit> PXDSimHitArray (PXDrelationArray->getToAccessorInfo()); // das PXDSimHitArray
  StoreArray <PXDSimHit> PXDSimHitArray("PXDSimHits");
  StoreArray <Relation> SVDrelationArray("MCParticleToSVDSimHits");  // Relations zwischen MCParticles und SVDSimHits
//   StoreArray <SVDSimHit> SVDSimHitArray (SVDrelationArray->getToAccessorInfo()); // das SVDSimHitArray
  StoreArray <SVDSimHit> SVDSimHitArray("SVDSimHits");

  //Loop over the primary particles. The MCParticle collection has to be
  //sorted breadth first: primary particles come first and then the daughters
  for (int iPart = 0; iPart < MCParticles.GetEntries(); iPart++) {

    //get the current particle
    MCParticle *currParticle = MCParticles[iPart];

    //get member variables:
    m_MCP_index = currParticle->getIndex();
    m_MCP_status = currParticle->getStatus();
    m_MCP_mass = currParticle->getMass();
    m_MCP_energy = currParticle->getEnergy();
    TVector3 vector = currParticle->getMomentum();
    m_MCP_px = vector.X();
    m_MCP_py = vector.Y();
    m_MCP_pz = vector.Z();

    m_MCP_mother = currParticle->getMother(1);
    m_MCP_firstdaughter = currParticle->getFirstDaughter() - 1;
    m_MCP_lastdaughter = currParticle->getLastDaughter() - 1;
    m_MCP_pdg = currParticle->getPDG();

    m_MCP_validVertex = currParticle->hasValidVertex();
    vector = currParticle->getProductionVertex();
    m_MCP_productionVertex_x = vector.X();
    m_MCP_productionVertex_y = vector.Y();
    m_MCP_productionVertex_z = vector.Z();
    m_MCP_productionTime = currParticle->getProductionTime();

    vector = currParticle->getDecayVertex();
    m_MCP_decayVertex_x = vector.X(); m_MCP_decayVertex_z = vector.Y();
    m_MCP_decayVertex_z = vector.Z();
    m_MCP_decayTime = currParticle->getDecayTime();

    //now lets get the PXD hits:
    m_PXD_hcount = 0;//counting the hits for each track
    for (int jj = 0; jj < PXDrelationArray->GetEntriesFast(); jj++) { // Loop ueber relations
      if (PXDrelationArray[jj]->getFromIndex() == iPart && m_PXD_hcount < 10) {// das bedeutet, die Relation zeigt auf das ii. MCParticle
        m_PXD_layerID[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getLayerID();
        m_PXD_ladderID[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getLayerID();
        m_PXD_sensorID[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getLayerID();
        m_PXD_theta[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getTheta();
        m_PXD_PDGcode[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getPDGcode();
        m_PXD_trackID[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getTrackID();
        m_PXD_energyDep[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getEnergyDep();
        m_PXD_stepLength[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getStepLength();
        m_PXD_globalTime[m_PXD_hcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getGlobalTime();


        TVector3 pxdvect = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getPosIn();
        m_PXD_posInX[m_PXD_hcount] = pxdvect.X();
        m_PXD_posInY[m_PXD_hcount] = pxdvect.Y();
        m_PXD_posInZ[m_PXD_hcount] = pxdvect.Z();
        pxdvect = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getPosOut();
        m_PXD_posOutX[m_PXD_hcount] = pxdvect.X();
        m_PXD_posOutY[m_PXD_hcount] = pxdvect.Y();
        m_PXD_posOutZ[m_PXD_hcount] = pxdvect.Z();
        pxdvect = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getMomIn();
        m_PXD_momInX[m_PXD_hcount] = pxdvect.X();
        m_PXD_momInY[m_PXD_hcount] = pxdvect.Y();
        m_PXD_momInZ[m_PXD_hcount] = pxdvect.Z();

        m_PXD_hcount++;
      }//end PXD if statement
    }//end PXD relation loop

    //now lets get the SVD hits:
    int m_SVD_hcount = 0;//counting the hits for each track
    for (int jj = 0; jj < SVDrelationArray->GetEntriesFast(); jj++) { // Loop ueber relations
      if (SVDrelationArray[jj]->getFromIndex() == iPart && m_SVD_hcount < 20) {// das bedeutet, die Relation zeigt auf das ii. MCParticle
        m_SVD_layerID[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getLayerID();
        m_SVD_ladderID[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getLayerID();
        m_SVD_sensorID[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getLayerID();
        m_SVD_theta[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getTheta();
        m_SVD_PDGcode[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getPDGcode();
        m_SVD_trackID[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getTrackID();
        m_SVD_energyDep[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getEnergyDep();
        m_SVD_stepLength[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getStepLength();
        m_SVD_globalTime[m_SVD_hcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getGlobalTime();


        TVector3 pxdvect = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getPosIn();
        m_SVD_posInX[m_SVD_hcount] = pxdvect.X();
        m_SVD_posInY[m_SVD_hcount] = pxdvect.Y();
        m_SVD_posInZ[m_SVD_hcount] = pxdvect.Z();
        pxdvect = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getPosOut();
        m_SVD_posOutX[m_SVD_hcount] = pxdvect.X();
        m_SVD_posOutY[m_SVD_hcount] = pxdvect.Y();
        m_SVD_posOutZ[m_SVD_hcount] = pxdvect.Z();
        pxdvect = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getMomIn();
        m_SVD_momInX[m_SVD_hcount] = pxdvect.X();
        m_SVD_momInY[m_SVD_hcount] = pxdvect.Y();
        m_SVD_momInZ[m_SVD_hcount] = pxdvect.Z();

        m_SVD_hcount++;
      }//end SVD if statement
    }//end SVDhit relations

    //store  all infos for this particle in the root tree
    m_tree->Fill();
  }


}


void makeSimulationTreeModule::terminate()
{
  m_file->cd();
  m_tree->Write();
  m_file->Close();

//   delete m_file;
//   delete m_tree;
}
