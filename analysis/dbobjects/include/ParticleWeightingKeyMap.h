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
#include <vector>
#include <string>
#include <analysis/dbobjects/ParticleWeightingAxis.h>


namespace Belle2 {

  /**
   * Multidimensional bin: first element contains
   * combination of bin IDs from 1D axis,
   * second elements contain ID ("key") associated with this combination
   */
  typedef std::pair<std::vector<int>, int> MultiDimBin;

  /**
   * N-dim bin: pairs of bin limits with name of the axis variable.
   */
  typedef std::map<std::string, ParticleWeightingBinLimits*> NDBin;

  /**
   * Class for handling KeyMap
   */
  class ParticleWeightingKeyMap {

    /**
     * Axes mapped with their names
     */
    std::map<std::string, ParticleWeightingAxis*> m_axes;
    /**
     * Vector of keys
     */
    std::vector<MultiDimBin> m_bins;

    /**
     * We assign unique bin ID for out-of-range bin.
     */
    const int m_outOfRangeBin = -1;

  public:

    /**
     * Constructor
     */
    ParticleWeightingKeyMap() {}

    /**
    * Add axis
    */
    void addAxis(std::string name);

    /**
     * Adding predefined ID to the table.
     * @param bin contains bin ranges and names of te variables
     * @param key_ID predefined bin ID
     * @return added key ID
     */
    int addKey(NDBin bin, int key_ID);

    /**
     * Adding predefined ID to the table.
     * @param bin contains bin ranges and names of te variables
     * @return added key ID
     */
    double addKey(NDBin bin);


    /**
     * Get global bin ID for given observable values
     * @param values map of axis names and correspoinding values
     * @return global ID
     */
    double getKey(std::map<std::string, double> values) const;

    /**
    * Get vector of names ParticleWeightingKeyMap
    * @return vector of axes names
    */
    std::vector<std::string> getNames() const;

    /**
    * Print content of the key map
    */
    void printKeyMap() const;

  };

} // Belle2 namespace


