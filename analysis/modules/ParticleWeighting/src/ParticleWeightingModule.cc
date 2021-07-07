/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleWeighting/ParticleWeightingModule.h>

#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/VariableManager/Manager.h>

// framework aux
#include <framework/logging/Logger.h>

#include <memory>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleWeighting)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleWeightingModule::ParticleWeightingModule() : Module()

  {
    setDescription("Weights particles according to LookUp table");
    addParam("tableName", m_tableName, "ID of table used for reweighing");
    addParam("particleList", m_inputListName, "Name of the ParticleList to reduce to the best candidates");
  }


  // Getting LookUp info for given particle in given event
  WeightInfo ParticleWeightingModule::getInfo(const Particle* p)
  {
    std::vector<std::string> variables =  Variable::Manager::Instance().resolveCollections((
                                            *m_ParticleWeightingLookUpTable.get())->getAxesNames());
    std::map<std::string, double> values;
    for (const auto& i_variable : variables) {
      const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(i_variable);
      if (!var) {
        B2ERROR("Variable '" << i_variable << "' is not available in Variable::Manager!");
      }
      values.insert(std::make_pair(i_variable, var->function(p)));
    }

    return (*m_ParticleWeightingLookUpTable.get())->getInfo(values);
  }


  void ParticleWeightingModule::initialize()
  {
    m_particles.isRequired();
    m_inputList.isRequired(m_inputListName);
    m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(m_tableName);
  }


  void ParticleWeightingModule::event()
  {
    if (!m_inputList) {
      B2WARNING("Input list " << m_inputList.getName() << " was not created?");
      return;
    }
    const unsigned int numParticles = m_inputList->getListSize();
    for (unsigned int i = 0; i < numParticles; i++) {
      const Particle* ppointer = m_inputList->getParticle(i);
      double index = ppointer->getArrayIndex();
      Particle* p = m_particles[index];
      WeightInfo info = getInfo(p);
      for (const auto& entry : info) {
        p->addExtraInfo(m_tableName + "_" + entry.first, entry.second);
      }
    }
  }

} // end Belle2 namespace

