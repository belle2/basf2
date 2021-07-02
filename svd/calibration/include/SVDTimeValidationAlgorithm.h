/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa, Giulio Dujany             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <vxd/geometry/GeoCache.h>
#include "TH1F.h"
#include <svd/dataobjects/SVDHistograms.h>

#include <optional>

namespace Belle2 {
  /**
   * Class implementing SVDTimeValidation algorithm
   */
  class SVDTimeValidationAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor set the prefix to SVDTimeCalibrationCollector*/
    explicit SVDTimeValidationAlgorithm();

    /** Destructor*/
    virtual ~SVDTimeValidationAlgorithm() {}

    /** Setter for m_allowedT0Shift*/
    void setAllowedTimeShift(float value) {m_allowedDeviationMean = value;}

    /** Getter for m_allowedT0Shift*/
    float getAllowedTimeShift() {return m_allowedDeviationMean;}

    /** Set the minimum entries required in the histograms*/
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    /** Get the minimum entries required in the histograms*/
    int getMinEntries() {return m_minEntries;}

  protected:

    /** Run algo on data*/
    virtual EResult calibrate() override;


  private:

    std::string m_id = ""; /**< Parameter given to set the UniqueID of the payload*/
    std::optional<float> m_previousRawTimeMeanL3V; /**< CoG time mean of the previous run for V side of layer 3*/

    float m_allowedDeviationMean = 0.5; /**< Allowed deviation of clsOnTracks histo wrt EventT0 histo in  number of EventT0 RMS */
    float m_minEntries = 10000; /**< Set the minimun number of entries required in the histograms of layer 3*/
  };
} // namespace Belle2
