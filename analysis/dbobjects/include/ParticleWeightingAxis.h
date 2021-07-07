/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <map>
#include <string>
#include <analysis/dbobjects/ParticleWeightingBinLimits.h>

namespace Belle2 {

  /** Map of keys with bin limits */
  typedef std::map<int, ParticleWeightingBinLimits*> BinMap;

  /**
   * Class for handling LookUp tables
   */
  class ParticleWeightingAxis {

    BinMap m_unnamedAxis; /**< Named axis with bins */

    std::string m_axisName; /**< Axis name */

    const int m_outOfRangeBinID = -1; /**< ID of out-of-range bin */

    /**
     * Return TRUE if bin exists or overlaps with existing
     * @param bin - tested bin
     * @return is this bin overlapping with existing bins or not
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
    std::string getName() const { return m_axisName;}

    /**
     * Sets name of an axis
     * @param name axis name
     */
    void setName(const std::string& name) { m_axisName = name;}

    /**
     * Check if bin exists and creates it if not. Returns bin ID
     * Issues fatal error in case of overlapping bins.
     * @param bin bin limits
     * @return key ID
     */
    int addBin(ParticleWeightingBinLimits* bin);

    /**
     * Returns id of bin with given bin limits
     * Returns out-of-range binID if can't find
     * @param bin bin limits
     * @return key ID
     */
    int findBin(ParticleWeightingBinLimits* bin) const;

    /**
     * Returns id of bin containing value
     * @param value value
     * @return key ID
     */
    int findBin(double value) const;

    /**
     * Prints axis information to the B2INFO stream
     */
    void printAxis() const;
  };

} // Belle2 namespace


