/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/PrintMCParticles/PrintMCParticlesModule.h>

#include <mdst/dataobjects/MCParticle.h>

#include <framework/logging/LogConnectionConsole.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <TDatabasePDG.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintMCParticles)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

namespace {
  /** Helper function to convert MCParticle status to a comma separated string */
  std::string statusToString(const MCParticle& mc)
  {
    static std::map<int, std::string> names {
      {MCParticle::c_PrimaryParticle, "PrimaryParticle"},
      {MCParticle::c_StableInGenerator, "StableInGenerator"},
      {MCParticle::c_LeftDetector, "LeftDetector"},
      {MCParticle::c_StoppedInDetector, "StoppedInDetector"},
      {MCParticle::c_IsVirtual, "IsVirtual"},
      {MCParticle::c_Initial, "Initial"},
      {MCParticle::c_IsISRPhoton, "IsISRPhoton"},
      {MCParticle::c_IsFSRPhoton, "IsFSRPhoton"},
      {MCParticle::c_IsPHOTOSPhoton, "IsPHOTOSPhoton"},
    };
    std::vector<std::string> set;
    for (auto && [status, name] : names) {
      if (mc.hasStatus(status)) set.emplace_back(name);
    }
    return boost::join(set, ", ");
  }

  /** Helper function to convert process number from Geant4 into a string */
  std::string g4ProcessToName(int process)
  {
    // Just copy all the values from the enums given by MCParticle.h documentation ...
    // This was made from the externals version v01-09-01 so we might
    static std::map<int, std::string> translation{
      {1, "CoulombScattering"},
      {2, "Ionisation"},
      {3, "Bremsstrahlung"},
      {4, "PairProdByCharged"},
      {5, "Annihilation"},
      {6, "AnnihilationToMuMu"},
      {7, "AnnihilationToHadrons"},
      {8, "NuclearStopping"},
      {9, "ElectronGeneralProcess"},
      {10, "MultipleScattering"},
      {11, "Rayleigh"},
      {12, "PhotoElectricEffect"},
      {13, "ComptonScattering"},
      {14, "GammaConversion"},
      {15, "GammaConversionToMuMu"},
      {16, "GammaGeneralProcess"},
      {21, "Cerenkov"},
      {22, "Scintillation"},
      {23, "SynchrotronRadiation"},
      {24, "TransitionRadiation"},
      {111, "HadronElastic"},
      {121, "HadronInelastic"},
      {131, "Capture"},
      {132, "MuAtomicCapture"},
      {141, "Fission"},
      {151, "HadronAtRest"},
      {152, "LeptonAtRest"},
      {161, "ChargeExchange"},
      {210, "RadioactiveDecay"},
      {201, "Decay"} ,
      {202, "Decay_WithSpin"} ,
      {203, "Decay_PionMakeSpin"} ,
      {210, "Decay_Radioactive"},
      {211, "Decay_Unknown"},
      {221, "Decay_MuAtom "},
      {231, "Decay_External"},
    };
    if (auto it = translation.find(process); it != translation.end()) {
      return it->second;
    }
    return "[Unknown process]";
  }
}

PrintMCParticlesModule::PrintMCParticlesModule() : Module()
{
  //Set module properties
  setDescription("Print an MCParticle List");

  //Parameter definition
  addParam("storeName", m_particleList, "Name of the StoreArray to print", m_particleList);
  addParam("onlyPrimaries", m_onlyPrimaries, "Show only primary particles", true);
  addParam("maxLevel", m_maxLevel, "Show only up to specified depth level, -1 means no limit", -1);
  addParam("showVertices", m_showVertices, "Show also the particle production vertices and times", false);
  addParam("showMomenta", m_showMomenta, "Show also the particle momenta", false);
  addParam("showProperties", m_showProperties, "Show the basic particle properties", false);
  addParam("showStatus", m_showStatus, "Show extendend status information of the particle", false);
}

void PrintMCParticlesModule::initialize()
{
  m_mcparticles.isRequired(m_particleList);
}

void PrintMCParticlesModule::event()
{
  // clear any previous outputs
  m_output.str(std::string());

  if (not m_particleList.empty()) {
    m_output << "Content from MCParticle list '" << m_mcparticles.getName() << "'" << std::endl;
  } else {
    m_output << "Content of MCParticle list" << std::endl;
  }

  //Loop over the top level particles (no mother particle exists)
  std::vector<MCParticle*> first_gen;
  for (MCParticle& mc : m_mcparticles) {
    if (mc.getMother() != nullptr) continue;
    first_gen.emplace_back(&mc);
  }
  filterPrimaryOnly(first_gen);
  printTree(first_gen);

  B2INFO(m_output.str());
}

void PrintMCParticlesModule::filterPrimaryOnly(std::vector<MCParticle*>& particles) const
{
  if (!m_onlyPrimaries or particles.empty()) return;
  particles.erase(std::remove_if(particles.begin(), particles.end(), [](MCParticle * mc) {
    return not mc->hasStatus(MCParticle::c_PrimaryParticle);
  }), particles.end());
}

void PrintMCParticlesModule::printTree(const std::vector<MCParticle*>& particles, int level, const std::string& indent)
{
  //If we show extra content make the particle name and pdg code bold if supported
  //And if we also show secondaries make those red to distuingish
  const bool useColor = LogConnectionConsole::terminalSupportsColors(STDOUT_FILENO);
  const bool anyExtraInfo = m_showProperties or m_showMomenta or m_showVertices or m_showStatus;
  std::string colorStart[] = {"", ""};
  std::string colorEnd = "";
  if (useColor) {
    // make it bold if we have extra info, otherwise no need
    colorStart[0] = anyExtraInfo ? "\x1b[31;1m" : "\x1b[31m";
    colorStart[1] = anyExtraInfo ? "\x1b[1m" : "";
    colorEnd = "\x1b[m";
  }

  TDatabasePDG* pdb = TDatabasePDG::Instance();

  // ok, go over all particles and print them
  const auto num_particles = particles.size();
  size_t particle_index{0};
  for (const auto* mc : particles) {
    // are we the last in the list? if so we need a different box char
    const bool last = ++particle_index == num_particles;
    m_output << indent << (last ? "╰" : "├");
    m_output << (mc->hasStatus(MCParticle::c_PrimaryParticle) ? "── " : "╶╶ ");
    // now print the name and pdg code. Optionally with color
    TParticlePDG* pdef = pdb->GetParticle(mc->getPDG());
    m_output << colorStart[mc->hasStatus(MCParticle::c_PrimaryParticle)] << (pdef ? pdef->GetTitle() : "[UNKNOWN]");
    m_output << " (" << mc->getPDG() << ")" << colorEnd;

    // Particle itself is done but we might need to show more things like properties and daughters so ...
    auto daughters = mc->getDaughters();
    filterPrimaryOnly(daughters);
    const bool showDaughters = (not daughters.empty()) and (m_maxLevel <= 0 or level < m_maxLevel);

    // new indent level for any information related to this particle.
    const std::string newIndent = indent + (last ? " " : "│") + "   ";
    // and the indent level for any properties associated to this particle
    const std::string propIndent = "\n" + newIndent + (showDaughters ? "│ " : "");
    // limited by level restriction? add an indicator that something is skipped
    if ((not showDaughters) and (not daughters.empty())) {
      m_output << " → …";
    }
    // Now show whatever the user wants
    if (m_showProperties) {
      m_output << propIndent;
      m_output << boost::format("mass=%.3g energy=%.3g charge=%d") % mc->getMass() % mc->getEnergy() % mc->getCharge();
      if (not mc->hasStatus(MCParticle::c_LeftDetector)) m_output << boost::format(" lifetime=%.3g") % mc->getLifetime();
    }
    if (m_showMomenta) {
      const TVector3& p = mc->getMomentum();
      m_output << propIndent;
      m_output << boost::format("p=(%.3g, %.3g, %.3g) |p|=%.3g") % p.X() % p.Y() % p.Z() % p.Mag();
    }
    if (m_showVertices) {
      const TVector3& v = mc->getVertex();
      m_output << propIndent;
      m_output << boost::format("production vertex=(%.3g, %.3g, %.3g), time=%.3g") % v.X() % v.Y() % v.Z() % mc->getProductionTime();
    }
    if (m_showStatus) {
      m_output << propIndent;
      m_output << "status flags=" << statusToString(*mc);
      if (not mc->hasStatus(MCParticle::c_PrimaryParticle)) {
        m_output << propIndent;
        m_output << "creation process=" << g4ProcessToName(mc->getSecondaryPhysicsProcess());
      }
      m_output << propIndent;
      m_output << "list index=" << mc->getIndex();
    }
    // and if we have shown any extra info and have daughters leave a blank line
    if (showDaughters and anyExtraInfo) m_output << propIndent;
    // now we're done with the particle ...
    m_output << std::endl;
    // but maybe we need to show the daughters
    if (showDaughters) {
      printTree(daughters, level + 1, newIndent);
    }
    // and if we show any extra info also leave blank line to sibling
    if (anyExtraInfo and not last) m_output << newIndent << std::endl;
  }
}
