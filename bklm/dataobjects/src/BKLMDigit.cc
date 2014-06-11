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

// empty constructor for ROOT - do not use this
BKLMDigit::BKLMDigit() :
  RelationsObject(),
  m_ModuleID(0),
  m_SimTime(0.0),
  m_Time(0.0),
  m_SimEDep(0.0),
  m_EDep(0.0),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_FitStatus(0)
{
}

// Constructor with initial values for an RPC hit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit, int strip) :
  RelationsObject(),
  m_SimTime(simHit->getTime()),
  m_Time(m_SimTime),
  m_SimEDep(simHit->getEDep()),
  m_EDep(m_SimEDep),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_FitStatus(0)
{
  m_ModuleID = simHit->getModuleID() & ~(BKLM_STRIP_MASK | BKLM_MAXSTRIP_MASK);
  m_ModuleID |= ((strip - 1) << BKLM_STRIP_BIT);
  m_ModuleID |= ((strip - 1) << BKLM_MAXSTRIP_BIT);
}

// Constructor with initial values for a scint hit
BKLMDigit::BKLMDigit(const BKLMSimHit* simHit) :
  RelationsObject(),
  m_ModuleID(simHit->getModuleID()),
  m_SimTime(simHit->getTime()),
  m_Time(m_SimTime),
  m_SimEDep(simHit->getEDep()),
  m_EDep(m_SimEDep),
  m_SimNPixel(0),
  m_NPixel(0.0),
  m_FitStatus(0)
{
}

// Copy constructor
BKLMDigit::BKLMDigit(const BKLMDigit& digit) :
  RelationsObject(digit),
  m_ModuleID(digit.m_ModuleID),
  m_SimTime(digit.m_SimTime),
  m_Time(digit.m_Time),
  m_SimEDep(digit.m_SimEDep),
  m_EDep(digit.m_EDep),
  m_SimNPixel(digit.m_SimNPixel),
  m_NPixel(digit.m_NPixel),
  m_FitStatus(digit.m_FitStatus)
{
}
