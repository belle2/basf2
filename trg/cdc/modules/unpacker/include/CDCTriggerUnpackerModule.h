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
#include <framework/database/DBObjPtr.h>

#include <trg/cdc/dataobjects/Bitstream.h>
#include <trg/cdc/Unpacker.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerFinderClone.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPInput.h>
#include <trg/cdc/dbobjects/CDCTrigger2DConfig.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>

#include <array>
#include <bitset>
#include <vector>
#include <string>
#include <iomanip>

namespace Belle2 {

  /// Node list
  using NodeList = std::vector<std::vector<int> >;

  /** Merger data width */
  static constexpr int mergerWidth = 256;
  /** Number of Mergers */
  static constexpr int nAllMergers = 146;
  /** width of a single word in the raw int buffer */
  static constexpr int wordWidth = 32;
  /** Number of FINESSE in the copper */
  static constexpr int nFinesse = 4;
  /** Merger data bus */
  using MergerBus = std::array<std::bitset<mergerWidth>, nAllMergers>;
  /** Merger data bus Bitstream */
  using MergerBits = Bitstream<MergerBus>;

  /** enum class SubTriggerType : unsigned char {Merger, TSF, T2D, T3D, Neuro, ETF}; */
  struct SubTrigger {
    /** constructor */
    SubTrigger(const std::string& inName,
               unsigned inEventWidth, unsigned inOffset,
               int inHeaderSize, const std::vector<int>& inNodeID,
               int& inDelay, int& inCnttrg, int inDebugLevel = 0) :
      name(inName), eventWidth(inEventWidth), offset(inOffset),
      headerSize(inHeaderSize), iNode(inNodeID.front()),
      iFinesse(inNodeID.back()), delay(inDelay),
      cnttrg(inCnttrg),
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
    /** counter of trgger signal, total 32 bits, the 20 LSBs recorded in the event header */
    int& cnttrg;

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
      // int nWordsize = 3075;  // temporary solution to hard coded the correct event size (for 2D only?)
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

      // need one more check, give a warning if the event has wrong data size

      // event data block header:
      // 0xdddd  --> correct event data (for 2D only?)
      // 0xbbbb  --> dummy buffer supposed to be used for only suppressed events.
      if (nWords[iFinesse] > headerSize) {
        //dataHeader = CDCTriggerUnpacker::rawIntToAscii(data32tab.at(iFinesse)[headerSize]&0xFFFF0000 >> 16);
        //bool dataHeader = ( (data32tab.at(iFinesse)[headerSize]&0xffff0000) == 0xdddd0000);
        long dataHeader = (data32tab.at(iFinesse)[headerSize] & 0xffff0000);
        if (dataHeader != 0xdddd0000) {
          B2DEBUG(30, "The module " << name << " has an event data header " << std::hex << std::setfill('0') << std::setw(4) <<
                  (dataHeader >> 16) <<
                  " in this event. It will be ignore.");
          return 0;
        }
        B2DEBUG(50, "subdet and head size " <<  std::setfill('0') << std::hex << std::setw(8) << iNode << ", " << std::dec <<  std::setw(
                  0) << nWords[iFinesse] <<
                " : " << std::hex << std::setw(8) << data32tab.at(iFinesse)[0] << " " << data32tab.at(iFinesse)[1] << " " << data32tab.at(
                  iFinesse)[2] <<
                " " << data32tab.at(iFinesse)[3] << " dataheader = " << dataHeader);
      }

      /* get event header information
       * Ideally, these parameters should not change in the same run,
       * so it is more efficiency to do it in beginRun().
       * However, since they are present in all events,
       * let's check if they really remain unchanged.
       */
      if (headerSize >= 2) {
        // supposedly these two Words will stay for all the versions
        firmwareType = CDCTriggerUnpacker::rawIntToAscii(data32tab.at(iFinesse)[0]);
        firmwareVersion = CDCTriggerUnpacker::rawIntToString(data32tab.at(iFinesse)[1]);
        //int cnttrg = 0;    // temporary solution, this should be one as a reference for comparison
        int l1_revoclk = -1;

        if (headerSize >= 3) {
          std::bitset<wordWidth> thirdWord(data32tab.at(iFinesse)[2]);
          l1_revoclk = CDCTriggerUnpacker::subset<32, 0, 11>(thirdWord).to_ulong();

          if (firmwareType == "2D  ") {  // temporary solcuion, the following version number check is valid only for 2D

            if (firmwareVersion >  "19041700")  {  // started since 19041705
              // the third word is cnttrg and L1_revoclk
              int newCnttrg = CDCTriggerUnpacker::subset<32, 12, 31>(thirdWord).to_ulong();
              cnttrg = newCnttrg;
            } else if (firmwareVersion > "17121900") {  // upto that version, headerSize == 2?
              // the third word is b2l delay and L1_revoclk
              int newDelay = CDCTriggerUnpacker::subset<32, 12, 20>
                             (thirdWord).to_ulong();   // or should be <32,12,19>? bit 31-20 are for prescale?
              if (delay > 0 && delay != newDelay) {
                B2WARNING(" the Belle2Link delay for " << name <<
                          "has changed from " << delay << " to " << newDelay << "!");
              }
              delay = newDelay;
            }
          }
        }

        B2DEBUG(20, name << ": " << firmwareType << ", version " <<
                firmwareVersion << ", node " << std::hex << iNode <<
                ", finesse " << iFinesse << ", delay: " << delay <<
                ", cnttrg: " << cnttrg << std::dec << " == " << cnttrg <<  ", L1_revoclk " << l1_revoclk);


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
    void initialize() override;

    /** Delete dynamically allocated variables */
    void terminate() override;

    /** begin Run */
    void beginRun() override;

    /** convert raw data (in B2L buffer to bitstream) */
    void event() override;

    /** small function to rescale the NN output from -1, 1 to output scale */
    std::vector<float> unscaleNNOutput(std::vector<float> input) const;
    /** data width of a single merger unit */
    /** number of merger unit in each super layers */
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

  private:
    /** number of words (number of bits / 32) of the B2L header */
    int m_headerSize;

    StoreArray<RawTRG> m_rawTriggers; /**< array containing the raw trigger data object */

    NodeList m_mergerNodeID; /**< list of (COPPER ID, HSLB ID) of Merger reader (TSF) */
    bool m_unpackMerger;  /**< flag to unpack merger data (recorded by Merger Reader / TSF) */
    MergerBits m_mergerBitsPerClock; /**< Merger bits per clock*/
    StoreArray<MergerBits> m_mergerBits; /**< merger output bitstream */

    bool m_decodeTSHit = false;  /**< flag to decode track segment  */
    NodeList m_tracker2DNodeID; /**< list of (COPPER ID, HSLB ID) of 2D tracker */
    bool m_unpackTracker2D;  /**< flag to unpack 2D tracker data */
    bool m_decode2DFinderTrack;  /**< flag to decode 2D finder track  */
    bool m_decode2DFinderInputTS;  /**< flag to decode 2D finder input TS */
    bool m_alignFoundTime;  /**< flag to align found time in different sub-modules */
    int  m_n2DTS = 0; //TODO whats the best def val?  /**< flag to unpack 2D tracker data with 15TS*/

    NodeList m_neuroNodeID;  /**< list of (COPPER ID, HSLB ID) of neurotrigger */
    bool m_unpackNeuro;  /**< flag to unpack neurotrigger data */
    bool m_decodeNeuro;  /**< flag to decode neurotrigger data */

    std::vector<int> m_delayNNOutput; /**< delay of the NN output values clock cycle after the NN enable bit (by quadrant) */
    std::vector<int> m_delayNNSelect; /**< delay of the NN selected TS clock cycle after the NN enable bit (by quadrant) */

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


    /** bitstream of Neuro input and output (including intermediate results) */
    StoreArray<CDCTriggerUnpacker::NNBitStream> m_bitsNN;

    /** decoded Neuro tracks */
    StoreArray<CDCTriggerTrack> m_NeuroTracks;

    /** decoded input vector for neural network */
    StoreArray<CDCTriggerMLPInput> m_NeuroInputs;

    /** decoded 2D finder tracks from the neural network input */
    StoreArray<CDCTriggerTrack> m_NNInput2DFinderTracks;

    /** decoded track segment hits from the neural network input */
    StoreArray<CDCTriggerSegmentHit> m_NNInputTSHits;

    /** all decoded stereo track segment hits from the neural network input */
    StoreArray<CDCTriggerSegmentHit> m_NNInputTSHitsAll;

    /** debug level specified in the steering file */
    int m_debugLevel = 0;

    /** Belle2Link delay of the merger reader */
    int m_mergerDelay = 0;

    /** Belle2Link delay of the 2D finder */
    //int m_2DFinderDelay = 0;
    // since version 19041705, the B2L delay is removed, it should a fixed number for a long period and recorded in database.
    int m_2DFinderDelay = 45;   // 0x2d: changed from 0x28 since some time in 201902-03

    /** Belle2Link delay of the neurotrigger */
    int m_NeuroDelay = 0;

    /** cnttrg */
    // int m_Cnttrg = 0; // not used, commented out at 2019/07/31 by ytlai
    /** Merger cnttrg */
    int m_mergerCnttrg = 0;
    /** 2D cnttrg */
    int m_2DFinderCnttrg = 0;
    /** NN cnttrg */
    int m_NeuroCnttrg = 0;

    /** exp number */
    unsigned m_exp = 0;
    /** run number */
    unsigned m_run = 0;

    /** vector holding the pointers to all the dynamically allocated SubTriggers */
    std::vector<SubTrigger*> m_subTrigger;

    //condition database for number of TS in 2D
    DBObjPtr<CDCTrigger2DConfig> m_dbn2DTS;
    /** current neurotrigger config from database; used for unscaling network target */
    DBObjPtr<CDCTriggerNeuroConfig> m_cdctriggerneuroconfig;
    /** output scale for the neural network output */
    std::vector<float> m_NNOutputScale;
    /** fake object to assign the user set scaling values to */
    CDCTriggerMLP m_mlp_scale;
    /** bool value for wether to use the conditions database */
    bool m_useDB;
    /** bool value wether to simulate 13 bit drift time by using 2dcc */
    bool m_sim13dt;
  };


}

#endif /* CDCTRIGGERUNPACKERMODULE_H */
