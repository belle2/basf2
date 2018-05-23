/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <map>
#include <string>
#include <analysis/dbobjects/ParticleWeightingBinLimits.h>
#include <stdio.h>

namespace Belle2 {

  // Map of keys with bin limits
  typedef std::map<int, ParticleWeightingBinLimits*> BinMap;

  /**
   * Class for handling LookUp tables
   */
  class ParticleWeightingAxis {

    BinMap m_unnamedAxis; /**< Named axis with bins */

    std::string m_axisName; /**< Axis name */

    const int m_outOfRangeBinID = -1; /**< ID of out-of-range bin */

    /**
     * Return TRUE if bin exisits or overlaps with existing
     * @param bin - tested bin
     * @return is this bin overlatpping with existing bins or not
     */
    bool isOverlappingBin(ParticleWeightingBinLimits* bin);

  public:

    /**
     * Constructor
     */
    ParticleWeightingAxis() {}

    /**
     * Returns name of an axis
     */
    std::string getName() { return m_axisName;}

    /**
     * Sets name of an axis
     * @param name axis name
     */
    void setName(std::string name) { m_axisName = name;}

    /**
     * Check if bin exists and creates it if not. Returns bin ID
     * Issues fatal error in case of ovarlaping bins.
     * @param bin bin limits
     * @return key ID
     */
    int addBin(ParticleWeightingBinLimits* bin);

    /**
     * Returns id of bin with given bin limits
     * Returns out-of-range binID if can't find
     * @param limits bin limits
     * @return key ID
     */
    int findBin(ParticleWeightingBinLimits* bin);

    /**
     * Returns id of bin containig value
     * @param value value
     * @return key ID
     */
    int findBin(double value);

    void printAxis();
  };

} // Belle2 namespace


