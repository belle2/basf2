/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHMergerMapping.h>
#include <iostream>
#include<iomanip>

using namespace std;
using namespace Belle2;

ARICHMergerMapping:: ARICHMergerMapping()
{
  m_merger2module.assign(N_MERGERS * N_FEB2MERGER, 0);
  m_module2merger.assign(N_MERGERS * N_FEB2MERGER, 0);
  m_isMapped.assign(N_MERGERS * N_FEB2MERGER, 0);
}

unsigned ARICHMergerMapping::getModuleID(unsigned mergerID, unsigned febSlot) const
{
  if (!(mergerID > 0 && mergerID < N_MERGERS + 1 && febSlot > 0 && febSlot < N_FEB2MERGER + 1)) { B2ERROR("ARICHMergerMapping::getModuleID: invalid merger ID or febSlot!"); return 0;}
  unsigned id = (mergerID - 1) * N_FEB2MERGER + (febSlot - 1);
  return (unsigned)m_merger2module[id];
}

unsigned ARICHMergerMapping::getMergerID(unsigned moduleID) const
{
  if (!m_isMapped[moduleID - 1]) { B2WARNING("ARICHMergerMapping::getMergerID: Module " << moduleID << " not not mapped to merger!"); return 0;}
  unsigned id = (unsigned)m_module2merger[moduleID - 1] / N_FEB2MERGER + 1;
  return id;
}

unsigned ARICHMergerMapping::getFEBSlot(unsigned moduleID) const
{
  if (!m_isMapped[moduleID - 1]) { B2WARNING("ARICHMergerMapping::getMergerID: Module " << moduleID << " not not mapped to merger!"); return 0;}
  unsigned id = (unsigned)m_module2merger[moduleID - 1] % N_FEB2MERGER + 1;
  return id;
}

void ARICHMergerMapping::addMapping(unsigned moduleID, unsigned mergerID, unsigned febSlot)
{

  if (!(mergerID > 0 && mergerID < N_MERGERS + 1 && febSlot > 0 && febSlot < N_FEB2MERGER + 1
        && moduleID < N_MERGERS * N_FEB2MERGER)) { B2ERROR("ARICHMergerMapping::addMapping: invalid mergerID/febSlot/moduleID!"); return;}

  m_module2merger[moduleID - 1] = (uint16_t)((mergerID - 1) * N_FEB2MERGER + (febSlot - 1));
  m_merger2module[(mergerID - 1) * N_FEB2MERGER + (febSlot - 1)] = (uint16_t)moduleID;
  m_isMapped[moduleID - 1] = 1;
}

void ARICHMergerMapping::print()
{

  std::cout << std::endl;
  std::cout << "Mapping of modules to mergers" << std::endl;
  std::cout << std::endl;
  for (int i = 1; i < N_MERGERS + 1; i++) {
    bool isUsed = false;
    for (int j = 1; j < N_FEB2MERGER + 1; j++) {
      if (getModuleID(i, j)) { isUsed = true; break;}
    }

    if (!isUsed) continue;
    std::cout << " Modules connected to merger " << i << std::endl;
    for (int j = 1; j < N_FEB2MERGER + 1; j++) {
      if (getModuleID(i, j)) std::cout << "  FEB slot " << j << ": " << getModuleID(i, j) << std::endl;
      else std::cout << "  FEB slot " << j << " not connected" << std::endl;
    }

  }
}

