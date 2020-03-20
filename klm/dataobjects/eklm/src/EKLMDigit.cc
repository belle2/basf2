/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMDigit.h>

/* KLM headers. */
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>

using namespace Belle2;

EKLMDigit::EKLMDigit() : KLMDigit()
{
}

EKLMDigit::EKLMDigit(const EKLMSimHit* hit) : KLMDigit(hit)
{
}

unsigned int EKLMDigit::getUniqueChannelID() const
{
  return m_ElementNumbers->stripNumber(m_Section, m_Layer, m_Sector, m_Plane,
                                       m_Strip);
}

DigitBase::EAppendStatus EKLMDigit::addBGDigit(const DigitBase* bg)
{
  const EKLMDigit* bgDigit = (EKLMDigit*)bg;
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
