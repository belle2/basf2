
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
  m_sMCTime(-1)
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
  m_sMCTime(-1)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}
