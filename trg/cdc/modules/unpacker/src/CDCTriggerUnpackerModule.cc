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
#include <queue>
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
using dataWord = std::bitset<wordWidth>;

namespace Belle2 {
  //-----------------------------------------------------------------
  //                 Definition of Subtriggers
  //-----------------------------------------------------------------
  /*
   * To implement an unpacker for a new subtrigger module,
   * 1. Declare a new struct, inherit from SubTrigger
   * 2. Hold the pointers to the bitstream(s)
   * 3. declare as many variables as needed
   * 4. Implement the virtual methods reserve() and unpack()
   * 5. Instantiate it in initialize() (one object for one UT3)
   * 6. Push them back to m_subTrigger
   */

  /** unpacker for the merger reader (TSF which reads the merger output) */
  struct Merger : SubTrigger {
    Merger(StoreArray<MergerBits>* inArrayPtr, std::string inName,
           unsigned inEventWidth, unsigned inOffset,
           int inHeaderSize, std::vector<int> inNodeID,
           unsigned inNInnerMergers, int& inDelay,
           int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset,
                 inHeaderSize, inNodeID, inDelay, inDebugLevel),
      arrayPtr(inArrayPtr),
      nInnerMergers(inNInnerMergers) {};

    /** pointer to the merger output Bitstream */
    StoreArray<MergerBits>* arrayPtr;
    /** number of merger units in the inner super layer than this one */
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
          dataWord word(data32tab[iFinesse][i + j]);
          for (int k = 0; k < wordWidth; ++k) {
            mergerClock->m_signal[iMerger].set(pos * wordWidth + k, word[k]);
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

  /** unpacker for the 2D tracker */
  struct Tracker2D : SubTrigger {
    /** constructor */
    Tracker2D(StoreArray<TSFOutputBitStream>* inArrayPtr,
              StoreArray<T2DOutputBitStream>* outArrayPtr,
              std::string inName, unsigned inEventWidth, unsigned inOffset,
              unsigned inHeaderSize, std::vector<int> inNodeID,
              unsigned inNumTS, int& inDelay, int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset / wordWidth, inHeaderSize, inNodeID,
                 inDelay, inDebugLevel),
      inputArrayPtr(inArrayPtr), outputArrayPtr(outArrayPtr),
      iTracker(std::stoul(inName.substr(inName.length() - 1))),
      numTS(inNumTS), offsetBitWidth(inOffset) {};

    /** pointer to the Bitstream of 2D input */
    StoreArray<TSFOutputBitStream>* inputArrayPtr;
    /** pointer to the Bitstream of 2D output to 3D/Neuro */
    StoreArray<T2DOutputBitStream>* outputArrayPtr;
    /** ID of the 2D tracker (0 to 3) */
    unsigned iTracker;
    /** Number of TS sent to 2D (0 to 20) */
    unsigned numTS;
    /** starting point of the input data in an Belle2Link event */
    unsigned offsetBitWidth;
    /** bit width of a single TS hit information */
    static constexpr unsigned TSWidth = 21;

    /**
     *  Calculate the number of clocks in the data,
     *  reserve that much of clocks in the Bitstream(s)
     *
     *  @param subDetectorId   COPPER id of the current data
     *
     *  @param nWords          Number of words of each FINESSE in the COPPER
     */
    void reserve(int subDetectorId, std::array<int, nFinesse> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        // reset the input and output bitstream if they won't be filled later
        for (auto& inputClock : (*inputArrayPtr)) {
          for (auto& tsfIn : inputClock.m_signal) {
            tsfIn[iTracker].fill(zero_val);
          }
        }
        for (auto& outputClock : (*outputArrayPtr)) {
          outputClock.m_signal[iTracker].fill(zero_val);
        }
        return;
      } else if (nWords[iFinesse] == headerSize) {
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

    /**
     *  Unpack the Belle2Link data and fill the Bitstream
     *
     *  @param subDetectorId   COPPER id of the current data
     *
     *  @param data32tab       list of pointers to the Belle2Link data buffers
     *
     *  @param nWords          Number of words of each FINESSE in the COPPER
     */
    void unpack(int subDetectorId,
                std::array<int*, 4> data32tab,
                std::array<int, 4> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      // Recently, the content of the last clock appears at the beginning.
      // Now we decide whether we will change the order of the clocks
      // based on the 127MHz counter on the 2nd half of the first word [15:0]
      int ccShift = 0;
      using halfDataWord = std::bitset<16>;
      std::vector<halfDataWord> counters;
      counters.reserve(inputArrayPtr->getEntries());
      for (int iclock = 0; iclock < inputArrayPtr->getEntries(); ++iclock) {
        counters.emplace_back(data32tab[iFinesse]
                              [headerSize + eventWidth * iclock] & 0xffff);
      }
      while (counters.at(1).to_ulong() - counters.at(0).to_ulong() != 4) {
        std::rotate(counters.begin(), counters.begin() + 1, counters.end());
        ccShift++;
      }
      if (! std::is_sorted(counters.begin(), counters.end(),
      [](halfDataWord i, halfDataWord j) {
      return (j.to_ulong() - i.to_ulong() == 4);
      })) {
        B2WARNING("clock counters are still out of order");
        for (const auto& c : counters) {
          B2DEBUG(90, "" << c.to_ulong());
        }
      }
      if (ccShift) {
        B2DEBUG(15, "shifting the first " << ccShift <<
                " clock(s) to the end for " << name);
      }

      // get event body information
      // make bitstream
      // loop over all clocks
      for (int i = headerSize; i < nWords[iFinesse]; i += eventWidth) {
        int iclock = (i - headerSize) / eventWidth - ccShift;
        if (iclock < 0) {
          iclock += inputArrayPtr->getEntries();
        }
        auto inputClock = (*inputArrayPtr)[iclock];
        auto outputClock = (*outputArrayPtr)[iclock];
        // clear output bitstream
        outputClock->m_signal[iTracker].fill(zero_val);
        B2DEBUG(90, "unpacker clock " << iclock);
        if (debugLevel >= 100) {
          printBuffer(data32tab[iFinesse] + headerSize + eventWidth * iclock,
                      eventWidth);
        }
        // get the clock counters
        std::array<dataWord, 2> ccword({
          data32tab[iFinesse][i + 2], data32tab[iFinesse][i + 3]
        });
        // fill input
        // Careful! this iTSF is (8 - iSL) / 2
        for (unsigned iTSF = 0; iTSF < nAxialTSF; ++iTSF) {
          // clear input bitstream
          inputClock->m_signal[nAxialTSF - 1 - iTSF][iTracker].fill(zero_val);

          if (firmwareVersion < "18012600") {
            /*
              data_b2l_r <=
              x"dddd" & cntr125M(15 downto 0) &
              cc(4) & cc(3) & cc(2) & cc(1) & cc(0) &
              ccError &
              -- 82
              tsfs(4)(209 downto 0) &
              tsfs(3)(209 downto 0) &
              tsfs(2)(209 downto 0) &
              tsfs(1)(209 downto 0) &
              tsfs(0)(209 downto 0) &
            */
            // fill the clock counters
            for (unsigned pos = 0; pos < clockCounterWidth; ++pos) {
              const int j = (pos + iTSF * clockCounterWidth) / wordWidth;
              const int k = (pos + iTSF * clockCounterWidth) % wordWidth;
              /* The index behaves differently in each containers
                 Here is an example of a 64-bit wide MSB data
                 |-------- smaller index in the firmware ----->

                 data in the firmware/VHDL std_logic_vector(63 downto 0)
                  63 62 61 60 ...                         01 00

                 data in B2L/RawTRG (when using std::bitset<32> for a word)
                 (31 30 29 28 ... 01 00) (31 30 29 28 ... 01 00)
                 |------ word 0 -------| |------ word 1 -------|

                 XSim / Bitstream of std::array<char, N> / std::string
                  00 01 02 03 ...                         62 63
               */
              // Here we are filling std::array<char, N>, which has reversed order
              // to std::bitset, so there is a - sign in [wordWidth - k]
              inputClock->m_signal[nAxialTSF - 1 - iTSF][iTracker][pos] =
                std_logic(ccword[j][wordWidth - k]);
            }
            // fill the TS hit
            offsetBitWidth = 82;
            for (unsigned pos = 0; pos < numTS * TSWidth; ++pos) {
              const int j = (offsetBitWidth + pos + iTSF * numTS * TSWidth) / wordWidth;
              const int k = (offsetBitWidth + pos + iTSF * numTS * TSWidth) % wordWidth;
              dataWord word(data32tab[iFinesse][i + j]);
              // MSB (leftmost) in firmware -> smallest index in Bitstream's
              // std::array (due to XSIM) -> largest index in std::bitset
              // so the index is reversed in this assignment
              inputClock->m_signal[nAxialTSF - 1 - iTSF][iTracker]
              [clockCounterWidth + pos] = std_logic(word[wordWidth - 1 - k]);
            }
          } else {
            /*
              x"dddd" & (15 downto 11 => '0') & revoclk &
              cntr125M(15 downto 0) &
              (15 downto 5 => '0') & ccError &
              -- 64
              cc(4) & tsfs(4)(209 downto 0) &
              cc(3) & tsfs(3)(209 downto 0) &
              cc(2) & tsfs(2)(209 downto 0) &
              cc(1) & tsfs(1)(209 downto 0) &
              cc(4) & tsfs(0)(209 downto 0) &
            */
            // fill the cc and TS hit
            offsetBitWidth = 64;
            unsigned TSFWidth = clockCounterWidth + numTS * TSWidth;
            for (unsigned pos = 0; pos < TSFWidth; ++pos) {
              const int j = (offsetBitWidth + pos + iTSF * TSFWidth) / wordWidth;
              const int k = (offsetBitWidth + pos + iTSF * TSFWidth) % wordWidth;
              dataWord word(data32tab[iFinesse][i + j]);
              inputClock->m_signal[nAxialTSF - 1 - iTSF][iTracker][pos] =
                std_logic(word[wordWidth - 1 - k]);
            }
          }
          if (debugLevel >= 100) {
            display_hex(inputClock->m_signal[nAxialTSF - 1 - iTSF][iTracker]);
          }
        }
        // fill output
        if (firmwareVersion < "18012600") {
          /*
            -- 1132
            Main_out(731 downto 0) &
          */
          const int outputOffset = nAxialTSF * numTS * TSWidth;
          const int oldtrackWidth = 6;
          for (unsigned pos = 0; pos < 732; ++pos) {
            const int j = (offsetBitWidth + pos + outputOffset) / wordWidth;
            const int k = (offsetBitWidth + pos + outputOffset) % wordWidth;
            dataWord word(data32tab[iFinesse][i + j]);
            outputClock->m_signal[iTracker][clockCounterWidth + oldtrackWidth + pos]
              = std_logic(word[wordWidth - 1 - k]);
          }
        } else {
          /*
            -- 1159
            old_track(5 downto 0) &
            Main_out(731 downto 0) &
           */
          const int outputOffset = 1159;
          for (unsigned pos = 0; pos < T2DOutputWidth; ++pos) {
            const int j = (pos + outputOffset) / wordWidth;
            const int k = (pos + outputOffset) % wordWidth;
            dataWord word(data32tab[iFinesse][i + j]);
            outputClock->m_signal[iTracker][clockCounterWidth + pos]
              = std_logic(word[wordWidth - 1 - k]);
          }
        }
        if (debugLevel >= 100) {
          display_hex(outputClock->m_signal[iTracker]);
        }
      }
    }
  };

  struct Neuro : SubTrigger {
    Neuro(StoreArray<NNInputBitStream>* inArrayPtr,
          StoreArray<NNOutputBitStream>* outArrayPtr,
          std::string inName, unsigned inEventWidth, unsigned inOffset,
          unsigned inHeaderSize, std::vector<int> inNodeID, int& inDelay,
          int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset / wordWidth, inHeaderSize, inNodeID,
                 inDelay, inDebugLevel),
      inputArrayPtr(inArrayPtr), outputArrayPtr(outArrayPtr),
      iTracker(std::stoul(inName.substr(inName.length() - 1))),
      offsetBitWidth(inOffset) {};

    StoreArray<NNInputBitStream>* inputArrayPtr;
    StoreArray<NNOutputBitStream>* outputArrayPtr;
    unsigned iTracker;
    unsigned offsetBitWidth;

    void reserve(int subDetectorId, std::array<int, nFinesse> nWords)
    {
      size_t nClocks = (nWords[iFinesse] - headerSize) / eventWidth;
      size_t entries = inputArrayPtr->getEntries();
      if (subDetectorId == iNode) {
        if (entries == 0) {
          for (unsigned i = 0; i < nClocks; ++i) {
            inputArrayPtr->appendNew();
            outputArrayPtr->appendNew();
          }
          B2DEBUG(20, name << ": " << nClocks << " clocks");
        } else if (entries != nClocks) {
          B2ERROR("Number of clocks in " << name << "conflicts with others!");
        }
      }
    };

    void unpack(int subDetectorId,
                std::array<int*, nFinesse> data32tab,
                std::array<int, nFinesse> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      // make bitstream
      // loop over all clocks
      for (int i = headerSize; i < nWords[iFinesse]; i += eventWidth) {
        int iclock = (i - headerSize) / eventWidth;
        auto inputClock = (*inputArrayPtr)[iclock];
        auto outputClock = (*outputArrayPtr)[iclock];
        // clear bitstreams
        inputClock->m_signal[iTracker].fill(zero_val);
        outputClock->m_signal[iTracker].fill(zero_val);
        B2DEBUG(20, "clock " << iclock);
        if (debugLevel >= 100) {
          printBuffer(data32tab[iFinesse] + headerSize + eventWidth * iclock,
                      eventWidth);
        }
        // fill output
        for (unsigned pos = 0; pos < NNOutputWidth; ++pos) {
          const int j = (offsetBitWidth + pos) / wordWidth;
          const int k = (offsetBitWidth + pos) % wordWidth;
          std::bitset<wordWidth> word(data32tab[iFinesse][i + j]);
          outputClock->m_signal[iTracker][pos] = std_logic(word[wordWidth - 1 - k]);
        }
        if (debugLevel >= 100) {
          display_hex(outputClock->m_signal[iTracker]);
        }
        // fill input
        for (unsigned pos = 0; pos < NNInputWidth; ++pos) {
          const int j = (offsetBitWidth + pos + NNOutputWidth) / wordWidth;
          const int k = (offsetBitWidth + pos + NNOutputWidth) % wordWidth;
          std::bitset<wordWidth> word(data32tab[iFinesse][i + j]);
          inputClock->m_signal[iTracker][pos] = std_logic(word[wordWidth - 1 - k]);
        }
        if (debugLevel >= 100) {
          display_hex(inputClock->m_signal[iTracker]);
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
  addParam("unpackNeuro", m_unpackNeuro,
           "whether to unpacker neurotrigger data", false);
  addParam("decode2DFinderTrack", m_decode2DFinderTrack,
           "flag to decode 2D finder track", false);
  addParam("decode2DFinderInput", m_decode2DFinderInputTS,
           "flag to decode input TS to 2D", false);
  addParam("decodeNeuro", m_decodeNeuro,
           "flag to decode neurotrigger data", false);
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
  NodeList defaultNeuroNodeID = {
    {0x11000003, 1},
    {0, 0},
    {0, 0},
    {0, 0}
  };
  addParam("NeuroNodeId", m_neuroNodeID,
           "list of FTSW ID of neurotrigger", defaultNeuroNodeID);
  addParam("headerSize", m_headerSize,
           "number of words (number of bits / 32) of the B2L header", 3);
  addParam("alignFoundTime", m_alignFoundTime,
           "Whether to align out-of-sync Belle2Link data between different sub-modules", true);

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
  if (m_unpackNeuro) {
    m_bitsToNN.registerInDataStore("CDCTriggerNNInputBits");
    m_bitsFromNN.registerInDataStore("CDCTriggerNNOutputBits");
  }
  if (m_decodeTSHit or m_decode2DFinderTrack or
      m_decode2DFinderInputTS or m_decodeNeuro) {
    m_TSHits.registerInDataStore("CDCTriggerSegmentHits");
  }
  if (m_decode2DFinderTrack or m_decodeNeuro) {
    m_2DFinderTracks.registerInDataStore("CDCTrigger2DFinderTracks");
    m_2DFinderTracks.registerRelationTo(m_TSHits);
    m_2DFinderClones.registerInDataStore("CDCTrigger2DFinderClones");
    m_2DFinderClones.registerRelationTo(m_2DFinderTracks);
  }
  for (int iSL = 0; iSL < 9; iSL += 2) {
    if (m_unpackMerger) {
      const int nInnerMergers = std::accumulate(nMergers.begin(),
                                                nMergers.begin() + iSL, 0);
      B2DEBUG(20, "in: " << nInnerMergers);
      Merger* m_merger =
        new Merger(&m_mergerBits,
                   "Merger" + std::to_string(iSL), mergerWidth * nMergers[8] / wordWidth,
                   mergerWidth * (nMergers[8] - nMergers[iSL]) / wordWidth, m_headerSize,
                   m_mergerNodeID[iSL / 2], nInnerMergers,
                   m_mergerDelay,
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
                      m_2DFinderDelay,
                      m_debugLevel);
      m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_tracker2d));
    }
    if (m_unpackNeuro) {
      Neuro* m_neuro =
        new Neuro(&m_bitsToNN, &m_bitsFromNN,
                  "Neuro" + std::to_string(iTracker), 64, 496, m_headerSize,
                  m_neuroNodeID[iTracker], m_NeuroDelay, m_debugLevel);
      m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_neuro));
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
  setReturnValue(0);
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
        // only unpack when there are enough words in the event
        if (trg->getHeaders(subDetectorId, data32tab, nWords)) {
          trg->unpack(subDetectorId, data32tab, nWords);
          setReturnValue(1);
        }
      }
    }
  }

  // decode bitstream and make TSIM objects
  if (m_decode2DFinderTrack) {
    for (short iclock = 0; iclock < m_bits2DTo3D.getEntries(); ++iclock) {
      decode2DOutput(iclock - m_2DFinderDelay,
                     m_bits2DTo3D[iclock],
                     &m_2DFinderTracks,
                     &m_2DFinderClones,
                     &m_TSHits);
    }
  }
  if (m_decode2DFinderInputTS) {
    std::array<int, 4> clockCounter2D = {0, 0, 0, 0};
    std::array<int, 4> timeOffset2D = {0, 0, 0, 0};
    // Align the data in other boards to 2D0 by looking at the CC in the midpoint of the time window
    for (int iTracker = 0; iTracker < nTrackers; ++iTracker) {
      if (! m_alignFoundTime || m_bitsTo2D.getEntries() == 0) {
        break;
      }
      auto& trackerData = m_bitsTo2D[m_bitsTo2D.getEntries() / 2]->signal()[0][iTracker];
      std::string strInput = slv_to_bin_string(trackerData);
      clockCounter2D[iTracker] = std::stoi(strInput.substr(0, clockCounterWidth), 0, 2);
      int clockCounterDiff = clockCounter2D[iTracker] - clockCounter2D[0];
      // clock counter rolls back to 0 from 319
      if (clockCounterDiff > 300) {
        clockCounterDiff -= 320;
      } else if (clockCounterDiff < -300) {
        clockCounterDiff += 320;
      }
      timeOffset2D[iTracker] = clockCounterDiff;
      if (clockCounterDiff != 0) {
        B2DEBUG(100, "Adding " << clockCounterDiff << " clock(s) to 2D" << iTracker << " found time");
      }
      if (std::abs(clockCounterDiff) > 2) {
        B2WARNING("Clock counters between 2D differ by " << clockCounterDiff << " clocks!");
      }
    }
    for (short iclock = 0; iclock < m_bitsTo2D.getEntries(); ++iclock) {
      B2DEBUG(30, "clock " << iclock);
      decode2DInput(iclock - m_2DFinderDelay, timeOffset2D, m_bitsTo2D[iclock], &m_TSHits);
    }
  }
  if (m_decodeNeuro) {
    for (short iclock = 0; iclock < m_bitsFromNN.getEntries(); ++iclock) {
      decodeNNInput(iclock, m_bitsToNN[iclock], &m_2DFinderTracks, &m_TSHits);
    }
  }
}

