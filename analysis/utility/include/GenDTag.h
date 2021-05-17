/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {

  /**
   * Class to determine generated decay mode of D*, Ds, D0, D+, and their anti-particles
   */
  class GenDTag {

  public:

    /**
     * Constructor
     */
    GenDTag();

    int Mode_Dst_plus(std::vector<int> genDAU); /**< returns D*+ mode identifier */
    int Mode_Dst_minus(std::vector<int> genDAU); /**< returns D*- mode identifier */
    int Mode_Ds_plus(std::vector<int> genDAU); /**< returns Ds+ mode identifier */
    int Mode_Ds_minus(std::vector<int> genDAU); /**< returns Ds- mode identifier */
    int Mode_D_plus(std::vector<int> genDAU); /**< returns D+ mode identifier */
    int Mode_D_minus(std::vector<int> genDAU); /**< returns D- mode identifier */
    int Mode_D0(std::vector<int> genDAU); /**< returns D0 mode identifier */
    int Mode_anti_D0(std::vector<int> genDAU); /**< returns D0bar mode identifier */

  private:
    int m_nPhotos; /**< number of photons */
    /** check decay with two daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2);
    /** check decay with three daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3);
    /** check decay with four daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4);
    /** check decay with five daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5);
    /** check decay with six daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6);
    /** check decay with seven daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7);
    /** check decay with eight daughters */
    bool PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7, int da8);
  };

} //End of Belle2 namespace