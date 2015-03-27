/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/NamedFloatTuple.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

NamedFloatTuple::NamedFloatTuple(const string& prefix) :
  m_prefix(prefix)
{
}

NamedFloatTuple::~NamedFloatTuple()
{
}

std::string NamedFloatTuple::getNameWithPrefix(int iValue) const
{
  return m_prefix + getName(iValue);
}

std::map<std::string, Float_t> NamedFloatTuple::getNamedValues() const
{
  std::map<std::string, Float_t> namedValues;
  size_t nValues = size();

  for (size_t iValue = 0; iValue < nValues; ++iValue) {
    std::string name = getName(iValue);
    Float_t value = get(iValue);
    namedValues[name] = value;
  }

  return namedValues;
}

std::map<std::string, Float_t> NamedFloatTuple::getNamedValuesWithPrefix() const
{
  std::map<std::string, Float_t> namedValues;
  size_t nValues = size();

  for (size_t iValue = 0; iValue < nValues; ++iValue) {
    std::string name = getNameWithPrefix(iValue);
    Float_t value = get(iValue);
    namedValues[name] = value;
  }

  return namedValues;
}
