/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <framework/utilities/FileSystem.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/evtgen/EvtGenModelRegister.h>

#include <string>
#include <queue>

#include <EvtGenExternal/EvtExternalGenList.hh>
#include <EvtGenBase/EvtAbsRadCorr.hh>
#include <EvtGenBase/EvtDecayTable.hh>
#include <EvtGenBase/EvtDecayBase.hh>
#include <EvtGenBase/EvtPDL.hh>

#include <TLorentzVector.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

EvtGenFwRandEngine EvtGenInterface::m_eng;

EvtGenInterface::~EvtGenInterface()
{
  EvtDecayTable* evtDecayTable = EvtDecayTable::getInstance();
  for (unsigned int i = 0; i < EvtPDL::entries(); ++i) {
    for (int j = 0; j < evtDecayTable->getNMode(i); ++j) {
      delete evtDecayTable->getDecay(i, j);
    }
  }
  if (m_Generator) delete m_Generator;
}

EvtGen* EvtGenInterface::createEvtGen(const std::string& DECFileName)
{
  IOIntercept::OutputToLogMessages initLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Debug, 100, 100);
  initLogCapture.start();
  EvtRandom::setRandomEngine(&EvtGenInterface::m_eng);

  // Official BelleII models
  std::list<EvtDecayBase*> extraModels = EvtGenModelRegister::getModels();

  //fill model list with pythia, photos etc.
  EvtExternalGenList genList;
  EvtAbsRadCorr* radCorrEngine = genList.getPhotosModel();
  list<EvtDecayBase*> modelList = genList.getListOfModels();
  extraModels.insert(extraModels.end(), modelList.begin(), modelList.end());

  FileSystem::TemporaryFile tmp;
  EvtGenDatabasePDG::Instance()->WriteEvtGenTable(tmp);
  return new EvtGen(DECFileName.c_str(), tmp.getName().c_str(), &EvtGenInterface::m_eng,
                    radCorrEngine, &extraModels, EvtCPUtil::Coherent);

  initLogCapture.finish();
}

int EvtGenInterface::setup(const std::string& DECFileName, const std::string& parentParticle,
                           const std::string& userFileName)
{
  B2DEBUG(150, "Begin initialisation of EvtGen Interface.");

  //tauola prints normal things to stderr.. oh well.
  IOIntercept::OutputToLogMessages initLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Debug, 100, 100);
  initLogCapture.start();
  if (!m_Generator) {
    m_Generator = createEvtGen(DECFileName);
  }
  if (!userFileName.empty()) {
    m_Generator->readUDecay(userFileName.c_str());
  }

  // Setup Parent Particle in rest frame
  if (parentParticle != "") {
    m_ParentInitialized = true;
    m_ParentParticle = EvtPDL::getId(parentParticle);
  }
  initLogCapture.finish();

  B2DEBUG(150, "End initialisation of EvtGen Interface.");

  return 0;
}


int EvtGenInterface::simulateEvent(MCParticleGraph& graph, TLorentzVector pParentParticle, TVector3 pPrimaryVertex,
                                   int inclusiveType, const std::string& inclusiveParticle)
{
  if (!m_ParentInitialized)
    B2FATAL("Parent particle is not initialized.");
  //Init evtgen
  m_pinit.set(pParentParticle.E(), pParentParticle.X(), pParentParticle.Y(), pParentParticle.Z());

  EvtId Inclusive_Particle_ID = EvtPDL::getId(inclusiveParticle);
  EvtId Inclusive_Anti_Particle_ID = EvtPDL::chargeConj(Inclusive_Particle_ID);

  bool we_got_inclusive_particle = false;
  do {
    m_logCapture.start();
    m_parent = EvtParticleFactory::particleFactory(m_ParentParticle, m_pinit);
    m_parent->setVectorSpinDensity();
    m_Generator->generateDecay(m_parent);
    m_logCapture.finish();

    if (inclusiveType != 0) {
      EvtParticle* p = m_parent;
      // following loop will go through generated event and check it for
      // presense of inclusive particle
      do {
        //for (int ii = 0; ii < iPart ; ii++) {
        //std::cout << p->getPDGId() << std::endl;
        if (p->getId() == Inclusive_Particle_ID ||
            (inclusiveType == 2 && p->getId() == Inclusive_Anti_Particle_ID)) {
          we_got_inclusive_particle = true;
          break;
        }
        p = p->nextIter(m_parent);
      } while (p != 0);

      if (!we_got_inclusive_particle) {
        m_parent->deleteTree();
      }
    } else {
      // we don't do inclusive skimming so any generated event will do
      we_got_inclusive_particle = true;
    }
  } while (!we_got_inclusive_particle);

  //  B2INFO("after generate Decay.");

  int iPart = addParticles2Graph(m_parent, graph, pPrimaryVertex, NULL);
  graph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  m_parent->deleteTree();

  return iPart; //returns the number of generated particles from evtgen
}

int EvtGenInterface::simulateDecay(MCParticleGraph& graph,
                                   MCParticleGraph::GraphParticle& parent)
{
  int pdg;
  EvtId id;
  TLorentzVector momentum = parent.get4Vector();
  TVector3 vertex = parent.getVertex();
  m_pinit.set(momentum.E(), momentum.X(), momentum.Y(), momentum.Z());
  m_logCapture.start();
  pdg = parent.getPDG();
  id = EvtPDL::evtIdFromStdHep(pdg);
  m_parent = EvtParticleFactory::particleFactory(id, m_pinit);
  if (pdg == 10022) // Virtual photon
    m_parent->setVectorSpinDensity();
  else
    m_parent->setDiagonalSpinDensity();
  m_Generator->generateDecay(m_parent);
  m_logCapture.finish();
  int iPart = addParticles2Graph(m_parent, graph, vertex, &parent);
  m_parent->deleteTree();
  return iPart;
}

int EvtGenInterface::addParticles2Graph(EvtParticle* top, MCParticleGraph& graph, TVector3 pPrimaryVertex,
                                        MCParticleGraph::GraphParticle* parent)
{
  //Fill top particle in the tree & starting the queue:
  const int existingParticles = graph.size();
  MCParticleGraph::GraphParticle* p;
  if (parent == NULL)
    p = &graph.addParticle();
  else
    p = parent;
  updateGraphParticle(top, p, pPrimaryVertex);

  typedef pair<MCParticleGraph::GraphParticle*, EvtParticle*> halfFamily;
  halfFamily currFamily;
  queue < halfFamily > heritancesQueue;

  for (uint idaughter = 0; idaughter < top->getNDaug(); idaughter++) {
    currFamily.first = p;
    currFamily.second = top->getDaug(idaughter);
    heritancesQueue.push(currFamily);
  }

  //now we can go through the queue:
  while (!heritancesQueue.empty()) {
    currFamily = heritancesQueue.front(); //get the first entry from the queue
    heritancesQueue.pop(); //remove the entry.

    MCParticleGraph::GraphParticle* currMother = currFamily.first;
    EvtParticle* currDaughter = currFamily.second;

    //putting the daughter in the graph:
    MCParticleGraph::GraphParticle* graphDaughter = &graph.addParticle();
    updateGraphParticle(currDaughter, graphDaughter, pPrimaryVertex);

    //add relation between mother and daughter to graph:
    currMother->decaysInto((*graphDaughter));

    int nGrandChildren = currDaughter->getNDaug();

    if (nGrandChildren == 0)
      graphDaughter->addStatus(MCParticle::c_StableInGenerator);
    else {
      for (int igrandchild = 0; igrandchild < nGrandChildren; igrandchild++) {
        currFamily.first = graphDaughter;
        currFamily.second = currDaughter->getDaug(igrandchild);
        heritancesQueue.push(currFamily);
      }
    }
  }

  return graph.size() - existingParticles;
}


void EvtGenInterface::updateGraphParticle(EvtParticle* eParticle, MCParticleGraph::GraphParticle* gParticle,
                                          TVector3 pPrimaryVertex)
{
  //updating the GraphParticle information from the EvtParticle information

  gParticle->setStatus(MCParticle::c_PrimaryParticle);
  gParticle->setMass(eParticle->mass());
  gParticle->setPDG(EvtPDL::getStdHep(eParticle->getId()));

  EvtVector4R EvtP4 = eParticle->getP4Lab();
  TLorentzVector p4(EvtP4.get(1), EvtP4.get(2), EvtP4.get(3), EvtP4.get(0));
  gParticle->set4Vector(p4);

  EvtVector4R Evtpos = eParticle->get4Pos();

  TVector3 pVertex(Evtpos.get(1)*Unit::mm, Evtpos.get(2)*Unit::mm, Evtpos.get(3)*Unit::mm);
  pVertex = pVertex + pPrimaryVertex;

  gParticle->setProductionVertex(pVertex(0), pVertex(1), pVertex(2));
  gParticle->setProductionTime(Evtpos.get(0)*Unit::mm / Const::speedOfLight);
  gParticle->setValidVertex(true);

  //add PHOTOS flag
  if (eParticle->getAttribute("FSR")) {
    gParticle->addStatus(MCParticle::c_IsPHOTOSPhoton);
  }

}
