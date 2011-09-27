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
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <pxd/vxd/VxdID.h>
#include <boost/foreach.hpp>


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
  m_tree->Branch("PXDhitID", &m_PXDhitID, "PXDhitID[PXDhcount]/I");

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

  m_tree->Branch("PXDenergyDep", &m_PXDenergyDep, "PXDenergyDep[PXDhcount]/F");
  m_tree->Branch("PXDglobalTime", &m_PXDglobalTime, "PXDglobalTime[PXDhcount]/F");

  //tree branches for the SVD hits:
  m_tree->Branch("SVDhcount", &m_SVDhcount, "SVDhcount/I");
  m_tree->Branch("SVDhitID", &m_SVDhitID, "SVDhitID[SVDhcount]/I");

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

  m_tree->Branch("SVDenergyDep", &m_SVDenergyDep, "SVDenergyDep[SVDhcount]/F");
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
  StoreArray<MCParticle> MCParticles;

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

    typedef RelationIndex<PXDSimHit, MCParticle> indexPXDHitMC_t;
    indexPXDHitMC_t indexPXDHitMC ;
    BOOST_FOREACH(const indexPXDHitMC_t::Element &element, indexPXDHitMC.getTo(currParticle)) {
      const PXDSimHit * relatedHit = element.from ;
      m_PXDhitID[m_PXDhcount] = relatedHit->getSensorID().getID();
      m_PXDtheta[m_PXDhcount] = relatedHit->getTheta();
      m_PXDenergyDep[m_PXDhcount] = relatedHit->getEnergyDep();
      m_PXDglobalTime[m_PXDhcount] = relatedHit->getGlobalTime();

      TVector3 pxdvect = relatedHit->getPosIn();
      m_PXDposInX[m_PXDhcount] = pxdvect.X();
      m_PXDposInY[m_PXDhcount] = pxdvect.Y();
      m_PXDposInZ[m_PXDhcount] = pxdvect.Z();
      pxdvect = relatedHit->getPosOut();
      m_PXDposOutX[m_PXDhcount] = pxdvect.X();
      m_PXDposOutY[m_PXDhcount] = pxdvect.Y();
      m_PXDposOutZ[m_PXDhcount] = pxdvect.Z();
      pxdvect = relatedHit->getMomIn();
      m_PXDmomInX[m_PXDhcount] = pxdvect.X();
      m_PXDmomInY[m_PXDhcount] = pxdvect.Y();
      m_PXDmomInZ[m_PXDhcount] = pxdvect.Z();

      m_PXDhcount++;
    }//end of BOOST_FOREACH


    typedef RelationIndex<SVDSimHit, MCParticle> indexSVDHitMC_t;
    indexSVDHitMC_t indexSVDHitMC ;
    BOOST_FOREACH(const indexSVDHitMC_t::Element &element, indexSVDHitMC.getTo(currParticle)) {
      const SVDSimHit * relatedHit = element.from ;
      m_SVDhitID[m_SVDhcount] = relatedHit->getSensorID().getID();
      m_SVDtheta[m_SVDhcount] = relatedHit->getTheta();
      m_SVDenergyDep[m_SVDhcount] = relatedHit->getEnergyDep();
      m_SVDglobalTime[m_SVDhcount] = relatedHit->getGlobalTime();

      TVector3 svdvect = relatedHit->getPosIn();
      m_SVDposInX[m_SVDhcount] = svdvect.X();
      m_SVDposInY[m_SVDhcount] = svdvect.Y();
      m_SVDposInZ[m_SVDhcount] = svdvect.Z();
      svdvect = relatedHit->getPosOut();
      m_SVDposOutX[m_SVDhcount] = svdvect.X();
      m_SVDposOutY[m_SVDhcount] = svdvect.Y();
      m_SVDposOutZ[m_SVDhcount] = svdvect.Z();
      svdvect = relatedHit->getMomIn();
      m_SVDmomInX[m_SVDhcount] = svdvect.X();
      m_SVDmomInY[m_SVDhcount] = svdvect.Y();
      m_SVDmomInZ[m_SVDhcount] = svdvect.Z();

      m_SVDhcount++;
    }//end of BOOST_FOREACH

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
