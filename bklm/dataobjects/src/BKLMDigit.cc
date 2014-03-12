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

ClassImp(BKLMDigit)

//! empty constructor
BKLMDigit::BKLMDigit() : RelationsObject()
{
}

//! Constructor with initial values for an RPC hit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit, int strip) :
  RelationsObject(),
  m_Status(simHit->getStatus()),
  m_PDG(simHit->getPDG()),
  m_IsForward(simHit->isForward()),
  m_Sector(simHit->getSector()),
  m_Layer(simHit->getLayer()),
  m_IsPhiReadout(simHit->isPhiReadout()),
  m_Strip(strip),
  m_ModuleID(simHit->getModuleID()),
  m_SimGlobalPosition(simHit->getGlobalPosition()),
  m_SimLocalPosition(simHit->getLocalPosition()),
  m_SimTime(simHit->getTime()),
  m_Time(m_SimTime),
  m_SimEDep(simHit->getEDep()),
  m_EDep(m_SimEDep),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_FitStatus(0)
{
}

//! Constructor with initial values for a scint hit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit) :
  RelationsObject(),
  m_Status(simHit->getStatus()),
  m_PDG(simHit->getPDG()),
  m_IsForward(simHit->isForward()),
  m_Sector(simHit->getSector()),
  m_Layer(simHit->getLayer()),
  m_IsPhiReadout(simHit->isPhiReadout()),
  m_Strip(simHit->getStrip()),
  m_ModuleID(simHit->getModuleID()),
  m_SimGlobalPosition(simHit->getGlobalPosition()),
  m_SimLocalPosition(simHit->getLocalPosition()),
  m_SimTime(simHit->getTime()),
  m_Time(m_SimTime),
  m_SimEDep(simHit->getEDep()),
  m_EDep(m_SimEDep),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_FitStatus(0)
{
}


//! Copy constructor
BKLMDigit::BKLMDigit(const BKLMDigit& digit) :
  RelationsObject(digit),
  m_Status(digit.m_Status),
  m_PDG(digit.m_PDG),
  m_IsForward(digit.m_IsForward),
  m_Sector(digit.m_Sector),
  m_Layer(digit.m_Layer),
  m_IsPhiReadout(digit.m_IsPhiReadout),
  m_Strip(digit.m_Strip),
  m_ModuleID(digit.m_ModuleID),
  m_SimGlobalPosition(digit.m_SimGlobalPosition),
  m_SimLocalPosition(digit.m_SimLocalPosition),
  m_SimTime(digit.m_SimTime),
  m_Time(digit.m_Time),
  m_SimEDep(digit.m_SimEDep),
  m_EDep(digit.m_EDep),
  m_SimNPixel(digit.m_SimNPixel),
  m_NPixel(digit.m_NPixel),
  m_FitStatus(digit.m_FitStatus)
{
}
