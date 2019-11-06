/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMPlaneArrayIndex.h>
#include <klm/dbobjects/KLMStripEfficiency.h>

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

/* ROOT headers. */
#include <TH1.h>

namespace Belle2 {

  /**
   * KLM channel status calibration algorithm.
   */
  class KLMStripEfficiencyAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor.
     */
    KLMStripEfficiencyAlgorithm();

    /**
     * Copy constructor (disabled).
     */
    KLMStripEfficiencyAlgorithm(const KLMStripEfficiencyAlgorithm&) = delete;

    /**
     * Operator = (disabled).
     */
    KLMStripEfficiencyAlgorithm& operator=(const KLMStripEfficiencyAlgorithm&) = delete;

    /**
     * Destructor.
     */
    ~KLMStripEfficiencyAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

  private:

    /** Hist of planes eff (data for calibration) */
    TH1F* m_planesEff;

    /** Output root file */
    TFile* m_file;

    /** Element numbers */
    const KLMElementNumbers* m_ElementNumbers;

    /** Plane array index. */
    const KLMPlaneArrayIndex* m_PlaneArrayIndex;

    /** Efficiency data object */
    KLMStripEfficiency* m_StripEfficiency;

  };

}
