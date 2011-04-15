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
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <generators/dataobjects/MCParticle.h>
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

}


void makeSimulationTreeModule::initialize()
{
  B2INFO("initialising makeSimulationTreeModule");
  m_tree = new TTree("SIMTREE", "SIMTREE");

  //declare a tree branch for all data members of MCParticle.
  m_tree->Branch("exp", &m_expNum, "exp/I");
  m_tree->Branch("evt", &m_evtNum, "evt/I");
  m_tree->Branch("run", &m_runNum, "run/I");
  m_tree->Branch("index", &m_MCPindex, "index/I");
  m_tree->Branch("status", &m_MCPstatus, "status/I");
  m_tree->Branch("mass", &m_MCPmass, "mass/F");
  m_tree->Branch("charge", &m_MCPcharge, "charge/F");
  m_tree->Branch("energy", &m_MCPenergy, "energy/F");

  m_tree->Branch("pX", &m_MCPpX, "px/F");
  m_tree->Branch("pY", &m_MCPpY, "py/F");
  m_tree->Branch("pZ", &m_MCPpZ, "pz/F");
  m_tree->Branch("pTot", &m_MCPpTot, "pTot/F");
  m_tree->Branch("pT", &m_MCPpT, "pT/F");
  m_tree->Branch("theta", &m_MCPtheta, "theta/F");
  m_tree->Branch("phi", &m_MCPphi, "phi/F");

  m_tree->Branch("mother", &m_MCPmother, "mother/I");
  m_tree->Branch("firstDaughter", &m_MCPfirstDaughter, "firstDaughter/I");
  m_tree->Branch("lastDaughter", &m_MCPlastDaughter, "lastDaughter/I");
  m_tree->Branch("PIDcode", &m_MCPpdg, "PIDcode/I");

  m_tree->Branch("validVertex", &m_MCPvalidVertex, "validVertex/O");
  m_tree->Branch("productionVertexX", &m_MCPproductionVertexX, "productionVertexX/F");
  m_tree->Branch("productionVertexY", &m_MCPproductionVertexY, "productionVertexY/F");
  m_tree->Branch("productionVertexZ", &m_MCPproductionVertexZ, "productionVertexZ/F");
  m_tree->Branch("productionTime", &m_MCPproductionTime, "productionTime/F");

  m_tree->Branch("decayVertexX", &m_MCPdecayVertexX, "decayVertexX/F");
  m_tree->Branch("decayVertexY", &m_MCPdecayVertexY, "decayVertexY/F");
  m_tree->Branch("decayVertexZ", &m_MCPdecayVertexZ, "decayVertexZ/F");
  m_tree->Branch("decayTime", &m_MCPdecayTime, "decayTime/F");


//tree branches for the PXD hits:
  m_tree->Branch("PXDhcount", &m_PXDhcount, "PXDhcount/I");
  m_tree->Branch("PXDlayerID", &m_PXDlayerID, "PXDlayerID[PXDhcount]/I");
  m_tree->Branch("PXDladderID", &m_PXDladderID, "PXDladderID[PXDhcount]/I");
  m_tree->Branch("PXDsensorID", &m_PXDsensorID, "PXDsensorID[PXDhcount]/I");

  m_tree->Branch("PXDposInX", &m_PXDposInX, "PXDposInX[PXDhcount]/F");
  m_tree->Branch("PXDposInY", &m_PXDposInY, "PXDposInY[PXDhcount]/F");
  m_tree->Branch("PXDposInZ", &m_PXDposInZ, "PXDposInZ[PXDhcount]/F");

  m_tree->Branch("PXDposOutX", &m_PXDposOutX, "PXDposOutX[PXDhcount]/F");
  m_tree->Branch("PXDposOutY", &m_PXDposOutY, "PXDposOutY[PXDhcount]/F");
  m_tree->Branch("PXDposOutZ", &m_PXDposOutZ, "PXDposOutZ[PXDhcount]/F");

  m_tree->Branch("PXDtheta", &m_PXDtheta, "PXDtheta[PXDhcount]/F");
  m_tree->Branch("PXDmomInX", &m_PXDmomInX, "PXDmomInX[PXDhcount]/F");
  m_tree->Branch("PXDmomInY", &m_PXDmomInY, "PXDmomInY[PXDhcount]/F");
  m_tree->Branch("PXDmomInZ", &m_PXDmomInZ, "PXDmomInZ[PXDhcount]/F");

  m_tree->Branch("PXDtrackID", &m_PXDtrackID, "PXDtrackID[PXDhcount]/I");
  m_tree->Branch("PXDPDGcode", &m_PXDPDGcode, "PXDPDGcode[PXDhcount]/I");

  m_tree->Branch("PXDenergyDep", &m_PXDenergyDep, "PXDenergyDep[PXDhcount]/F");
  m_tree->Branch("PXDstepLength", &m_PXDstepLength, "PXDstepLength[PXDhcount]/F");
  m_tree->Branch("PXDglobalTime", &m_PXDglobalTime, "PXDglobalTime[PXDhcount]/F");

  //tree branches for the SVD hits:
  m_tree->Branch("SVDhcount", &m_SVDhcount, "SVDhcount/I");
  m_tree->Branch("SVDlayerID", &m_SVDlayerID, "SVDlayerID[SVDhcount]/I");
  m_tree->Branch("SVDladderID", &m_SVDladderID, "SVDladderID[SVDhcount]/I");
  m_tree->Branch("SVDsensorID", &m_SVDsensorID, "SVDsensorID[SVDhcount]/I");

  m_tree->Branch("SVDposInX", &m_SVDposInX, "SVDposInX[SVDhcount]/F");
  m_tree->Branch("SVDposInY", &m_SVDposInY, "SVDposInY[SVDhcount]/F");
  m_tree->Branch("SVDposInZ", &m_SVDposInZ, "SVDposInZ[SVDhcount]/F");

  m_tree->Branch("SVDposOutX", &m_SVDposOutX, "SVDposOutX[SVDhcount]/F");
  m_tree->Branch("SVDposOutY", &m_SVDposOutY, "SVDposOutY[SVDhcount]/F");
  m_tree->Branch("SVDposOutZ", &m_SVDposOutZ, "SVDposOutZ[SVDhcount]/F");

  m_tree->Branch("SVDtheta", &m_SVDtheta, "SVDtheta[SVDhcount]/F");
  m_tree->Branch("SVDmomInX", &m_SVDmomInX, "SVDmomInX[SVDhcount]/F");
  m_tree->Branch("SVDmomInY", &m_SVDmomInY, "SVDmomInY[SVDhcount]/F");
  m_tree->Branch("SVDmomInZ", &m_SVDmomInZ, "SVDmomInZ[SVDhcount]/F");

  m_tree->Branch("SVDtrackID", &m_SVDtrackID, "SVDtrackID[SVDhcount]/I");
  m_tree->Branch("SVDPDGcode", &m_SVDPDGcode, "SVDPDGcode[SVDhcount]/I");

  m_tree->Branch("SVDenergyDep", &m_SVDenergyDep, "SVDenergyDep[SVDhcount]/F");
  m_tree->Branch("SVDstepLength", &m_SVDstepLength, "SVDstepLength[SVDhcount]/F");
  m_tree->Branch("SVDglobalTime", &m_SVDglobalTime, "SVDglobalTime[SVDhcount]/F");

}


void makeSimulationTreeModule::event()
{
  //Printout statement to indicate we are in the event function
  //  B2INFO("makeSimulationTreeModule: in event");


  //get run, experiment and event number:
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_expNum = eventMetaDataPtr->getExperiment();
  m_runNum = eventMetaDataPtr->getRun();
  m_evtNum = eventMetaDataPtr->getEvent();

  //get collection of MCParticles from the data store:
  StoreArray<MCParticle> MCParticles("MCParticles");

  //get collection of relations between PXDSimHits and MCParticles
  StoreArray <Relation> PXDrelationArray("MCParticlesToPXDSimHits");

  //StoreArray <PXDSimHit> PXDSimHitArray (PXDrelationArray->getToAccessorInfo());
  StoreArray <PXDSimHit> PXDSimHitArray("PXDSimHits");

  //get collection of relations between SVDSimHits and MCParticles
  StoreArray <Relation> SVDrelationArray("MCParticlesToSVDSimHits");

  //StoreArray <SVDSimHit> SVDSimHitArray (SVDrelationArray->getToAccessorInfo());
  StoreArray <SVDSimHit> SVDSimHitArray("SVDSimHits");

  //Loop over the primary particles. The MCParticle collection was
  //sorted: primary particles come first and then the daughters
  for (int iPart = 0; iPart < MCParticles.GetEntries(); iPart++) {

    //this is an example of how we can get a printout statement
    //B2INFO("makeSimulationTreeModule: in particle loop: "<<iPart);

    //get the current particle
    MCParticle *currParticle = MCParticles[iPart];

    //get member variables:
    m_MCPindex = currParticle->getArrayIndex();
    m_MCPstatus = currParticle->getStatus();
    m_MCPmass = currParticle->getMass();
    m_MCPcharge = currParticle->getCharge();
    m_MCPenergy = currParticle->getEnergy();
    TVector3 vector = currParticle->getMomentum();
    m_MCPpX = vector.X();
    m_MCPpY = vector.Y();
    m_MCPpZ = vector.Z();
    m_MCPpTot = vector.Mag();
    m_MCPpT = vector.Perp();
    m_MCPtheta = vector.Theta();
    m_MCPphi = vector.Phi();


    if (currParticle->getMother())
      m_MCPmother = currParticle->getMother()->getArrayIndex();
    else m_MCPmother = -1;

    m_MCPfirstDaughter = currParticle->getFirstDaughter() - 1;
    m_MCPlastDaughter = currParticle->getLastDaughter() - 1;
    m_MCPpdg = currParticle->getPDG();

    m_MCPvalidVertex = currParticle->hasValidVertex();
    vector = currParticle->getProductionVertex();
    m_MCPproductionVertexX = vector.X();
    m_MCPproductionVertexY = vector.Y();
    m_MCPproductionVertexZ = vector.Z();
    m_MCPproductionTime = currParticle->getProductionTime();

    vector = currParticle->getDecayVertex();
    m_MCPdecayVertexX = vector.X();
    m_MCPdecayVertexY = vector.Y();
    m_MCPdecayVertexZ = vector.Z();
    m_MCPdecayTime = currParticle->getDecayTime();


    //now lets get the PXD hits:
    m_PXDhcount = 0;//counting the hits for each track
    for (long int jj = 0; jj < PXDrelationArray->GetEntriesFast(); jj++) { // Looping PXD relations
      if (PXDrelationArray[jj]->getFromIndex() == iPart) {//PXD if statement
        if (m_PXDhcount > 99) {
          B2INFO("not storing a PXD entry. Track has more than 100 PXD hits.");
        } else {//storing the entry
          if (!PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]) { // Problem with to-index
            continue;
          }

          m_PXDlayerID[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getLayerID();
          m_PXDladderID[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getLadderID();
          m_PXDsensorID[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getSensorID();
          m_PXDtheta[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getTheta();
          m_PXDPDGcode[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getPDGcode();
          m_PXDtrackID[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getTrackID();
          m_PXDenergyDep[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getEnergyDep();
          m_PXDstepLength[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getStepLength();
          m_PXDglobalTime[m_PXDhcount] = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getGlobalTime();


          TVector3 pxdvect = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getPosIn();
          m_PXDposInX[m_PXDhcount] = pxdvect.X();
          m_PXDposInY[m_PXDhcount] = pxdvect.Y();
          m_PXDposInZ[m_PXDhcount] = pxdvect.Z();
          pxdvect = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getPosOut();
          m_PXDposOutX[m_PXDhcount] = pxdvect.X();
          m_PXDposOutY[m_PXDhcount] = pxdvect.Y();
          m_PXDposOutZ[m_PXDhcount] = pxdvect.Z();
          pxdvect = PXDSimHitArray[PXDrelationArray[jj]->getToIndex()]->getMomIn();
          m_PXDmomInX[m_PXDhcount] = pxdvect.X();
          m_PXDmomInY[m_PXDhcount] = pxdvect.Y();
          m_PXDmomInZ[m_PXDhcount] = pxdvect.Z();

          m_PXDhcount++;
        }//end of else statement
      }//end PXD if statement
    }//end PXD relation loop

    //now lets get the SVD hits:
    m_SVDhcount = 0;//counting the hits for each track
    for (int jj = 0; jj < SVDrelationArray->GetEntriesFast(); jj++) { // Looping SVD relations
      if (SVDrelationArray[jj]->getFromIndex() == iPart) {//SVD if statment
        if (m_SVDhcount > 99) {
          B2INFO("not storing SVD entry. Track has more than 100 SVD hits!");
        } else {//storing the entry
          if (!SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]) {// something is wrong with the hit index
            continue;
          }

          m_SVDlayerID[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getLayerID();
          m_SVDladderID[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getLadderID();
          m_SVDsensorID[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getSensorID();
          m_SVDtheta[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getTheta();
          m_SVDPDGcode[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getPDGcode();
          m_SVDtrackID[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getTrackID();
          m_SVDenergyDep[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getEnergyDep();
          m_SVDstepLength[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getStepLength();
          m_SVDglobalTime[m_SVDhcount] = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getGlobalTime();

          TVector3 pxdvect = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getPosIn();
          m_SVDposInX[m_SVDhcount] = pxdvect.X();
          m_SVDposInY[m_SVDhcount] = pxdvect.Y();
          m_SVDposInZ[m_SVDhcount] = pxdvect.Z();
          pxdvect = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getPosOut();
          m_SVDposOutX[m_SVDhcount] = pxdvect.X();
          m_SVDposOutY[m_SVDhcount] = pxdvect.Y();
          m_SVDposOutZ[m_SVDhcount] = pxdvect.Z();
          pxdvect = SVDSimHitArray[SVDrelationArray[jj]->getToIndex()]->getMomIn();
          m_SVDmomInX[m_SVDhcount] = pxdvect.X();
          m_SVDmomInY[m_SVDhcount] = pxdvect.Y();
          m_SVDmomInZ[m_SVDhcount] = pxdvect.Z();

          m_SVDhcount++;
        }//end of else statement
      }//end SVD if statement
    }//end SVDhit relations

    //store  all infos for this particle in the root tree
    // if there is a PXD/SVD hit for the particle:
    if ((m_PXDhcount + m_SVDhcount) > 0)
      m_tree->Fill();
  }


}


void makeSimulationTreeModule::terminate()
{
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "SimulationTreeFile", 1);
  m_file->cd();
  m_tree->Write();
  m_file->Close();

//   delete m_file;
//   delete m_tree;
}
