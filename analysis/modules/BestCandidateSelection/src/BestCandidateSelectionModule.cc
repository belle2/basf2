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

#include <analysis/VariableManager/Utility.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <map>

using namespace std;
using namespace Belle2;


REG_MODULE(BestCandidateSelection)


BestCandidateSelectionModule::BestCandidateSelectionModule():
  m_variable(nullptr)
{
  //the "undefined order" bit is not strictly true in the current implementation, but details (with anti-particle lists) are tricky
  setDescription("Selects Particles with the highest values of 'variable' in the input list and removes all other particles from the list. Particles will receive an extra-info field '${variable}_rank' containing their rank as an integer starting at 1 (best). The ranking also takes antiparticles into account, so there will only be one B+- candidate with rank=1. Candidates with same value of 'variable' will have different ranks, with undefined order. The remaining list is sorted from best to worst candidate (each charge, e.g. B+/B-, separately).");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("particleList", m_inputListName, "Name of the ParticleList to reduce to the best candidates");
  addParam("variable", m_variableName, "Variable which defines the candidate ranking (see selectLowest for ordering)");
  addParam("selectLowest", m_selectLowest, "Candidate with lower value are better (default: higher is better))", false);
  addParam("allowMultiRank", m_allowMultiRank, "Candidates with identical values get identical rank", false);
  addParam("numBest", m_numBest, "Keep this many of the best candidates (0: keep all)", 0);
  addParam("outputVariable", m_outputVariableName,
           "Name for created variable, which contains the rank for the particle. If not provided, the standard name '${variable}_rank' is used.");

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
  if (m_numBest < 0) {
    B2ERROR("value of numBest must be >= 0!");
  }
}

void BestCandidateSelectionModule::event()
{
  StoreArray<Particle> particles;

  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    return;
  }
  std::function<bool(double, double)> betterThan = [](double a, double b) -> bool { return a > b; };
  if (m_selectLowest) {
    betterThan = [](double a, double b) -> bool { return a < b; };
  }

  //create list of particle index and the corresponding value of variable
  std::multimap<double, unsigned int, decltype(betterThan)> valueToIndex(betterThan);
  const unsigned int numParticles = m_inputList->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    const Particle* p = m_inputList->getParticle(i);
    double value = m_variable->function(p);
    valueToIndex.insert(std::make_pair(value, p->getArrayIndex()));
  }

  std::string extraInfoName;
  if (m_outputVariableName.empty()) {
    std::string root_compatible_VariableName = makeROOTCompatible(m_variableName);
    extraInfoName = root_compatible_VariableName + "_rank";
  } else {
    extraInfoName = m_outputVariableName;
  }
  //remove everything but best candidates
  m_inputList->clear();
  int rank = 1;
  double previous_val;
  bool first_candidate = true;
  for (const auto& candidate : valueToIndex) {
    if (first_candidate) {
      first_candidate = false;
      previous_val = candidate.first;
    }
    Particle* p = particles[candidate.second];
    p->addExtraInfo(extraInfoName, rank);
    m_inputList->addParticle(p);
    if (m_allowMultiRank) {
      if (candidate.first != previous_val) {
        rank++;
      }
    } else {
      rank++;
    }

    previous_val = candidate.first;

    if (m_numBest != 0 and rank > m_numBest)
      break;
  }
}
