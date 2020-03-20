
/* Own header. */
#include <klm/dataobjects/KLMDigit.h>

using namespace Belle2;

KLMDigit::KLMDigit() :
  m_Plane(-1),
  m_Strip(-1),
  m_Charge(0),
  m_CTime(0),
  m_TDC(0),
  m_GeneratedNPE(-1),
  m_FitStatus(-1),
  m_MCTime(0),
  m_sMCTime(0)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMDigit::KLMDigit(const EKLMSimHit* hit) :
  m_Plane(hit->getPlane()),
  m_Strip(hit->getStrip()),
  m_Charge(0),
  m_CTime(0),
  m_TDC(0),
  m_GeneratedNPE(-1),
  m_FitStatus(-1),
  m_MCTime(hit->getTime()),
  m_sMCTime(0)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMDigit::KLMDigit(const BKLMSimHit* simHit, int strip) : KLMDigit()
{
  m_Section = simHit->getSection();
  m_Sector = simHit->getSector();
  m_Layer = simHit->getLayer();
  m_Plane = simHit->getPlane();
  m_Strip = strip;
  m_Time = simHit->getTime() + simHit->getPropagationTime();
  m_EnergyDeposit = simHit->getEnergyDeposit();
  m_MCTime = simHit->getTime();
}

KLMDigit::KLMDigit(const BKLMSimHit* simHit) : KLMDigit()
{
  m_Section = simHit->getSection();
  m_Sector = simHit->getSector();
  m_Layer = simHit->getLayer();
  m_Plane = simHit->getPlane();
  m_Strip = simHit->getStrip();
  m_Time = simHit->getTime() + simHit->getPropagationTime();
  m_EnergyDeposit = simHit->getEnergyDeposit();
  m_MCTime = simHit->getTime();
}

