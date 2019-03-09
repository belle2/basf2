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
  setDescription(R"DOC(Sort particles by the value of a given ``variable``
in the input list and optionally remove particles after the nth position

Particles will receive an extra-info field containing their rank as an integer
starting at 1 (best). The name of this extra-info field defaults to
``${variable}_rank`` but can be chosen freely using the ``outputVariable``
parameter.

The ranking also takes antiparticles into account, so there will only be one
B+- candidate with `rank=1`.  The remaining list is sorted from best to worst
candidate (each charge, e.g. B+/B-, separately). The sorting is guaranteed
to be stable between particle and anti particle list: particles with the same
value for ``variable`` will keep their relative order. That is, a particle "A"
which was before another particle "B" in the same list and has the same value
for ``variable`` will also stay before "B" after sorting.

If ``allowMultiRank=False`` (the default) candidates with same value of
``variable`` will have different ranks. If ``allowMultiRank=True`` they will
share the same rank.

IF ``numBest>0`` only candidates with this rank or better will remain in the
output list. If ``allowMultiRank=True`` that means that there can be more than
``numBest`` candidates in the output list if they share ranks.
)DOC");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("particleList", m_inputListName, "Name of the ParticleList to rank for best candidate");
  addParam("variable", m_variableName, "Variable which defines the candidate ranking (see ``selectLowest`` for ordering)");
  addParam("selectLowest", m_selectLowest, "If true, candidate with lower values of ``variable`` are better, otherwise higher is better", false);
  addParam("allowMultiRank", m_allowMultiRank, "If true, candidates with identical values get identical rank", false);
  addParam("numBest", m_numBest, "Keep only particles with this rank or better. If ``allowMultiRank=False`` this is "
           "identical to the maximum amount of candidates left in the list. Otherwise there may be more candidates if "
           "some share the same rank (0: keep all)", 0);
  addParam("cut", m_cutParameter, "Only candidates passing the cut will be ranked. The others will have rank -1.", std::string(""));
  addParam("outputVariable", m_outputVariableName,
           "Name for created variable, which contains the rank for the particle. If not provided, the standard name ``${variable}_rank`` is used.");

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

    if (m_numBest != 0 and rank >= m_numBest)
      break;
  }
}
