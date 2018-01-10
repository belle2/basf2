/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/cdc/modules/unpacker/CDCTriggerUnpackerModule.h>

#include <array>
#include <bitset>
#include <string>
#include <sstream>
#include <utility>
#include <numeric>
#include <iomanip>

using namespace Belle2;
using namespace CDCTriggerUnpacker;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

constexpr std::array<int, 9> CDCTriggerUnpackerModule::nMergers;

namespace Belle2 {
  struct Merger : SubTrigger {
    Merger(StoreArray<MergerBits>* inArrayPtr, std::string inName,
           unsigned inEventWidth, unsigned inOffset,
           unsigned inHeaderSize, std::pair<int, int> inNodeID,
           unsigned inNInnerMergers) :
      SubTrigger(inName, inEventWidth, inOffset, inHeaderSize, inNodeID),
      arrayPtr(inArrayPtr),
      nInnerMergers(inNInnerMergers) {};

    StoreArray<MergerBits>* arrayPtr;
    unsigned nInnerMergers;
    /** reserve */
    void reserve(int subDetectorId, std::array<int, nFinesse> nWords)
    {
      size_t nClocks = (nWords[iFinesse] - 3) * 32 / eventWidth;
      size_t entries = arrayPtr->getEntries();
      if (subDetectorId == iNode) {
        if (entries == 0) {
          for (unsigned i = 0; i < nClocks; ++i) {
            arrayPtr->appendNew();
          }
          B2DEBUG(20, name << ": " << nClocks << " clocks");
        } else if (entries != nClocks) {
          B2ERROR("Number of clocks in " << name << "conflicts with others!");
        }
      }
    };

    void unpack(int subDetectorId,
                std::array<int*, 4> data32tab,
                std::array<int, 4> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      // get header information
      std::string firmwareType = rawIntToAscii(data32tab.at(iFinesse)[0]);
      std::string firmwareVersion = rawIntToString(data32tab.at(iFinesse)[1]);
      B2DEBUG(90, name << ", " << firmwareType << ", version " << firmwareVersion
              << ", node " << std::hex << iNode << ", finesse " << iFinesse);

      // make bitstream
      // loop over all clocks
      for (int i = headerSize; i < nWords[iFinesse]; i += eventWidth) {
        int iclock = (i - headerSize) / eventWidth;
        auto mergerClock = (*arrayPtr)[iclock];
        B2DEBUG(20, "clock " << iclock);
        // loop over all mergers
        for (unsigned j = offset; j < eventWidth; ++j) {
          int iMerger = (eventWidth - j - 1) / 8 + nInnerMergers;
          int pos = (eventWidth - j - 1) % 8;
          std::bitset<32> evt(data32tab[iFinesse][i + j]);
          for (int k = 0; k < 32; ++k) {
            mergerClock->m_signal[iMerger].set(pos * 32 + k, evt[k]);
          }
        }
      }
      printBuffer(data32tab[iFinesse] + 3, eventWidth);
      B2DEBUG(20, "");
      printBuffer(data32tab[iFinesse] + 3 + eventWidth, eventWidth);
      for (int i = 0; i < std::accumulate(nMergers.begin(), nMergers.end(), 0); ++i) {
        B2DEBUG(99, (*arrayPtr)[0]->m_signal[i].to_string());
      }
    }
  };
};

CDCTriggerUnpackerModule::CDCTriggerUnpackerModule() : Module(), m_rawTriggers("RawTRGs")
{
  // Set module properties
  setDescription("Unpack the CDC trigger data recorded in B2L");

  // Parameter definitions
  addParam("unpackMerger", m_unpackMerger,
           "flag to unpack merger data (recorded by Merger Reader / TSF)", false);
  NodeList defaultMergerNodeID = {{0x11000001, 0},
    {0x11000003, 0},
    {0x11000001, 1},
    {0x11000002, 0},
    {0x11000002, 1}
  };
  addParam("MergerNodeId", m_mergerNodeID,
           "list of FTSW ID of Merger reader (TSF)", defaultMergerNodeID);

}


void CDCTriggerUnpackerModule::initialize()
{
  m_rawTriggers.isRequired();
  m_mergerBits.registerInDataStore("CDCTriggerMergerBits");
  for (int iSL = 0; iSL < 9; iSL += 2) {
    const int nInnerMergers = std::accumulate(nMergers.begin(),
                                              nMergers.begin() + iSL, 0);
    B2DEBUG(20, "in: " << nInnerMergers);
    Merger* m_merger =
      new Merger(&m_mergerBits,
                 "Merger" + std::to_string(iSL), mergerWidth * nMergers[8] / 32,
                 mergerWidth * (nMergers[8] - nMergers[iSL]) / 32, 3,
                 m_mergerNodeID[iSL / 2],
                 nInnerMergers);
    m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_merger));
  }

}

void CDCTriggerUnpackerModule::terminate()
{
  for (auto bits : m_subTrigger) {
    delete bits;
  }
}

void CDCTriggerUnpackerModule::event()
{
  // Read RawTRG data block.
  B2DEBUG(99,  m_rawTriggers.getEntries() << " COPPERs in RawTRGs");

  // loop over all COPPERs
  for (auto& rawTRG : m_rawTriggers) {
    const int subDetectorId = rawTRG.GetNodeID(0);
    // const int iNode = (subDetectorId & 0xFFFFFF);
    // number of entries in the rawTRG object.
    const int nEntriesRawTRG = rawTRG.GetNumEntries();

    // number of entries in rawTRG object ()
    B2DEBUG(99, "nEntries of rawTRG: " << nEntriesRawTRG);
    for (int j = 0; j < nEntriesRawTRG; ++j) {
      //
      // Search Data from Finesse A to D (0->3).
      //
      std::array<int, nFinesse> nWords;
      std::array<int*, nFinesse> data32tab;
      nWords.fill(0);

      for (int iFinesse = 0; iFinesse < nFinesse; ++iFinesse) {
        nWords[iFinesse] = rawTRG.GetDetectorNwords(j, iFinesse);
        if (nWords[iFinesse] == 0) {
          continue;
        }
        data32tab[iFinesse] = (int*)rawTRG.GetDetectorBuffer(j, iFinesse);
      }

      for (auto trg : m_subTrigger) {
        trg->reserve(subDetectorId, nWords);
        trg->unpack(subDetectorId, data32tab, nWords);
      }
    }
  }
}

