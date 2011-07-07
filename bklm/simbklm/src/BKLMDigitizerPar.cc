/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <bklm/simbklm/BKLMDigitizerPar.h>

#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

BKLMDigitizerPar* BKLMDigitizerPar::m_BKLMDigitizerParDB = 0;

BKLMDigitizerPar* BKLMDigitizerPar::Instance()
{
  if (!m_BKLMDigitizerParDB) m_BKLMDigitizerParDB = new BKLMDigitizerPar();
  return m_BKLMDigitizerParDB;
}

BKLMDigitizerPar::BKLMDigitizerPar()
{
  clear();
  read();
}

BKLMDigitizerPar::~BKLMDigitizerPar()
{
}

void BKLMDigitizerPar::clear()
{
  m_hitTimeMax = 0.0;
}

void BKLMDigitizerPar::read()
{
  // Get Gearbox parameters for BKLM
  GearDir content = Gearbox::Instance().getContent("BKLM");

  // Maximum global hit time for a recorded sim hit
  m_hitTimeMax = content.getParamAngle("HitTimeMax");

}
