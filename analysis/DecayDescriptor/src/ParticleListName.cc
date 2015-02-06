#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

#include <analysis/utility/EvtPDLUtil.h>

using namespace Belle2;

std::string ParticleListName::antiParticleListName(const std::string& listName)
{
  DecayDescriptor decayDescriptor;
  bool valid = decayDescriptor.init(listName);
  if (!valid)
    throw std::runtime_error("Invalid input listName: " + listName);

  int nProducts = decayDescriptor.getNDaughters();
  if (nProducts > 0)
    throw std::runtime_error("Invalid input listName " + listName
                             + ". DecayString should not contain any daughters, only the mother particle.");

  const DecayDescriptorParticle* mother = decayDescriptor.getMother();
  const int pdgCode = mother->getPDGCode();

  bool isSelfConjugatedParticle = !(EvtPDLUtil::hasAntiParticle(pdgCode));
  if (isSelfConjugatedParticle)
    return listName;
  else
    return EvtPDLUtil::antiParticleListName(pdgCode, mother->getLabel());
}

