/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/cdc/modules/unpacker/CDCTriggerUnpackerModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>

#include <array>
#include <bitset>
#include <string>
#include <numeric>
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
    /** Constructor */
    Merger(StoreArray<MergerBits>* inArrayPtr, const std::string& inName,
           unsigned inEventWidth, unsigned inOffset,
           int inHeaderSize, const std::vector<int>& inNodeID,
           unsigned inNInnerMergers, int& inDelay,
           int& inCnttrg,
           int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset,
                 inHeaderSize, inNodeID, inDelay, inCnttrg, inDebugLevel),
      arrayPtr(inArrayPtr),
      nInnerMergers(inNInnerMergers) {};

    /** pointer to the merger output Bitstream */
    StoreArray<MergerBits>* arrayPtr;
    /** number of merger units in the inner super layer than this one */
    unsigned nInnerMergers;
    /** reserve enough number of clocks (entries) in the Bitstream StoreArray */
    void reserve(int subDetectorId, std::array<int, nFinesse> nWords) override
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
        B2DEBUG(20, "Number of clocks in " << name << " conflicts with others!");
      }
    };

    /** Unpack function */
    void unpack(int subDetectorId,
                std::array<int*, 4> data32tab,
                std::array<int, 4> nWords) override
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        B2DEBUG(20, "The module " << name << " does not have enough data (" <<
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
      if (debugLevel >= 300) {
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
              const std::string& inName, unsigned inEventWidth, unsigned inOffset,
              unsigned inHeaderSize, const std::vector<int>& inNodeID,
              unsigned inNumTS, int& inDelay,
              int& inCnttrg,
              int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset / wordWidth, inHeaderSize, inNodeID,
                 inDelay, inCnttrg, inDebugLevel),
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

    /**
     *  Calculate the number of clocks in the data,
     *  reserve that much of clocks in the Bitstream(s)
     *
     *  @param subDetectorId   COPPER id of the current data
     *
     *  @param nWords          Number of words of each FINESSE in the COPPER
     */
    void reserve(int subDetectorId, std::array<int, nFinesse> nWords) override
    {
      size_t nClocks = (nWords[iFinesse] - headerSize) / eventWidth;
      size_t entries = inputArrayPtr->getEntries();
      if (subDetectorId == iNode) {
        if (entries == 0) {
          for (unsigned i = 0; i < nClocks; ++i) {
            TSFOutputBitStream* inputClock = inputArrayPtr->appendNew();
            T2DOutputBitStream* outputClock = outputArrayPtr->appendNew();
            // fill bitstreams for all trackers with zeros
            for (unsigned j = 0; j < nTrackers; ++j) {
              for (unsigned iAxialTSF = 0; iAxialTSF < nAxialTSF; ++iAxialTSF) {
                inputClock->m_signal[iAxialTSF][j].fill(zero_val);
              }
              outputClock->m_signal[j].fill(zero_val);
            }
          }
          B2DEBUG(20, name << ": " << nClocks << " clocks");
        } else if (entries != nClocks) {
          B2DEBUG(20, "Number of clocks in " << name << " conflicts with others!");
        }
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
                std::array<int, 4> nWords) override
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
        B2DEBUG(100, "iclock " << iclock << " --> " << counters.at(iclock).to_ulong() << " : " << std::hex << counters.at(iclock));
      }
      bool counter_correct_error = false;
      while (counters.at(1).to_ulong() - counters.at(0).to_ulong() != 4) {
        std::rotate(counters.begin(), counters.begin() + 1, counters.end());
        ccShift++;
        // 2019,0410 This situation, looks like clockcounter shifted 6 bits left, was first seen in exp5 data.
        //           Later it has been understood that it is due to data from a dummy BRAM buffer, which is supposed to be used for suppressed data only.
        //           The data header is 0xbbbb instead of 0xdddd.
        //           getHeader in the CDCTriggerUnpackerModule.h is modified to skip this kind event.
        // 2019,0419 unfortunately, clockcounter found disorder since expt 7, run 2553, after update of B2L firmware to replace the b2dly with trigger counter.
        //           for that run, it seems the problem happens only at 2D1, 2D2, and 2D3.
        if (ccShift >= inputArrayPtr->getEntries()) {
          B2DEBUG(90, "PHYSJG: clock counter rotation over one cycle: " << ccShift);
          for (const auto& c : counters) {
            B2DEBUG(90, "" << c.to_ulong() << " : " << std::hex << c);
          }
          counter_correct_error = true;
          break;
        }
      }
      if (counter_correct_error) {
        B2DEBUG(20, "PHYSJG: " <<  name << " too many clock counter rotation corrections: " << ccShift << " data object skipped.");
        // maybe implement an option for user to decide if this data block should be kept or not?!
        return;
      }
      if (! std::is_sorted(counters.begin(), counters.end(),
      [](halfDataWord i, halfDataWord j) {
      return (j.to_ulong() - i.to_ulong() == 4);
      })) {
        B2DEBUG(20, "clock counters are still out of order");
        for (const auto& c : counters) {
          B2DEBUG(90, "" << c.to_ulong());
        }
      }
      // This could happen when the clock counter is over 1279 and roll back to 0, since only 48 clock counter will be in the data.
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
        if (debugLevel >= 300) {
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
            for (unsigned pos = 0; pos < numTS * lenTS; ++pos) {
              const int j = (offsetBitWidth + pos + iTSF * numTS * lenTS) / wordWidth;
              const int k = (offsetBitWidth + pos + iTSF * numTS * lenTS) % wordWidth;
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
              cc(0) & tsfs(0)(209 downto 0) &
            */
            // fill the cc and TS hit
            offsetBitWidth = 64;
            unsigned TSFWidth = clockCounterWidth + numTS * lenTS;
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
          const int outputOffset = nAxialTSF * numTS * lenTS;
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
          //const int outputOffset = 1159;
          const int outputOffset = offsetBitWidth +  nAxialTSF * numTS * lenTS + 45;  //1159 with numTS=10 ,  1684 with numTS=15
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

  /** unpacker for the Neuro */
  struct Neuro : SubTrigger {
    /** Constructor */
    Neuro(StoreArray<NNBitStream>* arrPtr,
          const std::string& inName, unsigned inEventWidth, unsigned inOffset,
          unsigned inHeaderSize, const std::vector<int>& inNodeID, int& inDelay,
          int& inCnttrg,
          int inDebugLevel) :
      SubTrigger(inName, inEventWidth, inOffset / wordWidth, inHeaderSize, inNodeID,
                 inDelay, inCnttrg, inDebugLevel),
      ArrayPtr(arrPtr),
      iTracker(std::stoul(inName.substr(inName.length() - 1))),
      offsetBitWidth(inOffset) {};

    /** Array pointer for NN */
    StoreArray<NNBitStream>* ArrayPtr;
    /** Tracker board ID */
    unsigned iTracker;
    /** Offset bit width */
    unsigned offsetBitWidth;

    void reserve(int subDetectorId, std::array<int, nFinesse> nWords) override
    {
      size_t nClocks = (nWords[iFinesse] - headerSize) / eventWidth;
      size_t entries = ArrayPtr->getEntries();
      if (subDetectorId == iNode) {
        if (entries == 0) {
          for (unsigned i = 0; i < nClocks; ++i) {
            NNBitStream* nnclock = ArrayPtr->appendNew();
            // fill bitstreams for all trackers with zeros
            for (unsigned j = 0; j < nTrackers; ++j) {
              nnclock->m_signal[j].fill(zero_val);
            }
          }
          B2DEBUG(20, name << ": " << nClocks << " clocks");
        } else if (entries != nClocks) {
          B2DEBUG(20, "Number of clocks in " << name << " conflicts with others!");
        }
      }
    };

    void unpack(int subDetectorId,
                std::array<int*, nFinesse> data32tab,
                std::array<int, nFinesse> nWords) override
    {
      if (subDetectorId != iNode) {
        return;
      }
      // make bitstream
      // loop over all clocks
      for (int i = headerSize; i < nWords[iFinesse]; i += eventWidth) {
        int iclock = (i - headerSize) / eventWidth;
        auto nnclock = (*ArrayPtr)[iclock];
        B2DEBUG(20, "clock " << iclock);
        if (debugLevel >= 300) {
          printBuffer(data32tab[iFinesse] + headerSize + eventWidth * iclock,
                      eventWidth);
        }
        // fill output
        for (unsigned pos = 0; pos < NN_WIDTH; ++pos) {
          const int j = (offsetBitWidth + pos) / wordWidth;
          const int k = (offsetBitWidth + pos) % wordWidth;
          std::bitset<wordWidth> word(data32tab[iFinesse][i + j]);
          nnclock->m_signal[iTracker][pos] = std_logic(word[wordWidth - 1 - k]);
        }
        if (debugLevel >= 100) {
          display_hex(nnclock->m_signal[iTracker]);
        }
      }
    }
  };
};

CDCTriggerUnpackerModule::CDCTriggerUnpackerModule() : Module(), m_rawTriggers("RawTRGs")
{
  // Set module properties
  setDescription("Unpack the CDC trigger data recorded in B2L");
  setPropertyFlags(c_ParallelProcessingCertified);

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
  //  https://confluence.desy.de/display/BI/DAQ+and+Operation for CPR/HSLB
  NodeList defaultMergerNodeID = {    // These should be very temporary ones since no merger to B2L yet.
    {0x11000001, 0},
    {0x11000003, 0},
    {0x11000001, 1},
    {0x11000002, 0},
    {0x11000002, 1}
  };
  addParam("MergerNodeId", m_mergerNodeID,
           "list of COPPER and HSLB ID of Merger reader (TSF)", defaultMergerNodeID);
  NodeList defaultTracker2DNodeID = {
    {0x11000001, 0},
    {0x11000001, 1},
    {0x11000002, 0},
    {0x11000002, 1}
  };
  addParam("2DNodeId", m_tracker2DNodeID,
           "list of COPPER and HSLB ID of 2D tracker", defaultTracker2DNodeID);
  NodeList defaultNeuroNodeID = {
    {0x11000005, 0},
    {0x11000005, 1},
    {0x11000006, 0},
    {0x11000006, 1}
  };
  addParam("NeuroNodeId", m_neuroNodeID,
           "list of COPPER and HSLB ID of neurotrigger", defaultNeuroNodeID);
  addParam("headerSize", m_headerSize,
           "number of words (number of bits / 32) of the B2L header", 3);
  addParam("alignFoundTime", m_alignFoundTime,
           "Whether to align out-of-sync Belle2Link data between different sub-modules", true);
  addParam("useDB", m_useDB,
           "Use values stored in the payload of the ConditionsDB."
           "This affects the output scaling of the Neurotrigger as well as the"
           "bit configuration of its unpacker. If false, an old unpacker version with fixed scalings and old bit adresses is used.",
           true);
  addParam("sim13dt", m_sim13dt,
           "Simulate 13 bit drift time by using 2d clock counter value.",
           false);
}

void CDCTriggerUnpackerModule::initialize()
{
  m_debugLevel = getLogConfig().getDebugLevel();

  //m_rawTriggers.isRequired();
  if (m_unpackMerger) {
    m_mergerBits.registerInDataStore("CDCTriggerMergerBits");
  }
  if (m_unpackTracker2D) {
    m_bitsTo2D.registerInDataStore("CDCTriggerTSFTo2DBits");
    m_bits2DTo3D.registerInDataStore("CDCTrigger2DTo3DBits");
  }
  if (m_unpackNeuro) {
    m_bitsNN.registerInDataStore("CDCTriggerNNBits");
  }
  if (m_decodeTSHit or m_decode2DFinderTrack or
      m_decode2DFinderInputTS) {
    m_TSHits.registerInDataStore("CDCTriggerSegmentHits");
  }
  if (m_decode2DFinderTrack) {
    m_2DFinderTracks.registerInDataStore("CDCTrigger2DFinderTracks");
    m_2DFinderTracks.registerRelationTo(m_TSHits);
    m_2DFinderClones.registerInDataStore("CDCTrigger2DFinderClones");
    m_2DFinderClones.registerRelationTo(m_2DFinderTracks);
  }
  if (m_decodeNeuro) {
    m_NNInputTSHitsAll.registerInDataStore("CDCTriggerNNInputAllStereoSegmentHits");
    m_NNInputTSHits.registerInDataStore("CDCTriggerNNInputSegmentHits");
    m_NNInput2DFinderTracks.registerInDataStore("CDCTriggerNNInput2DFinderTracks");
    m_NeuroTracks.registerInDataStore("CDCTriggerNeuroTracks");
    m_NeuroInputs.registerInDataStore("CDCTriggerNeuroTracksInput");
    m_NeuroTracks.registerRelationTo(m_NNInputTSHits);
    m_NNInput2DFinderTracks.registerRelationTo(m_NNInputTSHits);
    m_NNInput2DFinderTracks.registerRelationTo(m_NNInputTSHitsAll);
    m_NNInput2DFinderTracks.registerRelationTo(m_NeuroTracks);
    m_NeuroTracks.registerRelationTo(m_NNInput2DFinderTracks);
    m_NeuroTracks.registerRelationTo(m_NeuroInputs);
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
                   m_mergerDelay, m_mergerCnttrg,
                   m_debugLevel);
      m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_merger));
    }
  }
  // TODO In the default scenario, data in all trackers will be recorded.
  // This is not the case for now (around first collision), where some coppers are lacking.
  // Therefore it might help to make the following code more flexible
  // so that we won't have a hard fail when some boards are missing

  m_n2DTS = m_dbn2DTS->getnTS();
  int datasize_2D = 64;
  if (m_n2DTS == 10) {
    datasize_2D = 64;
  } else if (m_n2DTS == 15) {
    datasize_2D = 82;
  }

  for (int iTracker = 0; iTracker < 4; ++iTracker) {
    if (m_unpackTracker2D) {
      Tracker2D* m_tracker2d =
        new Tracker2D(&m_bitsTo2D, &m_bits2DTo3D,
                      "Tracker2D" + std::to_string(iTracker), datasize_2D, 82, m_headerSize,
                      m_tracker2DNodeID[iTracker], m_n2DTS,
                      m_2DFinderDelay, m_2DFinderCnttrg,
                      m_debugLevel);
      m_subTrigger.push_back(dynamic_cast<SubTrigger*>(m_tracker2d));
    }
    if (m_unpackNeuro) {
      Neuro* m_neuro =
        new Neuro(&m_bitsNN,
                  "Neuro" + std::to_string(iTracker), 64, 0, m_headerSize,
                  m_neuroNodeID[iTracker], m_NeuroDelay, m_NeuroCnttrg,  m_debugLevel);
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

void CDCTriggerUnpackerModule::beginRun()
{

  StoreObjPtr<EventMetaData> bevt;
  m_exp = bevt->getExperiment();
  m_run = bevt->getRun();

  if (not m_cdctriggerneuroconfig.isValid())
    B2FATAL("CDCTriggerNeuroConfig is not valid.");
  if (m_useDB == true) {
    B2DEBUG(2, "Load Neurotrigger configuration for network " << m_cdctriggerneuroconfig->getNNName() << " from database ");
    B2DEBUG(10, padright("Name", 50) << padright("start", 10) << padright("end", 10) << padright("offset", 10));
    for (auto x : m_cdctriggerneuroconfig->getB2Format()) {
      B2DEBUG(10, padright(x.name, 48) << ": " << padright(std::to_string(x.start), 10) <<  padright(std::to_string(x.end),
              10) << padright(std::to_string(x.offset), 10));
    }
  }
}


void CDCTriggerUnpackerModule::event()
{

  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, "----------------------------------------------------------------------------------------------------");
  B2DEBUG(10, padright(" ", 100));
  StoreObjPtr<EventMetaData> eventMetaData;
  std::string experimentstring = "Experiment " + std::to_string(eventMetaData->getExperiment()) + "  Run " +
                                 std::to_string(eventMetaData->getRun()) + "  Event " + std::to_string(eventMetaData->getEvent());
  B2DEBUG(10, padright(experimentstring, 100));

  setReturnValue(0);

  if (m_exp < 7) {
    B2DEBUG(20, "exp<7: skip cdctrg unpacker for DQM");
    return;
  }

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
        // only unpack when there are enough words in the event
        if (trg->getHeaders(subDetectorId, data32tab, nWords)) {
          trg->reserve(subDetectorId, nWords);
          B2DEBUG(99, "starting to unpack a subTrigger, subDetectorId" << std::hex << subDetectorId);
          trg->unpack(subDetectorId, data32tab, nWords);
          setReturnValue(1);
        }
      }
    }
    B2DEBUG(99, "looped over entries and filled words " << nEntriesRawTRG);
  }
  B2DEBUG(99, "looped over rawTriggers, unpacking 2D ");

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
  B2DEBUG(99, "unpack 2D Input TS ");
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
      /*
      // clock counter rolls back to 0 from 319
      if (clockCounterDiff > 300) {
        clockCounterDiff -= 320;
      } else if (clockCounterDiff < -300) {
        clockCounterDiff += 320;
      }
      */
      // clock counter rolls back to 0 from 1279, since certain B2L version, it has been changed to like this
      if (clockCounterDiff > 1250) {
        clockCounterDiff -= 1280;
      } else if (clockCounterDiff < -1250) {
        clockCounterDiff += 1280;
      }
      timeOffset2D[iTracker] = clockCounterDiff;
      if (clockCounterDiff != 0) {
        B2DEBUG(100, "Adding " << clockCounterDiff << " clock(s) to 2D" << iTracker << " found time");
      }
      if (std::abs(clockCounterDiff) > 2) {
        B2DEBUG(20, "Clock counters between 2D [0," << iTracker << "] differ by " << clockCounterDiff << " clocks! (" \
                << clockCounter2D[0] << ", " << clockCounter2D[iTracker] << ")");
      }
    }
    for (short iclock = 0; iclock < m_bitsTo2D.getEntries(); ++iclock) {
      B2DEBUG(30, "clock " << iclock);
      decode2DInput(iclock - m_2DFinderDelay, timeOffset2D, m_bitsTo2D[iclock], &m_TSHits);
    }
  }
  B2DEBUG(99, "now unpack neuro ");
  if (m_decodeNeuro) {
    if (m_useDB == true) {
      decodeNNIO(&m_bitsNN, &m_NNInput2DFinderTracks, &m_NeuroTracks, &m_NNInputTSHits, &m_NNInputTSHitsAll, &m_NeuroInputs,
                 m_cdctriggerneuroconfig, m_sim13dt);
    } else {
      decodeNNIO_old(&m_bitsNN, &m_NNInput2DFinderTracks, &m_NeuroTracks, &m_NNInputTSHits, &m_NeuroInputs);
    }
  }
  B2DEBUG(99, " all is unpacked ##### ");
}

