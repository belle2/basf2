/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/EvtPDLUtil.h>
#include <TDatabasePDG.h>

bool Belle2::EvtPDLUtil::hasAntiParticle(int pdgCode)
{
  TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(pdgCode);
  TParticlePDG* antiParticle = particle->AntiParticle();

  return antiParticle and (particle != antiParticle);
}

std::string Belle2::EvtPDLUtil::particleName(int pdgCode)
{
  return TDatabasePDG::Instance()->GetParticle(pdgCode)->GetName();
}

std::string Belle2::EvtPDLUtil::antiParticleName(int pdgCode)
{
  return TDatabasePDG::Instance()->GetParticle(pdgCode)->AntiParticle()->GetName();
}

std::string Belle2::EvtPDLUtil::antiParticleListName(int pdgCode, std::string label)
{
  if (label.empty())
    return antiParticleName(pdgCode);
  return antiParticleName(pdgCode) + ":" + label;
}

std::string Belle2::EvtPDLUtil::particleListName(int pdgCode, std::string label)
{
  if (label.empty())
    return particleName(pdgCode);
  return particleName(pdgCode) + ":" + label;
}

