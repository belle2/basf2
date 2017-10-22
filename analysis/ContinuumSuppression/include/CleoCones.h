/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Röhrken, Pablo Goldenzweig (KIT)                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLEOCONES_H
#define CLEOCONES_H

#include <TVector3.h>

#include <vector>

namespace Belle2 {

  /** Class to calculate the Cleo clone variables */
  class CleoCones {
  public:

    /**
     * Constructor
     */
    CleoCones(std::vector<TVector3> p3_cms_all, std::vector<TVector3> p3_cms_roe, TVector3 thrustB, bool calc_CleoCones_with_all,
              bool calc_CleoCones_with_roe);

    /**
     * Destructor
     */
    ~CleoCones() {};

    /**
     * Returns calculated Cleo Cones constructed from all tracks
     */
    std::vector<float> cleo_cone_with_all() { return m_cleo_cone_with_all; }

    /**
     * Returns calculated Cleo Cones constructed from only ROE tracks
     */
    std::vector<float> cleo_cone_with_roe() { return m_cleo_cone_with_roe; }

  private:

    std::vector<float> m_cleo_cone_with_all;  /**< Cleo Cones calculated from all tracks */
    std::vector<float> m_cleo_cone_with_roe;  /**< Cleo Cones calculated from only ROE tracks */

  };

} // Belle2 namespace

#endif
