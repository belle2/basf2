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

#include <TH1.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {

  /**
   * KLM channel status calibration algorithm.
   */
  class EKLMTrackMatchAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor.
     */
    EKLMTrackMatchAlgorithm();

    /**
     * Destructor.
     */
    ~EKLMTrackMatchAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

  private:

    /** Hist of planes eff (data for calibration) */
    TH1F* m_planesEff;

    TFile* m_file;

  };

}