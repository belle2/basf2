
/* Own header. */
#include <klm/dataobjects/KLMDigit.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMDigit::KLMDigit() :
  m_Subdetector(0),
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
  m_SiPMMCTime(0)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

KLMDigit::KLMDigit(const EKLMSimHit* simHit) :
  m_Subdetector(KLMElementNumbers::c_EKLM),
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
  m_SiPMMCTime(0)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

KLMDigit::KLMDigit(const BKLMSimHit* simHit, int strip) :
  m_Subdetector(KLMElementNumbers::c_BKLM),
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
  m_SiPMMCTime(0)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

KLMDigit::KLMDigit(const BKLMSimHit* simHit) :
  m_Subdetector(KLMElementNumbers::c_BKLM),
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
  m_SiPMMCTime(0)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

unsigned int KLMDigit::getUniqueChannelID() const
{
  return m_ElementNumbers->channelNumber(m_Subdetector, m_Section, m_Layer,
                                         m_Sector, m_Plane, m_Strip);
}

DigitBase::EAppendStatus KLMDigit::addBGDigit(const DigitBase* bg)
{
  const KLMDigit* bgDigit = (KLMDigit*)bg;
  if (!bgDigit->isGood())
    return DigitBase::c_DontAppend;
  if (!this->isGood())
    return DigitBase::c_Append;
  /* MC data from digit with larger energy. */
  if (this->getEnergyDeposit() < bgDigit->getEnergyDeposit())
    this->setMCTime(bgDigit->getMCTime());
  this->setEnergyDeposit(this->getEnergyDeposit() + bgDigit->getEnergyDeposit());
  if (this->getTime() > bgDigit->getTime())
    this->setTime(bgDigit->getTime());
  this->setCharge(std::min(this->getCharge(), bgDigit->getCharge()));
  this->setGeneratedNPE(this->getGeneratedNPE() + bgDigit->getGeneratedNPE());
  return DigitBase::c_DontAppend;
}

bool KLMDigit::isPhiReadout() const
{
  if (m_Subdetector != KLMElementNumbers::c_BKLM)
    B2FATAL("Function isPhiReadout() is called for EKLM digit.");
  return m_Plane == BKLMElementNumbers::c_PhiPlane;
}
