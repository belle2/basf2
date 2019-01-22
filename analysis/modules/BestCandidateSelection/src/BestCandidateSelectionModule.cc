/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher, Martin Ritter                    *
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
  addParam("cut", m_cutParameter, "Only candidates passing the cut will be ranked. The others will have rank -1.", std::string(""));
  addParam("outputVariable", m_outputVariableName,
           "Name for created variable, which contains the rank for the particle. If not provided, the standard name '${variable}_rank' is used.");

}

BestCandidateSelectionModule::~BestCandidateSelectionModule()
{
}

void BestCandidateSelectionModule::initialize()
{
  StoreArray<Particle>().isRequired();
  m_inputList.isRequired(m_inputListName);

  m_variable = Variable::Manager::Instance().getVariable(m_variableName);
  if (!m_variable) {
    B2ERROR("Variable '" << m_variableName << "' is not available in Variable::Manager!");
  }
  if (m_numBest < 0) {
    B2ERROR("value of numBest must be >= 0!");
  }
  m_cut = Variable::Cut::compile(m_cutParameter);

  // parse the name that the rank will be stored under
  if (m_outputVariableName.empty()) {
    std::string root_compatible_VariableName = makeROOTCompatible(m_variableName);
    m_outputVariableName = root_compatible_VariableName + "_rank";
  }
}

void BestCandidateSelectionModule::event()
{
  // input list
  StoreArray<Particle> particles;
  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    return;
  }

  // define the criteria for "best"
  typedef std::pair<double, unsigned int> ValueIndexPair;
  auto betterThan = [this](const ValueIndexPair & a, const ValueIndexPair & b) -> bool {
    // always sort NaN to the high ranks: it's never a good thing to have nan in front
    if (std::isnan(a.first)) return false;
    if (std::isnan(b.first)) return true;
    if (m_selectLowest)
      return a.first < b.first;
    else
      return a.first > b.first;
  };

  // create list of particle index and the corresponding value of variable
  std::vector<ValueIndexPair> valueToIndex;
  const unsigned int numParticles = m_inputList->getListSize();
  valueToIndex.reserve(numParticles);
  for (const Particle& p : *m_inputList) {
    double value = m_variable->function(&p);
    valueToIndex.emplace_back(value, p.getArrayIndex());
  }

  // use stable sort to make sure we keep the relative order of elements with
  // same value as it was before
  std::stable_sort(valueToIndex.begin(), valueToIndex.end(), betterThan);

  // assign ranks and (optionally) remove everything but best candidates
  m_inputList->clear();
  int rank{1};
  double previous_val{0};
  bool first_candidate{true};
  for (const auto& candidate : valueToIndex) {
    Particle* p = particles[candidate.second];
    if (!m_cut->check(p)) {
      p->addExtraInfo(m_outputVariableName, -1);
      m_inputList->addParticle(p);
      continue;
    }
    if (first_candidate) {
      first_candidate = false;
    } else {
      if (!m_allowMultiRank || (candidate.first != previous_val))  ++rank;
    }

    p->addExtraInfo(m_outputVariableName, rank);
    m_inputList->addParticle(p);

    previous_val = candidate.first;

    if (m_numBest != 0 and rank > m_numBest)
      break;
  }
}
