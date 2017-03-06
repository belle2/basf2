/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 **************************************************************************/

#include <analysis/utility/DecayForest.h>

#include <framework/logging/Logger.h>

#include <iostream>
#include <sstream>
#include <cassert>

using namespace Belle2;


DecayForest::DecayForest(const std::string& full_decaystring, bool save_memory)
{

  m_first_valid_original = 0;
  unsigned int start = 0;
  for (unsigned int i = 0; i < full_decaystring.size(); ++i) {
    if (full_decaystring[i] == '|') {
      forest.emplace_back(full_decaystring.substr(start, i - start));
      start = i + 1;
      if (save_memory and forest.back().isValid() and forest.size() > 1)
        break;
    }
  }

  if (not(save_memory and forest.back().isValid() and forest.size() > 1))
    forest.emplace_back(full_decaystring.substr(start, full_decaystring.size() - start));

  for (unsigned int j = 1; j < forest.size(); ++j) {
    if (forest[j].isValid()) {
      m_first_valid_original = j;
      break;
    }
  }

}


int DecayForest::decayHashFloatToInt(float decayHash, float decayHashExtended)
{

  // Convert unsigned int decay hash into a float keeping the same bit pattern
  assert(sizeof(float) == sizeof(uint32_t));

  union convert {
    uint32_t i;
    float f;
  };
  convert bitconverter;

  uint64_t integer = 0;

  bitconverter.f = decayHash;
  integer = static_cast<uint64_t>(bitconverter.i) << 32;
  bitconverter.f = decayHashExtended;
  integer += bitconverter.i;

  return integer;

}
