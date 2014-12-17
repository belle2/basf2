/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticlePrinter/ParticlePrinterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/ParticleList.h>
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
    // set module description (e.g. insert text)
    setDescription("Prints particle list to screen (useful for debugging)");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("listName", m_listName, "name of ParticleList", string(""));
    addParam("fullPrint", m_fullPrint, "execute Particle's internal print() function", true);
    vector<string> defaultVariables;
    addParam("variables", m_variables, "names of variables to be printed (see Variable::Manager)", defaultVariables);

  }

  ParticlePrinterModule::~ParticlePrinterModule()
  {
  }

  void ParticlePrinterModule::initialize()
  {
    if (!m_listName.empty()) {

      // obtain the input and output particle lists from the decay string
      bool valid = m_decaydescriptor.init(m_listName);
      if (!valid)
        B2ERROR("ParticlePrinterModule::initialize Invalid input DecayString: " << m_listName);

      int nProducts = m_decaydescriptor.getNDaughters();
      if (nProducts > 0)
        B2ERROR("ParticlePrinterModule::initialize Invalid input DecayString " << m_listName
                << ". DecayString should not contain any daughters, only the mother particle.");
    } else
      B2ERROR("ParticlePrinterModule::initialize Empty list name!");
  }

  void ParticlePrinterModule::beginRun()
  {
  }

  void ParticlePrinterModule::event()
  {
    B2INFO("[ParticlePrinterModule] START ------------------------------");

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    plist->print();

    bool includingVars = !(m_variables.empty());

    for (unsigned i = 0; i < plist->getListSize(); i++) {
      const Particle* particle = plist->getParticle(i);
      if (m_fullPrint) {
        particle->print();
      }
      if (includingVars) {
        B2INFO(" - " << particle->getArrayIndex() << " = " << particle->getPDGCode() << "[" << i << "]");
        if (particle->getParticleType() == Particle::EParticleType::c_Composite) {
          std::cout << "[INFO]     o) daughter indices = ";
          const std::vector<int> daughters = particle->getDaughterIndices();
          for (unsigned j = 0; j < daughters.size(); ++j) {
            std::cout << " " << daughters[j];
          }
          std::cout << std::endl;
        }
        printVariables(particle);
      }
    }
    B2INFO("[ParticlePrinterModule] END   ------------------------------");
  }


  void ParticlePrinterModule::endRun()
  {
  }

  void ParticlePrinterModule::terminate()
  {
  }


  void ParticlePrinterModule::printVariables(const Particle* particle) const
  {
    Variable::Manager& manager = Variable::Manager::Instance();

    for (auto & varName : m_variables) {
      auto var = manager.getVariable(varName);
      if (var == nullptr) {
        B2ERROR("ParticlePrinter: Variable::Manager doesn't have variable" << varName)
      } else {
        double value = var->function(particle);
        B2INFO("     o) " << varName << " = " << value);
      }
    }
  }

} // end Belle2 namespace

