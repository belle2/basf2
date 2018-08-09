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

#include <arich/dbobjects/ARICHBiasCablesMapping.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

ARICHBiasCablesMapping::ARICHBiasCablesMapping()
{
}

int ARICHBiasCablesMapping::getModuleID(int sectorID, int cableID, int innerID) const
{
  if (sectorID > 6 || sectorID < 1) { B2WARNING("ARICHBiasCablesMapping::getcableID: Sector number " << sectorID << " not valid!"); return 0;}
  if (cableID > 8 || cableID < 1) { B2WARNING("ARICHBiasCablesMapping::getcableID: Cable number " << cableID << " not valid!"); return 0;}
  if (innerID > 9 || innerID < 1) { B2WARNING("ARICHBiasCablesMapping::getcableID: Inner cable number " << innerID << " not valid!"); return 0;}

  int module = 0;
  std::vector<int> position{sectorID, cableID, innerID};
  for (auto& i : m_module2cable) {
    if (i.second == position) module = i.first;
  }
  return module;
}

int ARICHBiasCablesMapping::getSectorID(int moduleID) const
{
  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHBiasCablesMapping::getcableID: Module number " << moduleID << " not valid!"); return 0;}
  std::vector<int> sector = m_module2cable.find(moduleID)->second;
  return (int)sector[0];
}

int ARICHBiasCablesMapping::getCableID(int moduleID) const
{
  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHBiasCablesMapping::getcableID: Module number " << moduleID << " not valid!"); return 0;}
  std::vector<int> cable = m_module2cable.find(moduleID)->second;
  return (int)cable[1];
}

int ARICHBiasCablesMapping::getInnerID(int moduleID) const
{
  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHBiasCablesMapping::getcableID: Module number " << moduleID << " not valid!"); return 0;}
  std::vector<int> inner = m_module2cable.find(moduleID)->second;
  return (int)inner[2];
}

void ARICHBiasCablesMapping::addMapping(int moduleID, int sectorID, int cableID, int innerID)
{

  if (moduleID > 420 || moduleID < 1) { B2WARNING("ARICHBiasCablesMapping::addMapping: Module number " << moduleID << " not valid!"); }
  if (sectorID > 6 || sectorID < 1) { B2WARNING("ARICHBiasCablesMapping::addMapping: Sector number " << sectorID << " not valid!"); }
  if (cableID > 8 || cableID < 1) { B2WARNING("ARICHBiasCablesMapping::addMapping: Cable number " << cableID << " not valid!"); }
  if (innerID > 9 || innerID < 1) { B2WARNING("ARICHBiasCablesMapping::addMapping: Inner cable number " << innerID << " not valid!"); }

  std::vector<int> cableMap{sectorID, cableID, innerID};
  m_module2cable.insert(std::pair<int, std::vector<int>>(moduleID, cableMap));

}

void ARICHBiasCablesMapping::print() const
{

  for (int moduleID = 1; moduleID < N_MODULES + 1; moduleID++) {
    cout << " Module " << moduleID << ": sector " << getSectorID(moduleID) << ", cable " << getCableID(
           moduleID) << ", inner " << getInnerID(moduleID) << endl;
  }

  cout << endl;
  cout << "Moving to finding module number... " << endl;
  int l = 0;

  for (int i = 1; i < 7 ; i++) {
    for (int j = 1; j < 9 ; j++) {
      for (int k = 1; k < 10; k++) {
        if ((j == 2 || j == 6) && k == 9) continue;
        std::cout << " Module: " << getModuleID(i, j, k) << ": sector " << i << ", cable " << j << ", inner " << k << std::endl;
        l++;
      }
    }
  }

  cout << "number of modules = " << l << endl;

}

