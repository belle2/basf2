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
  m_good = false;
  m_NPE = -1;
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
  m_good = false;
  m_NPE = -1;
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
  this->setNPE(this->getNPE() + bgDigit->getNPE());
  this->setGeneratedNPE(this->getGeneratedNPE() + bgDigit->getGeneratedNPE());
  return DigitBase::c_DontAppend;
}

float EKLMDigit::getNPE() const
{
  return m_NPE;
}

void EKLMDigit::setNPE(float npe)
{
  m_NPE = npe;
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
  return m_good;
}

void EKLMDigit::isGood(bool status)
{
  m_good = status;
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

