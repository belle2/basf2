/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/KeyMap.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  typedef std::map<std::string, double> WeightInfo;

  /**
   * Class for handling lookup tables
   */
  class LookupTable {
    /**
    * Lookup table matches bins of the key map with the lookup info.
    * This matching is paired with the key map.
    */
    std::pair<KeyMap, std::map<double, WeightInfo>> m_LookupTable;

  public:
    /**
    * Constructor
    */
    LookupTable() {};

    /**
    * Adds entry to the lookup table and also adds entry to key table if needed.
    * @param entryValue is a line from lookup table
    * @param bin contains bin limits for given lookup table values
    */
    void addEntry(WeightInfo entryValue, NDBin bin);

    /**
    * Adds entry to the lookup table and also adds entry to key table with predefined bin ID if needed.
    * @param entryValue is a line from lookup table
    * @param bin contains bin limits for given lookup table values
    * @param key_ID is bin ID. You might want to use it to make several bins with the same ID or for non-square bins.
    */
    void addEntry(WeightInfo entryValue, NDBin bin, double key_ID);

    /**
    * Define reweight info for particles out of range. This info is stored in bin -1.
    * @param entryValue is a line from lookup table
    */
    void defineOutOfRangeWeight(WeightInfo entryValue);

    /**
    * Printing content of the lookup table
    */
    void printLookupTable();

    /**
    * Get lookup information for the particle
    * @param p particle
    * @return lookup information (map: key - lookup parameter; value - value of the parameter )
    */
    WeightInfo getInfo(const Particle* p);

    /**
    * Dummy function returning sample of lookup line
    * @return lookup information (map: key - lookup parameter; value - value of the parameter )
    */
    WeightInfo getFirst();
  };

} // Belle2 namespace


