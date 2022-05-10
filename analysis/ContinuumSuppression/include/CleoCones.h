/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Math/Vector3D.h>

#include <vector>

namespace Belle2 {

  /** Class to calculate the Cleo clone variables */
  class CleoCones {
  public:

    /**
     * Constructor
     */
    CleoCones(const std::vector<ROOT::Math::XYZVector>& p3_cms_all, const std::vector<ROOT::Math::XYZVector>& p3_cms_roe,
              const ROOT::Math::XYZVector& thrustB,
              bool calc_CleoCones_with_all, bool calc_CleoCones_with_roe);

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
