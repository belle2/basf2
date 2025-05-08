/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

std::string Belle2::EvtPDLUtil::antiParticleListName(int pdgCode, const std::string& label)
{
  if (label.empty())
    return antiParticleName(pdgCode);
  return antiParticleName(pdgCode) + ":" + label;
}

std::string Belle2::EvtPDLUtil::particleListName(int pdgCode, const std::string& label)
{
  if (label.empty())
    return particleName(pdgCode);
  return particleName(pdgCode) + ":" + label;
}

double Belle2::EvtPDLUtil::charge(int pdgCode)
{
  return TDatabasePDG::Instance()->GetParticle(pdgCode)->Charge() / 3.0;
}
