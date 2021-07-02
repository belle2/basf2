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

  /**
   * Creation of a plain TTree with residual and global derivative values
   * from GBL data saved to a ROOT file. This conversion is intended for
   * debugging.
   */
  class MillepedeTreeConversionAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor.
     */
    MillepedeTreeConversionAlgorithm();

    /**
     * Destructor.
     */
    ~MillepedeTreeConversionAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

    /**
     * Set output file name.
     */
    void setOutputFile(const char* outputFile);

  private:

    /** Output file name. */
    std::string m_OutputFile;

  };

}

