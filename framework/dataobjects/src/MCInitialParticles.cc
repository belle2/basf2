/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/MCInitialParticles.h>

using namespace Belle2;

std::string MCInitialParticles::getGenerationFlagString(const std::string& separator) const
{
  std::string flags = "";
  const std::vector<std::pair<int, std::string>> flagvalues{
    {c_generateCMS, "generateCMS"},
    {c_smearBeamEnergy, "smearBeamEnergy"},
    {c_smearBeamDirection, "smearBeamDirection"},
    {c_smearVertex, "smearVertex"}
  };
  for (auto& i : flagvalues) {
    if (hasGenerationFlags(i.first)) {
      if (flags.size() > 0) flags += separator;
      flags += i.second;
    }
  }
  return flags;
}
