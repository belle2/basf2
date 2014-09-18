/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/BestCandidateSelection/BestCandidateSelectionModule.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


REG_MODULE(BestCandidateSelection)


BestCandidateSelectionModule::BestCandidateSelectionModule():
  m_variable(nullptr)
{
  setDescription("Selects the Particle with the highest value of 'variable' in the input list and removes all other particles from the list. In case of multiple candidates with same value the first one encountered is preserved.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("particleList", m_inputListName, "Name of the ParticleList to reduce to the best candidate");
  addParam("variable", m_variableName, "Variable which defines the candidate ranking (higher is better)");
  addParam("selectLowest", m_selectLowest, "Select the candidate with the lowest value (instead of highest)", false);

}

BestCandidateSelectionModule::~BestCandidateSelectionModule()
{
}

void BestCandidateSelectionModule::initialize()
{
  StoreArray<Particle>::required();
  m_inputList.isRequired(m_inputListName);

  m_variable = Variable::Manager::Instance().getVariable(m_variableName);
  if (!m_variable) {
    B2ERROR("Variable '" << m_variableName << "' is not available in Variable::Manager!");
  }
}

void BestCandidateSelectionModule::event()
{
  int bestIndex = -1;
  double bestValue = 0.0;
  StoreArray<Particle> particles;

  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
  } else {
    unsigned int numParticles = m_inputList->getListSize();
    for (unsigned int i = 0; i < numParticles; i++) {
      const Particle* p = m_inputList->getParticle(i);
      double value = m_variable->function(p);
      if ((!m_selectLowest and value > bestValue)
          or (m_selectLowest and value < bestValue)
          or bestIndex == -1) {
        bestValue = value;
        bestIndex = p->getArrayIndex();
      }
    }

    //remove everything but best candidate:
    m_inputList->clear();
    if (bestIndex != -1)
      m_inputList->addParticle(particles[bestIndex]);
  }
}
