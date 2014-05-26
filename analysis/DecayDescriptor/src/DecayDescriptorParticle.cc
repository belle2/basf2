#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <framework/core/Environment.h>
#include <framework/logging/Logger.h>
#include <evtgen/EvtGenBase/EvtPDL.hh>
#include <evtgen/EvtGenBase/EvtId.hh>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace Belle2;
using namespace std;
using namespace boost::algorithm;

DecayDescriptorParticle::DecayDescriptorParticle() :
  m_strName(""),
  m_iSelected(0),
  m_strLabel(""),
  m_iPDGCode(0)
{}

DecayDescriptorParticle::DecayDescriptorParticle(const DecayDescriptorParticle& other) :
  m_strName(other.m_strName),
  m_iSelected(other.m_iSelected),
  m_strLabel(other.m_strLabel),
  m_iPDGCode(other.m_iPDGCode)
{}

bool DecayDescriptorParticle::init(const DecayStringParticle& p)
{
  // Set member variables from the information in the DecayStringParticle p
  m_strName = p.m_strName;
  m_iSelected = p.m_strSelector.empty() ? 0 : 1;
  if (!p.m_strLabel.empty()) m_strLabel = p.m_strLabel;
  // Determine PDG code using the particle names defined in evt.pdl
  if (EvtPDL::entries() == 0) {
    string strEvtPDL = Environment::Instance().getExternalsPath() + "/share/evtgen/evt.pdl";
    EvtPDL evtpdl;
    evtpdl.read(strEvtPDL.c_str());
  }
  EvtId evtId = EvtPDL::getId(m_strName);
  if (evtId.getId() == -1) {
    B2WARNING("Particle not in evt.pdl file!" << m_strName);
    return false;
  }
  m_iPDGCode = EvtPDL::getStdHep(evtId);
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
  return strNameSimple;
}
