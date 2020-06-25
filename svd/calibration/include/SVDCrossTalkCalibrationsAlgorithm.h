/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {

  /** Class implementing SVDCrossTalkCalibrations calibration algorithm. */

  class SVDCrossTalkCalibrationsAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor  */
    explicit SVDCrossTalkCalibrationsAlgorithm(const std::string& str);

    /** Destructor */
    virtual ~SVDCrossTalkCalibrationsAlgorithm() {}

    /** Set the minimum number of entries required in hist from collector for L4.2.1 u-side */
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

  protected:

    virtual EResult calibrate() override;

  private:
    /**Identifier string */
    std::string m_id;

    /**Minimum number of required entries for collector histogram L4.1.2 u-side */
    int m_minEntries = 10000;

  };


} // namespace Belle2
