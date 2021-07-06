/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <analysis/DecayDescriptor/DecayStringParticle.h>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <TDatabasePDG.h>

using namespace Belle2;
using namespace std;
using namespace boost::algorithm;

DecayDescriptorParticle::DecayDescriptorParticle() :
  m_strName(""),
  m_isSelected(false),
  m_properties(0),
  m_strLabel(""),
  m_iPDGCode(0)
{}

bool DecayDescriptorParticle::init(const DecayStringParticle& p)
{
  // Set member variables from the information in the DecayStringParticle p
  m_strName = p.m_strName;
  if (!p.m_strSelector.empty()) {
    m_isSelected    = (p.m_strSelector.find('^') != std::string::npos);

    if (p.m_strSelector.find('@') != std::string::npos)
      m_properties |= Particle::PropertyFlags::c_IsUnspecified;
    if (p.m_strSelector.find("(misID)") != std::string::npos)
      m_properties |= Particle::PropertyFlags::c_IsIgnoreMisID;
    if (p.m_strSelector.find("(decay)") != std::string::npos)
      m_properties |= Particle::PropertyFlags::c_IsIgnoreDecayInFlight;
  }
  if (!p.m_strLabel.empty()) m_strLabel = p.m_strLabel;
  // Determine PDG code using the particle names defined in evt.pdl
  TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(m_strName.c_str());
  if (!particle) {
    B2WARNING("Particle not in evt.pdl file! " << m_strName);
    return false;
  }
  m_iPDGCode = particle->PdgCode();
  return true;
}

string DecayDescriptorParticle::getNameSimple() const
{
  string strNameSimple(m_strName);
  erase_all(strNameSimple, "anti-");
  erase_all(strNameSimple, "+");
  erase_all(strNameSimple, "-");
  erase_all(strNameSimple, "/");
  erase_all(strNameSimple, "(");
  erase_all(strNameSimple, ")");
  replace_all(strNameSimple, "*", "ST");
  replace_all(strNameSimple, "'", "p");
  return strNameSimple;
}
