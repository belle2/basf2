/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <analysis/dbobjects/ParticleWeightingKeyMap.h>
#include <map>

namespace Belle2 {

  /** Weight information: a line from the weight lookup table */
  typedef std::map<std::string, double> WeightInfo;
  /** Weight map: the whole lookup table  */
  typedef std::map<int, WeightInfo> WeightMap;

  /**
   * Class for handling LookUp tables
   */
  class ParticleWeightingLookUpTable : public TObject {
    /**
    * LookUp table matches bins of the key map with the LookUp info.
    * This matching is paired with the key map.
    */
    ParticleWeightingKeyMap m_KeyMap; /**< Key map */
    WeightMap m_WeightMap; /**< Collection of weight infos for global bins */

    // We assign unique bin ID for out-of-range bin.
    const int m_OutOfRangeBinID = -1; /**< out-of-range bin ID */

    ClassDef(ParticleWeightingLookUpTable, 1); /**< ClassDef as this is a TObject */

  public:
    /**
    * Constructor
    */
    ParticleWeightingLookUpTable() {};

    /**
    * Adds entry to the LookUp table and also adds entry to key table if needed.
    * @param entryValue is a line from LookUp table
    * @param bin contains bin limits for given LookUp table values
    */
    void addEntry(WeightInfo entryValue, NDBin bin);

    /**
    * Adds entry to the LookUp table and also adds entry to key table with predefined bin ID if needed.
    * @param entryValue is a line from LookUp table
    * @param bin contains bin limits for given LookUp table values
    * @param key_ID is bin ID. You might want to use it to make several bins with the same ID or for non-square bins.
    */
    void addEntry(WeightInfo entryValue, NDBin bin, int key_ID);

    /**
    * Define reweight info for particles out of range. This info is stored in bin -1.
    * @param entryValue is a line from LookUp table
    */
    void defineOutOfRangeWeight(const WeightInfo& entryValue);

    /**
    * Get axes names
    * @return vector of axes names
    */
    std::vector<std::string> getAxesNames() const;

    /**
    * Get axes names
    * @param values  map of axes names with values along corresponding names
    * @return vector of axes names
    */
    WeightInfo getInfo(std::map<std::string, double> values) const;

    /**
    * Print content of LookUpTable
    */
    void printParticleWeightingLookUpTable() const;
  };

} // Belle2 namespace


