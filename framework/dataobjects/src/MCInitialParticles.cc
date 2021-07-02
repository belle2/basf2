/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
