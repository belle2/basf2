/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

#include <analysis/utility/EvtPDLUtil.h>

#include <stdexcept>

using namespace Belle2;

std::string ParticleListName::antiParticleListName(const std::string& listName)
{
  DecayDescriptor decayDescriptor;
  bool valid = decayDescriptor.init(listName);
  if (!valid)
    throw std::runtime_error("Invalid ParticleList name: '" + listName + "' Should be EVTPDLNAME[:LABEL], e.g. B+ or B+:mylist.");

  int nProducts = decayDescriptor.getNDaughters();
  if (nProducts > 0)
    throw std::runtime_error("Invalid ParticleList name " + listName
                             + ". DecayString should not contain any daughters, only the mother particle.");

  const DecayDescriptorParticle* mother = decayDescriptor.getMother();
  const int pdgCode = mother->getPDGCode();

  bool isSelfConjugatedParticle = !(EvtPDLUtil::hasAntiParticle(pdgCode));
  if (isSelfConjugatedParticle)
    return listName;
  else
    return EvtPDLUtil::antiParticleListName(pdgCode, mother->getLabel());
}

