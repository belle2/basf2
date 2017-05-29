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
  m_strLabel(""),
  m_iPDGCode(0)
{}

DecayDescriptorParticle::DecayDescriptorParticle(const DecayDescriptorParticle& other) :
  m_strName(other.m_strName),
  m_isSelected(other.m_isSelected),
  m_strLabel(other.m_strLabel),
  m_iPDGCode(other.m_iPDGCode)
{}

bool DecayDescriptorParticle::init(const DecayStringParticle& p)
{
  // Set member variables from the information in the DecayStringParticle p
  m_strName = p.m_strName;
  m_isSelected = !p.m_strSelector.empty();
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
