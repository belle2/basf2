/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNMENT_MILLEPEDETREECONVERSIONALGORITHM_H
#define ALIGNMENT_MILLEPEDETREECONVERSIONALGORITHM_H

/* Belle2 headers. */
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
    CalibrationAlgorithm::EResult calibrate();

    /**
     * Set output file name.
     */
    void setOutputFile(const char* outputFile);

  private:

    /** Output file name. */
    std::string m_OutputFile;

  };

}

#endif

