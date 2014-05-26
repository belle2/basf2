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

#include <evtgen/EvtGenBase/EvtPDL.hh>
#include <evtgen/EvtGenBase/EvtId.hh>

#include <framework/core/Environment.h>

bool Belle2::EvtPDLUtil::hasAntiParticle(int pdgCode)
{
  initEvtPDL();

  EvtId particle     = EvtPDL::evtIdFromStdHep(pdgCode);
  EvtId antiParticle = EvtPDL::chargeConj(particle);

  if (EvtPDL::getStdHep(antiParticle) == 0)
    return false;

  return particle != antiParticle;
}

std::string Belle2::EvtPDLUtil::particleName(int pdgCode)
{
  initEvtPDL();
  return EvtPDL::name(EvtPDL::evtIdFromStdHep(pdgCode));
}

std::string Belle2::EvtPDLUtil::antiParticleName(int pdgCode)
{
  initEvtPDL();

  EvtId particle     = EvtPDL::evtIdFromStdHep(pdgCode);
  EvtId antiParticle = EvtPDL::chargeConj(particle);

  if (EvtPDL::getStdHep(antiParticle) == 0)
    return EvtPDL::name(particle);

  return EvtPDL::name(antiParticle);
}

void Belle2::EvtPDLUtil::initEvtPDL()
{
  if (EvtPDL::entries() != 0)
    return;

  // EvtPDL is not initialized yet
  std::string strEvtPDL = Environment::Instance().getExternalsPath() + "/share/evtgen/evt.pdl";
  EvtPDL evtpdl;
  evtpdl.read(strEvtPDL.c_str());
}

std::string Belle2::EvtPDLUtil::antiParticleListName(int pdgCode, std::string label)
{
  return antiParticleName(pdgCode) + ":" + label;
}

std::string Belle2::EvtPDLUtil::particleListName(int pdgCode, std::string label)
{
  return particleName(pdgCode) + ":" + label;
}

