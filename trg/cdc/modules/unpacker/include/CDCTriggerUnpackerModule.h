/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRIGGERUNPACKERMODULE_H
#define CDCTRIGGERUNPACKERMODULE_H

#include <framework/core/Module.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <framework/datastore/StoreArray.h>

#include <trg/cdc/dataobjects/Bitstream.h>
#include <trg/cdc/Unpacker.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerFinderClone.h>

#include <array>
#include <bitset>
#include <vector>
#include <string>
#include <utility>
#include <typeinfo>

namespace Belle2 {

  using NodeList = std::vector<std::vector<int> >;

  static constexpr int mergerWidth = 256;
  static constexpr int nAllMergers = 146;
  /** width of a single word in the raw int buffer */
  static constexpr int wordWidth = 32;
  static constexpr int nFinesse = 4;
  using MergerBus = std::array<std::bitset<mergerWidth>, nAllMergers>;
  using MergerBits = Bitstream<MergerBus>;

  /* enum class SubTriggerType : unsigned char {Merger, TSF, T2D, T3D, Neuro, ETF}; */

  struct SubTrigger {
    /** constructor */
    SubTrigger(std::string inName,
               unsigned inEventWidth, unsigned inOffset,
               int inHeaderSize, std::vector<int> inNodeID,
               int& inDelay, int inDebugLevel = 0) :
      name(inName), eventWidth(inEventWidth), offset(inOffset),
      headerSize(inHeaderSize), iNode(inNodeID.front()),
      iFinesse(inNodeID.back()), delay(inDelay),
      debugLevel(inDebugLevel) {};

    /** Name of the UT3 */
    std::string name;
    /** Size of an event in the Belle2Link data in 32-bit words */
    unsigned eventWidth;
    /** The starting point of the data in an event */
    unsigned offset;
    /** Size of the B2L header in words */
    int headerSize;
    /** COPPER id of the board */
    int iNode;
    /** FINESSE (HSLB) id) of the board */
    int iFinesse;

    /* information from Belle2Link header */
    /** type of the FPGA firmware */
    std::string firmwareType;
    /** version of the FPGA firmware */
    std::string firmwareVersion;
    /** Reference to the variable of its Belle2Link delay */
    int& delay;

    /** debug level in the steering file */
    int debugLevel;

    /**
     *  Calculate the number of clocks in the data,
     *  reserve that much of clocks in the Bitstream(s)
     *
     *  @param subDetectorId   COPPER id of the current data
     *
     *  @param nWords          Number of words of each FINESSE in the COPPER
     */
    virtual void reserve(int, std::array<int, nFinesse>) {};

    /**
     *  Unpack the Belle2Link data and fill the Bitstream
     *
     *  @param subDetectorId   COPPER id of the current data
     *
     *  @param data32tab       list of pointers to the Belle2Link data buffers
     *
     *  @param nWords          Number of words of each FINESSE in the COPPER
     */
    virtual void unpack(int,
                        std::array<int*, nFinesse>,
                        std::array<int, nFinesse>) {};

    /**
     *  Get the Belle2Link header information
     *
     *  @param subDetectorId   COPPER id of the current data
     *
     *  @param data32tab       list of pointers to the Belle2Link data buffers
     *
     *  @param nWords          Number of words of each FINESSE in the COPPER
     *
     *  @return                1 if there are data other than the header
     */
    virtual int getHeaders(int subDetectorId,
                           std::array<int*, 4> data32tab,
                           std::array<int, 4> nWords)
    {
      if (subDetectorId != iNode) {
        return 0;
      }
      // empty data buffer
      if (nWords[iFinesse] < headerSize) {
        B2WARNING("The module " << name << " does not have enough data (" <<
                  nWords[iFinesse] << "). Nothing will be unpacked.");
        return 0;
      } else if (nWords[iFinesse] == headerSize) {
        B2DEBUG(20, "The module " << name <<
                " contains only the header. Nothing will be unpacked.");
        return 0;
      }

      /* get event header information
       * Ideally, these parameters should not change in the same run,
       * so it is more efficiency to do it in beginRun().
       * However, since they are present in all events,
       * let's check if they really remain unchanged.
       */
      if (headerSize >= 2) {
        firmwareType = CDCTriggerUnpacker::rawIntToAscii(data32tab.at(iFinesse)[0]);
        firmwareVersion = CDCTriggerUnpacker::rawIntToString(data32tab.at(iFinesse)[1]);
        // get the Belle2Link delay
        // TODO: what is the exact date that this word is introduced?
        if (headerSize >= 3 || firmwareVersion > "17121900") {
          std::bitset<wordWidth> thirdWord(data32tab.at(iFinesse)[2]);
          int newDelay = CDCTriggerUnpacker::subset<32, 12, 20>(thirdWord).to_ulong();
          if (delay > 0 && delay != newDelay) {
            B2WARNING(" the Belle2Link delay for " << name <<
                      "has changed from " << delay << " to " << newDelay << "!");
          }
          delay = newDelay;
        }
        B2DEBUG(50, name << ": " << firmwareType << ", version " <<
                firmwareVersion << ", node " << std::hex << iNode <<
                ", finesse " << iFinesse << ", delay: " << delay);
      }
      return 1;
    };

    /** destructor */
    virtual ~SubTrigger() {};
  };

  /**
   * Unpack the trigger data recorded in B2L
   *
   * There are 2 output formats:
   * 1. the Bitstream containing the bit content of each module
   *    This is useful for the firmware simulation
   * 2. decoded Basf2 TSIM object
   *    This is the same as the fast TSIM output
   *
   */
  class CDCTriggerUnpackerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCTriggerUnpackerModule();

    /** Register input and output data */
    virtual void initialize();

    /** Delete dynamically allocated variables */
    virtual void terminate();

    /** convert raw data (in B2L buffer to bitstream) */
    virtual void event();

    /** data width of a single merger unit */
    /** number of merger unit in each super layers */
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

  private:
    /** number of words (number of bits / 32) of the B2L header */
    int m_headerSize;

    StoreArray<RawTRG> m_rawTriggers; /**< array containing the raw trigger data object */

    NodeList m_mergerNodeID; /**< list of (COPPER ID, FTSW ID) of Merger reader (TSF) */
    bool m_unpackMerger;  /**< flag to unpack merger data (recorded by Merger Reader / TSF) */
    MergerBits m_mergerBitsPerClock;
    StoreArray<MergerBits> m_mergerBits; /**< merger output bitstream */

    bool m_decodeTSHit;  /**< flag to decode track segment  */
    NodeList m_tracker2DNodeID; /**< list of (COPPER ID, FTSW ID) of 2D tracker */
    bool m_unpackTracker2D;  /**< flag to unpack 2D tracker data */
    bool m_decode2DFinderTrack;  /**< flag to decode 2D finder track  */
    bool m_decode2DFinderInputTS;  /**< flag to decode 2D finder input TS */
    bool m_alignFoundTime;  /**< flag to align found time in different sub-modules */

    NodeList m_neuroNodeID;  /**< list of (COPPER ID, FTSW ID) of neurotrigger */
    bool m_unpackNeuro;  /**< flag to unpack neurotrigger data */
    bool m_decodeNeuro;  /**< flag to decode neurotrigger data */

    /** bitstream of TSF output to 2D tracker */
    StoreArray<CDCTriggerUnpacker::TSFOutputBitStream> m_bitsTo2D;

    /** decoded track segment hit */
    StoreArray<CDCTriggerSegmentHit> m_TSHits;

    /** bitstream of 2D output to 3D/Neuro */
    StoreArray<CDCTriggerUnpacker::T2DOutputBitStream> m_bits2DTo3D;

    /** decoded 2D finder track */
    StoreArray<CDCTriggerTrack> m_2DFinderTracks;

    /** additional information of the 2D finder track */
    StoreArray<CDCTriggerFinderClone> m_2DFinderClones;

    /** bitstream of Neuro input (combination of stereo TS and single 2D track) */
    StoreArray<CDCTriggerUnpacker::NNInputBitStream> m_bitsToNN;
    /** bitstream of Neuro output (including intermediate results) */
    StoreArray<CDCTriggerUnpacker::NNOutputBitStream> m_bitsFromNN;

    /** debug level specified in the steering file */
    int m_debugLevel;

    /** Belle2Link delay of the merger reader */
    int m_mergerDelay = 0;

    /** Belle2Link delay of the 2D finder */
    int m_2DFinderDelay = 0;

    /** Belle2Link delay of the neurotrigger */
    int m_NeuroDelay = 0;

    /** vector holding the pointers to all the dynamically allocated SubTriggers */
    std::vector<SubTrigger*> m_subTrigger;

  };

}

#endif /* CDCTRIGGERUNPACKERMODULE_H */
