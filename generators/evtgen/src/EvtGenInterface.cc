/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <generators/utilities/cm2LabBoost.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/evtgen/EvtGenModelRegister.h>

#include <string>
#include <queue>
#include <utility>

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <EvtGenExternal/EvtExternalGenList.hh>
#include <EvtGenBase/EvtAbsRadCorr.hh>
#include <EvtGenBase/EvtDecayBase.hh>


#include <TLorentzVector.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

int EvtGenInterface::setup(const std::string& DECFileName, const std::string& pdlFileName, const std::string& parentParticle, const std::string& userFileName)
{
  B2INFO("Begin initialisation of EvtGen Interface.");

  EvtRandom::setRandomEngine((EvtRandomEngine*)&m_eng);

  // Official BelleII models
  std::list<EvtDecayBase*> extraModels = EvtGenModelRegister::getModels();

  //fill model list with pythia, photos etc.
  EvtExternalGenList genList;
  EvtAbsRadCorr* radCorrEngine = genList.getPhotosModel();
  list<EvtDecayBase*> modelList = genList.getListOfModels();
  extraModels.insert(extraModels.end(), modelList.begin(), modelList.end());

  // Method to add User EvtGen models here

  if (!m_Generator) {
    int mixingType = EvtCPUtil::Coherent;
    m_Generator = new EvtGen(DECFileName.c_str(), pdlFileName.c_str(), (EvtRandomEngine*)&m_eng, radCorrEngine, &extraModels, mixingType);
  }
  if (!userFileName.empty()) {
    m_Generator->readUDecay(userFileName.c_str());
  }

  // Setup Parent Particle in rest frame
  m_ParentParticle = EvtPDL::getId(parentParticle);

  B2INFO("End initialisation of EvtGen Interface.");

  return 0;
}


int EvtGenInterface::simulateEvent(MCParticleGraph& graph, TLorentzVector pParentParticle, int inclusiveType, const std::string& inclusiveParticle)
{
  //  B2INFO("Starting event simulation.");

  //pParentParticle.SetXYZM(0.0, 0.0, 0.0, EvtPDL::getMass(m_ParentParticle));

  // Boost to CMS frame
  pParentParticle = m_labboost.Inverse() * pParentParticle;

  if (m_ParentParticle.getId() != 93) {

    // Boost to CMS frame
    //pParentParticle = m_labboost.Inverse() * pParentParticle;
    m_pinit.set(pParentParticle.E(), pParentParticle.X(), pParentParticle.Y(), pParentParticle.Z());

    //B2INFO(pParentParticle.E() << "tt" << pParentParticle.X() << "tt" << pParentParticle.Y() << "tt" << pParentParticle.Z());

  } else {
    EvtId Ups = EvtPDL::getId("Upsilon(4S)");
    //m_pinit.set(EvtPDL::getMass(Ups), 0.0, 0.0, 0.0);
    m_pinit.set(EvtPDL::getMass(Ups), pParentParticle.X(), pParentParticle.Y(), pParentParticle.Z());
  }

  EvtId Inclusive_Particle_ID = EvtPDL::getId(inclusiveParticle);
  EvtId Inclusive_Anti_Particle_ID = EvtPDL::chargeConj(Inclusive_Particle_ID);

  bool we_got_inclusive_particle = false;
  do {
    m_parent = EvtParticleFactory::particleFactory(m_ParentParticle, m_pinit);
    m_parent->setVectorSpinDensity();
    //B2INFO("Set starting particle");
    m_Generator->generateDecay(m_parent);

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

  int iPart = addParticles2Graph(m_parent, graph);
  graph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  //  B2INFO("doing the graph thing");

  m_parent->deleteTree();

  //  B2INFO("finished event simulation");

  return iPart; //returns the number of generated particles from evtgen
}



int EvtGenInterface::addParticles2Graph(EvtParticle* top, MCParticleGraph& graph)
{
  //Fill top particle in the tree & starting the queue:
  int position = graph.size();
  int nParticles = 0;
  graph.addParticle(); nParticles++;
  MCParticleGraph::GraphParticle* p = &graph[position];
  updateGraphParticle(top, p);

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
    position = graph.size();
    graph.addParticle(); nParticles++;
    MCParticleGraph::GraphParticle* graphDaughter = &graph[position];
    updateGraphParticle(currDaughter, graphDaughter);
    position = graph.size();

    //add relation between mother and daughter to graph:
    currMother->decaysInto((*graphDaughter));

    int nGrandChildren = currDaughter->getNDaug();

    //    B2INFO(" mother of current daughter: "<<graphDaughter->getMother()->getMass());
    //B2INFO(" mother of current daughter: "<<currMother->getDaughters()[0]->getMass());

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

  return nParticles;
}


void EvtGenInterface::updateGraphParticle(EvtParticle* eParticle, MCParticleGraph::GraphParticle* gParticle)
{
  //updating the GraphParticle information from the EvtParticle information

  gParticle->setStatus(MCParticle::c_PrimaryParticle);
  gParticle->setMass(eParticle->mass());
  gParticle->setPDG(EvtPDL::getStdHep(eParticle->getId()));
  gParticle->setChargeFromPDG();

  EvtVector4R EvtP4 = eParticle->getP4Lab();
  TLorentzVector p4(EvtP4.get(1), EvtP4.get(2), EvtP4.get(3), EvtP4.get(0));
  gParticle->set4Vector(p4);

  EvtVector4R Evtpos = eParticle->get4Pos();
  //  B2INFO("position EVT: "<<EvtPDL::getStdHep(eParticle->getId())<<"  "<<Evtpos);
  gParticle->setProductionVertex(Evtpos.get(1)*Unit::mm, Evtpos.get(2)*Unit::mm, Evtpos.get(3)*Unit::mm);
  gParticle->setProductionTime(Evtpos.get(0)*Unit::mm / Const::speedOfLight);
  gParticle->setValidVertex(true);

  gParticle->setSpinType((int)eParticle->getSpinType());
}
