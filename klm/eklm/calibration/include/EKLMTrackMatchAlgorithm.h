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
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

/* ROOT headers. */
#include <TH1.h>

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
     * Copy constructor (disabled).
     */
    EKLMTrackMatchAlgorithm(const EKLMTrackMatchAlgorithm&) = delete;

    /**
     * Operator = (disabled).
     */
    EKLMTrackMatchAlgorithm& operator=(const EKLMTrackMatchAlgorithm&) = delete;

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

    /** Output root file */
    TFile* m_file;

    /** EKLM element numbers */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /** Efficiency data object */
    KLMStripEfficiency* m_StripEfficiency;

  };

}
