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
#include <algorithm>

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
           int inHeaderSize, std::vector<int> inNodeID,
           unsigned inNInnerMergers, int& inDelay,
           int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset, inHeaderSize, inNodeID,
                 inDelay, inDebugLevel),
      arrayPtr(inArrayPtr),
      nInnerMergers(inNInnerMergers) {};

    StoreArray<MergerBits>* arrayPtr;
    unsigned nInnerMergers;
    /** reserve enough number of clocks (entries) in the Bitstream StoreArray */
    void reserve(int subDetectorId, std::array<int, nFinesse> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        return;
      }
      size_t nClocks = (nWords[iFinesse] - headerSize) / eventWidth;
      size_t entries = arrayPtr->getEntries();
      if (entries == 0) {
        for (unsigned i = 0; i < nClocks; ++i) {
          arrayPtr->appendNew();
        }
        B2DEBUG(20, name << ": " << nClocks << " clocks");
      } else if (entries != nClocks) {
        B2ERROR("Number of clocks in " << name << " conflicts with others!");
      }
    };

    void unpack(int subDetectorId,
                std::array<int*, 4> data32tab,
                std::array<int, 4> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        B2WARNING("The module " << name << " does not have enough data (" <<
                  nWords[iFinesse] << "). Nothing will be unpacked.");
        // TODO: need to clear the output bitstream because we return early here
        return;
      }
      // make bitstream
      // loop over all clocks
      for (int i = headerSize; i < nWords[iFinesse]; i += eventWidth) {
        int iclock = (i - headerSize) / eventWidth;
        auto mergerClock = (*arrayPtr)[iclock];
        B2DEBUG(100, "clock " << iclock);
        // loop over all mergers
        for (unsigned j = offset; j < eventWidth; ++j) {
          int iMerger = (eventWidth - j - 1) / 8 + nInnerMergers;
          int pos = (eventWidth - j - 1) % 8;
          std::bitset<32> word(data32tab[iFinesse][i + j]);
          for (int k = 0; k < 32; ++k) {
            mergerClock->m_signal[iMerger].set(pos * 32 + k, word[k]);
          }
        }
      }
      if (debugLevel >= 100) {
        printBuffer(data32tab[iFinesse] + headerSize, eventWidth);
        B2DEBUG(20, "");
        printBuffer(data32tab[iFinesse] + headerSize + eventWidth, eventWidth);
      }
      for (int i = 0; i < std::accumulate(nMergers.begin(), nMergers.end(), 0); ++i) {
        B2DEBUG(99, (*arrayPtr)[0]->m_signal[i].to_string());
      }
    }
  };

  struct Tracker2D : SubTrigger {
    Tracker2D(StoreArray<TSFOutputBitStream>* inArrayPtr,
              StoreArray<T2DOutputBitStream>* outArrayPtr,
              std::string inName, unsigned inEventWidth, unsigned inOffset,
              unsigned inHeaderSize, std::vector<int> inNodeID,
              unsigned inNumTS, int& inDelay, int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset / 32, inHeaderSize, inNodeID,
                 inDelay, inDebugLevel),
      inputArrayPtr(inArrayPtr), outputArrayPtr(outArrayPtr),
      iTracker(std::stoul(inName.substr(inName.length() - 1))),
      numTS(inNumTS), offsetBitWidth(inOffset) {};

    StoreArray<TSFOutputBitStream>* inputArrayPtr;
    StoreArray<T2DOutputBitStream>* outputArrayPtr;
    unsigned iTracker;
    unsigned numTS;
    unsigned offsetBitWidth;
    static constexpr unsigned TSWidth = 21;
    /** reserve */
    void reserve(int subDetectorId, std::array<int, nFinesse> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        return;
      }
      size_t nClocks = (nWords[iFinesse] - headerSize) / eventWidth;
      size_t entries = inputArrayPtr->getEntries();
      if (entries == 0) {
        for (unsigned i = 0; i < nClocks; ++i) {
          inputArrayPtr->appendNew();
          outputArrayPtr->appendNew();
        }
        B2DEBUG(20, name << ": " << nClocks << " clocks");
      } else if (entries != nClocks) {
        B2ERROR("Number of clocks in " << name << " conflicts with others!");
      }
    };

    void unpack(int subDetectorId,
                std::array<int*, 4> data32tab,
                std::array<int, 4> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        B2WARNING("The module " << name << " does not have enough data (" <<
                  nWords[iFinesse] << "). Nothing will be unpacked.");
        // clear the input and output bitstream
        for (auto& inputClock : (*inputArrayPtr)) {
          for (auto& tsfIn : inputClock.m_signal) {
            tsfIn[iTracker].fill(zero_val);
          }
        }
        for (auto& outputClock : (*outputArrayPtr)) {
          outputClock.m_signal[iTracker].fill(zero_val);
        }
        return;
      }

      // get event body information
      // make bitstream
      // loop over all clocks
      for (int i = headerSize; i < nWords[iFinesse]; i += eventWidth) {
        int iclock = (i - headerSize) / eventWidth;
        auto inputClock = (*inputArrayPtr)[iclock];
        auto outputClock = (*outputArrayPtr)[iclock];
        // clear output bitstream
        outputClock->m_signal[iTracker].fill(zero_val);
        B2DEBUG(20, "clock " << iclock);
        if (debugLevel >= 100) {
          printBuffer(data32tab[iFinesse] + headerSize + eventWidth * iclock,
                      eventWidth);
        }
        // fill input
        for (unsigned iTSF = 0; iTSF < nAxialTSF; ++iTSF) {
          // clear input bitstream
          inputClock->m_signal[iTSF][iTracker].fill(zero_val);
          for (unsigned pos = 0; pos < numTS * TSWidth; ++pos) {
            const int j = (offsetBitWidth + pos + iTSF * numTS * TSWidth) / 32;
            const int k = (offsetBitWidth + pos + iTSF * numTS * TSWidth) % 32;
            std::bitset<32> word(data32tab[iFinesse][i + j]);
            // MSB (leftmost) in firmware -> smallest index in Bitstream's
            // std::array (due to XSIM) -> largest index in std::bitset
            // so the index is reversed in this assignment
            inputClock->m_signal[iTSF][iTracker][clockCounterWidth + pos] =
              std_logic(word[31 - k]);
          }
          if (debugLevel >= 100) {
            display_hex(inputClock->m_signal[iTSF][iTracker]);
          }
        }
        // fill output
        const int outputOffset = nAxialTSF * numTS * TSWidth;
        for (unsigned pos = 0; pos < 732; ++pos) {
          const int j = (offsetBitWidth + pos + outputOffset) / 32;
          const int k = (offsetBitWidth + pos + outputOffset) % 32;
          std::bitset<32> word(data32tab[iFinesse][i + j]);
          outputClock->m_signal[iTracker][9 + pos] = word[31 - k] + 2;
        }
        if (debugLevel >= 100) {
          display_hex(outputClock->m_signal[iTracker]);
        }
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
           "whether to unpack merger data (recorded by Merger Reader / TSF)", false);
  addParam("unpackTracker2D", m_unpackTracker2D,
           "whether to unpack 2D tracker data", false);
  addParam("decode2DFinderTrack", m_decode2DFinderTrack,
           "flag to decode 2D finder track", false);
  NodeList defaultMergerNodeID = {
    {0x11000001, 0},
    {0x11000003, 0},
    {0x11000001, 1},
    {0x11000002, 0},
    {0x11000002, 1}
  };
  addParam("MergerNodeId", m_mergerNodeID,
           "list of FTSW ID of Merger reader (TSF)", defaultMergerNodeID);
  NodeList defaultTracker2DNodeID = {
    {0x11000001, 0},
    {0x11000001, 1},
    {0x11000002, 0},
    {0x11000002, 1}
  };
  addParam("2DNodeId", m_tracker2DNodeID,
           "list of FTSW ID of 2D tracker", defaultTracker2DNodeID);
  addParam("headerSize", m_headerSize,
           "number of words (number of bits / 32) of the B2L header", 3);

}


void CDCTriggerUnpackerModule::initialize()
{
  m_debugLevel = getLogConfig().getDebugLevel();
  m_rawTriggers.isRequired();
  if (m_unpackMerger) {
    m_mergerBits.registerInDataStore("CDCTriggerMergerBits");
  }
  if (m_unpackTracker2D) {
    m_bitsTo2D.registerInDataStore("CDCTriggerTSFTo2DBits");
    m_bits2DTo3D.registerInDataStore("CDCTrigger2DTo3DBits");
  }
  if (m_decodeTSHit or m_decode2DFinderTrack) {
    m_TSHits.registerInDataStore("CDCTriggerSegmentHits");
  }
  if (m_decode2DFinderTrack) {
    m_2DFinderTracks.registerInDataStore("CDCTrigger2DFinderTrack");
    m_2DFinderTracks.registerRelationTo(m_TSHits);
  }
  for (int iSL = 0; iSL < 9; iSL += 2) {
    if (m_unpackMerger) {
      const int nInnerMergers = std::accumulate(nMergers.begin(),
                                                nMergers.begin() + iSL, 0);
      B2DEBUG(20, "in: " << nInnerMergers);
      Merger* m_merger =
        new Merger(&m_mergerBits,
                   "Merger" + std::to_string(iSL), mergerWidth * nMergers[8] / 32,
                   mergerWidth * (nMergers[8] - nMergers[iSL]) / 32, m_headerSize,
                   m_mergerNodeID[iSL / 2], nInnerMergers,
                   m_debugLevel);
      m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_merger));
    }
  }
  for (int iTracker = 0; iTracker < 4; ++iTracker) {
    if (m_unpackTracker2D) {
      Tracker2D* m_tracker2d =
        new Tracker2D(&m_bitsTo2D, &m_bits2DTo3D,
                      "Tracker2D" + std::to_string(iTracker), 64, 82, m_headerSize,
                      m_tracker2DNodeID[iTracker], 10,
                      m_debugLevel);
      m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_tracker2d));
    }
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
        trg->getHeaders(subDetectorId, data32tab, nWords);
        trg->unpack(subDetectorId, data32tab, nWords);
      }
    }
  }

  // decode bitstream and make TSIM objects
  if (m_decode2DFinderTrack) {
    for (short iclock = 0; iclock < m_bits2DTo3D.getEntries(); ++iclock) {
      decode2DOutput(iclock - m_2DFinderDelay,
                     m_bits2DTo3D[iclock],
                     &m_2DFinderTracks,
                     &m_TSHits);
    }
  }

}

