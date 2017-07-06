/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  /**
   * Statistical class for DQ monitoring
   */
  class TOPDQStat {
  public:
    /**
     * Constructor
     */
    TOPDQStat();

    /**
     * Constructor
    * @param v_size number of variables
     */
    TOPDQStat(int v_size);

    /**
     * Destructor
     */
    ~TOPDQStat() {};

    /**
     * Set the size of variables
     * @param v_size number of variables
     */
    void SetSize(int v_size);

    /**
     * Add new value
     * @param idx index of variable
     * @param val value to be added
     */
    void Add(int idx, double val);

    /**
     * Get mean of a variable
     * @param idx index of variable
     */
    double GetMean(int idx);

    /**
     * Get RMS of a variable
     * @param idx index of variable
     */
    double GetRMS(int idx);

  private:
    std::vector<int> n; /**< number of entries */
    std::vector<double> sum; /**< sum of variables */
    std::vector<double> sum2; /**< sum^2 of variables */
  };
} //namespace Belle2
