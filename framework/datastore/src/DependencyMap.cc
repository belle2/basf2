/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/datastore/DependencyMap.h>
#include <framework/core/Module.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

std::string DependencyMap::getModuleID(const Module& mod)
{
  return mod.getType() + std::to_string(long(&mod));
}

void DependencyMap::ModuleInfo::addEntry(const std::string& name, EEntryType type, bool isRelation)
{
  if (isRelation)
    relations[type].insert(name);
  else
    entries[type].insert(name);
}

bool DependencyMap::isUsedAs(const std::string& branchName, EEntryType type) const
{
  return any_of(m_moduleInfo.begin(), m_moduleInfo.end(),
  [branchName, type](const pair<string, ModuleInfo>& info) {
    if (info.second.entries[type].count(branchName))
      return true;
    return (bool)info.second.relations[type].count(branchName);
  });
}
