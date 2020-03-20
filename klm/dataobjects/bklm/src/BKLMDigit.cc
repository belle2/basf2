/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/bklm/BKLMDigit.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMSimHit.h>

using namespace Belle2;

// empty constructor for ROOT - do not use this
BKLMDigit::BKLMDigit() :
  KLMDigit(),
  m_SimEnergyDeposit(0.0)
{
}

// Constructor with initial values for an RPC simHit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit, int strip) :
  KLMDigit(),
  m_SimEnergyDeposit(simHit->getEnergyDeposit())
{
  m_Section = simHit->getSection();
  m_Sector = simHit->getSector();
  m_Layer = simHit->getLayer();
  m_Plane = simHit->getPlane();
  m_Strip = strip;
  m_Time = simHit->getTime() + simHit->getPropagationTime();
  m_EnergyDeposit = m_SimEnergyDeposit;
  m_MCTime = simHit->getTime();
}

// Constructor with initial values for a scint simHit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit) :
  KLMDigit(),
  m_SimEnergyDeposit(simHit->getEnergyDeposit())
{
  m_Section = simHit->getSection();
  m_Sector = simHit->getSector();
  m_Layer = simHit->getLayer();
  m_Plane = simHit->getPlane();
  m_Strip = simHit->getStrip();
  m_Time = simHit->getTime() + simHit->getPropagationTime();
  m_EnergyDeposit = m_SimEnergyDeposit;
  m_MCTime = simHit->getTime();
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
    if (this->getEnergyDeposit() < bgDigit->getEnergyDeposit()) {
      //do something
    }
    this->setEnergyDeposit(this->getEnergyDeposit() + bgDigit->getEnergyDeposit());
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
  /* FIXME: reimplement after merging. */
  return 1;
}
