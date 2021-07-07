/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
