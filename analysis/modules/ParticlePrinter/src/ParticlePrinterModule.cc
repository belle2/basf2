/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticlePrinter/ParticlePrinterModule.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/VariableManager/Manager.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticlePrinter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticlePrinterModule::ParticlePrinterModule() : Module()

  {
    setDescription("Prints specified variables for all particles in the specified particle list to screen (useful for debugging).\n"
                   "Event-based variables can be printed by not specifying the particle list (empty string).");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("listName", m_listName, "name of ParticleList", string(""));
    addParam("fullPrint", m_fullPrint, "execute Particle's internal print() function", true);
    vector<string> defaultVariables;
    addParam("variables", m_variables, "names of variables to be printed (see Variable::Manager)", defaultVariables);

  }

  void ParticlePrinterModule::initialize()
  {
    if (!m_listName.empty()) {
      m_plist.isRequired(m_listName);

      // obtain the input and output particle lists from the decay string
      bool valid = m_decaydescriptor.init(m_listName);
      if (!valid)
        B2ERROR("ParticlePrinterModule::initialize Invalid input DecayString: " << m_listName);

      int nProducts = m_decaydescriptor.getNDaughters();
      if (nProducts > 0)
        B2ERROR("ParticlePrinterModule::initialize Invalid input DecayString " << m_listName
                << ". DecayString should not contain any daughters, only the mother particle.");
    }
  }

  void ParticlePrinterModule::event()
  {
    bool includingVars = !(m_variables.empty());

    // print event based variables (particle list is empty)
    if (m_listName.empty() && includingVars) {
      printVariables(nullptr);
      return;
    }

    // Print variables for all particles in the list
    if (!m_plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    if (!m_listName.empty() && m_plist->getListSize() == 0) return;

    B2INFO("[ParticlePrinterModule] START ------------------------------");

    m_plist->print();

    for (unsigned i = 0; i < m_plist->getListSize(); i++) {
      const Particle* particle = m_plist->getParticle(i);
      if (m_fullPrint) {
        particle->print();
      }
      if (includingVars) {
        B2INFO(" - " << particle->getArrayIndex() << " = " << particle->getPDGCode() << "[" << i << "]");
        if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite) {
          std::string s;
          for (int idx : particle->getDaughterIndices())
            s += " " + std::to_string(idx);
          B2INFO("     o) daughter indices =" << s);
        }
        printVariables(particle);
      }
    }
    B2INFO("[ParticlePrinterModule] END   ------------------------------");
  }


  void ParticlePrinterModule::printVariables(const Particle* particle) const
  {
    Variable::Manager& manager = Variable::Manager::Instance();

    for (auto& varName : m_variables) {
      auto var = manager.getVariable(varName);
      if (var == nullptr) {
        B2ERROR("ParticlePrinter: Variable::Manager doesn't have variable" << varName);
      } else {
        Variable::Manager::VarVariant result = var->function(particle);
        if (std::holds_alternative<double>(result)) {
          double value = std::get<double>(result);
          B2INFO("     o) " << varName << " = " << value);
        } else if (std::holds_alternative<int>(result)) {
          int value = std::get<int>(result);
          B2INFO("     o) " << varName << " = " << value);
        } else if (std::holds_alternative<bool>(result)) {
          bool value = std::get<bool>(result);
          B2INFO("     o) " << varName << " = " << value);
        }
      }
    }
  }

} // end Belle2 namespace

