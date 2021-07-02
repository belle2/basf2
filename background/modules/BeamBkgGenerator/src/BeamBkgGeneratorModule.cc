/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgGenerator/BeamBkgGeneratorModule.h>

// framework - DataStore
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
#include <generators/SAD/dataobjects/SADMetaHit.h>

// random generator
#include <TRandom.h>

// coordinates translation
#include <iostream>
#include <TGeoMatrix.h>
#include <generators/SAD/ReaderSAD.h>

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

    StoreArray<SADMetaHit> sadHits;
    sadHits.registerInDataStore();


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

    // for SADMetaHit
    m_tree->SetBranchAddress("ss", &m_sad.ss);
    m_tree->SetBranchAddress("sraw", &m_sad.sraw);
    m_tree->SetBranchAddress("ssraw", &m_sad.ssraw);
    m_tree->SetBranchAddress("nturn", &m_sad.nturn);
    m_tree->SetBranchAddress("xraw", &m_sad.xraw);
    m_tree->SetBranchAddress("yraw", &m_sad.yraw);
    m_tree->SetBranchAddress("r", &m_sad.r);
    m_tree->SetBranchAddress("rr", &m_sad.rr);
    m_tree->SetBranchAddress("dp_over_p0", &m_sad.dp_over_p0);
    m_tree->SetBranchAddress("watt", &m_sad.watt);

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
      m_ring = 2;
    } else {
      GearDir ring("/Detector/SuperKEKB/HER/");
      m_rotation.RotateY(ring.getAngle("angle"));
      m_ring = 1;
    }

    m_sectionOrdering.insert(m_sectionOrdering.end(), {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2});

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

    // make SADMetaHit
    int ring_section = -1;
    double ssraw = m_sad.ss;
    if (m_sad.ss < 0) ssraw += 3016.;
    int section = (int)(ssraw * 12 / 3016.);
    if ((unsigned)section < m_sectionOrdering.size()) ring_section = m_sectionOrdering[section];

    StoreArray<SADMetaHit> SADMetaHits;
    SADMetaHits.appendNew(SADMetaHit(m_sad.ssraw, m_sad.sraw, m_sad.ss, m_sad.s,
                                     0., m_sad.nturn,
                                     m_sad.x, m_sad.y, m_sad.px, m_sad.py, m_sad.xraw, m_sad.yraw,
                                     m_sad.r, m_sad.rr, m_sad.dp_over_p0, m_sad.E, m_sad.rate,
                                     m_sad.watt, m_ring, ring_section));


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

    // FarBeamLine region transformation
    if (abs(m_sad.s * Unit::m) > 4.0 * Unit::m) {
      // initial coordinates in SAD space
      double particlePosSADfar[] = {m_sad.x* Unit::m, -m_sad.y* Unit::m, 0.0 * Unit::m};
      double particleMomSADfar[] = {m_sad.px* Unit::GeV, -m_sad.py* Unit::GeV, pz* Unit::GeV};
      // final coordinates in Geant4 space
      double particlePosGeant4[] = {0.0, 0.0, 0.0};
      double particleMomGeant4[] = {0.0, 0.0, 0.0};

      // create a transformation matrix for a given ring
      TGeoHMatrix transMatrix; /**< Transformation matrix from SAD space into geant4 space. */
      if (m_ringName == "LER") {
        transMatrix = m_readerSAD.SADtoGeant(ReaderSAD::c_LER, m_sad.s * Unit::m);
      } else {
        transMatrix = m_readerSAD.SADtoGeant(ReaderSAD::c_HER, m_sad.s * Unit::m);
      }

      // calculate a new set of coordinates in Geant4 space
      transMatrix.LocalToMaster(particlePosSADfar, particlePosGeant4); // position
      transMatrix.LocalToMasterVect(particleMomSADfar, particleMomGeant4); // momentum
      // apply a new set of coordinates
      part->setMomentum(TVector3(particleMomGeant4));
      part->setProductionVertex(TVector3(particlePosGeant4));
    }
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

