/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgGenerator/BeamBkgGeneratorModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

// data objects
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/MCParticle.h>

// random generator
#include <TRandom.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgGenerator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgGeneratorModule::BeamBkgGeneratorModule() : Module()

  {
    // set module description
    setDescription("Beam background generator based on SAD files. "
                   "The generator picks up particles from the SAD file randomly "
                   "according to their rates. "
                   "Number of events is determined from 'realTime' and overall rate, and "
                   "the generator terminates the execution when this number is reached.");

    // Add parameters
    addParam("fileName", m_fileName, "name of the SAD file converted to root");
    addParam("treeName", m_treeName, "name of the TTree in the SAD file", string("sad"));
    addParam("ringName", m_ringName, "name of the superKEKB ring (LER or HER)");
    addParam("realTime", m_realTime,
             "equivalent superKEKB running time to generate sample [ns].");

  }

  BeamBkgGeneratorModule::~BeamBkgGeneratorModule()
  {
  }

  void BeamBkgGeneratorModule::initialize()
  {
    // register MCParticles

    StoreArray<MCParticle> mcParticles;
    mcParticles.registerInDataStore();

    // check steering parameters

    if (m_ringName != "LER" and m_ringName != "HER") {
      B2ERROR("ringName can only be LER or HER");
    }
    if (m_realTime <= 0) {
      B2ERROR("realTime must be positive");
    }

    // open SAD file and get TTree

    m_file = TFile::Open(m_fileName.c_str());
    if (!m_file) {
      B2ERROR(m_fileName << ": can't open file");
      return;
    }
    m_tree = (TTree*) m_file->Get(m_treeName.c_str());
    if (!m_tree) {
      B2ERROR(m_treeName << ": no such TTree in the SAD file");
      return;
    }

    m_tree->SetBranchAddress("s", &m_sad.s);
    m_tree->SetBranchAddress("x", &m_sad.x);
    m_tree->SetBranchAddress("px", &m_sad.px);
    m_tree->SetBranchAddress("y", &m_sad.y);
    m_tree->SetBranchAddress("py", &m_sad.py);
    m_tree->SetBranchAddress("E", &m_sad.E);
    m_tree->SetBranchAddress("rate", &m_sad.rate);

    int numEntries = m_tree->GetEntries();
    if (numEntries <= 0) {
      B2ERROR("SAD tree is empty");
      return;
    }

    // construct vector of cumulative rates and determine number of events to generate

    m_rates.push_back(0);
    for (int i = 0; i < numEntries; i++) {
      m_tree->GetEntry(i);
      m_rates.push_back(m_sad.rate + m_rates.back());
      m_counters.push_back(0);
    }

    m_numEvents = gRandom->Poisson(m_rates.back() * m_realTime / Unit::s);

    // set rotation from SAD to Belle II

    if (m_ringName == "LER") {
      GearDir ring("/Detector/SuperKEKB/LER/");
      m_rotation.RotateY(ring.getAngle("angle"));
    } else {
      GearDir ring("/Detector/SuperKEKB/HER/");
      m_rotation.RotateY(ring.getAngle("angle"));
    }

    B2RESULT("BG rate: " << m_rates.back() / 1e6 << " MHz, events to generate: "
             << m_numEvents);

  }

  void BeamBkgGeneratorModule::beginRun()
  {
  }

  void BeamBkgGeneratorModule::event()
  {

    // check event counter

    if (m_eventCounter >= m_numEvents) {
      StoreObjPtr<EventMetaData> eventMetaData;
      eventMetaData->setEndOfData(); // stop event processing
      return;
    }
    m_eventCounter++;

    // get SAD entry

    int i = generateEntry();
    m_tree->GetEntry(i);
    m_counters[i]++;

    // transform to Belle II (flip sign of y and s, rotate)

    TVector3 position(m_sad.x * Unit::m, -m_sad.y * Unit::m, -m_sad.s * Unit::m);
    position.Transform(m_rotation);

    double pz = sqrt(m_sad.E * m_sad.E - m_sad.px * m_sad.px - m_sad.py * m_sad.py);
    if (m_ringName == "LER") pz = -pz;
    TVector3 momentum(m_sad.px, -m_sad.py, pz);
    momentum.Transform(m_rotation);

    // PDG code and mass

    int pdgCode = Const::electron.getPDGCode();
    double mass = Const::electron.getMass();
    if (m_ringName == "LER") pdgCode = -pdgCode;

    // append SAD particle to MCParticles

    StoreArray<MCParticle> MCParticles;
    MCParticle* part = MCParticles.appendNew();
    part->setPDG(pdgCode);
    part->setMass(mass);
    part->setProductionVertex(position);
    part->setProductionTime(0);
    part->setMomentum(momentum);
    part->setEnergy(sqrt(momentum.Mag2() + mass * mass));
    part->setValidVertex(true);
    part->setStatus(MCParticle::c_PrimaryParticle);
    part->addStatus(MCParticle::c_StableInGenerator);

  }


  void BeamBkgGeneratorModule::endRun()
  {
  }

  void BeamBkgGeneratorModule::terminate()
  {
    // close SAD file
    m_file->Close();

    B2RESULT("BG rate: " << m_rates.back() / 1e6 << " MHz, equivalent time: "
             << m_realTime / Unit::us << " us, "
             << m_eventCounter << " events generated");
    if (m_eventCounter != m_numEvents)
      B2ERROR("Number of generated events does not match the equivalent running time: "
              << m_numEvents << " events needed, but "
              << m_eventCounter << " generated");

    int imax = 0;
    double average = 0;
    for (unsigned i = 0; i < m_counters.size(); i++) {
      if (m_counters[i] > m_counters[imax]) imax = i;
      average += m_counters[i];
    }
    average /= m_counters.size();
    B2RESULT("SAD particle usage: on average " << average << " times, max "
             << m_counters[imax] << " times (entry " << imax << ")");

  }


  int BeamBkgGeneratorModule::generateEntry() const
  {
    double rate = gRandom->Uniform(m_rates.back());
    int i1 = 0;
    int i2 = m_rates.size() - 1;
    while (i2 - i1 > 1) {
      int i = (i1 + i2) / 2;
      if (rate > m_rates[i]) {
        i1 = i;
      } else {
        i2 = i;
      }
    }
    return i1;
  }


} // end Belle2 namespace

