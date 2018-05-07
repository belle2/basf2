/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMDigit::EKLMDigit()
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_Plane = -1;
  m_Strip = -1;
  m_Charge = 0;
  m_CTime = 0;
  m_TDC = 0;
  m_generatedNPE = -1;
  m_fitStatus = -1;
  m_sMCTime = -1;
}

EKLMDigit::EKLMDigit(const EKLMSimHit* hit)
  : EKLMHitBase((EKLMHitBase)(*hit)),
    m_Plane(hit->getPlane()),
    m_Strip(hit->getStrip())
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_Charge = 0;
  m_CTime = 0;
  m_TDC = 0;
  m_generatedNPE = -1;
  m_fitStatus = -1;
  m_sMCTime = -1;
}

unsigned int EKLMDigit::getUniqueChannelID() const
{
  return m_ElementNumbers->stripNumber(m_Endcap, m_Layer, m_Sector, m_Plane,
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
  if (this->getEDep() < bgDigit->getEDep()) {
    this->setPDG(bgDigit->getPDG());
    this->setMCTime(bgDigit->getMCTime());
  }
  this->setEDep(this->getEDep() + bgDigit->getEDep());
  if (this->getTime() > bgDigit->getTime())
    this->setTime(bgDigit->getTime());
  this->setCharge(std::min(this->getCharge(), bgDigit->getCharge()));
  this->setGeneratedNPE(this->getGeneratedNPE() + bgDigit->getGeneratedNPE());
  return DigitBase::c_DontAppend;
}

uint16_t EKLMDigit::getCharge() const
{
  return m_Charge;
}

void EKLMDigit::setCharge(uint16_t charge)
{
  m_Charge = charge;
}

uint16_t EKLMDigit::getCTime() const
{
  return m_CTime;
}

void EKLMDigit::setCTime(uint16_t charge)
{
  m_CTime = charge;
}

uint16_t EKLMDigit::getTDC() const
{
  return m_TDC;
}

void EKLMDigit::setTDC(uint16_t tdc)
{
  m_TDC = tdc;
}

/*
 * TODO: the photoelectron / charge conversion constant should be determined
 * from calibration.
 */
float EKLMDigit::getNPE() const
{
  return float(m_Charge) / 32;
}

int EKLMDigit::getGeneratedNPE() const
{
  return m_generatedNPE;
}

void EKLMDigit::setGeneratedNPE(int npe)
{
  m_generatedNPE = npe;
}

bool EKLMDigit::isGood() const
{
  return m_fitStatus == EKLM::c_FPGASuccessfulFit;
}

int EKLMDigit::getPlane() const
{
  return m_Plane;
}

void EKLMDigit::setPlane(int plane)
{
  m_Plane = plane;
}

int EKLMDigit::getStrip() const
{
  return m_Strip;
}

void EKLMDigit::setStrip(int strip)
{
  m_Strip = strip;
}

void EKLMDigit::setFitStatus(int s)
{
  m_fitStatus = s;
}

int EKLMDigit::getFitStatus()
{
  return m_fitStatus;
}

float EKLMDigit::getSiPMMCTime() const
{
  return m_sMCTime;
}

void EKLMDigit::setSiPMMCTime(float t)
{
  m_sMCTime = t;
}

