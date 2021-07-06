/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <array>
#include <string>
#include <bitset>
#include <numeric>

namespace Belle2 {
  namespace CDCTrigger {
    constexpr int tsInfoWidth = 21;

    using tsOut = std::array<unsigned, 4>;
    using tsOutArray = std::array<tsOut, 5>;
    tsOut decodeTSHit(std::string tsIn)
    {
      constexpr unsigned lenID = 8;
      constexpr unsigned lenPriorityTime = 9;
      constexpr unsigned lenLR = 2;
      constexpr unsigned lenPriorityPosition = 2;
      constexpr std::array<unsigned, 4> tsLens = {lenID, lenPriorityTime, lenLR, lenPriorityPosition};
      std::array<unsigned, 5> tsPos = { 0 };
      std::partial_sum(tsLens.begin(), tsLens.end(), tsPos.begin() + 1);
      tsOut tsOutput;
      tsOutput[0] = std::bitset<tsLens[0]>(tsIn.substr(tsPos[0], tsLens[0])).to_ulong();
      tsOutput[1] = std::bitset<tsLens[1]>(tsIn.substr(tsPos[1], tsLens[1])).to_ulong();
      tsOutput[2] = std::bitset<tsLens[2]>(tsIn.substr(tsPos[2], tsLens[2])).to_ulong();
      tsOutput[3] = std::bitset<tsLens[3]>(tsIn.substr(tsPos[3], tsLens[3])).to_ulong();
      return tsOutput;
    }

    static constexpr std::array<int, 9> nWiresInSuperLayer = {160, 160, 192, 224, 256, 288, 320, 352, 384};

    static constexpr int nTrackers = 4;

    /**
     *  Calculate the global TS ID from the ID in a super layer
     *
     *  @param localID     Segment ID in TSF output
     *
     *  @param iSL         Super layer ID (0-8)
     */
    unsigned short globalSegmentID(unsigned short localID, unsigned short iSL)
    {
      auto itr = nWiresInSuperLayer.begin();
      unsigned short globalID = std::accumulate(itr, itr + iSL, 0);
      globalID += localID;
      return globalID;
    }
  }
}
