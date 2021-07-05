/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
