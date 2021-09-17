/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/fragmentation/FragmentationModule.h>

#include <generators/evtgen/EvtGenInterface.h>
#include <generators/utilities/GeneratorConst.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <framework/utilities/FileSystem.h>

#include <TRandom3.h>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/IOIntercept.h>

#include <string>

using namespace std;
using namespace Belle2;
using namespace Pythia8;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Fragmentation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
FragmentationModule::FragmentationModule() : Module()
{
  //Set module properties
  setDescription("Fragmention of (u/d/s/c) quarks using PYTHIA8");

  //Parameter definition
  addParam("ParameterFile", m_parameterfile, "Input parameter file for PYTHIA",
           FileSystem::findFile("generators/modules/fragmentation/data/pythia_belle2.dat"));
  addParam("ListPYTHIAEvent", m_listEvent, "List event record of PYTHIA after hadronization", 0);
  addParam("UseEvtGen", m_useEvtGen, "Use EvtGen for specific decays", 1);
  addParam("DecFile", m_DecFile, "EvtGen decay file (DECAY.DEC)",
           FileSystem::findFile("decfiles/dec/DECAY_BELLE2.DEC", true));
  addParam("UserDecFile", m_UserDecFile, "User EvtGen decay file", std::string(""));
  addParam("CoherentMixing", m_coherentMixing, "Decay the B0-B0bar coherently (should always be true)", true);

  //initialize member variables
  evtgen  = 0;
  nAdded  = 0;
  nQuarks = 0;
  nVpho   = 0;
  nAll    = 0;
  nGood   = 0;
}


FragmentationModule::~FragmentationModule()
{

}

void FragmentationModule::terminate()
{

  // print internal pythia error statistics
  IOIntercept::OutputToLogMessages statLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Error, 50, 100);
  statLogCapture.start();
  m_Pythia->stat();
  statLogCapture.finish();

  if (nAll != nGood) {
    double ratio = 0.; //ratio of good over all events
    if (nAll) ratio = 100.0 * nGood / nAll;

    B2WARNING("Not all events could be fragmented: " << nAll - nGood << " events failed.");
    B2WARNING("Total number of events: " << nAll << ", of these fragmented: " << nGood << ", success-ratio (should be >97%): " << ratio
              << "%");
    B2WARNING("Please contact the generator librarian if the success ratio is below 97%.");
    B2WARNING("Please treat the success-ratio as correction of the effective cross section due to unphysical events.");
  }
}

//-----------------------------------------------------------------
//                 Initialize
//-----------------------------------------------------------------
void FragmentationModule::initialize()
{
  m_mcparticles.isRequired(m_particleList);

  B2DEBUG(150, "Initialize PYTHIA8");

  // Generator and the shorthand m_PythiaEvent = pythia->event are declared in .h file
  // A simple way to collect all the changes is to store the parameter values in a separate file,
  // with one line per change. This should be done between the creation of the Pythia object
  // and the init call for it.

  IOIntercept::OutputToLogMessages initLogCapture("PYTHIA", LogConfig::c_Debug, LogConfig::c_Warning, 100, 100);
  initLogCapture.start();

  m_Pythia = new Pythia;
  m_PythiaEvent = &m_Pythia->event;
  (*m_PythiaEvent) = 0;

  // Load EvtGen particle data.
  EvtGen* evtGen = EvtGenInterface::createEvtGen(m_DecFile, m_coherentMixing);
  loadEvtGenParticleData(m_Pythia);

  // Switch off ProcessLevel
  m_Pythia->readString("ProcessLevel:all = off");

  // Read the PYTHIA input file, overrides parameters
  if (!m_Pythia->readFile(m_parameterfile))
    B2FATAL("Cannot read Pythia parameter file.");

  // Set framework generator
  FragmentationRndm* fragRndm = new FragmentationRndm();
  m_Pythia->setRndmEnginePtr(fragRndm);

  // Initialize PYTHIA
  m_Pythia->init();

  // Set EvtGen (after m_Pythia->init())
  evtgen = 0;

  if (m_useEvtGen) {
    B2INFO("Using PYTHIA EvtGen Interface");
    const std::string defaultDecFile = FileSystem::findFile("decfiles/dec/DECAY_BELLE2.DEC", true);
    if (m_DecFile.empty()) {
      B2ERROR("No global decay file defined, please make sure the parameter 'DecFile' is set correctly");
      return;
    }
    if (defaultDecFile.empty()) {
      B2WARNING("Cannot find default decay file");
    } else if (defaultDecFile != m_DecFile) {
      B2INFO("Using non-standard DECAY file \"" << m_DecFile << "\"");
    }
    evtgen = new EvtGenDecays(m_Pythia, evtGen);
    evtgen->readDecayFile(m_UserDecFile);
    // Workaround for Pythia bug. It is the only way to call
    // EvtGenDecays::updateData(true) to disable particle decays
    // for all particles from EvtGen decay table. Thus, EvtGen generation
    // has to be called before generation of the first Pythia event.
    // Since Pythia event is currently empty, it actually only updates
    // the particle properties (exactly what is necessary).
    evtgen->decay();
  }

  // List variable(s) that differ from their defaults
  m_Pythia->settings.listChanged();

  initLogCapture.finish();
}

//-----------------------------------------------------------------
//                 Event
//-----------------------------------------------------------------
void FragmentationModule::event()
{
  // Reset the indices of the graph
  mcParticleGraph.clear();
  mcParticleGraph.loadList(m_particleList);

  // Reset PYTHIA event record to allow for new event
  IOIntercept::OutputToLogMessages resetLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Error, 100, 100);
  resetLogCapture.start();
  m_PythiaEvent->reset();
  resetLogCapture.finish();

  // Reset counter for added quarks and vphos
  nAdded  = 0;
  nQuarks = 0;
  nVpho   = 0;

  // Store which MCParticle index belongs to which Pythia index
  std::map<int, int> indexPYTHIA;

  // Store which Pythia index belongs to which MCParticle index
  std::map<int, int> indexMCGraph;

  // Store position of the quark (mother of hadronized final state)
  int quarkPosition = 0;

  // Loop over all particles to find the quark pair
  int nPart = m_mcparticles.getEntries();
  for (int iPart = 0; iPart < nPart; iPart++) {
    MCParticle* currParticle = m_mcparticles[iPart];

    //returns quark id if it finds a quark, zero otherwise
    //increments a counter for the number of found quarks
    int pythiaIndex = addParticleToPYTHIA(*currParticle);

    if (pythiaIndex != 0) {
      indexPYTHIA[nAdded] = iPart;
      if (pythiaIndex > 0) quarkPosition = iPart;
    }
  }

  // Check needed if virtual exchange boson and two quarks are present
  if (nQuarks != 2) {
    B2FATAL("Invalid number of quarks: " << nQuarks << " (should be 2)!");
  }

  if (nVpho != 1) {
    B2WARNING("No virtual exchange particle given, no PYTHIA FSR in Decays");
  } else {
    // Adding QCD and QED FSR
    m_Pythia->forceTimeShower(2, 3, 20.00);
  }

  // Check needed if event is energetically possible
  // ...

  // Do the fragmentation using PYTHIA
  setReturnValue(1); //return value is 1...
  nAll = nAll + 1;

  IOIntercept::OutputToLogMessages eventLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Error, 50, 100);
  eventLogCapture.start();
  int success = m_Pythia->next();
  eventLogCapture.finish();

  if (!success) {
    IOIntercept::OutputToLogMessages listLogCapture("EvtGen", LogConfig::c_Debug, LogConfig::c_Error, 50, 100);
    listLogCapture.start();
    m_PythiaEvent->list();
    listLogCapture.finish();

    setReturnValue(-1); //return value becomes -1 if trials were not successfull
  } else {
    nGood = nGood + 1;
  }

  // use evtgen to perform the decay
  if (m_useEvtGen) {
    IOIntercept::OutputToLogMessages decayLogCapture("PYTHIA", LogConfig::c_Debug, LogConfig::c_Warning, 100, 100);
    decayLogCapture.start();
    evtgen->decay();
    decayLogCapture.finish();
  }

  // Loop over the PYTHIA list and assign the mother-daughter relation
  // Might not work if the mother appear below the daughter in the event record
  for (int iPythiaPart = 0; iPythiaPart < m_Pythia->event.size(); ++iPythiaPart) {
    auto oldindex = indexPYTHIA.find(iPythiaPart);

    //skip "system" particle generated by PYTHIA
    if (m_Pythia->event[iPythiaPart].id() == 90) continue;

    if (oldindex == end(indexPYTHIA)) {
      // --> new particle

      // Add to particle grapg
      int position = mcParticleGraph.size();
      mcParticleGraph.addParticle();
      indexMCGraph[iPythiaPart] = position;

      MCParticleGraph::GraphParticle* p = &mcParticleGraph[position];

      // from PYTHIA manual: "<1: an empty entry, with no meaningful information and
      // therefore to be skipped unconditionally (should not occur in PYTHIA)"
      if (m_Pythia->event[iPythiaPart].statusHepMC() < 1)  continue;

      // Set PDG code
      p->setPDG(m_Pythia->event[iPythiaPart].id());

      // Set four vector
      ROOT::Math::PxPyPzEVector p4(m_Pythia->event[iPythiaPart].px(), m_Pythia->event[iPythiaPart].py(),
                                   m_Pythia->event[iPythiaPart].pz(),
                                   m_Pythia->event[iPythiaPart].e());
      p->set4Vector(p4);
      p->setMass(m_Pythia->event[iPythiaPart].m());

      // Set vertex
      p->setProductionVertex(m_Pythia->event[iPythiaPart].xProd() * Unit::mm, m_Pythia->event[iPythiaPart].yProd() * Unit::mm,
                             m_Pythia->event[iPythiaPart].zProd() * Unit::mm);
      p->setProductionTime(m_Pythia->event[iPythiaPart].tProd() * Unit::mm / Const::speedOfLight);
      p->setValidVertex(true);

      // Set all(!) particles from the generator to primary
      p->addStatus(MCParticleGraph::GraphParticle::c_PrimaryParticle);

      // Set FSR flag from PYTHIA TimeShower:QEDshowerByQ
      if (m_Pythia->event[iPythiaPart].status() == 51 && m_Pythia->event[iPythiaPart].id() == 22) {
        p->addStatus(MCParticleGraph::GraphParticle::c_IsFSRPhoton);
      }

      // Set PHOTOS flag from PYTHIA-EvtGen
      if (m_Pythia->event[iPythiaPart].status() == GeneratorConst::FSR_STATUS_CODE && m_Pythia->event[iPythiaPart].id() == 22) {
        p->addStatus(MCParticleGraph::GraphParticle::c_IsPHOTOSPhoton);
      }

      // Set stable at generator level
      if (m_Pythia->event[iPythiaPart].statusHepMC() == 1) {
        p->addStatus(MCParticleGraph::GraphParticle::c_StableInGenerator);
      }

      //set mother
      const int motherid = m_Pythia->event[iPythiaPart].mother1();

      //check if mother exists in indexMCGraph
      auto motherindex = indexMCGraph.find(motherid);

      if (motherindex != end(indexMCGraph)) {
        int motheridingraph = indexMCGraph[motherid];
        MCParticleGraph::GraphParticle* q = &mcParticleGraph[motheridingraph];
        p->comesFrom(*q);
      } else {
        // Particle has no mother from PYTHIA, add quark as mother
        MCParticleGraph::GraphParticle* q = &mcParticleGraph[quarkPosition];
        p->comesFrom(*q);
      }

    } else {
      // particle is already in the graph
      // modify here if needed
      // ...
    }
  }

  // Print original PYTHIA list
  if (m_listEvent) m_PythiaEvent->list();

  // Create new ParticleGraph
  mcParticleGraph.generateList(m_particleList,
                               MCParticleGraph::c_clearParticles | MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

}

//-----------------------------------------------------------------
//                 addParticleToPYTHIA
//-----------------------------------------------------------------
int FragmentationModule::addParticleToPYTHIA(const MCParticle& mcParticle)
{
  //get PDG code
  const int id = mcParticle.getPDG();

  //check that this particle is a quark or a virtual photon(-like)
  bool isVPho  = false;
  bool isQuark = false;
  if (abs(id) >= 1 && abs(id) <= 5) isQuark = true;
  if (id == 23) isVPho = true;

  if (!(isVPho || isQuark)) return 0;

  //check that there is no daughter for the quarks
  if (isQuark && mcParticle.getDaughters().size()) {
    B2WARNING("Quark already has a daughter!");
    return 0;
  }

  //get some basic kinematics
  const double mass   = mcParticle.getMass();
  const TVector3& p   = mcParticle.getMomentum();
  const double energy = sqrt(mass * mass + p.Mag2());

  //add this (anti)quark to the m_PythiaEvent
  if (id == 23) {
    m_PythiaEvent->append(23, -22, 0, 0, 2, 3, 0, 0, p.X(), p.Y(), p.Z(), energy, mass);
    nVpho++;
  } else if (id > 0) {
    m_PythiaEvent->append(id, 23, 1, 0, 0, 0, 101, 0, p.X(), p.Y(), p.Z(), energy, mass, 20.0);
    nQuarks++;
  } else if (id < 0) {
    m_PythiaEvent->append(id, 23, 1, 0, 0, 0, 0, 101, p.X(), p.Y(), p.Z(), energy, mass, 20.0);
    nQuarks++;
  }

  nAdded++;

  return id;
}

void FragmentationModule::loadEvtGenParticleData(Pythia8::Pythia* pythia)
{
  Pythia8::ParticleData* particleData = &pythia->particleData;
  int nParticles = EvtPDL::entries();
  for (int i = 0; i < nParticles; ++i) {
    EvtId evtgenParticle = EvtPDL::getEntry(i);
    /*
     * Pythia uses absolute value of the PDG code, thus, only positive codes
     * are necessary. Only leptons (11 <= pdg <= 20) and hadrons (pdg > 100)
     * are updated.
     */
    int pdg = EvtPDL::getStdHep(evtgenParticle);
    if (pdg <= 10 || (pdg > 20 && pdg <= 100))
      continue;
    EvtId evtgenAntiParticle = EvtPDL::chargeConj(evtgenParticle);
    if (particleData->isParticle(pdg)) {
      particleData->setAll(pdg,
                           EvtPDL::name(evtgenParticle),
                           EvtPDL::name(evtgenAntiParticle),
                           EvtPDL::getSpinType(evtgenParticle),
                           EvtPDL::chg3(evtgenParticle),
                           // colType == 0 for uncolored particles.
                           0,
                           EvtPDL::getMass(evtgenParticle),
                           EvtPDL::getWidth(evtgenParticle),
                           EvtPDL::getMinMass(evtgenParticle),
                           EvtPDL::getMaxMass(evtgenParticle),
                           EvtPDL::getctau(evtgenParticle));
    }
  }
}

//-----------------------------------------------------------------
//                 random generator for PYTHIA
//-----------------------------------------------------------------
FragmentationRndm::FragmentationRndm() : Pythia8::RndmEngine()
{

}

double FragmentationRndm::flat()
{
  double value = gRandom->Rndm();
  return value;
}
