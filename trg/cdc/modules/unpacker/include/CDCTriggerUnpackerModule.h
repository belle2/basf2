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

#include <array>
#include <bitset>
#include <vector>
#include <string>
#include <utility>
#include <typeinfo>

namespace Belle2 {

  using NodeList = std::vector<std::pair<int, int> >;

  static constexpr int mergerWidth = 256;
  static constexpr int nAllMergers = 146;
  /** width of a single word in the raw int buffer */
  static constexpr int wordWidth = 32;
  static constexpr int nFinesse = 4;
  using MergerBus = std::array<std::bitset<mergerWidth>, nAllMergers>;
  using MergerBits = Bitstream<MergerBus>;

  /* enum class SubTriggerType : unsigned char {Merger, TSF, T2D, T3D, Neuro, ETF}; */

  struct SubTrigger {
    SubTrigger(std::string inName,
               unsigned inEventWidth, unsigned inOffset,
               unsigned inHeaderSize, std::pair<int, int> inNodeID) :
      name(inName), eventWidth(inEventWidth), offset(inOffset),
      headerSize(inHeaderSize), iNode(inNodeID.first), iFinesse(inNodeID.second) {};

    std::string name;
    unsigned eventWidth;
    unsigned offset;
    unsigned headerSize;
    int iNode;
    int iFinesse;
    virtual void reserve(int, std::array<int, nFinesse>) {};
    virtual void unpack(int, std::array<int*, nFinesse>, std::array<int, nFinesse>) {};
    virtual ~SubTrigger() {};
  };

  /**
   * Unpack the trigger data recorded in B2L
   *
   * There are 2 output formats:
   * 1. the Bitstream containing the bit content of each module
   * 2. decoded Basf2 object
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

    StoreArray<RawTRG> m_rawTriggers; /**< array containing the raw trigger data object */

    NodeList m_mergerNodeID; /**< list of (COPPER ID, FTSW ID) of Merger reader (TSF) */
    bool m_unpackMerger;  /**< flag to unpack merger data (recorded by Merger Reader / TSF) */
    MergerBits m_mergerBitsPerClock;
    StoreArray<MergerBits> m_mergerBits; /**< merger output bitstream */
    std::vector<SubTrigger*> m_subTrigger;

    void unpack(SubTrigger& subTrigger, int subDetectorId,
                std::array<int*, nFinesse> data32tab,
                std::array<int, nFinesse> nWords);

  };

}

#endif /* CDCTRIGGERUNPACKERMODULE_H */
