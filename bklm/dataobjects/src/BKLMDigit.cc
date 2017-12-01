/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMSimHit.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

// empty constructor for ROOT - do not use this
BKLMDigit::BKLMDigit() :
  DigitBase(),
  m_CTime(0),
  m_ModuleID(0),
  m_SimTime(0.0),
  m_Time(0.0),
  m_SimEDep(0.0),
  m_EDep(0.0),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_Charge(0.0),
  m_FitStatus(0)
{
}

// Constructor with initial values for an RPC simHit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit, int strip) :
  DigitBase(),
  m_CTime(0),
  m_SimTime(simHit->getTime()),
  m_Time(m_SimTime + simHit->getPropagationTime()),
  m_SimEDep(simHit->getEDep()),
  m_EDep(m_SimEDep),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_Charge(0.0),
  m_FitStatus(0)
{
  m_ModuleID = simHit->getModuleID() & ~(BKLM_STRIP_MASK | BKLM_MAXSTRIP_MASK);
  m_ModuleID |= ((strip - 1) << BKLM_STRIP_BIT);
  m_ModuleID |= ((strip - 1) << BKLM_MAXSTRIP_BIT);
}

BKLMDigit::BKLMDigit(int moduleID, int ctime, short tdc, short charge) :
  DigitBase(),
  m_SimTime(0.0),
  m_SimEDep(0.0),
  m_EDep(0.0),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_FitStatus(0)
{
  //this assaumes that the strip is already zero based...
  m_ModuleID = moduleID;
  m_CTime = ctime;
  m_Time = tdc;
  //m_NPixel = charge;
  m_Charge = charge;

}

// Constructor with initial values for a scint simHit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit) :
  DigitBase(),
  m_CTime(0),
  m_ModuleID(simHit->getModuleID()),
  m_SimTime(simHit->getTime()),
  m_Time(m_SimTime + simHit->getPropagationTime()),
  m_SimEDep(simHit->getEDep()),
  m_EDep(m_SimEDep),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_Charge(0.0),
  m_FitStatus(0)
{
}

// Copy constructor
BKLMDigit::BKLMDigit(const BKLMDigit& digit) :
  DigitBase(digit),
  m_CTime(digit.m_CTime),
  m_ModuleID(digit.m_ModuleID),
  m_SimTime(digit.m_SimTime),
  m_Time(digit.m_Time),
  m_SimEDep(digit.m_SimEDep),
  m_EDep(digit.m_EDep),
  m_SimNPixel(digit.m_SimNPixel),
  m_NPixel(digit.m_NPixel),
  m_Charge(digit.m_Charge),
  m_FitStatus(digit.m_FitStatus)
{
}

// Assignment operator
BKLMDigit& BKLMDigit::operator=(const BKLMDigit& digit)
{
  m_CTime = digit.m_CTime;
  m_ModuleID = digit.m_ModuleID;
  m_SimTime = digit.m_SimTime;
  m_Time = digit.m_Time;
  m_SimEDep = digit.m_SimEDep;
  m_EDep = digit.m_EDep;
  m_SimNPixel = digit.m_SimNPixel;
  m_NPixel = digit.m_NPixel;
  m_Charge = digit.m_Charge;
  m_FitStatus = digit.m_FitStatus;
  return *this;
}

DigitBase::EAppendStatus BKLMDigit::addBGDigit(const DigitBase* bg)
{
  const BKLMDigit* bgDigit = static_cast<const BKLMDigit*>(bg);

  //the below is copied from the EKLMDigits (minus the stuff we don't have..)

  // isGood would need status implementation
  //  if (!bgDigit->isGood())
  //    return DigitBase::c_DontAppend;
  //  if (!this->isGood())
  //    return DigitBase::c_Append;

  //in rpc energy deposits are not additive
  if (!bgDigit->inRPC()) {
    if (this->getEDep() < bgDigit->getEDep()) {
      //do something
    }
    this->setEDep(this->getEDep() + bgDigit->getEDep());
  }
  //is the timing actually affected in the scinti layers if the first hit is below threshold?
  //i.e. should we check if it is above?
  if (this->getTime() > bgDigit->getTime())
    this->setTime(bgDigit->getTime());

  //  this->setNPE(this->getNPE() + bgDigit->getNPE());
  //  this->setGeneratedNPE(this->getGeneratedNPE() + bgDigit->getGeneratedNPE());



  return DigitBase::c_DontAppend;
}


unsigned int BKLMDigit::getUniqueChannelID() const
{
  return m_ModuleID;;
}
