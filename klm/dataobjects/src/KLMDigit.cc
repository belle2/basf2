
/* Own header. */
#include <klm/dataobjects/KLMDigit.h>

using namespace Belle2;

KLMDigit::KLMDigit() :
  m_Section(0),
  m_Sector(0),
  m_Layer(0),
  m_Plane(0),
  m_Strip(0),
  m_Charge(0),
  m_CTime(0),
  m_TDC(0),
  m_EnergyDeposit(0),
  m_NPE(0),
  m_GeneratedNPE(0),
  m_FitStatus(0),
  m_MCTime(0),
  m_sMCTime(0)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMDigit::KLMDigit(const EKLMSimHit* simHit) :
  m_Section(simHit->getSection()),
  m_Sector(simHit->getSector()),
  m_Layer(simHit->getLayer()),
  m_Plane(simHit->getPlane()),
  m_Strip(simHit->getStrip()),
  m_Charge(0),
  m_CTime(0),
  m_TDC(0),
  m_Time(0),
  m_EnergyDeposit(simHit->getEnergyDeposit()),
  m_NPE(0),
  m_GeneratedNPE(0),
  m_FitStatus(0),
  m_MCTime(simHit->getTime()),
  m_sMCTime(0)
{
}

KLMDigit::KLMDigit(const BKLMSimHit* simHit, int strip) :
  m_Section(simHit->getSection()),
  m_Sector(simHit->getSector()),
  m_Layer(simHit->getLayer()),
  m_Plane(simHit->getPlane()),
  m_Strip(strip),
  m_Charge(0),
  m_CTime(0),
  m_TDC(0),
  m_Time(simHit->getTime() + simHit->getPropagationTime()),
  m_EnergyDeposit(simHit->getEnergyDeposit()),
  m_NPE(0),
  m_GeneratedNPE(0),
  m_FitStatus(0),
  m_MCTime(simHit->getTime()),
  m_sMCTime(0)
{
}

KLMDigit::KLMDigit(const BKLMSimHit* simHit) :
  m_Section(simHit->getSection()),
  m_Sector(simHit->getSector()),
  m_Layer(simHit->getLayer()),
  m_Plane(simHit->getPlane()),
  m_Strip(simHit->getStrip()),
  m_Charge(0),
  m_CTime(0),
  m_TDC(0),
  m_Time(simHit->getTime() + simHit->getPropagationTime()),
  m_EnergyDeposit(simHit->getEnergyDeposit()),
  m_NPE(0),
  m_GeneratedNPE(0),
  m_FitStatus(0),
  m_MCTime(simHit->getTime()),
  m_sMCTime(0)
{
}

