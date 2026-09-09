/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/herwigfragmentation/HerwigLHEWriterModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>

#include <TRandom.h>  // gRandom

using namespace Belle2;

//-----------------------------------------------------------------
REG_MODULE(HerwigLHEWriter);
//-----------------------------------------------------------------

HerwigLHEWriterModule::HerwigLHEWriterModule() : Module()
{
  setDescription("Stage 1 of the Herwig multistep pipeline. "
                 "Collects KKMC MCParticles (e+, e-, virtual photon/Z0, q, qbar) into a single "
                 "multi-event LHE file and writes per-event KKMC sidecar files. ");
  // shared LHE stream must be written in event order

  addParam("WorkDir",     m_workDir,     "Working directory for LHE and sidecar files; auto-created if empty.",
           std::string(""));
  addParam("ShowerScale", m_showerScale, "Parton shower scale ceiling in GeV (Herwig's SCALUP upper bound)", 11.0);
}

HerwigLHEWriterModule::~HerwigLHEWriterModule() {}

//-----------------------------------------------------------------
void HerwigLHEWriterModule::initialize()
{
  // Create working directory
  if (m_workDir.empty()) {
    char tmpl[] = "/tmp/herwig_batch_XXXXXX";
    char* dir = mkdtemp(tmpl);
    if (!dir) B2FATAL("HerwigLHEWriter: could not create temp directory");
    m_workingDir = std::string(dir);
  } else {
    m_workingDir = m_workDir;
    mkdir(m_workingDir.c_str(), 0755);
  }
  B2INFO("HerwigLHEWriter: working directory = " << m_workingDir);

  // Draw batch seed from basf2 RNG - ties Herwig's batch seed to b2.set_random_seed()
  m_batchSeed = gRandom->Integer(2000000000u) + 1u;
  B2INFO("HerwigLHEWriter: batch seed = " << m_batchSeed);

  // Open all_events.lhe and write LHEF header + init block
  // Use m_showerScale as the nominal init-block beam energy so that per-event
  // lab-frame beam energies (which vary due to ISR boost) never exceed it.
  const double beamE = m_showerScale;
  m_lheStream.open(m_workingDir + "/all_events.lhe");
  if (!m_lheStream.is_open())
    B2FATAL("HerwigLHEWriter: could not open " << m_workingDir << "/all_events.lhe");

  m_lheStream << "<LesHouchesEvents version=\"1.0\">\n<header></header>\n";
  m_lheStream << "<init>\n  11 -11  " << std::scientific << std::setprecision(8)
              << beamE << "  " << beamE
              << "  0  0  -1  -1  3  1\n"
              << "  1.0000000000e+00  0.0000000000e+00  1.0000000000e+00  1\n</init>\n";

  StoreArray<MCParticle> mcParticles;
  mcParticles.isRequired();
}

//-----------------------------------------------------------------
void HerwigLHEWriterModule::terminate()
{
  // Close the LHE file
  if (m_lheStream.is_open()) {
    m_lheStream << "</LesHouchesEvents>\n";
    m_lheStream.close();
  }

  // Write manifest.txt: batch seed, working directory, and per-event sidecar index used by Stage 3.
  // Written once in terminate(): absent = Stage 1 aborted mid-run; present = all N events complete.
  const std::string manifestPath = m_workingDir + "/manifest.txt";
  std::ofstream mf(manifestPath);
  if (!mf.is_open()) {
    B2ERROR("HerwigLHEWriter: could not write manifest to " << manifestPath);
    return;
  }
  mf << "# HerwigLHEWriter manifest\n";
  mf << "work_dir " << m_workingDir << "\n";
  mf << "batch_seed " << m_batchSeed << "\n";
  mf << "total_events " << m_eventCounter << "\n";
  for (const auto& kv : m_manifest)
    mf << kv.first << " " << kv.second.sidecarFile << "\n";
  mf.close();
  B2INFO("HerwigLHEWriter: wrote manifest to " << manifestPath);
  B2RESULT("HerwigLHEWriter: " << m_eventCounter << " events written to "
           << m_workingDir << "/all_events.lhe");
}

//-----------------------------------------------------------------
void HerwigLHEWriterModule::event()
{
  m_eventCounter++;

  StoreObjPtr<EventMetaData> eventMetaData;
  const int globalEvtNum = static_cast<int>(eventMetaData->getEvent());

  StoreArray<MCParticle> mcParticles;

  // Find quarks (|PDG| 1-5, no daughters); in contrast to pythia, the vphoton not needed for LHE (Herwig uses beam structure)
  std::vector<MCParticle*> quarks;
  for (int i = 0; i < mcParticles.getEntries(); ++i) {
    MCParticle* mc = mcParticles[i];
    int absPdg = std::abs(mc->getPDG());
    if (absPdg >= 1 && absPdg <= 5 && mc->getDaughters().empty())
      quarks.push_back(mc);
  }

  if (quarks.size() != 2) {
    B2WARNING("HerwigLHEWriter: event " << globalEvtNum
              << ": expected 2 quarks, got " << quarks.size() << " -- skipping LHE write");
    return;
  }

  if (!writeLHEEventBlock(quarks)) {
    B2ERROR("HerwigLHEWriter: failed to write LHE event block for event " << globalEvtNum);
    return;
  }

  if (!writeKKMCSidecar(globalEvtNum)) {
    B2ERROR("HerwigLHEWriter: failed to write KKMC sidecar for event " << globalEvtNum);
    return;
  }

  // Record in manifest
  std::ostringstream sidecarName;
  sidecarName << "kkmc_evt" << std::setw(6) << std::setfill('0') << globalEvtNum << ".sidecar";
  m_manifest[globalEvtNum].sidecarFile = sidecarName.str();
}

//-----------------------------------------------------------------
bool HerwigLHEWriterModule::writeLHEEventBlock(const std::vector<MCParticle*>& quarks)
{
  if (quarks.size() != 2) return false;
  if (!m_lheStream.is_open()) return false;

  MCParticle* quark    = nullptr;
  MCParticle* antiquark = nullptr;
  for (auto* q : quarks) {
    if (q->getPDG() > 0) quark = q;
    else antiquark = q;
  }
  if (!quark || !antiquark) return false;

  const double sE  = quark->getEnergy() + antiquark->getEnergy();
  const double sPx = quark->getMomentum().X() + antiquark->getMomentum().X();
  const double sPy = quark->getMomentum().Y() + antiquark->getMomentum().Y();
  const double sPz = quark->getMomentum().Z() + antiquark->getMomentum().Z();
  const double M2 = sE * sE - sPx * sPx - sPy * sPy - sPz * sPz;
  const double scalup = (M2 > 0.0) ?
                        std::min(m_showerScale, std::sqrt(M2)) : m_showerScale;
  const double beamEnergy = sE / 2.0;

  // IDPRUP = 1 to match the single process declared in the <init> block (LPRUP=1).
  // Herwig 7.2 rejects events with IDPRUP that don't match a declared process.
  // Stage 3 uses sequential manifest ordering (index-based) for event mapping.
  m_lheStream << "<event>\n  4  1  "
              << std::scientific << std::setprecision(8)
              << "1.00000000e+00  " << scalup << "  7.81653700e-03  1.18000000e-01\n";

  m_lheStream << "   11  -1    0    0    0    0  " << std::showpos << 0.0
              << "  " << 0.0 << "  " << beamEnergy << "  "
              << std::noshowpos << beamEnergy << "  0.0  0.0  -1.0\n";
  m_lheStream << "  -11  -1    0    0    0    0  " << std::showpos << 0.0
              << "  " << 0.0 << "  " << -beamEnergy << "  "
              << std::noshowpos << beamEnergy << "  0.0  0.0  1.0\n";
  {
    double px = quark->getMomentum().X(), py = quark->getMomentum().Y(),
           pz = quark->getMomentum().Z(), e = quark->getEnergy(), m = quark->getMass();
    m_lheStream << "  " << quark->getPDG() << "   1    1    2  101    0  "
                << std::showpos << px << "  " << py << "  " << pz << "  "
                << std::noshowpos << e << "  " << m << "  0.0  9.0\n";
  }
  {
    double px = antiquark->getMomentum().X(), py = antiquark->getMomentum().Y(),
           pz = antiquark->getMomentum().Z(), e = antiquark->getEnergy(), m = antiquark->getMass();
    m_lheStream << "  " << antiquark->getPDG() << "   1    1    2    0  101  "
                << std::showpos << px << "  " << py << "  " << pz << "  "
                << std::noshowpos << e << "  " << m << "  0.0  9.0\n";
  }
  m_lheStream << "</event>\n";
  return !m_lheStream.fail();
}

//-----------------------------------------------------------------
bool HerwigLHEWriterModule::writeKKMCSidecar(int globalEvtNum)
{
  std::ostringstream fname;
  fname << m_workingDir << "/kkmc_evt" << std::setw(6) << std::setfill('0') << globalEvtNum << ".sidecar";
  std::ofstream sf(fname.str());
  if (!sf.is_open()) return false;

  StoreArray<MCParticle> mcParticles;
  const int n = mcParticles.getEntries();

  sf << "# KKMC sidecar event " << globalEvtNum << "\n";
  sf << n << "\n";
  sf << std::scientific << std::setprecision(10);
  for (int i = 0; i < n; ++i) {
    MCParticle* mc = mcParticles[i];
    int motherIdx = -1;
    const MCParticle* mother = mc->getMother();
    if (mother)
      motherIdx = mother->getArrayIndex();
    const ROOT::Math::XYZVector& mom  = mc->getMomentum();
    const ROOT::Math::XYZVector& vtx  = mc->getProductionVertex();
    sf << mc->getPDG()    << " "
       << mc->getEnergy() << " "
       << mom.X()         << " "
       << mom.Y()         << " "
       << mom.Z()         << " "
       << vtx.X()         << " "
       << vtx.Y()         << " "
       << vtx.Z()         << " "
       << mc->getProductionTime() << " "
       << mc->getMass()   << " "
       << mc->getStatus() << " "
       << motherIdx       << "\n";
  }
  sf.close();
  return !sf.fail();
}
