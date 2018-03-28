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
#include <TObject.h>
#include <analysis/dbobjects/ParticleWeightingKeyMap.h>

//#include <analysis/dataobjects/Particle.h>
//#include <analysis/dataobjects/ParticleWeightingKeyMap.h>
//#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  typedef std::map<std::string, double> WeightInfo;
  typedef std::map<double, WeightInfo> WeightMap;

  /**
   * Class for handling LookUp tables
   */
  class ParticleWeightingLookUpTable : public TObject {
    /**
    * LookUp table matches bins of the key map with the LookUp info.
    * This matching is paired with the key map.
    */
    std::pair<ParticleWeightingKeyMap, WeightMap> m_ParticleWeightingLookUpTable;

    // We assign unique bin ID for out-of-range bin.
    const double m_OutOfRangeBinID = -1;

    ClassDef(ParticleWeightingLookUpTable, 1);

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
    void addEntry(WeightInfo entryValue, NDBin bin, double key_ID);

    /**
    * Define reweight info for particles out of range. This info is stored in bin -1.
    * @param entryValue is a line from LookUp table
    */
    void defineOutOfRangeWeight(WeightInfo entryValue);

    /**
    * Printing content of the LookUp table
    */
    void printParticleWeightingLookUpTable();


    /**
    * Get ParticleWeightingKeyMap
    */
    ParticleWeightingKeyMap getParticleWeightingKeyMap();

    /**
    * Get weight map
    */
    WeightMap getWeightMap();

    /**
    * Get LookUp information for the particle
    * @param p particle
    * @return LookUp information (map: key - LookUp parameter; value - value of the parameter )
    */
    //WeightInfo getInfo(const Particle* p);

    /**
    * Dummy function returning sample of LookUp line
    * @return LookUp information (map: key - LookUp parameter; value - value of the parameter )
    */
    WeightInfo getFirst();
  };

} // Belle2 namespace


