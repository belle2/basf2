/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace Belle2 {

  /**
   * Simple class to encapsulate a detector surface's boundaries
   * in cylindrical coordinates.
   */
  class DetSurfCylBoundaries {

  public:
    /**
     * Default constructor.
     */
    DetSurfCylBoundaries() {};
    /**
     * Constructor with parameters.
     */
    DetSurfCylBoundaries(float rho, float zfwd, float zbwd, float th_fwd, float th_fwd_brl, float th_bwd_brl, float th_bwd)
    {
      m_rho = rho;
      m_zfwd = zfwd;
      m_zbwd = zbwd;
      m_th_fwd = th_fwd;
      m_th_fwd_brl = th_fwd_brl;
      m_th_bwd_brl = th_bwd_brl;
      m_th_bwd = th_bwd;
    };
    float m_rho; /**< Inner surface radius [cm] */
    float m_zfwd; /**< Inner surface z fwd [cm] */
    float m_zbwd; /**< Inner surface z bwd [cm] */
    float m_th_fwd; /**< Lower theta edge of fwd region [rad] */
    float m_th_fwd_brl; /**< fwd/barrel separation theta [rad] */
    float m_th_bwd_brl; /**< bwd/barrel separation theta [rad] */
    float m_th_bwd; /**< Upper theta edge of bwd region [rad] */
  };

  /**
   * Detector surfaces information.
   * Contains the unordered_maps that associate to each detector its valid cylindrical surface's boundaries.
   */
  struct DetectorSurface {

    /**
     * Map that associates to each detector its valid cylindrical surface's boundaries.
     * Values are taken from the B2 TDR.
     */
    static const std::unordered_map<std::string, DetSurfCylBoundaries> detToSurfBoundaries;

    /**
     * Map that associates to each detector layer its valid cylindrical surface's boundaries.
     * Values are taken from the B2 TDR.
     */
    static const std::unordered_map<std::string, DetSurfCylBoundaries> detLayerToSurfBoundaries;

    /**
     * Map that associates to each detector its list of valid layers.
     */
    static const std::unordered_map<std::string, std::vector<int>> detToLayers;
  };
}
