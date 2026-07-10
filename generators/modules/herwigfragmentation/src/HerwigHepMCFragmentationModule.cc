/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/herwigfragmentation/HerwigHepMCFragmentationModule.h>
#include <generators/modules/herwigfragmentation/HerwigFragHelper.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/utilities/FileSystem.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/particledb/EvtGenDatabasePDG.h>

#include <HepMC/IO_GenEvent.h>
#include <HepMC/GenEvent.h>

#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtDecayTable.hh>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <sys/stat.h>

using namespace Belle2;

//-----------------------------------------------------------------
REG_MODULE(HerwigHepMCFragmentation);
//-----------------------------------------------------------------

HerwigHepMCFragmentationModule::HerwigHepMCFragmentationModule() : Module()
{
  setDescription("Stage 3 of the Herwig multistep pipeline. "
                 "Reads pre-generated HepMC events and KKMC sidecar files, reconstructs "
                 "the full MCParticle truth tree, and applies EvtGen + PHOTOS decays. "
                 "No subprocess calls in event() (thus parallelizable with c_ParallelProcessingCertified).");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("WorkDir",        m_workDir,        "Working directory written by HerwigLHEWriterModule.", std::string(""));
  addParam("UseEvtGen",      m_useEvtGen,      "Use EvtGen for particle decays", true);
  addParam("DecFile",        m_decFile,        "EvtGen decay file",
           FileSystem::findFile("decfiles/dec/DECAY_BELLE2.DEC", true));
  addParam("UserDecFile",    m_userDecFile,    "EvtGen user decay file path", std::string(""));
  addParam("CoherentMixing", m_coherentMixing, "Coherent B0-B0bar mixing", true);
  addParam("KeepTempFiles",  m_keepTempFiles,
           "Keep WorkDir and all pipeline files after terminate(). "
           "False (default): delete WorkDir recursively after all events are processed. "
           "True: keep intermediate files.", false);
}

HerwigHepMCFragmentationModule::~HerwigHepMCFragmentationModule()
{
  // Free all loaded HepMC events
  for (auto& kv : m_hepmcEvents)
    delete kv.second;
  m_hepmcEvents.clear();
}

//-----------------------------------------------------------------
//                 Initialize
//-----------------------------------------------------------------
void HerwigHepMCFragmentationModule::initialize()
{
  if (m_workDir.empty())
    B2FATAL("HerwigHepMCFragmentation: WorkDir parameter must be set to the directory "
            "written by HerwigLHEWriterModule.");

  m_hepmcPath = m_workDir + "/all_events.hepmc";

  struct stat hepmcStat;
  if (stat(m_hepmcPath.c_str(), &hepmcStat) != 0)
    B2FATAL("HerwigHepMCFragmentation: HepMC file not found: " << m_hepmcPath);

  if (!loadManifest())
    B2FATAL("HerwigHepMCFragmentation: failed to read manifest from " << m_workDir);

  if (!loadHepMCEvents())
    B2FATAL("HerwigHepMCFragmentation: failed to load HepMC events from " << m_hepmcPath);

  // EvtGen setup - called once; interface owns internal state that must persist across event() calls
  if (m_useEvtGen) {
    try {
      m_evtGenInterface.setup(m_decFile, "", m_userDecFile, m_coherentMixing);
      B2INFO("HerwigHepMCFragmentation: EvtGen initialized");
    } catch (std::exception& e) {
      B2ERROR("EvtGen initialization failed: " << e.what());
      m_useEvtGen = false;
    }
  }

  m_helper = std::make_unique<HerwigFragHelper>();
  StoreArray<MCParticle> mcParticles;
  mcParticles.registerInDataStore();

  B2INFO("HerwigHepMCFragmentation: loaded " << m_hepmcEvents.size()
         << " HepMC events from " << m_hepmcPath);
}

//-----------------------------------------------------------------
//                 Terminate
//-----------------------------------------------------------------
void HerwigHepMCFragmentationModule::terminate()
{
  double rate = m_eventCounter > 0 ?
                100.0 * m_successCounter / m_eventCounter : 0.0;
  B2RESULT("HerwigHepMCFragmentation: " << m_successCounter << "/" << m_eventCounter
           << " events (" << std::fixed << std::setprecision(2) << rate << "%)");
  if (rate < 97.0 && m_eventCounter > 100)
    B2WARNING("Success rate below 97% threshold");

  if (!m_keepTempFiles && !m_workDir.empty()) {
    try {
      std::filesystem::remove_all(m_workDir);
      B2INFO("HerwigHepMCFragmentation: removed working directory " << m_workDir);
    } catch (std::exception& e) {
      B2WARNING("HerwigHepMCFragmentation: could not remove working directory "
                << m_workDir << ": " << e.what());
    }
  }
}

//-----------------------------------------------------------------
//                 Event
//-----------------------------------------------------------------
void HerwigHepMCFragmentationModule::event()
{
  m_eventCounter++;

  StoreObjPtr<EventMetaData> eventMetaData;
  const int globalEvtNum = static_cast<int>(eventMetaData->getEvent());

  // Look up the HepMC event for this global event number
  auto evtIt = m_hepmcEvents.find(globalEvtNum);
  if (evtIt == m_hepmcEvents.end()) {
    // Herwig skipped this event
    setReturnValue(0); return;
  }
  HepMC::GenEvent* hepmc = evtIt->second;

  m_mcParticleGraph.clear();  // must clear before each event

  // Reconstruct KKMC truth sub-tree from sidecar
  std::vector<int> quarkIndices;
  if (!loadKKMCSidecar(globalEvtNum, quarkIndices)) {
    B2ERROR("HerwigHepMCFragmentation: failed to load sidecar for event " << globalEvtNum);
    setReturnValue(0); return;
  }

  // Map quark graph-indices: quarks are added from sidecar; quarkIndices already populated.
  // Add Herwig hadrons to the graph (appended after the KKMC particles already in the graph)
  int nAdded = m_helper->addHepMCToGraph(hepmc, m_mcParticleGraph, quarkIndices);
  if (nAdded < 0) {
    B2ERROR("HerwigHepMCFragmentation: addHepMCToGraph failed for event " << globalEvtNum);
    setReturnValue(0); return;
  }

  // First generateList: sync graph to StoreArray before EvtGen pass
  m_mcParticleGraph.generateList("",
                                 MCParticleGraph::c_clearParticles |
                                 MCParticleGraph::c_setDecayInfo   |
                                 MCParticleGraph::c_checkCyclic);

  // EvtGen decays + PHOTOS (four-layer filter with PDG remaps for Herwig-specific particles).
  // applyEvtGenDecays() returns false only on exception - stable/unknown particles are
  // handled inside the loop via continue and never affect the return value.
  // A false return means EvtGen crashed mid-decay and the second generateList() did not
  // run, so the StoreArray is incomplete. Treated as a failed event.
  if (m_useEvtGen) {
    if (!applyEvtGenDecays()) {
      B2ERROR("HerwigHepMCFragmentation: EvtGen decays failed for event "
              << globalEvtNum << " - event skipped (incomplete MCParticle tree)");
      setReturnValue(0); return;
    }
  }

  m_successCounter++;
  setReturnValue(1);
}

//-----------------------------------------------------------------
//                 loadManifest
//-----------------------------------------------------------------
bool HerwigHepMCFragmentationModule::loadManifest()
{
  const std::string manifestPath = m_workDir + "/manifest.txt";
  std::ifstream mf(manifestPath);
  if (!mf.is_open()) {
    B2ERROR("HerwigHepMCFragmentation: cannot open " << manifestPath);
    return false;
  }

  std::string line;
  while (std::getline(mf, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream ss(line);
    std::string key;
    ss >> key;
    if (key == "work_dir" || key == "batch_seed" || key == "total_events") continue;
    // Every other line: event_number sidecar_filename
    try {
      int evtNum = std::stoi(key);
      std::string sidecarFile;
      ss >> sidecarFile;
      if (!sidecarFile.empty())
        m_sidecarMap[evtNum] = m_workDir + "/" + sidecarFile;
    } catch (...) {
      B2WARNING("HerwigHepMCFragmentation: skipping manifest line: " << line);
    }
  }
  B2DEBUG(10, "HerwigHepMCFragmentation: manifest loaded, " << m_sidecarMap.size() << " events");
  return !m_sidecarMap.empty();
}

//-----------------------------------------------------------------
//                 loadHepMCEvents
//-----------------------------------------------------------------
bool HerwigHepMCFragmentationModule::loadHepMCEvents()
{
  HepMC::IO_GenEvent ascii_in(m_hepmcPath.c_str(), std::ios::in);
  HepMC::GenEvent* evt;
  int count = 0;
  bool signal_id_ok = true;
  std::vector<HepMC::GenEvent*> ordered;  // for fallback sequential mapping

  while ((evt = ascii_in.read_next_event()) != nullptr) {
    ordered.push_back(evt);
    const int spid = evt->signal_process_id();
    if (spid > 0) {
      m_hepmcEvents[spid] = evt;
    } else {
      signal_id_ok = false;
    }
    count++;
  }

  if (count == 0) {
    B2ERROR("HerwigHepMCFragmentation: no events read from " << m_hepmcPath);
    return false;
  }

  // Check if signal_process_id-based mapping is valid (all non-zero, all unique)
  if (!signal_id_ok || static_cast<int>(m_hepmcEvents.size()) != count) {
    B2WARNING("HerwigHepMCFragmentation: signal_process_id not unique or missing. "
              "Herwig 7.2.0 sets signal_process_id=-1 so we use sequential manifest-to-HepMC ordering for this version.");

    // Fall back: map manifest order -> HepMC order
    m_hepmcEvents.clear();
    int hepmc_idx = 0;
    for (const auto& kv : m_sidecarMap) {
      if (hepmc_idx >= count) break;
      m_hepmcEvents[kv.first] = ordered[hepmc_idx];
      hepmc_idx++;
    }
    if (hepmc_idx < count) {
      B2WARNING("HerwigHepMCFragmentation: " << count - hepmc_idx
                << " HepMC events could not be mapped to manifest entries.");
      // Free unmapped HepMC events: they are in ordered[] but not in m_hepmcEvents
      for (; hepmc_idx < count; ++hepmc_idx)
        delete ordered[hepmc_idx];
    }
  } else {
    B2INFO("HerwigHepMCFragmentation: signal_process_id mapping verified for "
           << count << " events.");
  }

  return !m_hepmcEvents.empty();
}

//-----------------------------------------------------------------
//                 loadKKMCSidecar
//-----------------------------------------------------------------
bool HerwigHepMCFragmentationModule::loadKKMCSidecar(int globalEvtNum,
                                                     std::vector<int>& quarkIndices)
{
  quarkIndices.clear();

  auto it = m_sidecarMap.find(globalEvtNum);
  if (it == m_sidecarMap.end()) {
    B2ERROR("HerwigHepMCFragmentation: no sidecar entry for event " << globalEvtNum);
    return false;
  }

  std::ifstream sf(it->second);
  if (!sf.is_open()) {
    B2ERROR("HerwigHepMCFragmentation: cannot open sidecar " << it->second);
    return false;
  }

  std::string line;
  // Skip comment header
  std::getline(sf, line);  // "# KKMC sidecar event N"

  int n = 0;
  sf >> n;
  if (n <= 0) { B2ERROR("HerwigHepMCFragmentation: empty sidecar for event " << globalEvtNum); return false; }

  // Add N KKMC particles to graph in order; record graph indices
  const int graphBase = static_cast<int>(m_mcParticleGraph.size());  // should be 0 (clear() was called)

  struct SidecarEntry {
    int pdg, status, motherIdx;
    double e, px, py, pz, vx, vy, vz, vt, mass;
  };
  std::vector<SidecarEntry> entries(n);

  // Read all entries first (needed so we can set parent links after all particles are added)
  for (int i = 0; i < n; ++i) {
    auto& e = entries[i];
    sf >> e.pdg >> e.e >> e.px >> e.py >> e.pz
       >> e.vx  >> e.vy >> e.vz >> e.vt
       >> e.mass >> e.status >> e.motherIdx;
  }
  sf.close();

  for (int i = 0; i < n; ++i) {
    const auto& entry = entries[i];
    m_mcParticleGraph.addParticle();
    MCParticleGraph::GraphParticle& p = m_mcParticleGraph[graphBase + i];

    p.setPDG(entry.pdg);
    p.setMomentum(ROOT::Math::XYZVector(entry.px, entry.py, entry.pz));
    p.setMass(entry.mass);
    // Recalculate E from p and m so the 4-momentum is exactly on-shell
    const double e2 = entry.px * entry.px + entry.py * entry.py
                      + entry.pz * entry.pz + entry.mass * entry.mass;
    p.setEnergy(e2 > 0.0 ? std::sqrt(e2) : 0.0);
    p.setProductionVertex(ROOT::Math::XYZVector(entry.vx, entry.vy, entry.vz));
    p.setProductionTime(entry.vt);
    p.setStatus(entry.status);

    // Restore parent link using local sidecar index
    if (entry.motherIdx >= 0 && entry.motherIdx < n)
      p.comesFrom(m_mcParticleGraph[graphBase + entry.motherIdx]);

    // Record quark indices for HerwigFragHelper fallback parent linking
    const int absPdg = std::abs(entry.pdg);
    if (absPdg >= 1 && absPdg <= 5 && entry.motherIdx >= 0)
      quarkIndices.push_back(graphBase + i);
  }

  return true;
}

//-----------------------------------------------------------------
//                 applyEvtGenDecays
//-----------------------------------------------------------------
bool HerwigHepMCFragmentationModule::applyEvtGenDecays()
{
  if (!m_useEvtGen) return true;
  try {
    int nDecayed = 0;
    const size_t nBefore = m_mcParticleGraph.size();

    for (size_t i = 0; i < nBefore; ++i) {
      MCParticleGraph::GraphParticle& gp = m_mcParticleGraph[i];

      // Layer 1: skip particles that already have daughters
      if (gp.getFirstDaughter() != 0) continue;

      // Layer 2a: Herwig 7.2.0 uses PDG 10433 for D_s1(2460); EvtGen calls it D_s1+.
      // Remove if upgraded Herwig resolved this issue.
      if (std::abs(gp.getPDG()) == 10433) {
        const char* name = (gp.getPDG() > 0) ? "D_s1+" : "D_s1-";
        const auto* part = EvtGenDatabasePDG::Instance()->GetParticle(name);
        if (part) gp.setPDG(part->PdgCode());
      }

      // Layer 2b: Herwig uses PDG 10221 for f_0(1370); EvtGen calls it f'_0
      if (gp.getPDG() == 10221) {
        const auto* part = EvtGenDatabasePDG::Instance()->GetParticle("f'_0");
        if (part) gp.setPDG(part->PdgCode());
      }

      // Layer 3: skip particles absent from evt.pdl
      const EvtId id = EvtPDL::evtIdFromStdHep(gp.getPDG());
      if (id.getId() < 0) continue;

      // Layer 4: skip particles with no decay modes in DECAY_BELLE2.DEC or UserDecFile
      if (EvtDecayTable::getInstance()->getNModes(id) == 0) continue;

      int result = m_evtGenInterface.simulateDecay(m_mcParticleGraph, gp);
      if (result > 0) nDecayed++;
    }

    // Second generateList: write EvtGen daughters and PHOTOS photons to StoreArray
    m_mcParticleGraph.generateList("",
                                   MCParticleGraph::c_clearParticles |
                                   MCParticleGraph::c_setDecayInfo   |
                                   MCParticleGraph::c_checkCyclic);
    return true;
  } catch (std::exception& e) {
    B2ERROR("EvtGen decay failed: " << e.what());
    return false;
  }
}
