/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

using namespace Belle2;
using namespace TrackFindingCDC;

NamedFloatTuple::~NamedFloatTuple() = default;

std::map<std::string, Float_t> NamedFloatTuple::getNamedValues(std::string prefix) const
{
  std::map<std::string, Float_t> namedValues;
  size_t nVars = size();

  for (size_t iVar = 0; iVar < nVars; ++iVar) {
    std::string name = prefix + getName(iVar);
    Float_t value = get(iVar);
    namedValues[name] = value;
  }

  return namedValues;
}

std::vector<Named<Float_t*> > NamedFloatTuple::getNamedVariables(std::string prefix)
{
  std::vector<Named<Float_t*> > namedVariables;
  int nVars = size();
  namedVariables.reserve(nVars);

  for (int iVar = 0; iVar < nVars; ++iVar) {
    std::string name = prefix + getName(iVar);
    Float_t& value = operator[](iVar);
    namedVariables.emplace_back(name, &value);
  }

  return namedVariables;
}
