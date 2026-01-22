/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* KLM headers. */
#include <klm/dbobjects/bklm/BKLMGeometryPar.h>

namespace Belle2 {

  /**
   * KLM layer area map.
   */
  class KLMLayerArea: public TObject {

  public:

    /**
     * Constructor.
     */
    KLMLayerArea();

    /**
     * Destructor.
     */
    ~KLMLayerArea();

    /**
     * Calculate and store layer areas for BKLM and EKLM using geometry parameter classes.
     */
    void calculateLayerAreas();

    /**
     * Get total area for a specific BKLM layer.
     * BKLM has 240 total layers: 2 sections × 8 sectors × 15 layers
     * @param[in] layer  Flattened layer index (1-based, 1-240)
     * @return Area in cm^2
     */
    double getBKLMLayerArea(int layer) const;

    /**
     * Get total area for EKLM layers (same for all layers).
     * @return Area in cm^2
     */
    double getEKLMLayerArea() const;

  private:
    // BKLM: flat array of 240 areas
    double m_BKLMLayerAreas[240] = {};
    // EKLM: single area value (same for all layers)
    double m_EKLMLayerArea = 0.0;

    /** Class version. */
    ClassDef(Belle2::KLMLayerArea, 1);
  };

}