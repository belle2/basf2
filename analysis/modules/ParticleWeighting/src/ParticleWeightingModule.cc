/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/modules/ParticleWeighting/ParticleWeightingModule.h>

#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/VariableManager/Manager.h>

// framework aux
#include <framework/logging/Logger.h>

#include <memory>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(ParticleWeighting);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParticleWeightingModule::ParticleWeightingModule() : Module()
{
  setDescription("Append weights from the database into the extraInfo of Particles.");
  addParam("tableName", m_tableName, "ID of table used for reweighing");
  addParam("particleList", m_inputListName, "Name of the ParticleList to reduce to the best candidates");
  addParam("selectedDaughters", m_selectedDaughters, "Daughters for which one wants to append weights", std::string(""));
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
    values.insert(std::make_pair(i_variable, std::get<double>(var->function(p))));
  }

  return (*m_ParticleWeightingLookUpTable.get())->getInfo(values);
}


void ParticleWeightingModule::initialize()
{
  m_particles.isRequired();
  m_inputList.isRequired(m_inputListName);
  if (m_selectedDaughters != "")
    m_decayDescriptor.init(m_selectedDaughters);
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

    if (m_selectedDaughters != "") {
      auto selParticles = (m_decayDescriptor.getSelectionParticles(p));
      for (auto& selParticle : selParticles) {
        Particle* pp = m_particles[selParticle->getArrayIndex()];
        WeightInfo info = getInfo(pp);
        for (const auto& entry : info) {
          pp->addExtraInfo(m_tableName + "_" + entry.first, entry.second);
        }
      }
    } else {
      WeightInfo info = getInfo(p);
      for (const auto& entry : info) {
        p->addExtraInfo(m_tableName + "_" + entry.first, entry.second);
      }
    }
  }
}
