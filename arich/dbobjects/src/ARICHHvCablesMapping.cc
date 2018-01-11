/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHHvCablesMapping.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

ARICHHvCablesMapping::ARICHHvCablesMapping()
{
}

int ARICHHvCablesMapping::getModuleID(int sectorID, int cableID, int innerID) const
{
  if (sectorID > 6 || sectorID < 1) { B2WARNING("ARICHHvCablesMapping::getcableID: Sector number " << sectorID << " not valid!"); return 0;}
  if (cableID > 4 || cableID < 1) { B2WARNING("ARICHHvCablesMapping::getcableID: Cable number " << cableID << " not valid!"); return 0;}
  if (innerID > 20 || innerID < 2) { B2WARNING("ARICHHvCablesMapping::getcableID: Inner cable number " << innerID << " not valid!"); return 0;}

  int module = 0;
  std::vector<int> position{sectorID, cableID, innerID};
  for (auto& i : m_module2cable) {
    if (i.second == position) module = i.first;
  }
  return module;
}

int ARICHHvCablesMapping::getSectorID(int moduleID) const
{
  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHHvCablesMapping::getcableID: Module number " << moduleID << " not valid!"); return 0;}
  std::vector<int> sector = m_module2cable.find(moduleID)->second;
  return (int)sector[0];
}

int ARICHHvCablesMapping::getCableID(int moduleID) const
{
  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHHvCablesMapping::getcableID: Module number " << moduleID << " not valid!"); return 0;}
  std::vector<int> cable = m_module2cable.find(moduleID)->second;
  return (int)cable[1];
}

int ARICHHvCablesMapping::getInnerID(int moduleID) const
{
  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHHvCablesMapping::getcableID: Module number " << moduleID << " not valid!"); return 0;}
  std::vector<int> inner = m_module2cable.find(moduleID)->second;
  return (int)inner[2];
}

void ARICHHvCablesMapping::addMapping(int moduleID, int sectorID, int cableID, int innerID)
{

  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHHvCablesMapping::addMapping: Module number " << moduleID << " not valid!"); }
  if (sectorID > 6 || sectorID < 1) { B2WARNING("ARICHHvCablesMapping::addMapping: Sector number " << sectorID << " not valid!"); }
  if (cableID > 8 || cableID < 1) { B2WARNING("ARICHHvCablesMapping::addMapping: Cable number " << cableID << " not valid!"); }
  if (innerID > 20 || innerID < 2) { B2WARNING("ARICHHvCablesMapping::addMapping: Inner cable number " << innerID << " not valid!"); }

  std::vector<int> cableMap{sectorID, cableID, innerID};
  m_module2cable.insert(std::pair<int, std::vector<int>>(moduleID, cableMap));

}

void ARICHHvCablesMapping::print()
{

  for (int moduleID = 1; moduleID < N_MODULES + 1; moduleID++) {
    cout << " Module " << moduleID << ": sector " << getSectorID(moduleID) << ", cable " << getCableID(
           moduleID) << ", inner " << getInnerID(moduleID) << endl;
  }

  cout << endl;
  cout << "Moving to finding module number... " << endl;
  int l = 0;

  for (int i = 1; i < 7 ; i++) {
    for (int j = 1; j < 5 ; j++) {
      for (int k = 2; k < 21; k++) {
        if (k == 7) continue;
        if ((j == 1 || j == 4) && k == 20) continue;
        std::cout << " Module: " << getModuleID(i, j, k) << ": sector " << i << ", cable " << j << ", inner " << k << std::endl;
        l++;
      }
    }
  }

  cout << "number of modules = " << l << endl;

}

